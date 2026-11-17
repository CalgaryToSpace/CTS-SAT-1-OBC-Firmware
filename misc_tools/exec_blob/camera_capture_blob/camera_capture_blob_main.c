// This is a blob (executable) that replaces the "CTS1+camera_capture" telecommand ("v1").
//
// Motivation: The built-in `camera_capture` telecommand has a small bug where certain captures
// of complex (large) images end early. Since we're upgrading it anyway, there are a few new features.
//
// New Features:
//  1. The `.img` file is converted to a `.jpg` file (saved alongside the original).
//  2. Metadata (timestamp, ADCS attitude) is written to the `.img` file as text and to the `.jpg` file as EXIF metadata.
//
// Args Format: <img_file_path>;<lighting_mode>
// - lighting_mode: single char: d (daylight), m (medium), n (night), s (solar sail contrast)
//
// Usage Example:
// After uplinking the blob as "blobs/camera_capture_v2.blob", run:
// CTS1+camera_setup()!
// CTS1+exec_blob_from_fs(blobs/camera_capture_v2.blob,0,camera/2026-11/2026-11-17_023451.img;d)!
//
// Notes:
// 1. Run `CTS1+camera_setup()!` before each capture (same as v1). This blob turns the camera's EPS
//    channel off when it's done (also the same as v1).
// 2. In the image capture example above, the `.jpg` file generated will have the filename
//    `camera/2026-11/2026-11-17_023451.jpg`. The `.jpg` file is much smaller (about half the filesize),
//    and is recommended for downlinking.
//
// #########################################
// ##### Implementation Details ############
// #########################################
//
// Response Format (JSON):
// {"action":"camera_capture_v2","result":"OK","img":"<path>","jpg":"<path>","meta":{<same as .img header>},
//  "capture":{"end_reason":"idle","rx_bytes":N,"written":N,"write_errs":N,"close_err":N,
//    "first_data_ms":N,"duration_ms":N,"dma_restarts":N},
//  "jpg_result":{"err":0,"bytes":N,"sentences":N,"first":N,"last":N,"total_field":N,"gaps":N,
//    "missing":N,"bad_lines":N,"exif":true,"face":"<64 hex chars>"|null}}
//
// Header Format (first line of the .img file):
// START_CAM:{"blob":"camera_capture_v2","lighting":"d","epoch_ms":N,"sync_src":"G",
//  "sec_since_sync":N|null,"adcs":{"ok":true,"ctrl_mode":N,"est_mode":N,"rpy_deg":[F,F,F],
//  "target":{"lon_deg":F,"lat_deg":F,"alt_m":F}}}
// - "epoch_ms" and "sec_since_sync" are read right before the header is written, just
//   before the capture trigger is sent to the camera (which is when the camera takes the picture).
// - "target" is only present when ctrl_mode is a target-tracking mode (7 or 15).
// - If the ADCS doesn't respond: "adcs":{"ok":false,"err":N}.
// - If only the attitude/target query fails: "att_err":N / "target":{"err":N}.
//
// EXIF Metadata (APP1 segment in the .jpg, inserted right after the SOI marker):
// - Make: "CalgaryToSpace", Model: "FrontierSat (CTS-SAT-1)", Software: "camera_capture_v2"
// - DateTime and DateTimeOriginal: the capture time (UTC, from "epoch_ms"), with
//   OffsetTimeOriginal "+00:00". The full-ms time is in the description's "epoch_ms".
// - ImageDescription: the full metadata JSON (same as the .img header).
// "exif" in the response is false if the start of the JPEG (SOI marker) wasn't found (e.g., the
// first sentence was lost), in which case the .jpg is written without EXIF.
// 
// This blob does the following:
//  1. Right before the capture, queries the ADCS (control mode, estimation mode, estimated attitude
//     angles, and the tracking target if in a target-tracking control mode), then reads the
//     current time/time-since-sync.
//  2. Writes that metadata as a JSON header line in the .img file: `START_CAM:{...}\n`.
//  3. Captures the image (raw camera ASCII sentences) into the .img file, using:
//      - a first-data timeout (trigger -> first sentence): FIRST_DATA_TIMEOUT_MS,
//      - a true inter-byte idle timeout (via the DMA's remaining-bytes counter), which is the nominal
//        exit condition: IDLE_TIMEOUT_MS,
//      - a max duration (with the watchdog petted throughout): MAX_DURATION_MS,
//      - automatic restart of the RX DMA if a UART error aborts it.
//  4. Converts the .img to a .jpg (if the filename ends in ".img", it's replaced with ".jpg";
//     otherwise ".jpg" is appended).
//     If the given filename ends in ".jpg", the raw data is saved to the same name with ".img"
//     instead, and the converted image is saved to the given ".jpg" name.
//  5. Embeds EXIF metadata in the .jpg.
//
// Camera Data Format (each "sentence" is 67 bytes):
//   '@' + 4 hex (sentence number) + 4 hex (total sentences) + 56 hex (28 JPEG bytes) + "\r\n"
// The final sentence starts with "@FACE" and holds camera telemetry (not JPEG data).
//
// Errors before the capture starts (e.g., bad args, LFS not mounted) respond with only
// "camera_capture_v2 error: <ERROR_NAME>" (see CAM_capture_blob_error_enum_t). Once the capture has
// run, the full JSON response is always sent, with the error name in "result".

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "../lfs.h"
#include "timekeeping/timekeeping.h"
#include "adcs_drivers/adcs_types.h"
#include "eps_drivers/eps_types.h"

#define LFS_MAX_PATH_LENGTH 200
#define JPG_PATH_BUF_LEN (LFS_MAX_PATH_LENGTH + 4) // Room to append ".jpg".

static const char ARG_DELIM = ';';
static const char *BLOB_NAME = "camera_capture_v2";

#define FIRST_DATA_TIMEOUT_MS 10000
#define IDLE_TIMEOUT_MS 2000
#define MAX_DURATION_MS 90000

// Data "starts" once this many bytes have arrived (one sentence). Avoids a stray byte (e.g., noise
// when the trigger is sent) switching from the first-data timeout to the much shorter idle timeout.
#define FIRST_DATA_MIN_BYTES 67

#define MAX_DMA_RESTARTS 20

