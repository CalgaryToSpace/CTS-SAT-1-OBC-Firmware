// This is a blob (executable) that analyzes an MPI science data file in LFS, and reports a summary
// of its contents: its SHA256, CRC16, size, frame counts, time syncs (and the earliest/latest dates), and
// the "strong signals" (likely ion signals) found in the pixel data.
//
// Motivation: MPI data files are large, and downlinking them is slow. Analyzing them on-orbit
// lets us decide which files (and which byte ranges of them) are worth downlinking.
//
// Strong Signal Detection (per valid frame, based on mpi_viewer's cleaning step 2):
// 1. Skip frames that fail their CRC, background frames (the instrument sends its kept background
//      frame un-subtracted, at ~20000 DN), and warm-up frames (before the instrument's first
//      background estimate, ~frame 73, frames aren't background-subtracted and read 1000s of DN high).
// 2. Fit a straight line by least squares through the edge pixels (0, 1, 63, 64), and subtract it
//      from every pixel. This removes the frame's level and tilt.
// 3. The frame's signal is the mean residual over the interior pixels (2..62). If it's at least
//      `strong_threshold_dn`, the frame is "strong". (On the 2026 samples, quiet frames never
//      exceed ~120 DN, and ion events reach 400-550 DN.)
// 4. Strong frames whose frame counters are within MAX_SIGNAL_FRAME_GAP of each other are grouped
//      into one "strong signal" (an event usually spans several consecutive frames of a sweep).
//
// Args Format: <file_path>  or  <file_path>;kwarg1=val;kwarg2=val
// Supported kwargs:
//  - strong_threshold_dn: Minimum mean residual (DN) for a frame to be "strong". Default: 100.
//  - warmup_frames: Frames with a frame counter below this are skipped. Default: 80.
//
// Response Format (JSON):
// {
//     "action": "analyze_mpi_data_v2",
//     "file": "mpi_data/2026-07-21.mpi",
//     "sha256": "5d15...1471",
//     "crc16": "0xf821",
//     "size": 556728,
//     "frame_count": 2192,
//     "valid_frame_count": 2044,
//     "bad_frame_count": 148,
//     "restitched_frame_count": 14,
//     "background_frame_count": 8,
//     "warmup_frame_count": 78,
//     "frame_byte_range": [147, 556377],
//     "time_sync_count": 17,
//     "mpi_start_count": 1,
//     "malformed_time_sync_count": 0,
//     "earliest": {"timestamp_ms": 1784654580208, "datetime": "2026-07-21T172300.208Z_E"},
//     "latest": {"timestamp_ms": 1784654699138, "datetime": "2026-07-21T172459.138Z_E"},
//     "footer": {"uart_lost_bytes": 0, "reason_for_stopping": "TELECOMMAND"},
//     "strong_threshold_dn": 200,
//     "strong_frame_count": 8,
//     "strong_signal_count": 2,
//     "strong_signals": [
//         {"bytes": [163704, 164464], "frames": [1070, 1074], "peak_mean_dn": 541, "peak_pixel": 35, "peak_pixel_dn": 985},
//         {"bytes": [407868, 408324], "frames": [2666, 2668], "peak_mean_dn": 420, "peak_pixel": 39, "peak_pixel_dn": 742}
//     ]
// }
//
// Usage Example:
// After uplinking the blob as "blobs/analyze_mpi_data_v2.blob", run:
// CTS1+exec_blob_from_fs(blobs/analyze_mpi_data_v2.blob,0,mpi_data/your_file.mpi)!
// CTS1+exec_blob_from_fs(blobs/analyze_mpi_data_v2.blob,0,mpi_data/your_file.mpi;strong_threshold_dn=50)!
// CTS1+exec_blob_from_fs(blobs/analyze_mpi_data_v2.blob,0,mpi_data/your_file.mpi;strong_threshold_dn=150;warmup_frames=80)!
//
// Notes:
// 1. "frame_count" is the number of sync words. Each starts a frame, which is either valid (CRC
//     passes) or bad (CRC fails, or it was cut short by the next sync word). Time syncs spliced
//     into a frame are removed before its CRC check, and "restitched_frame_count" counts the valid
//     frames that had one removed. "frame_byte_range" is [start, end) of the valid frames.
// 2. "time_sync_count" includes the `mpi_start` header. "earliest"/"latest" are the time syncs with
//     the smallest/largest "timestamp_ms" (not necessarily the first/last in the file), or `null`.
//     A time sync is "malformed" if it's cut off, too long, or lacks a valid "timestamp_ms".
// 3. "footer" is from the end-of-recording footer: "uart_lost_bytes" is its "data_lost_bytes" (MPI
//     data dropped because both UART buffers were awaiting a write to flash), and
//     "reason_for_stopping" is copied as-is. It's `null` if the file has no (valid) footer, e.g.,
//     if the recording didn't stop cleanly.
// 4. Ranges are [start, end) with an exclusive end, like a Python slice. "frames" is the
//     [first, last] frame counter (inclusive). "peak_mean_dn" is the largest frame mean residual in
//     the signal, and "peak_pixel"/"peak_pixel_dn" are the pixel with the largest residual.
// 5. Only the first MAX_LISTED_SIGNALS strong signals are listed; "strong_signal_count" counts all.
// 6. "sha256" is `null` for files over SHA256_MAX_FILE_SIZE (1.1 MB), since hashing is about half
//     the runtime. "crc16" is always computed.
// 7. "crc16" is the whole file's CRC16, the same algorithm as the ADCS file CRC16 (ADCS Firmware
//     ICD `CRC_Calc()`), and the same as in `get_file_map_v1` and `bulk_downlink_start_v3`.
// 8. If the response doesn't fit in the response buffer, it's silently cut off (incomplete JSON).


