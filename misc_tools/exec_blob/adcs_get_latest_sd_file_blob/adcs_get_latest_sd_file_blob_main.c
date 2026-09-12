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
//  1. Sets the ADCS SD logging config to stop primary logging (in case it wasn't stopped yet).
//  2. Walks the ADCS SD card's file list exactly once, keeping the last (highest-index) entry.
//  3. Checks if that file is already downloaded/transfered into the `ADCS/` directory. If it is
//      not yet downloaded, it downloads it. Otherwise, it does nothing.
//  4. Starts the bulk downlink process to download the file.
//  5. Sends a telecommand response with the file name, size, hash, and crc16.
//
// Notes:
//  1. You MUST stop the ADCS SD logging before running this command (Step 1 tries to do it for
//     you, but a file which is still being written will never clear its Busy Updating flag).
//
// Usage Example:
// After uplinking the blob as "blobs/adcs_get_latest_sd_file_v2.blob", run:
// CTS1+exec_blob_from_fs(blobs/adcs_get_latest_sd_file_v2.blob,0,0)!
//
// Implementation Note: This blob also includes the fix from the `bulk_downlink_start_blob` blob,
// as is required to initiate bulk file downlinks.
// Full description of bug: https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/653
//
// ---------------------------------------------------------------------------------------------
// Fixes in this version (v2), relative to v1. The same three bugs exist in the main firmware, in
// every `ADCS_reset_file_list_read_pointer()` walk in `adcs_commands.c` (`ADCS_get_sd_card_file_list()`,
// `ADCS_save_sd_file_to_lfs_by_index()`, `ADCS_save_sd_file_to_lfs_by_checksum()`, and the
// `ADCS_get_file_*_by_index()` family):
//
// 1. The Busy Updating flag was never polled. Firmware Reference Manual [V7.5] Section 6.2.1 says:
//    "The File Information telemetry frame should be polled until the Busy Updating flag is
//    cleared. At this point the populated file information will be valid." v1 instead did one
//    fixed `HAL_Delay(100)` and a single read. If the ADCS had not finished populating the frame,
//    the read came back partially/entirely zeroed, which the all-zero check then misread as
//    "end of file list" -- the early end-of-list bug. We now poll until Busy Updating clears.
//
// 2. Symmetrically, a read issued *before* the ADCS started repopulating the frame returns the
//    *previous* file with Busy Updating already clear -- the "file pointer didn't advance" bug.
//    We now detect a repeated entry (file_type + file_counter uniquely identify a file, per the
//    manual) and re-read, then re-issue the advance, rather than silently accepting a duplicate.
//
// 3. The blob never petted the watchdog. The IWDG is configured with prescaler 256 / reload 2000
//    off the ~32 kHz LSI (see `MX_IWDG_Init()`), i.e. a ~16 s timeout, and the only nominal pet is
//    at the top of `TASK_execute_telecommands`, which does not run while a telecommand (this blob)
//    is executing. At the ~500 ms/file that v1 spent, the OBC rebooted at roughly 32 files. We now
//    pet during the walk and between download blocks, and the per-file cost is much lower because
//    the fixed delays are replaced by polling.
//
// Also: the file list is now walked only once. v1 walked it a second time inside
// `ADCS_save_sd_file_to_lfs_by_checksum()` to re-find the file. That is unnecessary, because
// `ADCS_load_sd_file_block_to_filesystem()` selects the file by (file_type, file_counter) from the
// `file_info` struct and does not use the file list read pointer at all.
// ---------------------------------------------------------------------------------------------

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
static const uint32_t LOG_SYSTEM_ADCS = 1 << 7;
static const uint32_t LOG_SINK_ALL = (1 << 4) - 1;

static const char *BLOB_NAME = "adcs_get_latest_sd_file_blob";

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
void STM32_pet_watchdog();

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
// Selects the file by (file_info.file_type, file_info.file_counter); does NOT use the file list
// read pointer. This is what lets us avoid a second walk of the file list.
int16_t ADCS_load_sd_file_block_to_filesystem(
    ADCS_file_info_struct_t file_info, uint8_t current_block, lfs_file_t *file
);
uint8_t ADCS_cmd_ack(ADCS_cmd_ack_struct_t *ack);
uint8_t ADCS_set_sd_log_config(
    uint8_t which_log, const uint8_t **log_array, uint8_t log_array_len, uint16_t log_period,
    ADCS_sd_log_destination_enum_t which_sd
);

// Overall cap on how long the single walk of the file list may take. Much larger than the
// firmware's 60 s, because we now pet the watchdog during the walk and each entry is far cheaper
// (poll-until-ready instead of two blind 100 ms delays).
static const uint32_t ADCS_FILE_POINTER_TIMEOUT_MS = 120000;