// The STM32 IWDG must be petted at least every 16 s, and no more often than every 200 ms.
#define WATCHDOG_PET_INTERVAL_MS 2000

// Camera sentence format.
#define CAM_LINE_LEN 65 // '@' + 64 hex chars (excluding "\r\n").
#define CAM_SENTENCE_DATA_BYTES 28
#define CAM_LINE_BUF_LEN 72

#define CONVERT_READ_BUF_LEN 256
#define CONVERT_SENTENCES_PER_WRITE 8

#define HEADER_BUF_LEN 448

#define EXIF_MAKE "CalgaryToSpace"
#define EXIF_MODEL "FrontierSat (CTS-SAT-1)"

// Values of `CAMERA_uart_write_state_enum_t` (uart_handler/uart_handler.h). That header can't be
// included because it needs the STM32 HAL headers. The enum is 1 byte (short enums).
#define CAM_STATE_IDLE 0
#define CAM_STATE_FILLING 1
#define CAM_STATE_FILLED_WAITING_FS_WRITE 2
#define CAM_STATE_WRITTEN_TO_FS 3

// Minimal mirror of the STM32 HAL's `DMA_Channel_TypeDef`.
typedef struct {
    volatile uint32_t CCR;
    volatile uint32_t CNDTR; // Bytes remaining before the circular buffer wraps.
    volatile uint32_t CPAR;
    volatile uint32_t CMAR;
} blob_dma_channel_t;
#define DMA_CCR_EN_BIT 0x1u

// Minimal mirror of the start of the STM32 HAL's `DMA_HandleTypeDef` (`Instance` is the first field).
typedef struct {
    blob_dma_channel_t *Instance;
} blob_dma_handle_prefix_t;

// Global variables defined in the firmware ELF (CTS-SAT-1_FW_rc3.elf).
extern lfs_t LFS_filesystem;
extern volatile uint8_t CAMERA_uart_half_1_state;
extern volatile uint8_t CAMERA_uart_half_2_state;
extern const uint16_t UART_camera_dma_buffer_len;
extern const uint16_t UART_camera_dma_buffer_len_half;
extern volatile uint8_t UART_camera_dma_buffer[];
extern volatile uint8_t UART_camera_pending_fs_write_half_1_buf[];
extern volatile uint8_t UART_camera_pending_fs_write_half_2_buf[];
extern void *UART_camera_port_handle; // Actually `UART_HandleTypeDef *`.
extern blob_dma_handle_prefix_t hdma_uart4_rx; // Camera UART RX DMA. Actually `DMA_HandleTypeDef`.
extern volatile uint32_t STM32_watchdog_uptime_last_pet_ms;
// TIME_* globals are declared in timekeeping/timekeeping.h.

extern int snprintf(char *buf, unsigned int size, const char *fmt, ...);
extern int vsnprintf(char *buf, unsigned int size, const char *fmt, va_list args);
extern int strlen(const char *s);
extern int strcmp(const char *a, const char *b);
extern unsigned int strnlen(const char *s, unsigned int max_len);
extern int memcmp(const void *a, const void *b, unsigned int n);
extern void *memcpy(void *dst, const void *src, unsigned int n);
extern void *memset(void *dst, int value, unsigned int n);

extern int8_t LFS_ensure_mounted(void);
extern uint8_t CAMERA_set_expecting_data(uint8_t new_enabled);
extern int HAL_UART_Transmit(void *huart, const uint8_t *pData, uint16_t Size, uint32_t Timeout);
extern uint8_t EPS_set_channel_enabled(EPS_CHANNEL_enum_t channel, uint8_t enabled);
extern void STM32_pet_watchdog(void);
void HAL_Delay(uint32_t Delay);
extern char TIME_sync_source_enum_to_letter_char(TIME_sync_source_enum_t source);
// TIME_convert_uptime_to_unix_epoch_time_ms() is declared in timekeeping/timekeeping.h.
extern void GEN_uint64_to_str(uint64_t value, char *buffer);

extern uint8_t ADCS_get_current_state_1(ADCS_current_state_1_struct_t *output_struct);
extern uint8_t ADCS_get_estimated_attitude_angles(ADCS_estimated_attitude_angles_struct_t *output_struct);
extern uint8_t ADCS_get_tracking_controller_target_reference(ADCS_tracking_controller_target_struct_t *output_struct);


// ==================== Error Enum ====================

// All named status/error codes returned by this blob's functions, including the values
// `blob_main()` itself returns to the telecommand executor (0 = success).
typedef enum {
    BLOB_ERR_OK = 0,
    BLOB_ERR_LFS_NOT_MOUNTED = 20, // LFS_ensure_mounted() failed.
    BLOB_ERR_IMG_OPEN_FAILED = 21, // Opening/creating the .img file failed.
    BLOB_ERR_HEADER_WRITE_FAILED = 22, // Writing the metadata header to the .img failed.
    BLOB_ERR_DMA_START_FAILED = 23, // Starting the camera UART RX DMA failed.
    BLOB_ERR_TRIGGER_SEND_FAILED = 24, // Sending the lighting mode (trigger) to the camera failed.
    BLOB_ERR_IMG_CLOSE_FAILED = 25, // Closing the .img failed (it may be incomplete).
    BLOB_ERR_NO_DATA = 26, // The camera sent no data before the first-data timeout.
    BLOB_ERR_JPG_CONVERT_FAILED = 27, // An LFS error during the .img to .jpg conversion.
    BLOB_ERR_MISSING_ARGS = 135, // The img file path arg is missing.
    BLOB_ERR_INVALID_LIGHTING_MODE = 136, // The lighting mode arg isn't one of d,m,n,s.
    BLOB_ERR_TOO_MANY_ARGS = 137, // There are args after the lighting mode.
} CAM_capture_blob_error_enum_t;