// -----------------------------------------------------
// ------------- Implementation Notes ------------------
// -----------------------------------------------------
//
// MPI Data File Format (written by `TASK_service_write_mpi_data()`):
// The file starts with a `{"mpi_start":1,...}` JSON header, then is a sequence of
// [MPI buffer][time sync JSON] pairs. Each MPI buffer contains many 152-byte MPI frames. The time
// syncs are spliced in wherever the buffer ends (usually mid-frame). Each time sync looks like:
// {"uptime_ms":123,"timestamp":"1719169299720+0000042000_N","datetime":"2026-07-01T123456.789Z_G","timestamp_ms":1782909296789}
// When recording stops, `MPI_write_file_footer()` writes a footer at the end, like:
// {"data_lost_bytes": 0, "time_taken_ms": 115005, "reason_for_stopping": "TELECOMMAND" }
//
// MPI Frame Layout (152 bytes, big-endian; see simple_sat_ops `utils/mpi_viewer.c`):
//      0..3     sync word 0C FF FF 0C
//      4..5     frame counter (starts at 0 when recording starts)
//      11..12   inner dome target voltage
//      13       inner dome scan index
//      20..149  pixels: 65x uint16
//      150..151 CCITT CRC-16 over bytes 0..149
//

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "../lfs.h"
#include "crypto/sha256.h"

#define LFS_MAX_PATH_LENGTH 200
#define BLOB_ENABLE_LOGS 0

#if BLOB_ENABLE_LOGS
typedef enum {
    LOG_SEVERITY_DEBUG = 1 << 0,
    LOG_SEVERITY_NORMAL = 1 << 1,
    LOG_SEVERITY_WARNING = 1 << 2,
    LOG_SEVERITY_ERROR = 1 << 3,
    LOG_SEVERITY_CRITICAL = 1 << 4,
} LOG_severity_enum_t;

static const uint32_t LOG_SYSTEM_TELECOMMAND = 1 << 12;
static const uint32_t LOG_SINK_ALL = (1 << 4) - 1;

#endif

static const char ARG_DELIM = ';';
static const char KWARG_DELIM = '=';
static const char *BLOB_NAME = "analyze_mpi_data_v2";

static const uint32_t DEFAULT_STRONG_THRESHOLD_DN = 100;
static const uint32_t DEFAULT_WARMUP_FRAMES = 80;

/// @brief The 4 sync bytes at the start of each MPI frame (0x0C 0xFF 0xFF 0x0C), as a big-endian
///     uint32 to compare against a rolling window of the last 4 bytes read.
static const uint32_t MPI_FRAME_SYNC_WORD = 0x0CFFFF0C;

#define MPI_FRAME_LEN 152
#define MPI_FRAME_COUNTER_OFFSET 4
#define MPI_FRAME_PIXELS_OFFSET 20
#define MPI_FRAME_PIXEL_COUNT 65
#define MPI_FRAME_CRC_OFFSET 150

/// @brief Frames whose mean pixel value is above this are the instrument's kept background frames
///     (sent un-subtracted, at ~20000 DN, where normal frames read ~2000 DN).
#define BACKGROUND_FRAME_MIN_MEAN_DN 8000

/// @brief Strong frames with frame counters up to this far apart are grouped into one signal.
#define MAX_SIGNAL_FRAME_GAP 2

/// @brief Max number of strong signals listed in the response (to fit the response buffer).
#define MAX_LISTED_SIGNALS 10

/// @brief Starts of the JSON objects written between MPI buffers. All are the same length, and
///     '{' only appears as their first char.
static const char TIME_SYNC_PREFIX[] = "{\"uptime_ms\":";
static const char MPI_START_PREFIX[] = "{\"mpi_start\":";
static const char FOOTER_PREFIX[] = "{\"data_lost_b"; // Cut to the same length as the others.
#define TIME_SYNC_PREFIX_LEN (sizeof(TIME_SYNC_PREFIX) - 1)
#define PREFIX_IS_TIME_SYNC (1 << 0)
#define PREFIX_IS_MPI_START (1 << 1)
#define PREFIX_IS_FOOTER (1 << 2)

/// @brief Max length of a time sync JSON object (the firmware's buffer is 200 bytes).
#define TIME_SYNC_MAX_LEN 200

/// @brief Max length of a "datetime" string kept from a time sync (e.g., "2026-07-01T123456.789Z_G").
#define DATETIME_STR_MAX_LEN 40

/// @brief Max length of the footer's "reason_for_stopping" string (e.g., "TEMPERATURE_EXCEEDED").
#define REASON_FOR_STOPPING_STR_MAX_LEN 32

/// @brief Max length of a uint64 in decimal (20 digits), plus a null terminator.
#define UINT64_STR_MAX_LEN 21

/// @brief Files larger than this skip the SHA256 (about half the runtime), and report it as null.
#define SHA256_MAX_FILE_SIZE 1100000

#define READ_BUFFER_SIZE 2048 // Matches the LFS cache size, so reads skip the cache.

