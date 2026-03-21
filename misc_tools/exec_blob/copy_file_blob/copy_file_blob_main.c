#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "lfs.h"

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


// Global variables in ELF.
static lfs_t * const LFS_filesystem_ptr = (lfs_t *)(0x20028064UL); // LFS_filesystem in ELF.

// TIME_uptime_ms_from_tim6 in ELF. Immutable pointer to a volatile variable.
static volatile uint32_t * const TIME_uptime_ms_from_tim6_ptr = (volatile uint32_t *)(0x200706d0UL);

typedef int (*snprintf_fn_t)(char *buf, unsigned int size, const char *fmt, ...);

typedef void (*LOG_message_fn_t)(
    uint32_t source, LOG_severity_enum_t severity, uint32_t sink_mask,
    const char *fmt, ...
);

// arm-none-eabi-nm -n CTS-SAT-1_FW_rc2.elf  | grep snprintf
// Below: Address from nm command above, but you MUST add 1 to make it odd.
#define FW_SNPRINTF ((snprintf_fn_t) (0x0802ff2cUL | 0x1))
#define FW_LOG_MESSAGE ((LOG_message_fn_t) (0x08015360UL | 0x1))

static int (*const fw_lfs_file_open)(
    lfs_t *lfs, lfs_file_t *file,
    const char *path, int flags
) = (void*)(0x08012f30UL | 0x1);

static lfs_soff_t (*const fw_lfs_file_size)(lfs_t *lfs, lfs_file_t *file) = (void*)(0x08012f70UL | 0x1);

static lfs_soff_t (*const fw_lfs_file_seek)(
    lfs_t *lfs, lfs_file_t *file,
    lfs_soff_t off, int whence
) = (void*)(0x08012f68UL | 0x1);

static lfs_ssize_t (*const fw_lfs_file_read)(
    lfs_t *lfs, lfs_file_t *file,
    void *buffer, lfs_size_t size
) = (void*)(0x08012f58UL | 0x1);

static lfs_ssize_t (*const fw_lfs_file_write)(
    lfs_t *lfs, lfs_file_t *file,
    const void *buffer, lfs_size_t size
) = (void*)(0x08012f60UL | 0x1);

static int (*const fw_lfs_file_close)(lfs_t *lfs, lfs_file_t *file) = (void*)(0x08012f48UL | 0x1);