/// @brief Convert a CAM_capture_blob_error_enum_t into a short human-readable name, for use in
///     output strings.
/// @return Static string; never NULL. "UNKNOWN_ERROR" for values not in the enum.
static const char *camera_capture_blob_error_to_str(CAM_capture_blob_error_enum_t err) {
    switch (err) {
        case BLOB_ERR_OK: return "OK";
        case BLOB_ERR_LFS_NOT_MOUNTED: return "LFS_NOT_MOUNTED";
        case BLOB_ERR_IMG_OPEN_FAILED: return "IMG_OPEN_FAILED";
        case BLOB_ERR_HEADER_WRITE_FAILED: return "HEADER_WRITE_FAILED";
        case BLOB_ERR_DMA_START_FAILED: return "DMA_START_FAILED";
        case BLOB_ERR_TRIGGER_SEND_FAILED: return "TRIGGER_SEND_FAILED";
        case BLOB_ERR_IMG_CLOSE_FAILED: return "IMG_CLOSE_FAILED";
        case BLOB_ERR_NO_DATA: return "NO_DATA";
        case BLOB_ERR_JPG_CONVERT_FAILED: return "JPG_CONVERT_FAILED";
        case BLOB_ERR_MISSING_ARGS: return "MISSING_ARGS";
        case BLOB_ERR_INVALID_LIGHTING_MODE: return "INVALID_LIGHTING_MODE";
        case BLOB_ERR_TOO_MANY_ARGS: return "TOO_MANY_ARGS";
    }
    return "UNKNOWN_ERROR";
}

/// @brief Fill the response with "<blob name> error: <error name>".
/// @return `err`, for `return error_response(...)`.
static CAM_capture_blob_error_enum_t error_response(
    char *response_buf, uint16_t response_buf_len, CAM_capture_blob_error_enum_t err
) {
    snprintf(
        response_buf, response_buf_len, "%s error: %s",
        BLOB_NAME, camera_capture_blob_error_to_str(err)
    );
    return err;
}


static inline uint32_t uptime_ms(void) {
    return TIME_uptime_ms_from_tim6;
}

/// @brief Pet the STM32 watchdog if it hasn't been petted recently (keeps clear of its 200 ms
///     minimum interval).
static void pet_watchdog_if_due(void) {
    if (uptime_ms() - STM32_watchdog_uptime_last_pet_ms >= WATCHDOG_PET_INTERVAL_MS) {
        STM32_pet_watchdog();
    }
}


// ==================== Arg Parsing ====================

static uint16_t parse_token(
    const char *src, uint16_t src_offset, uint16_t src_len,
    char *dst, uint16_t dst_size
) {
    uint16_t di = 0;
    uint16_t i  = src_offset;

    // Copy until next delimiter or end
    while (i < src_len && src[i] != ARG_DELIM && di < dst_size - 1) {
        dst[di++] = src[i++];
    }
    dst[di] = '\0';

    // Skip the delimiter itself
    if (i < src_len && src[i] == ARG_DELIM) i++;

    // Return index just past the token
    return i;
}

/// @brief Check if `path` (of length `len`) ends with the 4-char extension `ext` (e.g., ".img").
static bool has_extension(const char *path, uint16_t len, const char *ext) {
    return len >= 4 && strcmp(&path[len - 4], ext) == 0;
}

/// @brief If the user passed a ".jpg" filename, change it to ".img" (in place), so the raw camera
///     data goes to the .img and the converted image ends up at the requested .jpg path.
static void jpg_arg_to_img_path(char *path) {
    const uint16_t len = strlen(path);
    if (has_extension(path, len, ".jpg")) {
        memcpy(&path[len - 4], ".img", 4);
    }
}

/// @brief Make the .jpg path: replace a trailing ".img" with ".jpg", or else append ".jpg".
static void make_jpg_path(const char *img_path, char jpg_path[JPG_PATH_BUF_LEN]) {
    uint16_t len = strnlen(img_path, LFS_MAX_PATH_LENGTH - 1);
    memcpy(jpg_path, img_path, len);

    if (has_extension(img_path, len, ".img")) {
        len -= 4;
    }
    memcpy(&jpg_path[len], ".jpg", 5); // Includes the null terminator.
}


// ==================== Response Writer ====================

/// @brief Appends to a response buffer, tracking the position.
/// @note Anything that doesn't fit is silently cut off at the end of the buffer.
typedef struct {
    char *buf;
    uint16_t size;
    uint16_t pos;
} response_writer_t;

static void response_append(response_writer_t *w, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const int written = vsnprintf(&w->buf[w->pos], w->size - w->pos, fmt, args);
    va_end(args);

    if (written > 0) w->pos += written;
    if (w->pos >= w->size) w->pos = w->size - 1; // Full (vsnprintf null-terminated it).
}

/// @brief Append a uint64 as a JSON number (the firmware's printf doesn't support %llu).
static void append_uint64(response_writer_t *w, uint64_t value) {
    char str[21]; // Max uint64 is 20 digits.
    GEN_uint64_to_str(value, str);
    response_append(w, "%s", str);
}


// ==================== Metadata (Header) ====================

static bool is_target_tracking_mode(ADCS_control_mode_enum_t mode) {
    return (mode == ADCS_CONTROL_MODE_RWHEEL_TARGET_TRACKING)
        || (mode == ADCS_CONTROL_MODE_TARGET_TRACKING_YAW_ONLY_WHEEL);
}

/// @brief Append the ADCS JSON object to the header.
/// @details The current state request doubles as a probe: if it fails (e.g., ADCS powered off),
///     the other requests are skipped, as each failing request takes ~3.5 s.
static void append_adcs_json(response_writer_t *w) {
    ADCS_current_state_1_struct_t state;
    const uint8_t state_err = ADCS_get_current_state_1(&state);
    if (state_err != 0) {
        response_append(w, "{\"ok\":false,\"err\":%u}", state_err);
        return;
    }

    response_append(
        w, "{\"ok\":true,\"ctrl_mode\":%u,\"est_mode\":%u",
        (unsigned int)state.control_mode, (unsigned int)state.estimation_mode
    );

    ADCS_estimated_attitude_angles_struct_t att;
    const uint8_t att_err = ADCS_get_estimated_attitude_angles(&att);
    if (att_err != 0) {
        response_append(w, ",\"att_err\":%u", att_err);
    }
    else {
        response_append(
            // Divide as float (hardware FPU; double math is software-only on this MCU). Floats are
            // automatically promoted to double when passed to printf, as %f expects.
            w, ",\"rpy_deg\":[%.2f,%.2f,%.2f]",
            (float)att.estimated_roll_angle_mdeg / 1000.0f,
            (float)att.estimated_pitch_angle_mdeg / 1000.0f,
            (float)att.estimated_yaw_angle_mdeg / 1000.0f
        );
    }

    if (is_target_tracking_mode(state.control_mode)) {
        ADCS_tracking_controller_target_struct_t target;
        const uint8_t target_err = ADCS_get_tracking_controller_target_reference(&target);
        if (target_err != 0) {
            response_append(w, ",\"target\":{\"err\":%u}", target_err);
        }
        else {
            response_append(
                w, ",\"target\":{\"lon_deg\":%.6f,\"lat_deg\":%.6f,\"alt_m\":%.1f}",
                (double)target.longitude_degrees, (double)target.latitude_degrees,
                (double)target.altitude_meters
            );
        }
    }

    response_append(w, "}");
}