// Global variables defined in the firmware ELF (CTS-SAT-1_FW_rc3.elf).
extern lfs_t LFS_filesystem;

extern int snprintf(char *buf, unsigned int size, const char *fmt, ...);
extern int vsnprintf(char *buf, unsigned int size, const char *fmt, va_list args);
extern int strlen (const char *s);

#if BLOB_ENABLE_LOGS
extern void LOG_message(
    uint32_t source, LOG_severity_enum_t severity, uint32_t sink_mask,
    const char *fmt, ...
);
#endif

lfs_ssize_t LFS_file_size(const char file_name[], uint8_t enable_log_messages);

// sha256_init/update/final are declared in crypto/sha256.h.
// lfs_file_open/size/seek/read/write/close are already declared in lfs.h;
// their definitions are resolved against the firmware ELF at link time.

#define LOG(severity, fmt, ...) \
    LOG_message(LOG_SYSTEM_TELECOMMAND, severity, LOG_SINK_ALL, fmt, ##__VA_ARGS__)


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

/// @brief Parse a string of decimal digits into an integer.
/// @param s String to parse. Decimal digits only (no sign, no "0x" prefix).
/// @param[out] ok Set to true if `s` is a valid non-empty decimal number.
/// @returns Parsed integer, or 0 if invalid.
static int32_t parse_int(const char *s, bool *ok) {
    uint32_t result = 0;
    *ok = (s[0] != '\0');
    for (; *s != '\0'; s++) {
        if (*s < '0' || *s > '9') {
            *ok = false;
            return 0;
        }
        result = result * 10 + (*s - '0');
    }
    return (int32_t)result;
}

static bool str_equal(const char *a, const char *b) {
    while (*a != '\0' && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
}

/// @brief Parse a "key=value" token into a key and a non-negative integer value.
/// @param token The token. Modified in place (the '=' is replaced with a null terminator).
/// @param[out] key_out Set to point at the key within `token`.
/// @param[out] value_out Set to the parsed value.
/// @return true on success, false if there's no '=' or the value isn't a non-negative integer.
static bool parse_kwarg(char *token, const char **key_out, uint32_t *value_out) {
    uint16_t i = 0;
    while (token[i] != '\0' && token[i] != KWARG_DELIM) i++;
    if (token[i] != KWARG_DELIM) return false;

    token[i] = '\0';
    *key_out = token;

    bool ok;
    const int32_t value = parse_int(&token[i + 1], &ok);
    if (!ok || value < 0) return false;
    *value_out = (uint32_t)value;
    return true;
}

/// @brief Find `needle` within `haystack`.
/// @return Pointer to just past the first match in `haystack`, or NULL if not found.
static const char *find_after(const char *haystack, const char *needle) {
    for (; *haystack != '\0'; haystack++) {
        uint16_t i = 0;
        while (needle[i] != '\0' && haystack[i] == needle[i]) i++;
        if (needle[i] == '\0') return &haystack[i];
    }
    return 0;
}

static inline uint16_t read_be16(const uint8_t *buf) {
    return ((uint16_t)buf[0] << 8) | buf[1];
}

/// @brief Check an MPI frame's CRC (CCITT CRC-16, seeded 0xFFFF, over bytes 0..149, stored
///     big-endian in bytes 150..151).
/// @note Matches avr-libc's `_crc_ccitt_update()`, which the MPI's own firmware uses.
static bool mpi_frame_crc_ok(const uint8_t *frame) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < MPI_FRAME_CRC_OFFSET; i++) {
        uint8_t t = frame[i] ^ (uint8_t)(crc & 0xFF);
        t ^= (uint8_t)(t << 4);
        crc = (uint16_t)((crc >> 8) ^ ((uint16_t)t << 8) ^ ((uint16_t)t << 3) ^ ((uint16_t)t >> 4));
    }
    return crc == read_be16(&frame[MPI_FRAME_CRC_OFFSET]);
}

/// @brief Update a CRC16 with one byte.
/// @note The per-byte update rule matches `CRC_Calc()` from the ADCS Firmware ICD (Section 6.4.1).
///     Same as in the get_file_map and bulk_downlink_start blobs.
static inline uint16_t crc16_update(uint16_t crc, uint8_t byte) {
    crc = (crc >> 8) | (crc << 8);      // byte-swap the CRC
    crc ^= byte;                         // XOR in the next byte
    crc ^= (crc & 0xff) >> 4;            // mix low nibble down
    crc ^= crc << 12;                    // mix up by 12 bits
    crc ^= (crc & 0xff) << 5;            // mix low byte up by 5 bits
    return crc;
}

/// @brief A time sync's timestamp, kept as both a number (for comparing) and the original decimal
///     string (for printing, since the firmware's snprintf can't print uint64).
typedef struct {
    uint64_t timestamp_ms;
    char timestamp_ms_str[UINT64_STR_MAX_LEN];
    char datetime_str[DATETIME_STR_MAX_LEN];
} time_sync_t;

