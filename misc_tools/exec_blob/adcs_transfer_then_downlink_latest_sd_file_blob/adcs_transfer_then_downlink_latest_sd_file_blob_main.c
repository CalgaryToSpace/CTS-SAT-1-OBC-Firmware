// This is a blob (executable) that will nearly double the rate of ADCS commissioning.
//
// Motivation: Each ADCS commissioning step requires collecting data into an SD file, downlinking
// the list of files, selecting the right file (by its checksum), and then bulk downlinking it. It
// requires two uplink overpasses to get the file. This blob makes it so a single commissioning 
// step requires only one uplink overpass to fetch the file.
//
// Args Format: 0 (placeholder, not used)
//
// Description of Blob:
//  1. Lists all files on the ADCS SD card.
//  2. Determine's the latest file, by index, on the SD card.
//  3. Checks if that file has "is_busy_updating = true". Returns error code 96 if it does.
//  4. Checks if that file is already downloaded/transfered into the `ADCS/` directory. If it is
//      not yet downloaded, it downloads it. Otherwise, it does nothing.
//  5. Starts the bulk downlink process to download the file.
//  6. Sends a telecommand response with the file name, size, hash, and crc16.
//
// Notes:
//  1. Likely doesn't work if there are more than 70 files on the SD card. It's the way it has to be.
//  2. You MUST stop the ADCS SD logging before running this command.
//
// Usage Example:
// After uplinking the blob as "blobs/adcs_transfer_then_downlink_latest_sd_file_v1.blob", run:
// CTS1+exec_blob_from_fs(blobs/adcs_transfer_then_downlink_latest_sd_file_v1.blob,0,0)!
//
// Implementation Note: This blob also includes the fix from the `bulk_downlink_start_blob` blob,
// as is required to initiate bulk file downlinks.
// Full description of bug: https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/653

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "../lfs.h"
#include "adcs_drivers/adcs_types.h"

#define LFS_MAX_PATH_LENGTH 200


typedef enum {
    LOG_SEVERITY_DEBUG = 1 << 0,
    LOG_SEVERITY_NORMAL = 1 << 1,
    LOG_SEVERITY_WARNING = 1 << 2,
    LOG_SEVERITY_ERROR = 1 << 3,
    LOG_SEVERITY_CRITICAL = 1 << 4,
} LOG_severity_enum_t;

static const uint32_t LOG_SYSTEM_TELECOMMAND = 1 << 12;
// static const uint32_t LOG_SYSTEM_ADCS = 1 << 7;
static const uint32_t LOG_SINK_ALL = (1 << 4) - 1;

static const char *BLOB_NAME = "adcs_grab_and_go_blob";

// Global variables defined in the firmware ELF (CTS-SAT-1_FW_rc3.elf).
extern lfs_t LFS_filesystem;
extern volatile uint32_t TIME_uptime_ms_from_tim6;

extern int snprintf(char *buf, unsigned int size, const char *fmt, ...);
extern int strlen (const char *s);

extern void LOG_message(
    uint32_t source, LOG_severity_enum_t severity, uint32_t sink_mask,
    const char *fmt, ...
);

lfs_ssize_t LFS_file_size(const char file_name[], uint8_t enable_log_messages);
extern int8_t LFS_read_file_checksum_sha256(
    const char filepath[], uint32_t start_offset, uint32_t max_length, uint8_t sha256_dest[32]
);

void HAL_Delay(uint32_t Delay);

// Bulk file downlink state and control.
typedef enum {
    COMMS_BULK_FILE_DOWNLINK_STATE_IDLE,
    COMMS_BULK_FILE_DOWNLINK_STATE_DOWNLINKING,
    COMMS_BULK_FILE_DOWNLINK_STATE_PAUSED,
} COMMS_bulk_file_downlink_state_enum_t;
extern COMMS_bulk_file_downlink_state_enum_t COMMS_bulk_file_downlink_state;
extern char COMMS_bulk_file_downlink_file_path[LFS_MAX_PATH_LENGTH];
int32_t COMMS_bulk_file_downlink_start(char *file_path, uint32_t start_offset, uint32_t max_bytes);