// Same value as the firmware's ADCS_FILE_DOWNLOAD_TIMEOUT_MS (adcs_internal_drivers.h).
static const uint32_t ADCS_FILE_DOWNLOAD_TIMEOUT_MS = 290000;

// Upper bound on entries in the ADCS SD card's file list. Only a backstop against a file list that
// never terminates; the real limit is ADCS_FILE_POINTER_TIMEOUT_MS.
static const uint16_t ADCS_MAX_FILE_LIST_ENTRIES = 4096;

// After a Reset/Advance File List Read Pointer command, wait this long before the first File
// Information read, so that the ADCS has started repopulating the frame (and thus has raised its
// Busy Updating flag) before we look at it.
static const uint32_t ADCS_FILE_POINTER_SETTLE_MS = 50;

// Busy Updating polling parameters (Firmware Reference Manual Section 6.2.1).
static const uint32_t ADCS_FILE_INFO_POLL_INTERVAL_MS = 20;
static const uint16_t ADCS_FILE_INFO_POLL_MAX_TRIES = 100; // => up to ~2 s per entry.

// If the entry we read back is identical to the previous one, re-read this many times (the frame
// may simply be stale) before concluding that the read pointer did not advance.
static const uint8_t ADCS_STALE_ENTRY_MAX_REREADS = 5;

// How many times to re-issue Advance File List Read Pointer for a single step before giving up.
static const uint8_t ADCS_ADVANCE_MAX_TRIES = 3;

// Minimum interval between watchdog pets. The IWDG is in window mode (Window=1975, Reload=2000),
// so petting sooner than ~200 ms after the previous pet triggers a reset. 1000 ms is a safe
// margin well under the ~16 s timeout.
static const uint32_t WATCHDOG_PET_INTERVAL_MS = 1000;

// ADCS file download block size, in bytes (20 bytes/packet * 1024 packets).
static const uint32_t ADCS_DOWNLOAD_BLOCK_SIZE_BYTES = 20480;

// lfs_file_open/size/seek/read/write/close are already declared in lfs.h;
// their definitions are resolved against the firmware ELF at link time.