/// @brief Parse the "timestamp_ms" and "datetime" fields out of a time sync JSON object.
/// @param json Null-terminated time sync JSON object.
/// @param[out] out Parsed time sync. "datetime" is left empty if missing.
/// @return true on success, false if "timestamp_ms" is missing or invalid.
static bool parse_time_sync(const char *json, time_sync_t *out) {
    const char *ts = find_after(json, "\"timestamp_ms\":");
    if (ts == 0) return false;

    out->timestamp_ms = 0;
    uint8_t digit_count = 0;
    while (ts[digit_count] >= '0' && ts[digit_count] <= '9') {
        if (digit_count >= UINT64_STR_MAX_LEN - 1) return false;
        out->timestamp_ms = out->timestamp_ms * 10 + (ts[digit_count] - '0');
        out->timestamp_ms_str[digit_count] = ts[digit_count];
        digit_count++;
    }
    out->timestamp_ms_str[digit_count] = '\0';
    if (digit_count == 0) return false;

    out->datetime_str[0] = '\0';
    const char *dt = find_after(json, "\"datetime\":\"");
    if (dt != 0) {
        uint8_t i = 0;
        while (dt[i] != '\0' && dt[i] != '"' && i < DATETIME_STR_MAX_LEN - 1) {
            out->datetime_str[i] = dt[i];
            i++;
        }
        out->datetime_str[i] = '\0';
    }
    return true;
}

/// @brief A run of strong frames (a likely ion signal).
typedef struct {
    uint32_t start_byte;
    uint32_t end_byte;
    uint16_t first_frame_counter;
    uint16_t last_frame_counter;
    int32_t peak_mean_dn;
    int32_t peak_pixel_dn;
    uint8_t peak_pixel;
} ion_signal_t;

typedef struct {
    uint32_t strong_threshold_dn;
    uint32_t warmup_frames;
} analysis_config_t;

/// @brief Summary of an MPI data file.
typedef struct {
    bool sha256_computed; // False if the file is over SHA256_MAX_FILE_SIZE.
    uint8_t sha256[32];
    uint16_t crc16;
    uint32_t size;

    uint32_t frame_count;
    uint32_t valid_frame_count;
    uint32_t bad_frame_count;
    uint32_t restitched_frame_count;
    uint32_t background_frame_count;
    uint32_t warmup_frame_count;
    uint32_t first_valid_frame_byte;
    uint32_t last_valid_frame_end_byte;

    uint32_t time_sync_count;
    uint32_t mpi_start_count;
    uint32_t malformed_time_sync_count;
    time_sync_t earliest;
    time_sync_t latest;

    bool footer_found;
    uint32_t uart_lost_bytes;
    char reason_for_stopping[REASON_FOR_STOPPING_STR_MAX_LEN];

    uint32_t strong_frame_count;
    uint32_t strong_signal_count;
    bool signal_open;
    ion_signal_t current_signal;
    ion_signal_t signals[MAX_LISTED_SIGNALS];
} mpi_file_stats_t;

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

    w->pos += written;
    if (w->pos >= w->size) w->pos = w->size - 1; // Full (vsnprintf null-terminated it).
}

/// @brief Appends `bytes` as lowercase hex.
static void append_hex(response_writer_t *w, const uint8_t *bytes, uint8_t byte_count) {
    for (uint8_t i = 0; i < byte_count; i++) {
        response_append(w, "%02x", bytes[i]);
    }
}

/// @brief Appends a time sync as a JSON object (e.g., `{"timestamp_ms":123,"datetime":"..."}`).
static void append_time_sync(response_writer_t *w, const time_sync_t *time_sync) {
    response_append(
        w, "{\"timestamp_ms\":%s,\"datetime\":\"%s\"}",
        time_sync->timestamp_ms_str, time_sync->datetime_str
    );
}

/// @brief Appends the strong_signals JSON list.
static void append_signals(response_writer_t *w, const mpi_file_stats_t *stats) {
    response_append(w, "[");
    for (uint32_t i = 0; i < stats->strong_signal_count && i < MAX_LISTED_SIGNALS; i++) {
        const ion_signal_t *s = &stats->signals[i];
        response_append(
            w,
            "%s{\"bytes\":[%lu,%lu],\"frames\":[%u,%u],\"peak_mean_dn\":%ld,"
            "\"peak_pixel\":%u,\"peak_pixel_dn\":%ld}",
            (i == 0) ? "" : ",",
            s->start_byte, s->end_byte,
            s->first_frame_counter, s->last_frame_counter,
            s->peak_mean_dn, s->peak_pixel, s->peak_pixel_dn
        );
    }
    response_append(w, "]");
}

/// @brief Handle a complete time sync JSON object found in the file.
static void process_time_sync(const char *json, bool is_mpi_start, mpi_file_stats_t *stats) {
    time_sync_t time_sync;
    if (!parse_time_sync(json, &time_sync)) {
        stats->malformed_time_sync_count++;
        return;
    }

    if (stats->time_sync_count == 0 || time_sync.timestamp_ms < stats->earliest.timestamp_ms) {
        stats->earliest = time_sync;
    }
    if (stats->time_sync_count == 0 || time_sync.timestamp_ms > stats->latest.timestamp_ms) {
        stats->latest = time_sync;
    }
    stats->time_sync_count++;
    if (is_mpi_start) stats->mpi_start_count++;
}