// ADCS SD-card file listing/transfer.
extern uint8_t LFS_is_lfs_mounted;

uint8_t ADCS_reset_file_list_read_pointer();
uint8_t ADCS_advance_file_list_read_pointer();
uint8_t ADCS_get_file_info_telemetry(ADCS_file_info_struct_t *output_struct);
// int16_t ADCS_save_sd_file_to_lfs_by_index(
//     bool index_file_bool, uint16_t file_index, bool enable_checksum_validation_bool, uint16_t checksum
// );
int16_t ADCS_save_sd_file_to_lfs_by_checksum(bool index_file_bool, uint16_t file_checksum);
uint8_t ADCS_cmd_ack(ADCS_cmd_ack_struct_t *ack);

// Worst-case time to walk the ADCS file-list pointer across all 255 files; same bound the firmware uses.
static const uint16_t ADCS_FILE_POINTER_TIMEOUT_MS = 60000;

// lfs_file_open/size/seek/read/write/close are already declared in lfs.h;
// their definitions are resolved against the firmware ELF at link time.

#define LOG(severity, fmt, ...) \
    LOG_message(LOG_SYSTEM_TELECOMMAND, severity, LOG_SINK_ALL, fmt, ##__VA_ARGS__)

// Must redefine here because it's inlined in the main code.
static inline uint32_t TIME_uptime_ms() {
    return TIME_uptime_ms_from_tim6;
}


/// @brief Find the file at the highest index in the ADCS SD card's file list.
/// @param[out] out_file_info Set to the file_info of the latest (highest-index) file, on success.
/// @param[out] out_index Set to the index (starting at 0) of the latest file, on success.
/// @return 0 on success (at least one file exists), 91 if the SD card's file list is empty,
///     otherwise the non-zero error code from the underlying ADCS command that failed.
///     ADCS error codes: 1 (Invalid TC), 2 (Incorrect Length), 3 (Incorrect Parameter), 4 (CRC check failed).
static uint8_t find_latest_sd_file(ADCS_file_info_struct_t *out_file_info, uint16_t *out_index) {
    const uint32_t function_start_time = TIME_uptime_ms();

    const uint8_t reset_status = ADCS_reset_file_list_read_pointer();
    HAL_Delay(200);
    if (reset_status != 0) {
        // Incantation: If the command fails but ACK succeeds, allow continuing.
        ADCS_cmd_ack_struct_t ack_status;
        ADCS_cmd_ack(&ack_status);
        if (ack_status.error_flag != 0) {
            return ack_status.error_flag;
        }
    }

    bool found_any = false;
    uint16_t latest_index = 0;

    // Walk the file list (bounded to 256 entries, matching ADCS_save_sd_file_to_lfs_by_index's limit)
    // until we hit the all-zero sentinel that marks the end of the list.
    for (uint16_t index = 0; index < 256; index++) {
        ADCS_file_info_struct_t file_info;
        const uint8_t file_info_status = ADCS_get_file_info_telemetry(&file_info);
        HAL_Delay(100);
        if (file_info_status != 0) {
            return file_info_status;
        }

        if (file_info.file_crc16 == 0 && file_info.file_date_time_msdos == 0 && file_info.file_size == 0) {
            // All-zero file_info means we've reached the end of the file list.
            break;
        }

        *out_file_info = file_info; // Copy struct into the caller's buffer.
        latest_index = index;
        found_any = true;

        if (TIME_uptime_ms() - function_start_time > ADCS_FILE_POINTER_TIMEOUT_MS) {
            break; // Timed out; use the latest file found so far.
        }

        const uint8_t advance_status = ADCS_advance_file_list_read_pointer();
        HAL_Delay(100);
        if (advance_status != 0) {
            ADCS_cmd_ack_struct_t ack_status;
            ADCS_cmd_ack(&ack_status);
            if (ack_status.error_flag != 0) {
                return ack_status.error_flag;
            }
        }
    }

    if (!found_any) {
        return 91; // No files found on the ADCS SD card.
    }

    *out_index = latest_index;
    return 0;
}

/// @brief Build the LittleFS path that `ADCS_save_sd_file_to_lfs_by_{index,checksum}()` uses/would use for a
///     given file, matching its naming convention exactly (see adcs_commands.c).
/// @param file_type The file's type, as reported by ADCS_get_file_info_telemetry().
/// @param file_crc16 The file's CRC16, as reported by ADCS_get_file_info_telemetry().
/// @param dest Destination buffer (at least 17 bytes, matching the firmware's `filename_string`).
/// @param dest_size Size of `dest`.
/// @return 0 on success, 92 if `file_type` isn't a recognized/downloadable type.
static uint8_t build_adcs_lfs_filename(
    ADCS_file_type_enum_t file_type, uint16_t file_crc16,
    char *dest, uint16_t dest_size
) {
    switch (file_type) {
        case ADCS_FILE_TYPE_TELEMETRY_LOG:
            snprintf(dest, dest_size, "ADCS/log_%x.TLM", file_crc16);
            break;
        case ADCS_FILE_TYPE_JPG_IMAGE:
            snprintf(dest, dest_size, "ADCS/img_%x.jpg", file_crc16);
            break;
        case ADCS_FILE_TYPE_BMP_IMAGE:
            snprintf(dest, dest_size, "ADCS/img_%x.bmp", file_crc16);
            break;
        case ADCS_FILE_TYPE_INDEX:
            snprintf(dest, dest_size, "ADCS/index_file");
            break;
        default:
            return 92; // Unrecognized/undownloadable file type.
    }
    return 0;
}

/// @brief Check whether a regular file already exists in LittleFS at the given path.
/// @param file_path Path to check.
/// @return 1 if the file exists, 0 if it does not, negative on an LFS error other than "not found".
static int8_t LFS_does_file_exist(const char *file_path) {
    lfs_file_t file;
    const int open_result = lfs_file_open(&LFS_filesystem, &file, file_path, LFS_O_RDONLY);
    if (open_result == LFS_ERR_NOENT) {
        return 0;
    }
    if (open_result < 0) {
        return open_result;
    }

    const int close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0) {
        return close_result;
    }
    return 1;
}

