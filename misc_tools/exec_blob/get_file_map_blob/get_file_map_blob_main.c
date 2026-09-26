// This is a blob (executable) that reports a "map" of a file in LFS: its SHA256, CRC16, size,
// the ranges of long runs of null (0x00) bytes, and a CRC16 of each fixed-size chunk.
//
// Motivation: When a bulk downlink (or uplink) of a large file is missing packets, it's hard to
// tell which parts of the file on the ground are wrong. Comparing the per-chunk CRC16s against
// the ground copy narrows it down to a chunk, and the null ranges show which parts of the file
// are empty (e.g., never written) without downlinking them.
//
// Args Format: <file_path>  or  <file_path>;kwarg1=val;kwarg2=val
// Supported kwargs:
//  - minimum_null_length: Minimum length of a run of 0x00 bytes to be reported. Default: 40.
//  - crc16_chunk_size: Size, in bytes, of each chunk in the crc16_map. Default: file size split
//      into 16 chunks (rounded up). Max of 64 chunks.
//
// Response Format (JSON):
// {"action":"get_file_map_v1","file":"<path>","sha256":"<hex>","crc16":"0xabcd","size":1234,
//  "null_ranges":[[start,end],...],"crc16_map":{"start":"0xabcd",...},
//  "null_range_count":N,"null_ranges_truncated":false}
//
// Null ranges are [start, end) in bytes, with an exclusive end (like a Python slice), so
// `data[start:end]` is the range. Each crc16_map key is a chunk's start offset; the chunk runs
// until the next key's offset (or the end of the file). If the null ranges don't fit in the response buffer, the list
// is cut short and "null_ranges_truncated" is true; "null_range_count" is always the full count.
//
// Usage Example:
// After uplinking the blob as "blobs/get_file_map_v1.blob", run:
// CTS1+exec_blob_from_fs(blobs/get_file_map_v1.blob,0,your_file.bin)!
// CTS1+exec_blob_from_fs(blobs/get_file_map_v1.blob,0,your_file.bin;minimum_null_length=100;crc16_chunk_size=4096)!
//
// Notes:
// 1. Null ranges are `[start, end)` with an exclusive end, like a Python slice (`data[start:end]`).
//     Each `crc16_map` key is a chunk's start offset; the chunk runs until the next key's offset (or the end of the file).
// 2. The CRC16 is the same algorithm as the ADCS file CRC16 (ADCS Firmware ICD `CRC_Calc()`).
// 3. If the null ranges don't fit in the response, the list is cut short and `null_ranges_truncated` is `true`. `null_range_count` is always the full count. Re-run with a larger `minimum_null_length` to see them all.


#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "../lfs.h"
#include "crypto/sha256.h"

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
static const char KWARG_DELIM = '=';
static const char *BLOB_NAME = "get_file_map_v1";

static const uint32_t DEFAULT_MINIMUM_NULL_LENGTH = 40;
static const uint32_t DEFAULT_CRC16_CHUNK_COUNT = 16;
#define MAX_CRC16_CHUNK_COUNT 64

#define READ_BUFFER_SIZE 512

// Space kept free at the end of the response for the trailing keys after crc16_map.
static const uint16_t RESPONSE_TAIL_RESERVE_BYTES = 96;

// Global variables defined in the firmware ELF (CTS-SAT-1_FW_rc3.elf).
extern lfs_t LFS_filesystem;

extern int snprintf(char *buf, unsigned int size, const char *fmt, ...);
extern int vsnprintf(char *buf, unsigned int size, const char *fmt, va_list args);
extern int strlen (const char *s);

extern void LOG_message(
    uint32_t source, LOG_severity_enum_t severity, uint32_t sink_mask,
    const char *fmt, ...
);

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