/// @brief Handle the end-of-recording footer found in the file (see `MPI_write_file_footer()`).
/// @note Leaves `footer_found` false if the footer lacks either field.
static void process_footer(const char *json, mpi_file_stats_t *stats) {
    const char *lost = find_after(json, "\"data_lost_bytes\":");
    const char *reason = find_after(json, "\"reason_for_stopping\":");
    if (lost == 0 || reason == 0) return;

    while (*lost == ' ') lost++;
    uint32_t uart_lost_bytes = 0;
    if (*lost < '0' || *lost > '9') return;
    for (; *lost >= '0' && *lost <= '9'; lost++) {
        uart_lost_bytes = uart_lost_bytes * 10 + (*lost - '0');
    }

    while (*reason == ' ') reason++;
    if (*reason != '"') return;
    reason++;
    uint8_t i = 0;
    while (reason[i] != '\0' && reason[i] != '"' && i < REASON_FOR_STOPPING_STR_MAX_LEN - 1) {
        stats->reason_for_stopping[i] = reason[i];
        i++;
    }
    stats->reason_for_stopping[i] = '\0';

    stats->uart_lost_bytes = uart_lost_bytes;
    stats->footer_found = true;
}

static inline int32_t round_to_int(float x) {
    return (int32_t)(x + ((x >= 0.0f) ? 0.5f : -0.5f));
}

/// @brief Close the open strong signal (if any), adding it to the list.
static void close_signal(mpi_file_stats_t *stats) {
    if (!stats->signal_open) return;
    if (stats->strong_signal_count < MAX_LISTED_SIGNALS) {
        stats->signals[stats->strong_signal_count] = stats->current_signal;
    }
    stats->strong_signal_count++;
    stats->signal_open = false;
}

/// @brief Analyze a complete 152-byte MPI frame (CRC check, background/warm-up filtering, line
///     fit detrend, and strong signal grouping).
/// @param frame The frame, starting with the sync word, with any spliced-in time sync removed.
/// @param frame_start_byte Offset of the frame's first byte within the file.
/// @param frame_end_byte Offset just past the frame's last byte within the file. This is more
///     than `frame_start_byte + MPI_FRAME_LEN` if a time sync was spliced into the frame.
static void process_frame(
    const uint8_t *frame, uint32_t frame_start_byte, uint32_t frame_end_byte,
    const analysis_config_t *config, mpi_file_stats_t *stats
) {
    if (!mpi_frame_crc_ok(frame)) {
        stats->bad_frame_count++;
        return;
    }

    if (stats->valid_frame_count == 0) stats->first_valid_frame_byte = frame_start_byte;
    stats->last_valid_frame_end_byte = frame_end_byte;
    stats->valid_frame_count++;
    if (frame_end_byte - frame_start_byte != MPI_FRAME_LEN) stats->restitched_frame_count++;

    float pixels[MPI_FRAME_PIXEL_COUNT];
    float pixel_sum = 0.0f;
    for (uint8_t j = 0; j < MPI_FRAME_PIXEL_COUNT; j++) {
        pixels[j] = (float)read_be16(&frame[MPI_FRAME_PIXELS_OFFSET + 2 * j]);
        pixel_sum += pixels[j];
    }

    if (pixel_sum / MPI_FRAME_PIXEL_COUNT > BACKGROUND_FRAME_MIN_MEAN_DN) {
        stats->background_frame_count++;
        return;
    }

    const uint16_t frame_counter = read_be16(&frame[MPI_FRAME_COUNTER_OFFSET]);
    if (frame_counter < config->warmup_frames) {
        stats->warmup_frame_count++;
        return;
    }

    // Least-squares line through the edge pixels (0, 1, 63, 64), which carry no ion signal.
    // x is centered on the middle pixel (32), so the edge x's sum to zero.
    const int8_t edge_dx[4] = { -32, -31, 31, 32 };
    const uint8_t edge_idx[4] = { 0, 1, MPI_FRAME_PIXEL_COUNT - 2, MPI_FRAME_PIXEL_COUNT - 1 };
    float edge_mean = 0.0f;
    float slope_num = 0.0f;
    float slope_den = 0.0f;
    for (uint8_t i = 0; i < 4; i++) {
        edge_mean += pixels[edge_idx[i]];
        slope_num += edge_dx[i] * pixels[edge_idx[i]];
        slope_den += edge_dx[i] * edge_dx[i];
    }
    edge_mean /= 4.0f;
    const float slope = slope_num / slope_den;

    // Residuals above the line. The mean is over the interior pixels only.
    float interior_sum = 0.0f;
    float peak_residual = 0.0f;
    uint8_t peak_pixel = 0;
    for (uint8_t j = 0; j < MPI_FRAME_PIXEL_COUNT; j++) {
        const float residual = pixels[j] - (edge_mean + slope * ((float)j - 32.0f));
        if (j >= 2 && j <= MPI_FRAME_PIXEL_COUNT - 3) interior_sum += residual;
        if (j == 0 || residual > peak_residual) {
            peak_residual = residual;
            peak_pixel = j;
        }
    }
    const float mean_residual = interior_sum / (MPI_FRAME_PIXEL_COUNT - 4);

    if (mean_residual < (float)config->strong_threshold_dn) return;
    stats->strong_frame_count++;

    const int32_t mean_dn = round_to_int(mean_residual);
    const int32_t peak_dn = round_to_int(peak_residual);
    ion_signal_t *s = &stats->current_signal;

    const uint16_t gap = frame_counter - s->last_frame_counter; // Wraps (uint16).
    if (stats->signal_open && gap <= MAX_SIGNAL_FRAME_GAP) {
        // Extend the open signal.
        s->last_frame_counter = frame_counter;
        s->end_byte = frame_end_byte;
        if (mean_dn > s->peak_mean_dn) s->peak_mean_dn = mean_dn;
        if (peak_dn > s->peak_pixel_dn) {
            s->peak_pixel_dn = peak_dn;
            s->peak_pixel = peak_pixel;
        }
        return;
    }

    close_signal(stats);
    s->start_byte = frame_start_byte;
    s->end_byte = frame_end_byte;
    s->first_frame_counter = frame_counter;
    s->last_frame_counter = frame_counter;
    s->peak_mean_dn = mean_dn;
    s->peak_pixel_dn = peak_dn;
    s->peak_pixel = peak_pixel;
    stats->signal_open = true;
}