/// @brief Computes a CRC16 checksum of a file in LittleFS, reading it in 256-byte chunks.
/// @note The per-byte update rule matches `CRC_Calc()` from the ADCS Firmware ICD (Section 6.4.1).
/// @param filepath Path to the file to read and compute the checksum for.
/// @param[out] crc_dest Set to the computed checksum, on success.
/// @return 0 on success, negative LFS error code on error.
static int8_t LFS_read_file_checksum_crc16(const char *filepath, uint16_t *crc_dest) {
    const uint16_t chunk_size = 256;
    uint8_t read_buffer[chunk_size];

    lfs_file_t file;
    const int8_t open_result = lfs_file_open(&LFS_filesystem, &file, filepath, LFS_O_RDONLY);
    if (open_result < 0) {
        return open_result;
    }

    uint16_t crc = 0;
    while (true) {
        const int32_t bytes_read = lfs_file_read(&LFS_filesystem, &file, read_buffer, chunk_size);
        if (bytes_read < 0) {
            lfs_file_close(&LFS_filesystem, &file);
            return bytes_read;
        }
        if (bytes_read == 0) {
            break; // End of file.
        }

        for (int32_t i = 0; i < bytes_read; i++) {
            crc = (crc >> 8) | (crc << 8);      // byte-swap the CRC
            crc ^= read_buffer[i];               // XOR in the next byte
            crc ^= (crc & 0xff) >> 4;            // mix low nibble down
            crc ^= crc << 12;                    // mix up by 12 bits
            crc ^= (crc & 0xff) << 5;            // mix low byte up by 5 bits
        }
    }

    const int8_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0) {
        return close_result;
    }

    *crc_dest = crc;
    return 0;
}