/// @brief Build the metadata JSON object (no prefix, no newline) into `w`.
/// @note The ADCS is queried first, so that the time is read as close to the trigger as possible.
/// @param[out] epoch_ms_out The "epoch_ms" written to the JSON (for the EXIF timestamp).
static void build_meta_json(response_writer_t *w, char lighting_mode, uint64_t *epoch_ms_out) {
    // ADCS goes into a scratch writer first, so the time can be read after it.
    char adcs_json[256];
    response_writer_t adcs_w = { .buf = adcs_json, .size = sizeof(adcs_json), .pos = 0 };
    adcs_json[0] = '\0';
    append_adcs_json(&adcs_w);

    const uint32_t now_uptime_ms = uptime_ms();
    const uint64_t epoch_ms = TIME_convert_uptime_to_unix_epoch_time_ms(now_uptime_ms);
    *epoch_ms_out = epoch_ms;
    const TIME_sync_source_enum_t sync_source = TIME_last_synchronization_source;

    response_append(
        w, "{\"blob\":\"%s\",\"lighting\":\"%c\",\"epoch_ms\":",
        BLOB_NAME, lighting_mode
    );
    append_uint64(w, epoch_ms);
    response_append(
        w, ",\"sync_src\":\"%c\",\"sec_since_sync\":",
        TIME_sync_source_enum_to_letter_char(sync_source)
    );
    if (sync_source == TIME_SYNC_SOURCE_NONE) {
        response_append(w, "null");
    }
    else {
        response_append(w, "%lu", (now_uptime_ms - TIME_system_uptime_at_last_time_resync_ms) / 1000);
    }
    response_append(w, ",\"adcs\":%s}", adcs_json);
}


// ==================== Capture ====================

typedef enum {
    CAPTURE_END_IDLE,
    CAPTURE_END_MAX_DURATION,
    CAPTURE_END_NO_DATA,
    CAPTURE_END_DMA_RESTART_FAILED,
    CAPTURE_END_TOO_MANY_DMA_RESTARTS,
} capture_end_reason_t;

static const char *capture_end_reason_str(capture_end_reason_t reason) {
    switch (reason) {
        case CAPTURE_END_IDLE: return "idle";
        case CAPTURE_END_MAX_DURATION: return "max_duration";
        case CAPTURE_END_NO_DATA: return "no_data";
        case CAPTURE_END_DMA_RESTART_FAILED: return "dma_restart_failed";
        case CAPTURE_END_TOO_MANY_DMA_RESTARTS: return "too_many_dma_restarts";
    }
    return "unknown";
}

typedef struct {
    lfs_file_t *img_file;
    uint32_t bytes_written; // Including the header.
    uint32_t write_errors;
    uint32_t rx_bytes; // Bytes received by the DMA.
    uint32_t first_data_ms; // Trigger to FIRST_DATA_MIN_BYTES received. 0 if never.
    uint32_t duration_ms;
    uint8_t dma_restarts;
    uint8_t next_half; // Which half (1 or 2) is next in the stream, so halves are written in order.
    capture_end_reason_t end_reason;
} capture_state_t;

static void write_to_img(capture_state_t *s, const volatile uint8_t *data, uint32_t len) {
    if (len == 0) return;
    const lfs_ssize_t result = lfs_file_write(
        &LFS_filesystem, s->img_file, (const uint8_t *)data, len
    );
    if (result < 0) {
        s->write_errors++;
    }
    else {
        s->bytes_written += result;
    }
}

/// @brief Write any half-buffers the ISR has handed off, in stream order.
static void flush_pending_halves(capture_state_t *s) {
    for (uint8_t i = 0; i < 2; i++) {
        uint8_t half = s->next_half;
        volatile uint8_t *state = (half == 1) ? &CAMERA_uart_half_1_state : &CAMERA_uart_half_2_state;

        if (*state != CAM_STATE_FILLED_WAITING_FS_WRITE) {
            // Normally only the expected half is ever waiting. Handle the other one anyway (e.g.,
            // after a DMA restart).
            half = (half == 1) ? 2 : 1;
            state = (half == 1) ? &CAMERA_uart_half_1_state : &CAMERA_uart_half_2_state;
            if (*state != CAM_STATE_FILLED_WAITING_FS_WRITE) return;
        }

        write_to_img(
            s,
            (half == 1) ? UART_camera_pending_fs_write_half_1_buf : UART_camera_pending_fs_write_half_2_buf,
            UART_camera_dma_buffer_len_half
        );
        *state = CAM_STATE_WRITTEN_TO_FS;
        s->next_half = (half == 1) ? 2 : 1;
    }
}

/// @brief Write the partially-filled half of the DMA buffer (after the DMA has stopped).
/// @note The ISR zeros each half after copying it out, and the camera only sends ASCII, so the
///     data ends at the first 0x00 byte (same approach as the firmware's v1 capture).
static void write_partial_dma_half(capture_state_t *s) {
    uint16_t start;
    if (CAMERA_uart_half_1_state == CAM_STATE_FILLING) {
        start = 0;
    }
    else if (CAMERA_uart_half_2_state == CAM_STATE_FILLING) {
        start = UART_camera_dma_buffer_len_half;
    }
    else {
        return;
    }

    // The DMA is stopped here, so it's safe to drop `volatile`.
    const uint16_t len = strnlen((const char *)&UART_camera_dma_buffer[start], UART_camera_dma_buffer_len_half);
    write_to_img(s, &UART_camera_dma_buffer[start], len);
}