/// @brief Byte-by-byte parser state for a file scan.
/// @details Bytes pass through two stages:
///     1. Time sync detection, on the raw file bytes. The firmware splices time syncs into the
///        file wherever an MPI buffer ends (usually mid-frame). A byte that could start a time
///        sync is held back until the prefix either matches (the time sync's bytes are then
///        dropped) or doesn't (the held bytes are released to stage 2).
///     2. Frame parsing, on the file's bytes with the time syncs removed, so that frames with a
///        time sync spliced into them are pieced back together.
///     Each byte keeps its file offset, so frames' byte ranges still refer to the file.
typedef struct {
    const analysis_config_t *config;
    mpi_file_stats_t *stats;

    // Frame currently being captured (only valid while `frame_len > 0`).
    uint8_t frame_buf[MPI_FRAME_LEN];
    uint16_t frame_len;
    uint32_t frame_start_byte;
    uint32_t sync_window; // Last 4 bytes fed to the frame parser, big-endian.
    uint32_t sync_window_offsets[4]; // File offsets of those bytes (ring, indexed by `fed_count`).
    uint8_t fed_count; // Bytes fed to the frame parser (wraps; only the low 2 bits are used).

    // Time sync (or header/footer) currently being captured (only valid while `time_sync_len > 0`).
    char time_sync_buf[TIME_SYNC_MAX_LEN + 1];
    uint16_t time_sync_len;
    uint32_t time_sync_start_byte;
    uint8_t prefix_matches; // Which prefixes (PREFIX_IS_*) the capture still matches.
} scanner_t;

/// @brief Feed one MPI data byte (time syncs already removed) to the frame parser.
/// @param offset Offset of the byte within the file.
/// @note Always inlined, as it's called for nearly every byte (saves ~10-15 cycles/byte at -Os).
static inline __attribute__((always_inline)) void feed_frame_byte(
    scanner_t *sc, uint8_t byte, uint32_t offset
) {
    sc->sync_window_offsets[sc->fed_count & 3] = offset;
    sc->fed_count++;

    // Frames: capture the 152 bytes following each sync word.
    if (sc->frame_len > 0) {
        sc->frame_buf[sc->frame_len++] = byte;
        if (sc->frame_len == MPI_FRAME_LEN) {
            process_frame(sc->frame_buf, sc->frame_start_byte, offset + 1, sc->config, sc->stats);
            sc->frame_len = 0;
        }
    }
    sc->sync_window = (sc->sync_window << 8) | byte;
    if (sc->sync_window == MPI_FRAME_SYNC_WORD) {
        sc->stats->frame_count++;
        if (sc->frame_len > 0) sc->stats->bad_frame_count++; // Cut short by this sync word.
        sc->frame_buf[0] = 0x0C;
        sc->frame_buf[1] = 0xFF;
        sc->frame_buf[2] = 0xFF;
        sc->frame_buf[3] = 0x0C;
        sc->frame_len = 4;
        // Oldest of the last 4 offsets (not always `offset - 3`, if a time sync split the sync word).
        sc->frame_start_byte = sc->sync_window_offsets[sc->fed_count & 3];
    }
}

/// @brief Release the held-back bytes of a failed time sync prefix match to the frame parser.
static void release_time_sync_candidate(scanner_t *sc) {
    for (uint16_t i = 0; i < sc->time_sync_len; i++) {
        feed_frame_byte(sc, (uint8_t)sc->time_sync_buf[i], sc->time_sync_start_byte + i);
    }
    sc->time_sync_len = 0;
}