/// @brief Writes a byte array to a hex string (no spaces between bytes).
/// @param byte_array Input byte array.
/// @param byte_array_len Length of input `byte_array`.
/// @param dest_str The destination to write a C-string to.
/// @param dest_str_size The size of the `dest_str` array, allocated before calling. Must be at least
///     `byte_array_len * 3 + 1` to fit the entire string.
void GEN_byte_array_to_lower_hex_str(
    const uint8_t *byte_array, uint32_t byte_array_len, char *dest_str, uint32_t dest_str_size
) {
    if (dest_str_size == 0) return; // no space at all
    char *ptr = dest_str;
    uint32_t remaining = dest_str_size;

    for (uint32_t i = 0; i < byte_array_len; ++i) {
        if (remaining < 3) break;  // not enough space for 2 chars + null terminator
        int written = snprintf(ptr, remaining, "%02x", byte_array[i]);
        if (written != 2) break;   // safety check (should always write 2 characters)
        ptr += 2;
        remaining -= 2;
    }

    *ptr = '\0'; // null-terminate even if truncated
}


/// @brief Main operation in this blob.
/// @param src_file_path 
/// @param start_offset 
/// @param byte_count 
/// @return  
/// @details Implementation is like this:
/// The goal here is not to re-implement `COMMS_bulk_file_downlink_start()`, but rather to
/// set the state to trick that function into first closing the open file (if applicable), and
/// *then* doing its normal thing (which is 100% correct).
/// 
/// We are fixing a bug in the "end of bulk downlink" logic by a hack at the start of the next
/// downlink, basically.
static int8_t bulk_downlink_start_fixed(
    const char *src_file_path,
    uint32_t start_offset, uint32_t byte_count
) {

    // If we've bulk downlinked before (i.e., the filename is set), and the current state is idle,
    // then we will set the COMMS_bulk_file_downlink_state to PAUSED, which will cause the
    // immediately-upcoming run of COMMS_bulk_file_downlink_start() to first close the file.
    if (
        (COMMS_bulk_file_downlink_state == COMMS_BULK_FILE_DOWNLINK_STATE_IDLE)
        && (COMMS_bulk_file_downlink_file_path[0] != '\0') // i.e., Filename is set.
    ) {
        COMMS_bulk_file_downlink_state = COMMS_BULK_FILE_DOWNLINK_STATE_PAUSED;
    }

    // Now, regardless, we will call COMMS_bulk_file_downlink_start() to start a new downlink.
    const int32_t result = COMMS_bulk_file_downlink_start(
        (char*)src_file_path, // Cast away const.
        start_offset, byte_count
    );
    if (result != 0) {
        LOG(
            LOG_SEVERITY_ERROR,
            "During bulk_downlink_start_fixed(), COMMS_bulk_file_downlink_start() -> %ld",
            result
        );
        return result;
    };

    return 0; // Success.
}


/// @brief Fill the response output buffer with info about the downlinked file (very helpful for
///     re-assembling/decoding the file from the downlinked bulk-transfer packets).
/// @param src_file_path Path, in LittleFS, of the file that was downlinked.
/// @param adcs_crc16 The file's CRC16, as reported by the ADCS (from ADCS_get_file_info_telemetry()).
/// @param sd_card_index The file's index in the ADCS SD card's file list, at the time it was found.
/// @param response_output_buf
/// @param response_output_buf_len
/// @return 0 on success, non-zero on error.
static int8_t fill_response_output_buffer(
    const char *src_file_path, uint16_t sd_card_index,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    // Prepare the SHA256 destination buffer.
    uint8_t sha256_dest[32] = {0}; // 32 bytes for SHA256

    // Calculate the SHA256 hash of the whole file.
    const int8_t sha256_result = LFS_read_file_checksum_sha256(
        src_file_path, 0, 0, sha256_dest
    );

    if (sha256_result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error calculating SHA256: Err=%d", sha256_result);
        return sha256_result;
    }

    // Fetch the full file size.
    const int32_t file_size_bytes = LFS_file_size(src_file_path, 1);
    if (file_size_bytes < 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error getting file size: Err=%ld", file_size_bytes);
        return file_size_bytes;
    }

    // Convert the SHA256 hash to a lowercase hex string.
    char hex_hash_str[100]; // Should be 64 chars.
    GEN_byte_array_to_lower_hex_str(sha256_dest, sizeof(sha256_dest), hex_hash_str, sizeof(hex_hash_str));

    // Calculate the file_crc16.
    uint16_t crc16_calc = 0;
    const int8_t crc16_result = LFS_read_file_checksum_crc16(src_file_path, &crc16_calc);
    if (crc16_result != 0) {
        snprintf(response_output_buf, response_output_buf_len, "Error calculating CRC16: Err=%d", crc16_result);
        return crc16_result;
    }

    // Format like JSON.
    snprintf(
        response_output_buf, response_output_buf_len,
        "{\"action\":\"%s\",\"file\":\"%s\",\"file_size\":%ld,\"crc16\":\"0x%x\",\"sha256\":\"%s\",\"sd_card_index\":%u}",
        BLOB_NAME,
        src_file_path,
        file_size_bytes,
        crc16_calc,
        hex_hash_str,
        sd_card_index
    );
    return 0;
}