static void reset_camera_dma_buffers(void) {
    // Zeros are required for write_partial_dma_half(). Only called while the DMA is stopped, so
    // it's safe to drop `volatile`.
    memset((uint8_t *)UART_camera_dma_buffer, 0, UART_camera_dma_buffer_len);
    CAMERA_uart_half_1_state = CAM_STATE_IDLE;
    CAMERA_uart_half_2_state = CAM_STATE_IDLE;
}

/// @brief Receive the image into `s->img_file`. The DMA must already be started.
static void receive_image(capture_state_t *s) {
    blob_dma_channel_t *const dma = hdma_uart4_rx.Instance;
    const uint32_t dma_len = UART_camera_dma_buffer_len;

    const uint32_t start_ms = uptime_ms();
    uint32_t last_activity_ms = start_ms;
    uint32_t last_cndtr = dma->CNDTR;
    bool data_started = false;

    while (1) {
        flush_pending_halves(s);
        pet_watchdog_if_due();

        const uint32_t now_ms = uptime_ms();

        // Count received bytes via the DMA's remaining-bytes counter (counts down, then wraps).
        // This is a true inter-byte activity check, unlike the per-half-buffer ISR timestamps.
        const uint32_t cndtr = dma->CNDTR;
        if (cndtr != last_cndtr) {
            s->rx_bytes += (cndtr < last_cndtr) ? (last_cndtr - cndtr) : (last_cndtr + dma_len - cndtr);
            last_cndtr = cndtr;
            last_activity_ms = now_ms;

            if (!data_started && s->rx_bytes >= FIRST_DATA_MIN_BYTES) {
                data_started = true;
                s->first_data_ms = now_ms - start_ms;
            }
        }

        // A UART error (e.g., overrun) aborts the DMA, and the firmware doesn't restart it.
        if ((dma->CCR & DMA_CCR_EN_BIT) == 0) {
            if (s->dma_restarts >= MAX_DMA_RESTARTS) {
                s->end_reason = CAPTURE_END_TOO_MANY_DMA_RESTARTS;
                break;
            }
            flush_pending_halves(s);
            write_partial_dma_half(s);
            reset_camera_dma_buffers();
            s->next_half = 1;
            s->dma_restarts++;

            if (CAMERA_set_expecting_data(1) != 0) {
                s->end_reason = CAPTURE_END_DMA_RESTART_FAILED;
                // Return directly: the DMA is stopped and the buffers were already handled.
                s->duration_ms = uptime_ms() - start_ms;
                return;
            }
            last_cndtr = dma->CNDTR;
            last_activity_ms = now_ms;
            continue;
        }

        if (now_ms - start_ms > MAX_DURATION_MS) {
            s->end_reason = CAPTURE_END_MAX_DURATION;
            break;
        }
        if (!data_started && (now_ms - start_ms > FIRST_DATA_TIMEOUT_MS)) {
            s->end_reason = CAPTURE_END_NO_DATA;
            break;
        }
        if (data_started && (now_ms - last_activity_ms > IDLE_TIMEOUT_MS)) {
            s->end_reason = CAPTURE_END_IDLE; // Nominal exit.
            break;
        }

        HAL_Delay(1);
    }

    CAMERA_set_expecting_data(0);

    // Count anything that arrived since the last poll.
    const uint32_t cndtr = dma->CNDTR;
    if (cndtr != last_cndtr) {
        s->rx_bytes += (cndtr < last_cndtr) ? (last_cndtr - cndtr) : (last_cndtr + dma_len - cndtr);
    }

    // A half may have completed after the last flush.
    flush_pending_halves(s);
    write_partial_dma_half(s);

    s->duration_ms = uptime_ms() - start_ms;
}


// ==================== EXIF ====================

// Layout of the EXIF APP1 segment built here (TIFF offsets are relative to the TIFF header):
//   FF E1 <len:2> "Exif\0\0" | TIFF header (8) | IFD0 (6 entries) | Exif sub-IFD (2 entries) |
//   string data | ImageDescription (written separately, straight from the metadata JSON)
// All multi-byte values are big-endian ("MM").
#define EXIF_TIFF_START 10 // After the APP1 marker (2), length (2), and "Exif\0\0" (6).
#define EXIF_TYPE_ASCII 2
#define EXIF_TYPE_LONG 4
#define EXIF_IFD_SIZE(entry_count) (2 + 12 * (entry_count) + 4)
#define EXIF_IFD0_ENTRY_COUNT 6
#define EXIF_SUB_IFD_ENTRY_COUNT 2
#define EXIF_SUB_IFD_OFFSET (8 + EXIF_IFD_SIZE(EXIF_IFD0_ENTRY_COUNT))
#define EXIF_DATA_OFFSET (EXIF_SUB_IFD_OFFSET + EXIF_IFD_SIZE(EXIF_SUB_IFD_ENTRY_COUNT))
#define EXIF_PREFIX_BUF_LEN 256 // Uses ~234 bytes.

typedef struct {
    uint8_t *tiff; // Start of the TIFF header.
    uint16_t pos; // Next write position for IFD contents (TIFF offset).
    uint16_t data_pos; // Next free byte in the string data area (TIFF offset).
} exif_writer_t;

static void put_be(uint8_t *dst, uint32_t value, uint8_t byte_count) {
    for (uint8_t i = 0; i < byte_count; i++) {
        dst[i] = (uint8_t)(value >> (8 * (byte_count - 1 - i)));
    }
}

static void exif_entry(exif_writer_t *e, uint16_t tag, uint16_t type, uint32_t count, uint32_t value) {
    put_be(&e->tiff[e->pos], tag, 2);
    put_be(&e->tiff[e->pos + 2], type, 2);
    put_be(&e->tiff[e->pos + 4], count, 4);
    put_be(&e->tiff[e->pos + 8], value, 4);
    e->pos += 12;
}

