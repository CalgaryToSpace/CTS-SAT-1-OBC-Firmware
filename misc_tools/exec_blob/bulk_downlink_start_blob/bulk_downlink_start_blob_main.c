
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
// Usage Example:
// After uplinking the blob as "blobs/bulk_downlink_start_v2.blob", run:
// CTS1+exec_blob_from_fs(blobs/bulk_downlink_start_v2.bin,0,your_file.run;0;0)!

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "../lfs.h"

#define LFS_MAX_PATH_LENGTH 200


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
static const char *BLOB_NAME = "bulk_downlink_start_blob";

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


/// @brief Fill the response output buffer with info about the file (very helpful for decoding).
/// @note This is a totally new feature, independent from the original bulk downlink telecommand.
///     It is observed as a helpful feature for re-assembling the downlinked files during mission ops.
/// @param src_file_path 
/// @param start_offset 
/// @param max_length Max length of the file to read, or 0 to read full file.
/// @param response_output_buf 
/// @param response_output_buf_len 
/// @return 0 on success, non-zero on error.
static int8_t fill_response_output_buffer(
    const char *src_file_path,
    uint32_t start_offset, uint32_t max_length,
    char *response_output_buf, uint16_t response_output_buf_len
) {
    // Prepare the SHA256 destination buffer.
    uint8_t sha256_dest[32] = {0}; // 32 bytes for SHA256

    // Calculate the SHA256 hash of the file.
    const int8_t sha256_result = LFS_read_file_checksum_sha256(
        src_file_path, start_offset, max_length, sha256_dest
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
    const uint32_t real_length_hashed = (max_length == 0 || max_length >= (uint32_t)file_size_bytes) ? (uint32_t)file_size_bytes : max_length;

    // Convert the SHA256 hash to a little-endian hex string.
    char hex_hash_str[100]; // Should be 64 chars.
    GEN_byte_array_to_hex_str(sha256_dest, sizeof(sha256_dest), hex_hash_str, sizeof(hex_hash_str));

    // Format like JSON.
    snprintf(
        response_output_buf, response_output_buf_len,
        "{\"action\":\"%s\",\"file\":\"%s\",\"file_size\":%ld,\"sha256\":\"%s\",\"offset\":%lu,\"length\":%lu}",
        BLOB_NAME,
        src_file_path,
        file_size_bytes,
        hex_hash_str,
        start_offset, real_length_hashed
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
        // Missing args error.
        snprintf(
            response_buf, response_buf_len,
            "%s error: missing args!",
            BLOB_NAME
        );
        return 135;
    }

    bool arg1_start_offset_ok, arg2_byte_count_ok;
    int32_t start_offset = parse_int(arg1_start_offset, &arg1_start_offset_ok);
    int32_t byte_count   = parse_int(arg2_byte_count,   &arg2_byte_count_ok);

    if (!arg1_start_offset_ok || !arg2_byte_count_ok) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: invalid int args!",
            BLOB_NAME
        );
        return 136;
    }

    const int8_t resp_err = fill_response_output_buffer(
        arg0_file_path, start_offset, byte_count,
        response_buf, response_buf_len
    );
    if (resp_err != 0) {
        snprintf(
            response_buf, response_buf_len,
            "Running %s: '%s;%d;%d' failed in checking file size/hash. Error: %d.",
            BLOB_NAME,
            arg0_file_path, start_offset, byte_count,
            resp_err
        );
        return 10;
    }
    
    const int8_t start_bulk_err = bulk_downlink_start_fixed(
        arg0_file_path, start_offset, byte_count
    );
    if (start_bulk_err != 0) {
        snprintf(
            response_buf, response_buf_len,
            "Running %s: '%s;%d;%d' failed in bulk_downlink_start_fixed(). Error: %d.",
            BLOB_NAME,
            arg0_file_path, start_offset, byte_count,
            start_bulk_err
        );
        return 20;
    }

    return 0;
}