/// @brief Feed one raw file byte to the scanner.
/// @param offset Offset of the byte within the file.
static void feed_byte(scanner_t *sc, uint8_t byte, uint32_t offset) {
    // Capturing a time sync's body (prefix matched): capture until '}'.
    if (sc->time_sync_len >= TIME_SYNC_PREFIX_LEN) {
        const bool is_bad_char = (byte < 0x20 || byte > 0x7E || byte == '{');
        if (is_bad_char || sc->time_sync_len >= TIME_SYNC_MAX_LEN) {
            // Cut off (e.g., by missing data) or too long. Drop it (it's not MPI data either).
            if (sc->prefix_matches != PREFIX_IS_FOOTER) sc->stats->malformed_time_sync_count++;
            sc->time_sync_len = 0;
            // Fall through, as this byte may be MPI data or start a new time sync.
        }
        else {
            sc->time_sync_buf[sc->time_sync_len++] = byte;
            if (byte == '}') {
                sc->time_sync_buf[sc->time_sync_len] = '\0';
                if (sc->prefix_matches == PREFIX_IS_FOOTER) {
                    process_footer(sc->time_sync_buf, sc->stats);
                }
                else {
                    process_time_sync(
                        sc->time_sync_buf, (sc->prefix_matches == PREFIX_IS_MPI_START), sc->stats
                    );
                }
                sc->time_sync_len = 0;
            }
            return;
        }
    }

    // Matching a time sync prefix: hold the bytes back until it matches or doesn't.
    if (sc->time_sync_len > 0) {
        if (byte != (uint8_t)TIME_SYNC_PREFIX[sc->time_sync_len]) sc->prefix_matches &= ~PREFIX_IS_TIME_SYNC;
        if (byte != (uint8_t)MPI_START_PREFIX[sc->time_sync_len]) sc->prefix_matches &= ~PREFIX_IS_MPI_START;
        if (byte != (uint8_t)FOOTER_PREFIX[sc->time_sync_len]) sc->prefix_matches &= ~PREFIX_IS_FOOTER;
        if (sc->prefix_matches != 0) {
            sc->time_sync_buf[sc->time_sync_len++] = byte;
            return;
        }
        // Not a time sync, so the held bytes are MPI data. They can't contain the start of another
        // time sync, as '{' is only the first char of the prefixes.
        release_time_sync_candidate(sc);
    }

    if (byte == '{') {
        sc->time_sync_buf[0] = '{';
        sc->time_sync_len = 1;
        sc->time_sync_start_byte = offset;
        sc->prefix_matches = PREFIX_IS_TIME_SYNC | PREFIX_IS_MPI_START | PREFIX_IS_FOOTER;
        return;
    }

    feed_frame_byte(sc, byte, offset);
}

/// @brief Read the file once, computing the SHA256 (if not too big) and CRC16, finding and
///     analyzing frames (piecing back together frames with time syncs spliced in), and parsing
///     time syncs.
/// @param file_path Path of the file to read.
/// @param[in,out] stats `size` must be set before calling. All other fields are filled in.
/// @return 0 on success, negative LFS error code on read error, 1 if the file size changed.
static int32_t scan_file(
    const char *file_path, const analysis_config_t *config, mpi_file_stats_t *stats
) {
    uint8_t read_buffer[READ_BUFFER_SIZE];

    scanner_t sc;
    sc.config = config;
    sc.stats = stats;
    sc.frame_len = 0;
    sc.frame_start_byte = 0;
    sc.sync_window = 0;
    sc.fed_count = 0;
    sc.time_sync_len = 0;
    sc.prefix_matches = 0;

    stats->frame_count = 0;
    stats->valid_frame_count = 0;
    stats->bad_frame_count = 0;
    stats->restitched_frame_count = 0;
    stats->background_frame_count = 0;
    stats->warmup_frame_count = 0;
    stats->first_valid_frame_byte = 0;
    stats->last_valid_frame_end_byte = 0;
    stats->time_sync_count = 0;
    stats->mpi_start_count = 0;
    stats->malformed_time_sync_count = 0;
    stats->footer_found = false;
    stats->strong_frame_count = 0;
    stats->strong_signal_count = 0;
    stats->signal_open = false;

    lfs_file_t file;
    const int32_t open_result = lfs_file_open(&LFS_filesystem, &file, file_path, LFS_O_RDONLY);
    if (open_result < 0) {
        return open_result;
    }

    stats->sha256_computed = (stats->size <= SHA256_MAX_FILE_SIZE);
    SHA256_CTX sha256_ctx;
    sha256_init(&sha256_ctx);
    uint16_t crc16 = 0;

    uint32_t file_pos = 0; // Offset of the start of `read_buffer` within the file.

    while (1) {
        const int32_t bytes_read = lfs_file_read(&LFS_filesystem, &file, read_buffer, READ_BUFFER_SIZE);
        if (bytes_read < 0) {
            lfs_file_close(&LFS_filesystem, &file);
            return bytes_read;
        }
        if (bytes_read == 0) break; // EOF.

        if (stats->sha256_computed) sha256_update(&sha256_ctx, read_buffer, bytes_read);

        for (int32_t i = 0; i < bytes_read; i++) {
            crc16 = crc16_update(crc16, read_buffer[i]);
            feed_byte(&sc, read_buffer[i], file_pos + i);
        }

        file_pos += bytes_read;
    }

    // A time sync (or footer) cut off by the end of the file, or held-back bytes that turned out to be
    // MPI data.
    if (sc.time_sync_len >= TIME_SYNC_PREFIX_LEN) {
        if (sc.prefix_matches != PREFIX_IS_FOOTER) stats->malformed_time_sync_count++;
    }
    else {
        release_time_sync_candidate(&sc);
    }
    // A frame cut off by the end of the file.
    if (sc.frame_len > 0) stats->bad_frame_count++;
    close_signal(stats);

    const int32_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0) {
        return close_result;
    }

    if (file_pos != stats->size) {
        return 1; // File changed size while reading.
    }

    if (stats->sha256_computed) sha256_final(&sha256_ctx, stats->sha256);
    stats->crc16 = crc16;
    return 0;
}


