// This is a blob (executable) that replaces the "CTS1+bulk_file_downlink_start" command.
//
// Motivation: The existing FrontierSat bulk file downlink system contains a bug where you can only
// use it 40 times before the satellite needs a reboot to continue using the filesystem.
// This blob is a workaround to fix that bug/limitation.
//
// Full description of bug: https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/653
//
// Args Format: <file_path_to_read>;<start_offset>;<byte_count>
// The start_offset and byte_count can both be zero to downlink up to 1 MB.
//
// Response Format (JSON):
// {"action":"bulk_downlink_start_v3","file":"<path>","file_size":1234,"offset":0,"length":1234,
//  "file_sha256":"<hex>","file_crc16":"0xabcd"}
// The checksums cover exactly the downlinked bytes (`offset` to `offset + length`). The fields are
// "file_sha256"/"file_crc16" when that's the whole file, or "chunk_sha256"/"chunk_crc16" otherwise
// (non-zero offset, a smaller byte_count, or a file over 1 MB). `file_size` is always the full size.
//
// Usage Example:
// After uplinking the blob as "blobs/bulk_downlink_start_v3.blob", run:
// CTS1+exec_blob_from_fs(blobs/bulk_downlink_start_v3.blob,0,your_file.run;0;0)!

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "../lfs.h"
#include "crypto/sha256.h"

#define LFS_MAX_PATH_LENGTH 200
#define READ_BUFFER_SIZE 2048 // Matches the LFS cache size, so reads skip the cache.

// Must match `COMMS_bulk_file_downlink_max_allowable_total_bytes` in the firmware.
#define BULK_DOWNLINK_MAX_BYTES 1000000


typedef enum {
    LOG_SEVERITY_DEBUG = 1 << 0,
    LOG_SEVERITY_NORMAL = 1 << 1,
    LOG_SEVERITY_WARNING = 1 << 2,
    LOG_SEVERITY_ERROR = 1 << 3,
    LOG_SEVERITY_CRITICAL = 1 << 4,
} LOG_severity_enum_t;

static const uint32_t LOG_SYSTEM_TELECOMMAND = 1 << 12;
static const uint32_t LOG_SINK_ALL = (1 << 4) - 1;

static const char ARG_DELIM = ';';
static const char *BLOB_NAME = "bulk_downlink_start_v3";

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

extern void GEN_byte_array_to_hex_str(
    const uint8_t *byte_array, uint32_t byte_array_len, char *dest_str, uint32_t dest_str_size
);


// Bulk file downlink state and control.
typedef enum {
    COMMS_BULK_FILE_DOWNLINK_STATE_IDLE,
    COMMS_BULK_FILE_DOWNLINK_STATE_DOWNLINKING,
    COMMS_BULK_FILE_DOWNLINK_STATE_PAUSED,
} COMMS_bulk_file_downlink_state_enum_t;
extern COMMS_bulk_file_downlink_state_enum_t COMMS_bulk_file_downlink_state;
extern char COMMS_bulk_file_downlink_file_path[LFS_MAX_PATH_LENGTH];
int32_t COMMS_bulk_file_downlink_start(char *file_path, uint32_t start_offset, uint32_t max_bytes);

// lfs_file_open/size/seek/read/write/close are already declared in lfs.h;
// their definitions are resolved against the firmware ELF at link time.

#define LOG(severity, fmt, ...) \
    LOG_message(LOG_SYSTEM_TELECOMMAND, severity, LOG_SINK_ALL, fmt, ##__VA_ARGS__)

// Must redefine here because it's inlined in the main code.
static inline uint32_t TIME_uptime_ms() {
    return TIME_uptime_ms_from_tim6;
}

// MARK: Error Enum

// All named status/error codes returned by this blob's functions, including the values
// `blob_main()` itself returns to the telecommand executor (0 = success).
// For LFS and COMMS failures, the raw error code is included in the response string.
typedef enum {
    BLOB_ERR_OK = 0,
    BLOB_ERR_FILE_SIZE_FAILED = 11, // LFS_file_size() failed (e.g., the file doesn't exist).
    BLOB_ERR_OFFSET_PAST_END_OF_FILE = 12, // start_offset is at or past the end of the file.
    BLOB_ERR_CHECKSUM_READ_FAILED = 13, // An LFS open/seek/read/close failed while checksumming.
    BLOB_ERR_FILE_SHRANK = 14, // The file ended before `length` bytes were checksummed.
    BLOB_ERR_BULK_DOWNLINK_START_FAILED = 20, // COMMS_bulk_file_downlink_start() failed.
    BLOB_ERR_MISSING_ARGS = 135, // Fewer than 3 args were given.
    BLOB_ERR_INVALID_INT_ARG = 136, // start_offset or byte_count isn't a valid integer.
} COMMS_bulk_downlink_start_blob_error_enum_t;

/// @brief Convert a COMMS_bulk_downlink_start_blob_error_enum_t into a short human-readable name,
///     for use in log messages and output strings.
/// @return Static string; never NULL. "UNKNOWN_ERROR" for values not in the enum.
static const char *bulk_downlink_start_blob_error_to_str(COMMS_bulk_downlink_start_blob_error_enum_t err) {
    switch (err) {
        case BLOB_ERR_OK: return "OK";
        case BLOB_ERR_FILE_SIZE_FAILED: return "FILE_SIZE_FAILED";
        case BLOB_ERR_OFFSET_PAST_END_OF_FILE: return "OFFSET_PAST_END_OF_FILE";
        case BLOB_ERR_CHECKSUM_READ_FAILED: return "CHECKSUM_READ_FAILED";
        case BLOB_ERR_FILE_SHRANK: return "FILE_SHRANK";
        case BLOB_ERR_BULK_DOWNLINK_START_FAILED: return "BULK_DOWNLINK_START_FAILED";
        case BLOB_ERR_MISSING_ARGS: return "MISSING_ARGS";
        case BLOB_ERR_INVALID_INT_ARG: return "INVALID_INT_ARG";
    }
    return "UNKNOWN_ERROR";
}

// MARK: Parsing Helpers

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

static int8_t hex_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1; // Error.
}