/// @brief Add an ASCII entry, with its string stored in the data area.
/// @note Every string here is over 4 bytes, so none fit inline in the entry's value field.
static void exif_ascii_entry(exif_writer_t *e, uint16_t tag, const char *str) {
    const uint16_t count = strlen(str) + 1; // Includes the null terminator.
    exif_entry(e, tag, EXIF_TYPE_ASCII, count, e->data_pos);
    memcpy(&e->tiff[e->data_pos], str, count);
    e->data_pos += (count + 1) & ~1u; // Keep offsets word-aligned (TIFF spec).
}

static void exif_start_ifd(exif_writer_t *e, uint16_t entry_count) {
    put_be(&e->tiff[e->pos], entry_count, 2);
    e->pos += 2;
}

static void exif_end_ifd(exif_writer_t *e) {
    put_be(&e->tiff[e->pos], 0, 4); // Offset of the next IFD: none.
    e->pos += 4;
}

/// @brief Format a Unix time as an EXIF datetime ("YYYY:MM:DD HH:MM:SS", UTC).
static void format_exif_datetime(uint64_t epoch_ms, char out[20]) {
    char iso[24]; // "YYYY-MM-DDTHHMMSSZ" (18 chars).
    TIME_format_utc_datetime_str_no_ms(iso, sizeof(iso), epoch_ms);
    if (strlen(iso) != 18) {
        memcpy(out, "    :  :     :  :  ", 20); // EXIF's "unknown" value.
        return;
    }
    memcpy(out, iso, 10); // "YYYY-MM-DD"
    out[4] = ':';
    out[7] = ':';
    out[10] = ' ';
    out[11] = iso[11];
    out[12] = iso[12];
    out[13] = ':';
    out[14] = iso[13];
    out[15] = iso[14];
    out[16] = ':';
    out[17] = iso[15];
    out[18] = iso[16];
    out[19] = '\0';
}

/// @brief Build the EXIF APP1 segment, except for the ImageDescription string, which must be
///     written right after the returned bytes.
/// @param desc_len Length of the ImageDescription string, including its null terminator.
/// @return Number of bytes written to `buf`.
static uint16_t build_exif_prefix(uint8_t buf[EXIF_PREFIX_BUF_LEN], uint64_t epoch_ms, uint16_t desc_len) {
    char datetime[20];
    format_exif_datetime(epoch_ms, datetime);

    exif_writer_t e = { .tiff = &buf[EXIF_TIFF_START], .pos = 8, .data_pos = EXIF_DATA_OFFSET };
    memcpy(e.tiff, "MM\x00\x2a\x00\x00\x00\x08", 8); // Big-endian TIFF header. IFD0 at offset 8.

    // IFD0. Entries must be sorted by tag.
    exif_start_ifd(&e, EXIF_IFD0_ENTRY_COUNT);
    const uint16_t desc_entry_pos = e.pos;
    exif_entry(&e, 0x010E, EXIF_TYPE_ASCII, desc_len, 0); // ImageDescription. Offset filled in below.
    exif_ascii_entry(&e, 0x010F, EXIF_MAKE);
    exif_ascii_entry(&e, 0x0110, EXIF_MODEL);
    exif_ascii_entry(&e, 0x0131, BLOB_NAME); // Software
    exif_ascii_entry(&e, 0x0132, datetime); // DateTime
    exif_entry(&e, 0x8769, EXIF_TYPE_LONG, 1, EXIF_SUB_IFD_OFFSET); // Pointer to the Exif sub-IFD.
    exif_end_ifd(&e);

    // Exif sub-IFD.
    exif_start_ifd(&e, EXIF_SUB_IFD_ENTRY_COUNT);
    exif_ascii_entry(&e, 0x9003, datetime); // DateTimeOriginal
    exif_ascii_entry(&e, 0x9011, "+00:00"); // OffsetTimeOriginal (UTC)
    exif_end_ifd(&e);

    // The ImageDescription string goes after all the other strings.
    put_be(&e.tiff[desc_entry_pos + 8], e.data_pos, 4);

    buf[0] = 0xFF;
    buf[1] = 0xE1; // APP1
    put_be(&buf[2], 2 + 6 + e.data_pos + desc_len, 2); // Segment length (includes itself).
    memcpy(&buf[4], "Exif\0\0", 6);
    return EXIF_TIFF_START + e.data_pos;
}


// ==================== JPG Conversion ====================

typedef struct {
    lfs_file_t *jpg_file;
    int32_t err; // First LFS write error (negative), or 0.
    uint32_t jpg_bytes;
    uint32_t sentences;
    uint32_t bad_lines;
    uint32_t gaps;
    uint32_t missing;
    int32_t first_num; // -1 if none.
    int32_t last_num; // -1 if none.
    uint16_t total_field; // "Total sentences" field from the last valid sentence.
    bool face_found;
    char face_hex[CAM_LINE_LEN]; // 64 hex chars + null.
    uint8_t out_buf[CAM_SENTENCE_DATA_BYTES * CONVERT_SENTENCES_PER_WRITE];
    uint16_t out_len;

    // EXIF segment, inserted into the first flush (see flush_jpg_out()).
    const uint8_t *exif_prefix;
    uint16_t exif_prefix_len;
    const char *exif_desc;
    uint16_t exif_desc_len; // Including the null terminator.
    bool exif_checked; // True once the first flush has happened.
    bool exif_written;
} convert_state_t;

static int8_t hex_char_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static void jpg_write(convert_state_t *c, const void *data, uint32_t len) {
    if (len == 0) return;
    const lfs_ssize_t result = lfs_file_write(&LFS_filesystem, c->jpg_file, data, len);
    if (result < 0) {
        if (c->err == 0) c->err = result;
    }
    else {
        c->jpg_bytes += result;
    }
}

/// @brief Write `out_buf` to the .jpg. The first flush (which holds the start of the JPEG) also
///     inserts the EXIF segment right after the SOI marker.
/// @note The camera's JPEGs have no JFIF APP0 segment (SOI is directly followed by DQT), so
///     nothing else needs to come before the EXIF APP1 segment.
static void flush_jpg_out(convert_state_t *c) {
    uint16_t start = 0;
    if (!c->exif_checked) {
        c->exif_checked = true;
        // No SOI means the first sentence was lost. Skip EXIF rather than splicing it into the data.
        if (c->out_len >= 2 && c->out_buf[0] == 0xFF && c->out_buf[1] == 0xD8) {
            jpg_write(c, c->out_buf, 2);
            jpg_write(c, c->exif_prefix, c->exif_prefix_len);
            jpg_write(c, c->exif_desc, c->exif_desc_len);
            c->exif_written = (c->err == 0);
            start = 2;
        }
    }
    jpg_write(c, &c->out_buf[start], c->out_len - start);
    c->out_len = 0;
}