static bool str_equal(const char *a, const char *b) {
    while (*a != '\0' && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
}

/// @brief Parse a "key=value" token into a key and a positive integer value.
/// @param token The token. Modified in place (the '=' is replaced with a null terminator).
/// @param[out] key_out Set to point at the key within `token`.
/// @param[out] value_out Set to the parsed value.
/// @return true on success, false if there's no '=' or the value isn't a positive integer.
static bool parse_kwarg(char *token, const char **key_out, uint32_t *value_out) {
    uint16_t i = 0;
    while (token[i] != '\0' && token[i] != KWARG_DELIM) i++;
    if (token[i] != KWARG_DELIM) return false;

    token[i] = '\0';
    *key_out = token;

    bool ok;
    const int32_t value = parse_int(&token[i + 1], &ok);
    if (!ok || value <= 0) return false;
    *value_out = (uint32_t)value;
    return true;
}

/// @brief Update a CRC16 with one byte.
/// @note The per-byte update rule matches `CRC_Calc()` from the ADCS Firmware ICD (Section 6.4.1).
static inline uint16_t crc16_update(uint16_t crc, uint8_t byte) {
    crc = (crc >> 8) | (crc << 8);      // byte-swap the CRC
    crc ^= byte;                         // XOR in the next byte
    crc ^= (crc & 0xff) >> 4;            // mix low nibble down
    crc ^= crc << 12;                    // mix up by 12 bits
    crc ^= (crc & 0xff) << 5;            // mix low byte up by 5 bits
    return crc;
}

/// @brief Checksums and chunk layout of the file.
typedef struct {
    uint8_t sha256[32];
    uint16_t crc16;
    uint32_t size;
    uint32_t chunk_size;
    uint16_t chunk_count;
    uint16_t chunk_crc16s[MAX_CRC16_CHUNK_COUNT];
} file_checksums_t;

/// @brief Appends to a response buffer, tracking the position and whether it overflowed.
typedef struct {
    char *buf;
    uint16_t size;
    uint16_t pos;
    bool overflow;
} response_writer_t;

static void response_append(response_writer_t *w, const char *fmt, ...) {
    if (w->overflow) return;

    va_list args;
    va_start(args, fmt);
    const int written = vsnprintf(&w->buf[w->pos], w->size - w->pos, fmt, args);
    va_end(args);

    if (written < 0 || written >= (w->size - w->pos)) {
        w->overflow = true;
        w->buf[w->pos] = '\0'; // Drop the partial write.
        return;
    }
    w->pos += written;
}

/// @brief Overwrite `byte_count` bytes' worth of lowercase hex in place (no null terminator).
/// @note Used to fill in placeholders once the checksums are known.
static void write_hex_in_place(char *dest, const uint8_t *bytes, uint8_t byte_count) {
    static const char HEX_DIGITS[] = "0123456789abcdef";
    for (uint8_t i = 0; i < byte_count; i++) {
        dest[i * 2] = HEX_DIGITS[bytes[i] >> 4];
        dest[i * 2 + 1] = HEX_DIGITS[bytes[i] & 0x0f];
    }
}

/// @brief Appends the crc16_map JSON object, keyed by chunk start offset
///     (e.g., `{"0":"0x1234","100":"0xabcd"}`).
static void append_crc16_map(response_writer_t *w, const file_checksums_t *checksums) {
    response_append(w, "{");
    for (uint16_t i = 0; i < checksums->chunk_count; i++) {
        const uint32_t start = i * checksums->chunk_size;

        response_append(
            w, "%s\"%lu\":\"0x%04x\"",
            (i == 0) ? "" : ",",
            start, checksums->chunk_crc16s[i]
        );
    }
    response_append(w, "}");
}

/// @brief Read the file once, computing the SHA256, the whole-file CRC16, each chunk's CRC16,
///     and appending runs of 0x00 bytes to the response as a JSON list of [start,end].
/// @param file_path Path of the file to read.
/// @param minimum_null_length Minimum null run length to report.
/// @param[in,out] checksums `size`, `chunk_size`, and `chunk_count` must be set before calling.
/// @param w The response to append the null ranges list to.
/// @param reserve_bytes Bytes to leave free in the response for everything after the list.
/// @param[out] null_range_count Set to the total number of null ranges (including unlisted ones).
/// @param[out] truncated Set to true if some null ranges didn't fit in the response.
/// @return 0 on success, negative LFS error code on read error, 1 if the file size changed.
static int32_t scan_file(
    const char *file_path, uint32_t minimum_null_length,
    file_checksums_t *checksums,
    response_writer_t *w, uint16_t reserve_bytes,
    uint32_t *null_range_count, bool *truncated
) {
    uint8_t read_buffer[READ_BUFFER_SIZE];

    *null_range_count = 0;
    *truncated = false;

    lfs_file_t file;
    const int32_t open_result = lfs_file_open(&LFS_filesystem, &file, file_path, LFS_O_RDONLY);
    if (open_result < 0) {
        return open_result;
    }

    SHA256_CTX sha256_ctx;
    sha256_init(&sha256_ctx);

    uint16_t file_crc = 0;
    uint16_t chunk_crc = 0;
    uint16_t chunk_idx = 0;
    uint32_t pos_in_chunk = 0;

    uint32_t file_pos = 0;
    uint32_t run_start = 0;
    uint32_t run_len = 0;
    bool eof = false;

    response_append(w, "[");

    while (!eof) {
        const int32_t bytes_read = lfs_file_read(&LFS_filesystem, &file, read_buffer, READ_BUFFER_SIZE);
        if (bytes_read < 0) {
            lfs_file_close(&LFS_filesystem, &file);
            return bytes_read;
        }
        eof = (bytes_read == 0);

        if (!eof) {
            sha256_update(&sha256_ctx, read_buffer, bytes_read);
        }

        // At EOF, loop once more with a virtual non-null byte to close out any trailing run.
        const int32_t loop_len = eof ? 1 : bytes_read;
        for (int32_t i = 0; i < loop_len; i++) {
            if (!eof) {
                file_crc = crc16_update(file_crc, read_buffer[i]);
                chunk_crc = crc16_update(chunk_crc, read_buffer[i]);
                pos_in_chunk++;

                if (pos_in_chunk == checksums->chunk_size) {
                    if (chunk_idx < MAX_CRC16_CHUNK_COUNT) {
                        checksums->chunk_crc16s[chunk_idx] = chunk_crc;
                    }
                    chunk_idx++;
                    chunk_crc = 0;
                    pos_in_chunk = 0;
                }

                if (read_buffer[i] == 0x00) {
                    if (run_len == 0) run_start = file_pos;
                    run_len++;
                    file_pos++;
                    continue;
                }
            }

            if (run_len >= minimum_null_length) {
                char range_str[32];
                const int range_str_len = snprintf(
                    range_str, sizeof(range_str), "%s[%lu,%lu]",
                    (*null_range_count == 0) ? "" : ",",
                    run_start, run_start + run_len
                );

                if (!*truncated && (w->pos + range_str_len + reserve_bytes < w->size)) {
                    response_append(w, "%s", range_str);
                }
                else {
                    *truncated = true;
                }
                (*null_range_count)++;
            }
            run_len = 0;
            if (!eof) file_pos++;
        }
    }

    response_append(w, "]");

    // Final partial chunk.
    if (pos_in_chunk > 0) {
        if (chunk_idx < MAX_CRC16_CHUNK_COUNT) {
            checksums->chunk_crc16s[chunk_idx] = chunk_crc;
        }
        chunk_idx++;
    }

    const int32_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0) {
        return close_result;
    }

    if (file_pos != checksums->size || chunk_idx != checksums->chunk_count) {
        return 1; // File changed size while reading.
    }

    sha256_final(&sha256_ctx, checksums->sha256);
    checksums->crc16 = file_crc;
    return 0;
}