/// @brief Parse a string into an integer.
/// @param s String to parse. Valid formats: "0x<digits>" or "<digits>". Underscores are ignored.
/// @returns Parsed integer, or 0 if invalid.
static int32_t parse_int(const char *s, bool *ok) {
    uint32_t result = 0;
    bool hex = false;
    uint8_t i = 0;

    if (ok) *ok = false;
    if (!s || s[0] == '\0') return 0;

    // Detect 0x prefix
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        hex = true;
        i = 2;
        if (s[i] == '\0') return 0; // bare "0x" is invalid
    }

    bool has_digits = false;
    while (s[i] != '\0') {
        if (s[i] == '_') { i++; continue; } // skip delimiter

        if (hex) {
            int8_t d = hex_to_int(s[i]);
            if (d < 0) return 0; // invalid char
            result = (result << 4) | (uint8_t)d;
        } else {
            if (s[i] < '0' || s[i] > '9') return 0; // invalid char
            result = result * 10 + (s[i] - '0');
        }
        has_digits = true;
        i++;
    }

    if (!has_digits) return 0;
    if (ok) *ok = true;
    return (int32_t)result;
}


/// @brief Main operation in this blob.
/// @param src_file_path 
/// @param start_offset 
/// @param byte_count 
/// @param[out] comms_err Set to the raw COMMS_bulk_file_downlink_start() result.
/// @return BLOB_ERR_OK, or BLOB_ERR_BULK_DOWNLINK_START_FAILED.
/// @details Implementation is like this:
/// The goal here is not to re-implement `COMMS_bulk_file_downlink_start()`, but rather to
/// set the state to trick that function into first closing the open file (if applicable), and
/// *then* doing its normal thing (which is 100% correct).
/// 
/// We are fixing a bug in the "end of bulk downlink" logic by a hack at the start of the next
/// downlink, basically.
static COMMS_bulk_downlink_start_blob_error_enum_t bulk_downlink_start_fixed(
    const char *src_file_path,
    uint32_t start_offset, uint32_t byte_count,
    int32_t *comms_err
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
    *comms_err = COMMS_bulk_file_downlink_start(
        (char*)src_file_path, // Cast away const.
        start_offset, byte_count
    );
    if (*comms_err != 0) {
        LOG(
            LOG_SEVERITY_ERROR,
            "During bulk_downlink_start_fixed(), COMMS_bulk_file_downlink_start() -> %ld",
            *comms_err
        );
        return BLOB_ERR_BULK_DOWNLINK_START_FAILED;
    };

    return BLOB_ERR_OK;
}


/// @brief Update a CRC16 with one byte.
/// @note The per-byte update rule matches `CRC_Calc()` from the ADCS Firmware ICD (Section 6.4.1).
///     Same as in the get_file_map blob.
static inline uint16_t crc16_update(uint16_t crc, uint8_t byte) {
    crc = (crc >> 8) | (crc << 8);      // byte-swap the CRC
    crc ^= byte;                         // XOR in the next byte
    crc ^= (crc & 0xff) >> 4;            // mix low nibble down
    crc ^= crc << 12;                    // mix up by 12 bits
    crc ^= (crc & 0xff) << 5;            // mix low byte up by 5 bits
    return crc;
}