/// @brief Handle one complete line (without "\r\n").
static void process_line(convert_state_t *c, const char *line, uint8_t len) {
    if (len == 0 || line[0] != '@') {
        return; // The header line, or junk between sentences.
    }
    if (len != CAM_LINE_LEN) {
        c->bad_lines++;
        return;
    }

    if (memcmp(&line[1], "FACE", 4) == 0) {
        c->face_found = true;
        memcpy(c->face_hex, &line[1], CAM_LINE_LEN - 1);
        c->face_hex[CAM_LINE_LEN - 1] = '\0';
        return;
    }

    // Validate all hex chars before writing anything.
    for (uint8_t i = 1; i < CAM_LINE_LEN; i++) {
        if (hex_char_value(line[i]) < 0) {
            c->bad_lines++;
            return;
        }
    }

    uint16_t num = 0;
    uint16_t total = 0;
    for (uint8_t i = 0; i < 4; i++) {
        num = (num << 4) | hex_char_value(line[1 + i]);
        total = (total << 4) | hex_char_value(line[5 + i]);
    }

    if (c->last_num >= 0 && (int32_t)num != c->last_num + 1) {
        c->gaps++;
        if ((int32_t)num > c->last_num + 1) {
            c->missing += num - c->last_num - 1;
        }
    }
    if (c->first_num < 0) c->first_num = num;
    c->last_num = num;
    c->total_field = total;
    c->sentences++;

    if ((uint32_t)c->out_len + CAM_SENTENCE_DATA_BYTES > sizeof(c->out_buf)) {
        flush_jpg_out(c);
    }
    for (uint8_t i = 0; i < CAM_SENTENCE_DATA_BYTES; i++) {
        c->out_buf[c->out_len++] = (hex_char_value(line[9 + 2 * i]) << 4) | hex_char_value(line[10 + 2 * i]);
    }
}

/// @brief Convert the camera's ASCII .img file to a .jpg file.
/// @return 0 on success, negative LFS error code on failure.
static int32_t convert_img_to_jpg(const char *img_path, const char *jpg_path, convert_state_t *c) {
    lfs_file_t img_file;
    lfs_file_t jpg_file;

    int32_t result = lfs_file_open(&LFS_filesystem, &img_file, img_path, LFS_O_RDONLY);
    if (result < 0) return result;

    result = lfs_file_open(
        &LFS_filesystem, &jpg_file, jpg_path, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC
    );
    if (result < 0) {
        lfs_file_close(&LFS_filesystem, &img_file);
        return result;
    }
    c->jpg_file = &jpg_file;

    uint8_t read_buf[CONVERT_READ_BUF_LEN];
    char line[CAM_LINE_BUF_LEN];
    uint8_t line_len = 0;
    bool line_overflow = false;

    while (1) {
        pet_watchdog_if_due();

        const lfs_ssize_t bytes_read = lfs_file_read(&LFS_filesystem, &img_file, read_buf, sizeof(read_buf));
        if (bytes_read < 0) {
            result = bytes_read;
            break;
        }
        if (bytes_read == 0) {
            result = 0;
            break;
        }

        for (lfs_ssize_t i = 0; i < bytes_read; i++) {
            const char ch = (char)read_buf[i];
            if (ch == '@' || ch == '\r' || ch == '\n') {
                // A '@' mid-line means bytes were dropped (e.g., the "\r\n"): end the line there,
                // so the next sentence still parses.
                if (line_overflow) c->bad_lines++;
                else process_line(c, line, line_len);
                line_len = 0;
                line_overflow = false;
                if (ch != '@') continue;
            }
            if (line_len < sizeof(line)) line[line_len++] = ch;
            else line_overflow = true;
        }
    }

    // Last line, in case the file doesn't end with "\r\n".
    if (result == 0) {
        if (line_overflow) c->bad_lines++;
        else process_line(c, line, line_len);
    }

    flush_jpg_out(c);
    if (result == 0 && c->err < 0) result = c->err;

    const int32_t jpg_close = lfs_file_close(&LFS_filesystem, &jpg_file);
    const int32_t img_close = lfs_file_close(&LFS_filesystem, &img_file);
    if (result == 0 && jpg_close < 0) result = jpg_close;
    if (result == 0 && img_close < 0) result = img_close;
    return result;
}


// ==================== Main ====================

static void camera_power_off(void) {
    EPS_set_channel_enabled(EPS_CHANNEL_3V3_CAMERA, 0);
}