/// @brief Main operation in this blob. Fills the response buffer with the file map JSON.
/// @return 0 on success, non-zero error code on failure (with an error message in the response).
/// @details The file is only read once, but the sha256 and crc16 come before the null ranges in
///     the JSON. Since the file size is known up front and both checksums are fixed-width, the
///     header is written first with placeholders, and the placeholders are overwritten after the
///     scan. Likewise, the crc16_map's length is known before the scan, so the null ranges know
///     how much room to leave for it.
static uint8_t get_file_map(
    const char *file_path, uint32_t minimum_null_length, uint32_t crc16_chunk_size,
    char *response_buf, uint16_t response_buf_len
) {
    // Stored on the stack (not a global). ~180 bytes.
    file_checksums_t checksums;

    const lfs_ssize_t file_size = LFS_file_size(file_path, 1);
    if (file_size < 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: can't get size of '%s' (err=%ld)",
            BLOB_NAME, file_path, file_size
        );
        return 20;
    }
    checksums.size = (uint32_t)file_size;

    // Default: split the file into 16 chunks (rounded up).
    if (crc16_chunk_size == 0) {
        crc16_chunk_size = (checksums.size + DEFAULT_CRC16_CHUNK_COUNT - 1) / DEFAULT_CRC16_CHUNK_COUNT;
        if (crc16_chunk_size == 0) crc16_chunk_size = 1; // Empty file.
    }
    checksums.chunk_size = crc16_chunk_size;

    const uint32_t chunk_count = (checksums.size + crc16_chunk_size - 1) / crc16_chunk_size;
    if (chunk_count > MAX_CRC16_CHUNK_COUNT) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: crc16_chunk_size=%lu gives %lu chunks (max %d)",
            BLOB_NAME, crc16_chunk_size, chunk_count, MAX_CRC16_CHUNK_COUNT
        );
        return 21;
    }
    checksums.chunk_count = (uint16_t)chunk_count;
    for (uint16_t i = 0; i < MAX_CRC16_CHUNK_COUNT; i++) {
        checksums.chunk_crc16s[i] = 0;
    }

    // Measure the crc16_map's length (values are fixed-width, so zeros give the real length).
    response_writer_t w = { .buf = response_buf, .size = response_buf_len, .pos = 0, .overflow = false };
    append_crc16_map(&w, &checksums);
    if (w.overflow) {
        snprintf(response_buf, response_buf_len, "%s error: crc16_map too long for response", BLOB_NAME);
        return 23;
    }
    const uint16_t crc16_map_len = w.pos;

    // Write the header, with placeholders for the checksums.
    w.pos = 0;
    response_append(&w, "{\"action\":\"%s\",\"file\":\"%s\",\"sha256\":\"", BLOB_NAME, file_path);
    const uint16_t sha256_pos = w.pos;
    response_append(&w, "%064d\",\"crc16\":\"0x", 0);
    const uint16_t crc16_pos = w.pos;
    response_append(&w, "0000\",\"size\":%lu,\"null_ranges\":", checksums.size);

    // Scan the file, appending the null ranges.
    uint32_t null_range_count;
    bool null_ranges_truncated;
    const int32_t scan_result = scan_file(
        file_path, minimum_null_length, &checksums,
        &w, crc16_map_len + RESPONSE_TAIL_RESERVE_BYTES,
        &null_range_count, &null_ranges_truncated
    );
    if (scan_result != 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: file scan failed (err=%ld)",
            BLOB_NAME, scan_result
        );
        return 22;
    }

    response_append(&w, ",\"crc16_map\":");
    append_crc16_map(&w, &checksums);
    response_append(
        &w, ",\"null_range_count\":%lu,\"null_ranges_truncated\":%s}",
        null_range_count, null_ranges_truncated ? "true" : "false"
    );

    if (w.overflow) {
        // Should never happen (space is reserved), but don't send broken JSON silently.
        snprintf(response_buf, response_buf_len, "%s error: response overflow", BLOB_NAME);
        return 24;
    }

    // Fill in the placeholders.
    write_hex_in_place(&response_buf[sha256_pos], checksums.sha256, 32);
    const uint8_t crc16_bytes[2] = { checksums.crc16 >> 8, checksums.crc16 & 0xff };
    write_hex_in_place(&response_buf[crc16_pos], crc16_bytes, 2);

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
    pos = parse_token(args_str, pos, args_str_len, arg0_file_path, sizeof(arg0_file_path));

    if (arg0_file_path[0] == '\0') {
        snprintf(
            response_buf, response_buf_len,
            "%s error: missing file path arg!",
            BLOB_NAME
        );
        return 135;
    }

    uint32_t minimum_null_length = DEFAULT_MINIMUM_NULL_LENGTH;
    uint32_t crc16_chunk_size = 0; // 0 = default (split into DEFAULT_CRC16_CHUNK_COUNT chunks).

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
                "%s error: invalid kwarg '%s' (need key=positive_int)",
                BLOB_NAME, kwarg_token
            );
            return 136;
        }

        if (str_equal(key, "minimum_null_length")) {
            minimum_null_length = value;
        }
        else if (str_equal(key, "crc16_chunk_size")) {
            crc16_chunk_size = value;
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

    return get_file_map(
        arg0_file_path, minimum_null_length, crc16_chunk_size,
        response_buf, response_buf_len
    );
}