#define LOG(severity, fmt, ...) \
    FW_LOG_MESSAGE(LOG_SYSTEM_TELECOMMAND, severity, LOG_SINK_ALL, fmt, ##__VA_ARGS__)

void *memset(void *s, int c, __SIZE_TYPE__ n) {
    uint8_t *p = s;
    while (n--) *p++ = (uint8_t)c;
    return s;
}


static inline uint32_t TIME_uptime_ms() {
    return *TIME_uptime_ms_from_tim6_ptr;
}



static uint16_t parse_token(
    const char *src, uint16_t src_offset, uint16_t src_len,
    char *dst, uint16_t dst_size
) {
    uint16_t di = 0;
    uint16_t i  = src_offset;

    // Skip leading spaces
    while (i < src_len && src[i] == ' ') i++;

    // Copy until next space or end
    while (i < src_len && src[i] != ' ' && di < dst_size - 1) {
        dst[di++] = src[i++];
    }
    dst[di] = '\0';

    // Return index just past the token
    return i;
}

static uint16_t str_len(const char *s) {
    uint16_t n = 0;
    while (s[n]) n++;
    return n;
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

static int8_t copy_lfs_file_chunk(
    const char *src_file_path, const char *dest_file_path,
    uint32_t start_offset, uint32_t byte_count
) {
    // Open files.
    int err;
    lfs_file_t in_file;
    if ((err=fw_lfs_file_open(LFS_filesystem_ptr, &in_file, src_file_path, LFS_O_RDONLY)) < 0) {
        LOG(LOG_SEVERITY_ERROR, "Failed to open source file: '%s' (%d)", src_file_path, err);
        return err;
    }

    lfs_file_t out_file;
    if ((err=fw_lfs_file_open(LFS_filesystem_ptr, &out_file, dest_file_path, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC)) < 0) {
        LOG(LOG_SEVERITY_ERROR, "Failed to open destination file: '%s' (%d)", dest_file_path, err);
        fw_lfs_file_close(LFS_filesystem_ptr, &in_file);
        return err;
    }

    // Get the input file size.
    const lfs_soff_t src_file_size = fw_lfs_file_size(LFS_filesystem_ptr, &in_file);
    if (src_file_size < 0) {
        LOG(LOG_SEVERITY_ERROR, "Failed to get size of source file: '%s'", src_file_path);
        fw_lfs_file_close(LFS_filesystem_ptr, &in_file);
        fw_lfs_file_close(LFS_filesystem_ptr, &out_file);
        return err;
    }
    if (start_offset > src_file_size) {
        LOG(
            LOG_SEVERITY_ERROR,
            "Offset is higher than file size (%lu > %lu)",
            start_offset, src_file_size
        );
        fw_lfs_file_close(LFS_filesystem_ptr, &in_file);
        fw_lfs_file_close(LFS_filesystem_ptr, &out_file);
        return 191;
    }
    if (byte_count == 0 || (start_offset + byte_count) > src_file_size) {
        byte_count = src_file_size - start_offset;
    }

    // Seek the input file to the start offset.
    if (start_offset != 0) {
        if ((err=fw_lfs_file_seek(LFS_filesystem_ptr, &in_file, start_offset, LFS_SEEK_SET)) < 0) {
            fw_lfs_file_close(LFS_filesystem_ptr, &in_file);
            fw_lfs_file_close(LFS_filesystem_ptr, &out_file);
            return err;
        }
    }

    // Copy the file.
    uint8_t transfer_buffer[1024] = {0};
    int32_t bytes_left = byte_count;
    while (bytes_left > 0) {
        const lfs_ssize_t read_size = bytes_left > sizeof(transfer_buffer) ? sizeof(transfer_buffer) : bytes_left;
        if ((err=fw_lfs_file_read(LFS_filesystem_ptr, &in_file, transfer_buffer, read_size)) < 0) {
            fw_lfs_file_close(LFS_filesystem_ptr, &in_file);
            fw_lfs_file_close(LFS_filesystem_ptr, &out_file);
            return err;
        }
        if ((err=fw_lfs_file_write(LFS_filesystem_ptr, &out_file, transfer_buffer, read_size)) < 0) {
            fw_lfs_file_close(LFS_filesystem_ptr, &in_file);
            fw_lfs_file_close(LFS_filesystem_ptr, &out_file);
            return err;
        }
        bytes_left -= read_size;
    }

    // Close files.
    if ((err=fw_lfs_file_close(LFS_filesystem_ptr, &in_file)) < 0) {
        fw_lfs_file_close(LFS_filesystem_ptr, &out_file);
        return err;
    }
    if ((err=fw_lfs_file_close(LFS_filesystem_ptr, &out_file)) < 0) {
        return err;
    }

    return 0; // Success.
}


__attribute__((used, section(".text.entry")))
uint8_t blob_main(
    const char *args_str,
    char *response_buf, unsigned short response_buf_len
) {
    const uint32_t start_time_ms = TIME_uptime_ms();
    LOG(
        LOG_SEVERITY_NORMAL,
        "Blob args_str: '%s'",
        args_str
    );

    const uint16_t args_str_len = str_len(args_str);
    uint16_t pos = 0;

    char arg0_in_path[LFS_MAX_PATH_LENGTH]  = {0};
    char arg1_out_path[LFS_MAX_PATH_LENGTH] = {0};
    char arg2_start_offset[20]              = {0};
    char arg3_byte_count[20]                = {0};

    pos = parse_token(args_str, pos, args_str_len, arg0_in_path,      sizeof(arg0_in_path));
    pos = parse_token(args_str, pos, args_str_len, arg1_out_path,     sizeof(arg1_out_path));
    pos = parse_token(args_str, pos, args_str_len, arg2_start_offset, sizeof(arg2_start_offset));
    pos = parse_token(args_str, pos, args_str_len, arg3_byte_count,   sizeof(arg3_byte_count));

    if (arg0_in_path[0] == '\0' || arg1_out_path[0] == '\0' ||
        arg2_start_offset[0] == '\0' || arg3_byte_count[0] == '\0'
    ) {
        // Missing args error.
        return 135;
    }

    bool arg2_start_offset_ok, arg3_byte_count_ok;
    int32_t start_offset = parse_int(arg2_start_offset, &arg2_start_offset_ok);
    int32_t byte_count   = parse_int(arg3_byte_count,   &arg3_byte_count_ok);

    if (!arg2_start_offset_ok || !arg3_byte_count_ok) {
        return 136;
    }
    
    const int8_t err = copy_lfs_file_chunk(
        arg0_in_path, arg1_out_path, start_offset, byte_count
    );
    if (err != 0) {
        FW_SNPRINTF(
            response_buf, response_buf_len,
            "Running 'copy_file_blob %s %s %d %d' failed. Error: %d. Runtime: %d ms.",
            arg0_in_path, arg1_out_path, start_offset, byte_count,
            err,
            TIME_uptime_ms() - start_time_ms
        );
        return 1;
    }

    FW_SNPRINTF(
        response_buf, response_buf_len,
        "Running 'copy_file_blob %s %s %d %d' succeeded. Runtime: %d ms.",
        arg0_in_path, arg1_out_path, start_offset, byte_count,
        TIME_uptime_ms() - start_time_ms
    );
    return 0;
}