__attribute__((used, section(".text.entry")))
uint8_t blob_main(
    const char *args_str,
    char *response_buf, unsigned short response_buf_len
) {
    // Log that the blob is starting (important for tracing crashes).
    LOG(
        LOG_SEVERITY_DEBUG,
        "Blob (%s) args_str: '%s'",
        BLOB_NAME,
        args_str
    );

    // Step 1: find the latest (highest-index) file on the ADCS SD card.
    ADCS_file_info_struct_t latest_file_info;
    uint16_t latest_file_index;
    const uint8_t find_latest_err = find_latest_sd_file(&latest_file_info, &latest_file_index);
    if (find_latest_err != 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: find_latest_sd_file() -> %d.",
            BLOB_NAME, find_latest_err
        );
        return find_latest_err;
    }

    if (latest_file_info.busy_updating) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: latest file (index %u) is still busy_updating.",
            BLOB_NAME, latest_file_index
        );
        return 96;
    }

    // Step 2: figure out the LittleFS path this file would live at, and check whether it's
    // already been transferred from the ADCS SD card.
    char lfs_file_path[LFS_MAX_PATH_LENGTH];
    const uint8_t build_filename_err = build_adcs_lfs_filename(
        latest_file_info.file_type, latest_file_info.file_crc16,
        lfs_file_path, sizeof(lfs_file_path)
    );
    if (build_filename_err != 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: unrecognized file_type %d for latest file (index %u).",
            BLOB_NAME, latest_file_info.file_type, latest_file_index
        );
        return build_filename_err;
    }

    const int8_t exists_result = LFS_does_file_exist(lfs_file_path);
    if (exists_result < 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: LFS_does_file_exist('%s') -> %d.",
            BLOB_NAME, lfs_file_path, exists_result
        );
        return 93;
    }
    const bool already_transferred = (exists_result == 1);

    // Step 3: transfer the file from the ADCS SD card to LittleFS, unless it's already there.
    // Checksum validation guards against the SD card's file list shifting between our
    // find_latest_sd_file() call above and this transfer (e.g. a file being deleted/added).
    if (!already_transferred) {
        const int16_t transfer_err = ADCS_save_sd_file_to_lfs_by_checksum(
            false, latest_file_info.file_crc16
        );
        if (transfer_err != 0) {
            snprintf(
                response_buf, response_buf_len,
                "%s error: ADCS_save_sd_file_to_lfs_by_checksum(index=%u) -> %d.",
                BLOB_NAME, latest_file_index, transfer_err
            );
            return 94;
        }
    }

    // Step 4: start the bulk downlink of the transferred file.
    const int8_t start_bulk_err = bulk_downlink_start_fixed(lfs_file_path, 0, 0);
    if (start_bulk_err != 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: bulk_downlink_start_fixed('%s') -> %d.",
            BLOB_NAME, lfs_file_path, start_bulk_err
        );
        return 20;
    }

    // Send a telecommand response with the file name, size, hash, and crc16.
    const int8_t resp_err = fill_response_output_buffer(
        lfs_file_path, latest_file_index,
        response_buf, response_buf_len
    );
    if (resp_err != 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s: downlink of '%s' started, but building the response failed. Error: %d.",
            BLOB_NAME, lfs_file_path, resp_err
        );
        return 10;
    }

    return 0;
}