/// @brief Read `length` bytes of the file from `start_offset` once, computing the SHA256 and CRC16.
/// @param[out] lfs_err Set to the raw LFS error code on BLOB_ERR_CHECKSUM_READ_FAILED (else 0).
/// @return BLOB_ERR_OK, BLOB_ERR_CHECKSUM_READ_FAILED, or BLOB_ERR_FILE_SHRANK.
static COMMS_bulk_downlink_start_blob_error_enum_t compute_checksums(
    const char *file_path, uint32_t start_offset, uint32_t length,
    uint8_t sha256_dest[32], uint16_t *crc16_dest, int32_t *lfs_err
) {
    uint8_t read_buffer[READ_BUFFER_SIZE];
    *lfs_err = 0;

    lfs_file_t file;
    const int32_t open_result = lfs_file_open(&LFS_filesystem, &file, file_path, LFS_O_RDONLY);
    if (open_result < 0) {
        *lfs_err = open_result;
        return BLOB_ERR_CHECKSUM_READ_FAILED;
    }

    const lfs_soff_t seek_result = lfs_file_seek(&LFS_filesystem, &file, start_offset, LFS_SEEK_SET);
    if (seek_result < 0) {
        lfs_file_close(&LFS_filesystem, &file);
        *lfs_err = seek_result;
        return BLOB_ERR_CHECKSUM_READ_FAILED;
    }

    SHA256_CTX sha256_ctx;
    sha256_init(&sha256_ctx);
    uint16_t crc = 0;

    uint32_t bytes_remaining = length;
    while (bytes_remaining > 0) {
        const uint32_t bytes_to_read = (bytes_remaining < READ_BUFFER_SIZE) ? bytes_remaining : READ_BUFFER_SIZE;
        const int32_t bytes_read = lfs_file_read(&LFS_filesystem, &file, read_buffer, bytes_to_read);
        if (bytes_read < 0) {
            lfs_file_close(&LFS_filesystem, &file);
            *lfs_err = bytes_read;
            return BLOB_ERR_CHECKSUM_READ_FAILED;
        }
        if (bytes_read == 0) {
            lfs_file_close(&LFS_filesystem, &file);
            return BLOB_ERR_FILE_SHRANK; // File shrank since its size was checked.
        }

        sha256_update(&sha256_ctx, read_buffer, bytes_read);
        for (int32_t i = 0; i < bytes_read; i++) {
            crc = crc16_update(crc, read_buffer[i]);
        }
        bytes_remaining -= bytes_read;
    }

    const int32_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0) {
        *lfs_err = close_result;
        return BLOB_ERR_CHECKSUM_READ_FAILED;
    }

    sha256_final(&sha256_ctx, sha256_dest);
    *crc16_dest = crc;
    return BLOB_ERR_OK;
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


/// @brief Fill the response output buffer with info about the file (very helpful for decoding).
/// @note This is a totally new feature, independent from the original bulk downlink telecommand.
///     It is observed as a helpful feature for re-assembling the downlinked files during mission ops.
/// @param src_file_path
/// @param start_offset
/// @param byte_count Requested downlink length, or 0 for the rest of the file (up to 1 MB).
/// @param response_output_buf
/// @param response_output_buf_len
/// @return BLOB_ERR_OK on success, or an error (with an error message in the response).
/// @details The checksums cover exactly the bytes that will be downlinked (same length rules as
///     `COMMS_bulk_file_downlink_start()`). If that's the whole file, the checksum fields are named
///     "file_sha256"/"file_crc16"; otherwise, "chunk_sha256"/"chunk_crc16".
static COMMS_bulk_downlink_start_blob_error_enum_t fill_response_output_buffer(
    const char *src_file_path,
    uint32_t start_offset, uint32_t byte_count,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    // Fetch the full file size.
    const int32_t file_size_bytes = LFS_file_size(src_file_path, 1);
    if (file_size_bytes < 0) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "%s error: LFS_file_size('%s') -> %ld (%s).",
            BLOB_NAME, src_file_path, file_size_bytes,
            bulk_downlink_start_blob_error_to_str(BLOB_ERR_FILE_SIZE_FAILED)
        );
        return BLOB_ERR_FILE_SIZE_FAILED;
    }
    if (start_offset >= (uint32_t)file_size_bytes) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "%s error: start_offset %lu >= file size %ld of '%s' (%s).",
            BLOB_NAME, start_offset, file_size_bytes, src_file_path,
            bulk_downlink_start_blob_error_to_str(BLOB_ERR_OFFSET_PAST_END_OF_FILE)
        );
        return BLOB_ERR_OFFSET_PAST_END_OF_FILE;
    }

    // Same length rules as COMMS_bulk_file_downlink_start(): 0 or >1 MB means 1 MB, then limited
    // to the rest of the file.
    uint32_t length = (byte_count == 0 || byte_count > BULK_DOWNLINK_MAX_BYTES) ? BULK_DOWNLINK_MAX_BYTES : byte_count;
    if (length > (uint32_t)file_size_bytes - start_offset) {
        length = (uint32_t)file_size_bytes - start_offset;
    }
    const bool is_whole_file = (start_offset == 0) && (length == (uint32_t)file_size_bytes);
    const char *checksum_prefix = is_whole_file ? "file" : "chunk";

    uint8_t sha256_dest[32] = {0};
    uint16_t crc16 = 0;
    int32_t lfs_err = 0;
    const COMMS_bulk_downlink_start_blob_error_enum_t checksum_err = compute_checksums(
        src_file_path, start_offset, length, sha256_dest, &crc16, &lfs_err
    );
    if (checksum_err != BLOB_ERR_OK) {
        snprintf(
            response_output_buf, response_output_buf_len,
            "%s error: checksumming '%s' (offset=%lu, length=%lu) failed, LFS err=%ld (%s).",
            BLOB_NAME, src_file_path, start_offset, length, lfs_err,
            bulk_downlink_start_blob_error_to_str(checksum_err)
        );
        return checksum_err;
    }

    // Convert the SHA256 hash to a lowercase hex string.
    char hex_hash_str[100]; // Should be 64 chars.
    GEN_byte_array_to_lower_hex_str(sha256_dest, sizeof(sha256_dest), hex_hash_str, sizeof(hex_hash_str));

    // Format like JSON.
    snprintf(
        response_output_buf, response_output_buf_len,
        "{\"action\":\"%s\",\"file\":\"%s\",\"file_size\":%ld,\"offset\":%lu,\"length\":%lu,"
        "\"%s_sha256\":\"%s\",\"%s_crc16\":\"0x%04x\"}",
        BLOB_NAME,
        src_file_path,
        file_size_bytes,
        start_offset, length,
        checksum_prefix, hex_hash_str,
        checksum_prefix, crc16
    );
    return BLOB_ERR_OK;
}