/// @brief Capture the image, convert it to .jpg, and fill the response.
/// @return BLOB_ERR_OK on success. On failure before the capture runs, returns the error without
///     touching the response (blob_main() fills it in). Once the capture has run, the full JSON
///     response is always filled (with the error name in "result").
static CAM_capture_blob_error_enum_t capture_and_convert(
    const char *img_path, char lighting_mode,
    char *response_buf, uint16_t response_buf_len
) {
    char jpg_path[JPG_PATH_BUF_LEN];
    make_jpg_path(img_path, jpg_path);

    if (LFS_ensure_mounted() != 0) {
        camera_power_off();
        return BLOB_ERR_LFS_NOT_MOUNTED;
    }

    lfs_file_t img_file;
    const int32_t open_result = lfs_file_open(
        &LFS_filesystem, &img_file, img_path, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC
    );
    if (open_result < 0) {
        camera_power_off();
        return BLOB_ERR_IMG_OPEN_FAILED;
    }

    capture_state_t s = {
        .img_file = &img_file,
        .next_half = 1,
        .end_reason = CAPTURE_END_IDLE,
    };

    // Build the header (queries the ADCS, then reads the time) right before the trigger, so the
    // metadata is as close as possible to the moment the picture is taken.
    char header_buf[HEADER_BUF_LEN];
    response_writer_t header_w = { .buf = header_buf, .size = sizeof(header_buf), .pos = 0 };
    response_append(&header_w, "START_CAM:");
    const uint16_t meta_start = header_w.pos;
    uint64_t epoch_ms;
    build_meta_json(&header_w, lighting_mode, &epoch_ms);
    const uint16_t meta_end = header_w.pos;
    response_append(&header_w, "\n");

    // Write the header (also gets the slow first write out of the way before data arrives).
    write_to_img(&s, (const uint8_t *)header_buf, header_w.pos);
    if (s.write_errors > 0) {
        lfs_file_close(&LFS_filesystem, &img_file);
        camera_power_off();
        return BLOB_ERR_HEADER_WRITE_FAILED;
    }

    // Start the DMA *before* triggering, so no early bytes are missed.
    reset_camera_dma_buffers();
    if (CAMERA_set_expecting_data(1) != 0) {
        CAMERA_set_expecting_data(0);
        lfs_file_close(&LFS_filesystem, &img_file);
        camera_power_off();
        return BLOB_ERR_DMA_START_FAILED;
    }

    const uint8_t trigger = (uint8_t)lighting_mode;
    const uint8_t tx_status = (uint8_t)HAL_UART_Transmit(UART_camera_port_handle, &trigger, 1, 1000);
    if (tx_status != 0) {
        CAMERA_set_expecting_data(0);
        lfs_file_close(&LFS_filesystem, &img_file);
        camera_power_off();
        return BLOB_ERR_TRIGGER_SEND_FAILED;
    }

    receive_image(&s);
    camera_power_off();

    const int32_t close_result = lfs_file_close(&LFS_filesystem, &img_file);

    // Convert to JPG, with the metadata JSON (null-terminated in place) as the EXIF description.
    header_buf[meta_end] = '\0';
    uint8_t exif_prefix[EXIF_PREFIX_BUF_LEN];
    convert_state_t c = {
        .first_num = -1,
        .last_num = -1,
        .exif_prefix = exif_prefix,
        .exif_desc = &header_buf[meta_start],
        .exif_desc_len = meta_end - meta_start + 1,
    };
    c.exif_prefix_len = build_exif_prefix(exif_prefix, epoch_ms, c.exif_desc_len);
    int32_t convert_result = 0;
    if (close_result >= 0) {
        convert_result = convert_img_to_jpg(img_path, jpg_path, &c);
    }

    CAM_capture_blob_error_enum_t result = BLOB_ERR_OK;
    if (close_result < 0) {
        result = BLOB_ERR_IMG_CLOSE_FAILED;
    }
    else if (s.end_reason == CAPTURE_END_NO_DATA) {
        result = BLOB_ERR_NO_DATA;
    }
    else if (convert_result != 0) {
        result = BLOB_ERR_JPG_CONVERT_FAILED;
    }

    // Response (full JSON, even on failure here, as the capture details help diagnose it).
    response_writer_t w = { .buf = response_buf, .size = response_buf_len, .pos = 0 };
    response_append(
        &w, "{\"action\":\"%s\",\"result\":\"%s\",\"img\":\"%s\",\"jpg\":\"%s\"",
        BLOB_NAME, camera_capture_blob_error_to_str(result), img_path, jpg_path
    );

    response_append(&w, ",\"meta\":%s", &header_buf[meta_start]);

    response_append(
        &w,
        ",\"capture\":{\"end_reason\":\"%s\",\"rx_bytes\":%lu,\"written\":%lu,\"write_errs\":%lu,"
        "\"close_err\":%ld,\"first_data_ms\":%lu,\"duration_ms\":%lu,\"dma_restarts\":%u}",
        capture_end_reason_str(s.end_reason), s.rx_bytes, s.bytes_written, s.write_errors,
        (close_result < 0) ? close_result : 0, s.first_data_ms, s.duration_ms, s.dma_restarts
    );

    response_append(
        &w,
        ",\"jpg_result\":{\"err\":%ld,\"bytes\":%lu,\"sentences\":%lu,\"first\":%ld,\"last\":%ld,"
        "\"total_field\":%u,\"gaps\":%lu,\"missing\":%lu,\"bad_lines\":%lu,\"exif\":%s,\"face\":",
        convert_result, c.jpg_bytes, c.sentences, c.first_num, c.last_num,
        c.total_field, c.gaps, c.missing, c.bad_lines, c.exif_written ? "true" : "false"
    );
    if (c.face_found) {
        response_append(&w, "\"%s\"}}", c.face_hex);
    }
    else {
        response_append(&w, "null}}");
    }

    return result;
}


__attribute__((used, section(".text.entry")))
uint8_t blob_main(
    const char *args_str,
    char *response_buf, unsigned short response_buf_len
) {
    const uint16_t args_str_len = strlen(args_str);
    uint16_t pos = 0;

    char arg0_img_path[LFS_MAX_PATH_LENGTH];
    pos = parse_token(args_str, pos, args_str_len, arg0_img_path, sizeof(arg0_img_path));
    if (arg0_img_path[0] == '\0') {
        return error_response(response_buf, response_buf_len, BLOB_ERR_MISSING_ARGS);
    }
    jpg_arg_to_img_path(arg0_img_path);

    char arg1_lighting[4];
    pos = parse_token(args_str, pos, args_str_len, arg1_lighting, sizeof(arg1_lighting));
    const char lighting_mode = arg1_lighting[0];
    if (arg1_lighting[1] != '\0'
        || !(lighting_mode == 'd' || lighting_mode == 'm' || lighting_mode == 'n' || lighting_mode == 's')
    ) {
        return error_response(response_buf, response_buf_len, BLOB_ERR_INVALID_LIGHTING_MODE);
    }

    if (pos < args_str_len) {
        return error_response(response_buf, response_buf_len, BLOB_ERR_TOO_MANY_ARGS);
    }

    response_buf[0] = '\0';
    const CAM_capture_blob_error_enum_t err = capture_and_convert(
        arg0_img_path, lighting_mode, response_buf, response_buf_len
    );
    // Failures before the capture runs leave the response empty (see capture_and_convert()).
    if (err != BLOB_ERR_OK && response_buf[0] == '\0') {
        error_response(response_buf, response_buf_len, err);
    }
    return err;
}