/// @brief Main operation in this blob. Fills the response buffer with the analysis JSON.
/// @return 0 on success, non-zero error code on failure (with an error message in the response).
static uint8_t analyze_mpi_data(
    const char *file_path, const analysis_config_t *config,
    char *response_buf, uint16_t response_buf_len
) {
    // Stored on the stack (not a global). ~650 bytes.
    mpi_file_stats_t stats;

    const lfs_ssize_t file_size = LFS_file_size(file_path, 1);
    if (file_size < 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: can't get size of '%s' (err=%ld)",
            BLOB_NAME, file_path, file_size
        );
        return 20;
    }
    stats.size = (uint32_t)file_size;

    const int32_t scan_result = scan_file(file_path, config, &stats);
    if (scan_result != 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: file scan failed (err=%ld)",
            BLOB_NAME, scan_result
        );
        return 22;
    }

    response_writer_t w = { .buf = response_buf, .size = response_buf_len, .pos = 0 };
    response_append(&w, "{\"action\":\"%s\",\"file\":\"%s\",\"sha256\":", BLOB_NAME, file_path);
    if (stats.sha256_computed) {
        response_append(&w, "\"");
        append_hex(&w, stats.sha256, sizeof(stats.sha256));
        response_append(&w, "\"");
    }
    else {
        response_append(&w, "null");
    }
    response_append(&w, ",\"crc16\":\"0x%04x\",\"size\":%lu", stats.crc16, stats.size);

    response_append(
        &w,
        ",\"frame_count\":%lu,\"valid_frame_count\":%lu,\"bad_frame_count\":%lu"
        ",\"restitched_frame_count\":%lu,\"background_frame_count\":%lu,\"warmup_frame_count\":%lu",
        stats.frame_count, stats.valid_frame_count, stats.bad_frame_count, stats.restitched_frame_count,
        stats.background_frame_count, stats.warmup_frame_count
    );
    if (stats.valid_frame_count == 0) {
        response_append(&w, ",\"frame_byte_range\":null");
    }
    else {
        response_append(
            &w, ",\"frame_byte_range\":[%lu,%lu]",
            stats.first_valid_frame_byte, stats.last_valid_frame_end_byte
        );
    }

    response_append(
        &w, ",\"time_sync_count\":%lu,\"mpi_start_count\":%lu,\"malformed_time_sync_count\":%lu",
        stats.time_sync_count, stats.mpi_start_count, stats.malformed_time_sync_count
    );
    if (stats.time_sync_count == 0) {
        response_append(&w, ",\"earliest\":null,\"latest\":null");
    }
    else {
        response_append(&w, ",\"earliest\":");
        append_time_sync(&w, &stats.earliest);
        response_append(&w, ",\"latest\":");
        append_time_sync(&w, &stats.latest);
    }

    if (!stats.footer_found) {
        response_append(&w, ",\"footer\":null");
    }
    else {
        response_append(
            &w, ",\"footer\":{\"uart_lost_bytes\":%lu,\"reason_for_stopping\":\"%s\"}",
            stats.uart_lost_bytes, stats.reason_for_stopping
        );
    }

    response_append(
        &w, ",\"strong_threshold_dn\":%lu,\"strong_frame_count\":%lu,\"strong_signal_count\":%lu"
        ",\"strong_signals\":",
        config->strong_threshold_dn, stats.strong_frame_count, stats.strong_signal_count
    );
    append_signals(&w, &stats);
    response_append(&w, "}");

    return 0;
}


__attribute__((used, section(".text.entry")))
uint8_t blob_main(
    const char *args_str,
    char *response_buf, unsigned short response_buf_len
) {
    // Log that the blob is starting (important for tracing crashes).
    #if BLOB_ENABLE_LOGS
    LOG(
        LOG_SEVERITY_NORMAL,
        "Blob (%s) args_str: '%s'",
        BLOB_NAME,
        args_str
    );
    #endif

    const uint16_t args_str_len = strlen(args_str);
    uint16_t pos = 0;

    char arg0_file_path[LFS_MAX_PATH_LENGTH];
    pos = parse_token(args_str, pos, args_str_len, arg0_file_path, sizeof(arg0_file_path));

    if (arg0_file_path[0] == '\0') {
        snprintf(
            response_buf, response_buf_len,
            "%s error: missing file path arg!",
            BLOB_NAME
        );
        return 135;
    }

    analysis_config_t config = {
        .strong_threshold_dn = DEFAULT_STRONG_THRESHOLD_DN,
        .warmup_frames = DEFAULT_WARMUP_FRAMES,
    };

    // Parse kwargs.
    while (pos < args_str_len) {
        char kwarg_token[48];
        pos = parse_token(args_str, pos, args_str_len, kwarg_token, sizeof(kwarg_token));
        if (kwarg_token[0] == '\0') continue; // Tolerate empty tokens (e.g., trailing ';').

        const char *key;
        uint32_t value;
        if (!parse_kwarg(kwarg_token, &key, &value)) {
            snprintf(
                response_buf, response_buf_len,
                "%s error: invalid kwarg '%s' (need key=non_negative_int)",
                BLOB_NAME, kwarg_token
            );
            return 136;
        }

        if (str_equal(key, "strong_threshold_dn")) {
            if (value == 0) {
                snprintf(
                    response_buf, response_buf_len,
                    "%s error: strong_threshold_dn must be positive",
                    BLOB_NAME
                );
                return 138;
            }
            config.strong_threshold_dn = value;
        }
        else if (str_equal(key, "warmup_frames")) {
            config.warmup_frames = value;
        }
        else {
            snprintf(
                response_buf, response_buf_len,
                "%s error: unknown kwarg '%s'",
                BLOB_NAME, key
            );
            return 137;
        }
    }

    return analyze_mpi_data(arg0_file_path, &config, response_buf, response_buf_len);
}