#define LOG(severity, fmt, ...) \
    LOG_message(LOG_SYSTEM_TELECOMMAND, severity, LOG_SINK_ALL, fmt, ##__VA_ARGS__)

// Must redefine here because it's inlined in the main code.
static inline uint32_t TIME_uptime_ms() {
    return TIME_uptime_ms_from_tim6;
}


/// @brief Pet the STM32 watchdog, but only if enough time has passed since the last pet.
/// @param[in,out] last_pet_ms Uptime, in ms, of the last pet; updated when a pet happens.
/// @note The blob runs inside a telecommand, and `TASK_execute_telecommands` (the only nominal
///     petter) does not get to run again until the telecommand returns. Anything in here that can
///     take more than ~16 s must therefore pet the watchdog itself.
/// @note The blob is forbidden from having globals (the Makefile rejects a non-empty .data/.bss),
///     hence the caller-owned `last_pet_ms` state.
static void pet_watchdog_if_due(uint32_t *last_pet_ms) {
    if ((TIME_uptime_ms() - *last_pet_ms) >= WATCHDOG_PET_INTERVAL_MS) {
        STM32_pet_watchdog();
        *last_pet_ms = TIME_uptime_ms();
    }
}

/// @brief Check whether a file_info entry is the all-zero end-of-list sentinel.
/// @note Per Firmware Reference Manual Section 6.2.1: "If the populated file information comes
///     back as all zeroes, the end of the file list has been reached."
static bool is_end_of_file_list(const ADCS_file_info_struct_t *file_info) {
    return (file_info->file_crc16 == 0)
        && (file_info->file_date_time_msdos == 0)
        && (file_info->file_size == 0);
}

/// @brief Check whether two file_info entries describe the same file.
/// @note Per the Firmware Reference Manual, a file is uniquely identified by its File Type and
///     File Counter, so those two fields alone are sufficient.
static bool is_same_file(const ADCS_file_info_struct_t *a, const ADCS_file_info_struct_t *b) {
    return (a->file_type == b->file_type) && (a->file_counter == b->file_counter);
}

/// @brief Read the File Information telemetry frame, polling until its Busy Updating flag clears.
/// @param[out] out_file_info Set to the (now valid) file info, on success.
/// @param[in,out] last_pet_ms Watchdog pet state, for pet_watchdog_if_due().
/// @return 0 on success, 96 if Busy Updating never cleared (e.g. ADCS SD logging is still running
///     and writing to this file), otherwise the non-zero error code from the ADCS command.
/// @note This is the core fix: Firmware Reference Manual Section 6.2.1 requires polling here, and
///     reading the frame early returns zeroed/partial data that looks like the end-of-list sentinel.
static uint8_t read_file_info_when_ready(
    ADCS_file_info_struct_t *out_file_info, uint32_t *last_pet_ms
) {
    for (uint16_t try_num = 0; try_num < ADCS_FILE_INFO_POLL_MAX_TRIES; try_num++) {
        const uint8_t file_info_status = ADCS_get_file_info_telemetry(out_file_info);
        if (file_info_status != 0) {
            return file_info_status;
        }

        if (!out_file_info->busy_updating) {
            return 0; // The populated file information is now valid.
        }

        HAL_Delay(ADCS_FILE_INFO_POLL_INTERVAL_MS);
        pet_watchdog_if_due(last_pet_ms);
    }

    return 96; // Busy Updating never cleared.
}

/// @brief Send an Advance File List Read Pointer command, tolerating a failed command whose ACK is
///     nonetheless clean.
/// @return 0 on success, otherwise the ADCS ACK's error flag.
static uint8_t advance_file_list_read_pointer_checked() {
    const uint8_t advance_status = ADCS_advance_file_list_read_pointer();
    if (advance_status != 0) {
        // Incantation: If the command fails but ACK succeeds, allow continuing.
        // (Separate ACK, to avoid interference from the EPS.)
        ADCS_cmd_ack_struct_t ack_status;
        ADCS_cmd_ack(&ack_status);
        if (ack_status.error_flag != 0) {
            return ack_status.error_flag;
        }
    }
    return 0;
}

/// @brief Advance the file list read pointer by one, and read back the entry it now points at.
/// @param prev_file_info The entry the pointer was on before this call.
/// @param[out] out_file_info Set to the next entry (or the all-zero end-of-list sentinel).
/// @param[in,out] out_advance_retries Incremented each time the Advance command had to be re-issued.
/// @param[in,out] last_pet_ms Watchdog pet state, for pet_watchdog_if_due().
/// @return 0 on success, 97 if the read pointer would not advance off `prev_file_info`, otherwise
///     the non-zero error code from the underlying ADCS command that failed.
/// @note Handles the "pointer didn't advance" symptom: if we read back the same file we were
///     already on, we first assume the telemetry frame is merely stale and re-read a few times;
///     only if the entry is still unchanged do we re-issue the Advance command.
static uint8_t advance_and_read_next(
    const ADCS_file_info_struct_t *prev_file_info, ADCS_file_info_struct_t *out_file_info,
    uint16_t *out_advance_retries, uint32_t *last_pet_ms
) {
    for (uint8_t advance_try = 0; advance_try < ADCS_ADVANCE_MAX_TRIES; advance_try++) {
        if (advance_try > 0) {
            (*out_advance_retries)++;
        }

        const uint8_t advance_status = advance_file_list_read_pointer_checked();
        if (advance_status != 0) {
            return advance_status;
        }

        // Give the ADCS time to start repopulating the frame (and raise Busy Updating), so that
        // read_file_info_when_ready() below doesn't just see the pre-advance contents.
        HAL_Delay(ADCS_FILE_POINTER_SETTLE_MS);
        pet_watchdog_if_due(last_pet_ms);

        for (uint8_t reread = 0; reread < ADCS_STALE_ENTRY_MAX_REREADS; reread++) {
            const uint8_t read_status = read_file_info_when_ready(out_file_info, last_pet_ms);
            if (read_status != 0) {
                return read_status;
            }

            if (is_end_of_file_list(out_file_info) || !is_same_file(out_file_info, prev_file_info)) {
                return 0; // The pointer moved (or we reached the end of the list).
            }

            // Same file as before: probably a stale frame. Wait a little and look again.
            HAL_Delay(ADCS_FILE_POINTER_SETTLE_MS);
            pet_watchdog_if_due(last_pet_ms);
        }

        // Still the same file after several re-reads; the Advance command likely didn't take.
        LOG_message(
            LOG_SYSTEM_ADCS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "%s - File list read pointer did not advance off (type=%d, counter=%d); retrying.",
            BLOB_NAME, prev_file_info->file_type, prev_file_info->file_counter
        );
    }

    return 97; // The read pointer is stuck.
}

/// @brief Find the file at the highest index in the ADCS SD card's file list, walking the list once.
/// @param[out] out_file_info Set to the file_info of the latest (highest-index) file, on success.
/// @param[out] out_index Set to the index (starting at 0) of the latest file, on success.
/// @param[out] out_advance_retries Set to the number of times an Advance command had to be
///     re-issued. Non-zero means `out_index` may under-count (a retried Advance can step twice),
///     but the file itself is still correct, since it is identified by type/counter/CRC16.
/// @param[in,out] last_pet_ms Watchdog pet state, for pet_watchdog_if_due().
/// @return 0 on success (at least one file exists), 91 if the SD card's file list is empty,
///     96 if a Busy Updating flag never cleared, 97 if the read pointer got stuck, 98 if the walk
///     timed out before reaching the end of the list, otherwise the non-zero error code from the
///     underlying ADCS command that failed.
///     ADCS error codes: 1 (Invalid TC), 2 (Incorrect Length), 3 (Incorrect Parameter), 4 (CRC check failed).
/// @note A timeout is a hard error rather than "use the best file so far", because the whole point
///     of this blob is that the file it downlinks is definitely the last one.
static uint8_t find_latest_sd_file(
    ADCS_file_info_struct_t *out_file_info, uint16_t *out_index, uint16_t *out_advance_retries,
    uint32_t *last_pet_ms
) {
    const uint32_t function_start_time = TIME_uptime_ms();
    *out_advance_retries = 0;

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

    // Read the first entry. Note that Reset File List Read Pointer leaves the pointer *on* the
    // first file, so this read must happen before any Advance command.
    ADCS_file_info_struct_t file_info;
    const uint8_t first_read_status = read_file_info_when_ready(&file_info, last_pet_ms);
    if (first_read_status != 0) {
        return first_read_status;
    }
    if (is_end_of_file_list(&file_info)) {
        return 91; // No files found on the ADCS SD card.
    }

    // Walk the rest of the list, keeping the most recent entry, until we hit the all-zero sentinel.
    for (uint16_t index = 0; index < ADCS_MAX_FILE_LIST_ENTRIES; index++) {
        *out_file_info = file_info; // Copy struct into the caller's buffer.
        *out_index = index;

        pet_watchdog_if_due(last_pet_ms);

        if ((TIME_uptime_ms() - function_start_time) > ADCS_FILE_POINTER_TIMEOUT_MS) {
            return 98; // Timed out; we can't prove we found the last file, so don't guess.
        }

        const uint8_t advance_status = advance_and_read_next(
            out_file_info, &file_info, out_advance_retries, last_pet_ms
        );
        if (advance_status != 0) {
            return advance_status;
        }

        if (is_end_of_file_list(&file_info)) {
            // End of the file list: *out_file_info / *out_index hold the last real file.
            return 0;
        }
    }

    return 98; // Ran off the end of the bound without finding the end-of-list sentinel.
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

/// @brief Copy a file from the ADCS SD card into LittleFS, without touching the file list read pointer.
/// @param file_info The file to download, as returned by find_latest_sd_file().
/// @param dest_file_path LittleFS path to write to; any existing file is overwritten.
/// @param[in,out] last_pet_ms Watchdog pet state, for pet_watchdog_if_due().
/// @return 0 on success, 43 if LittleFS isn't mounted, 7 on download timeout, negative on an LFS
///     error, otherwise the non-zero error code from ADCS_load_sd_file_block_to_filesystem().
/// @note This replaces the call to `ADCS_save_sd_file_to_lfs_by_checksum()` that the previous
///     version of this blob made. That function re-walks the entire file list to re-find the file
///     we *just* found. `ADCS_load_sd_file_block_to_filesystem()` selects the file by
///     (file_type, file_counter) out of `file_info`, so the second walk buys nothing.
static int16_t download_adcs_file_to_lfs(
    const ADCS_file_info_struct_t *file_info, const char *dest_file_path, uint32_t *last_pet_ms
) {
    const uint32_t function_start_time = TIME_uptime_ms();

    if (!LFS_is_lfs_mounted) {
        return 43;
    }

    lfs_file_t file;
    const int open_result = lfs_file_open(
        &LFS_filesystem, &file, dest_file_path, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC
    );
    if (open_result < 0) {
        return open_result;
    }

    // Integer ceiling division, to avoid pulling floating-point math into the blob.
    const uint32_t total_blocks = (
        (file_info->file_size + ADCS_DOWNLOAD_BLOCK_SIZE_BYTES - 1) / ADCS_DOWNLOAD_BLOCK_SIZE_BYTES
    );

    int16_t download_err = 0;
    for (uint32_t current_block = 0; current_block < total_blocks; current_block++) {
        pet_watchdog_if_due(last_pet_ms);

        LOG_message(
            LOG_SYSTEM_ADCS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
            "%s - Loading block %lu/%lu from ADCS to LittleFS.",
            BLOB_NAME, current_block + 1, total_blocks
        );

        const int16_t load_block_status = ADCS_load_sd_file_block_to_filesystem(
            *file_info, (uint8_t)current_block, &file
        );
        if (load_block_status != 0) {
            download_err = load_block_status;
            break; // Fall through to close the file.
        }

        if ((TIME_uptime_ms() - function_start_time) > ADCS_FILE_DOWNLOAD_TIMEOUT_MS) {
            download_err = 7;
            break;
        }
    }

    // The file isn't updated in LittleFS until it's closed, so always close it.
    const int close_result = lfs_file_close(&LFS_filesystem, &file);
    if (download_err != 0) {
        return download_err;
    }
    if (close_result < 0) {
        return close_result;
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
/// @param sd_card_index The file's index in the ADCS SD card's file list, at the time it was found.
/// @param advance_retries How many Advance File List Read Pointer commands had to be re-issued
///     during the walk. Non-zero means `sd_card_index` may under-count; the file is still correct.
/// @param response_output_buf
/// @param response_output_buf_len
/// @return 0 on success, non-zero on error.
static int8_t fill_response_output_buffer(
    const char *src_file_path, uint16_t sd_card_index, uint16_t advance_retries,
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
        "{\"action\":\"%s\",\"file\":\"%s\",\"file_size\":%ld,\"crc16\":\"0x%x\",\"sha256\":\"%s\","
        "\"sd_card_index\":%u,\"advance_retries\":%u}",
        BLOB_NAME,
        src_file_path,
        file_size_bytes,
        crc16_calc,
        hex_hash_str,
        sd_card_index,
        advance_retries
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

    // The telecommand task petted the watchdog immediately before dispatching us, so "now" is a
    // correct starting point. Everything long-running below pets via pet_watchdog_if_due().
    uint32_t last_pet_ms = TIME_uptime_ms();

    // Step 0: Stop the ADCS SD logging.
    {
        const uint8_t sd_log_config[10] = {0,0,0,0,0,0,0,0,0,0};
        const uint8_t *sd_log_config_ptr[1] = {sd_log_config};
        const uint8_t stop_status = ADCS_set_sd_log_config(
            1, // (uint8_t) which_log [1 = primary, 2 = secondary]
            sd_log_config_ptr,
            1, // length of sd_log_config_ptr
            0, // (uint8_t) log_period [0 to disable]
            0 // (uint8_t) which_sd [0 = primary, 1 = secondary]
        );

        HAL_Delay(250);

        if (stop_status != 0) {
            LOG_message(
                LOG_SYSTEM_ADCS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
                "%s - Error stopping ADCS SD logging: %d",
                BLOB_NAME,
                stop_status
            );
            // Steamroll on error.
        }
    }

    // Step 1: find the latest (highest-index) file on the ADCS SD card. This is the only walk of
    // the ADCS file list that this blob performs.
    // Note that find_latest_sd_file() polls each entry's Busy Updating flag until it clears, so
    // there's no separate "is the latest file still busy_updating?" check to make here.
    ADCS_file_info_struct_t latest_file_info;
    uint16_t latest_file_index = 0;
    uint16_t advance_retries = 0;
    const uint8_t find_latest_err = find_latest_sd_file(
        &latest_file_info, &latest_file_index, &advance_retries, &last_pet_ms
    );
    if (find_latest_err != 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: find_latest_sd_file() -> %d.",
            BLOB_NAME, find_latest_err
        );
        return find_latest_err;
    }

    LOG_message(
        LOG_SYSTEM_ADCS, LOG_SEVERITY_NORMAL, LOG_SINK_ALL,
        "%s - Latest ADCS SD file: index=%u, type=%d, counter=%d, size=%lu, crc16=0x%x (advance_retries=%u).",
        BLOB_NAME, latest_file_index, latest_file_info.file_type, latest_file_info.file_counter,
        latest_file_info.file_size, latest_file_info.file_crc16, advance_retries
    );

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
    // The destination filename embeds the file's CRC16, so an existing file at this path is
    // already the file we want.
    if (!already_transferred) {
        const int16_t transfer_err = download_adcs_file_to_lfs(
            &latest_file_info, lfs_file_path, &last_pet_ms
        );
        if (transfer_err != 0) {
            snprintf(
                response_buf, response_buf_len,
                "%s error: download_adcs_file_to_lfs('%s', index=%u) -> %d.",
                BLOB_NAME, lfs_file_path, latest_file_index, transfer_err
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
        lfs_file_path, latest_file_index, advance_retries,
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