__attribute__((used, section(".text.entry")))
uint8_t blob_main(
    const char *args_str,
    char *response_buf, unsigned short response_buf_len
) {
    // Log that the blob is starting (important for tracing crashes).
    LOG(
        LOG_SEVERITY_NORMAL,
        "Blob (%s) args_str: '%s'",
        BLOB_NAME,
        args_str
    );

    const uint16_t args_str_len = strlen(args_str);
    uint16_t pos = 0;

    char arg0_file_path[LFS_MAX_PATH_LENGTH];
    char arg1_start_offset[20];
    char arg2_byte_count[20];

    pos = parse_token(args_str, pos, args_str_len, arg0_file_path,    sizeof(arg0_file_path));
    pos = parse_token(args_str, pos, args_str_len, arg1_start_offset, sizeof(arg1_start_offset));
    pos = parse_token(args_str, pos, args_str_len, arg2_byte_count,   sizeof(arg2_byte_count));

    if (arg0_file_path[0] == '\0'
        || arg1_start_offset[0] == '\0'
        || arg2_byte_count[0] == '\0'
    ) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: missing args! Need 3 args. (%s)",
            BLOB_NAME, bulk_downlink_start_blob_error_to_str(BLOB_ERR_MISSING_ARGS)
        );
        return BLOB_ERR_MISSING_ARGS;
    }

    bool arg1_start_offset_ok, arg2_byte_count_ok;
    int32_t start_offset = parse_int(arg1_start_offset, &arg1_start_offset_ok);
    int32_t byte_count   = parse_int(arg2_byte_count,   &arg2_byte_count_ok);

    if (!arg1_start_offset_ok || !arg2_byte_count_ok) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: invalid int args! (%s)",
            BLOB_NAME, bulk_downlink_start_blob_error_to_str(BLOB_ERR_INVALID_INT_ARG)
        );
        return BLOB_ERR_INVALID_INT_ARG;
    }

    // Fills the response with its own error message on failure.
    const COMMS_bulk_downlink_start_blob_error_enum_t resp_err = fill_response_output_buffer(
        arg0_file_path, start_offset, byte_count,
        response_buf, response_buf_len
    );
    if (resp_err != BLOB_ERR_OK) {
        return resp_err;
    }
    
    int32_t comms_err = 0;
    const COMMS_bulk_downlink_start_blob_error_enum_t start_bulk_err = bulk_downlink_start_fixed(
        arg0_file_path, start_offset, byte_count, &comms_err
    );
    if (start_bulk_err != BLOB_ERR_OK) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: '%s;%ld;%ld' failed in bulk_downlink_start_fixed() -> %ld (%s).",
            BLOB_NAME,
            arg0_file_path, start_offset, byte_count,
            comms_err, bulk_downlink_start_blob_error_to_str(start_bulk_err)
        );
        return start_bulk_err;
    }

    return BLOB_ERR_OK;
}
