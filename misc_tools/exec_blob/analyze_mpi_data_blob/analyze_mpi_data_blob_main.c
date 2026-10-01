// This is a blob (executable) that analyzes an MPI science data file in LFS, and reports a summary
// of its contents: its SHA256, size, number of MPI frames, number of time syncs, and the earliest
// and latest time sync dates.
//
// Motivation: MPI data files are large, and downlinking them is slow. Analyzing them on-orbit
// lets us decide which files are worth downlinking (and confirms that a recording worked).
//
// MPI Data File Format (written by `TASK_service_write_mpi_data()`):
// The file is a sequence of [MPI buffer][time sync JSON] pairs. Each MPI buffer contains many
// 160-byte MPI frames, each starting with the sync word 0x0C 0xFF 0xFF 0x0C. Each time sync is an
// ASCII JSON object written after the buffer, like:
// {"uptime_ms":123,"timestamp":"1719169299720+0000042000_N","datetime":"2026-07-01T123456.789Z_G","timestamp_ms":1782909296789}
//
// Args Format: <file_path>
//
// Response Format (JSON):
// {"action":"analyze_mpi_data_v1","file":"<path>","sha256":"<hex>","size":1234,
//  "frame_count":N,"time_sync_count":N,"malformed_time_sync_count":N,
//  "earliest":{"timestamp_ms":123,"datetime":"..."},"latest":{"timestamp_ms":123,"datetime":"..."}}
//
// The "earliest" and "latest" fields are the time syncs with the smallest and largest
// "timestamp_ms" (not necessarily the first and last in the file). They are `null` if there are
// no valid time syncs.
//
// Usage Example:
// After uplinking the blob as "blobs/analyze_mpi_data_v1.blob", run:
// CTS1+exec_blob_from_fs(blobs/analyze_mpi_data_v1.blob,0,mpi_data/your_file.bin)!
//
// Notes:
// 1. The frame count is the number of sync words (0x0C 0xFF 0xFF 0x0C) in the file.
// 2. A time sync is "malformed" if it starts with `{"uptime_ms":` but is too long, or is missing
//     a valid "timestamp_ms" field.


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
static const char *BLOB_NAME = "analyze_mpi_data_v1";

/// @brief The 4 sync bytes at the start of each MPI frame (0x0C 0xFF 0xFF 0x0C), as a big-endian
///     uint32 to compare against a rolling window of the last 4 bytes read.
static const uint32_t MPI_FRAME_SYNC_WORD = 0x0CFFFF0C;

/// @brief Start of each time sync JSON object written between MPI buffers.
static const char TIME_SYNC_PREFIX[] = "{\"uptime_ms\":";
#define TIME_SYNC_PREFIX_LEN (sizeof(TIME_SYNC_PREFIX) - 1)

/// @brief Max length of a time sync JSON object (the firmware's buffer is 200 bytes).
#define TIME_SYNC_MAX_LEN 200

/// @brief Max length of a "datetime" string kept from a time sync (e.g., "2026-07-01T123456.789Z_G").
#define DATETIME_STR_MAX_LEN 40

/// @brief Max length of a uint64 in decimal (20 digits), plus a null terminator.
#define UINT64_STR_MAX_LEN 21

#define READ_BUFFER_SIZE 512

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

/// @brief Summary of an MPI data file.
typedef struct {
    uint8_t sha256[32];
    uint32_t size;
    uint32_t frame_count;
    uint32_t time_sync_count;
    uint32_t malformed_time_sync_count;
    time_sync_t earliest;
    time_sync_t latest;
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

/// @brief Handle a complete time sync JSON object found in the file.
static void process_time_sync(const char *json, mpi_file_stats_t *stats) {
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
}

/// @brief Read the file once, computing the SHA256, counting frames, and parsing time syncs.
/// @param file_path Path of the file to read.
/// @param[in,out] stats `size` must be set before calling. All other fields are filled in.
/// @return 0 on success, negative LFS error code on read error, 1 if the file size changed.
static int32_t scan_file(const char *file_path, mpi_file_stats_t *stats) {
    uint8_t read_buffer[READ_BUFFER_SIZE];

    // Time sync currently being captured (only valid while `time_sync_len > 0`).
    char time_sync_buf[TIME_SYNC_MAX_LEN + 1];
    uint16_t time_sync_len = 0;
    uint8_t prefix_match_len = 0; // Number of bytes of TIME_SYNC_PREFIX matched so far.

    stats->frame_count = 0;
    stats->time_sync_count = 0;
    stats->malformed_time_sync_count = 0;

    lfs_file_t file;
    const int32_t open_result = lfs_file_open(&LFS_filesystem, &file, file_path, LFS_O_RDONLY);
    if (open_result < 0) {
        return open_result;
    }

    SHA256_CTX sha256_ctx;
    sha256_init(&sha256_ctx);

    uint32_t file_pos = 0;
    uint32_t sync_window = 0; // Last 4 bytes read, big-endian.

    while (1) {
        const int32_t bytes_read = lfs_file_read(&LFS_filesystem, &file, read_buffer, READ_BUFFER_SIZE);
        if (bytes_read < 0) {
            lfs_file_close(&LFS_filesystem, &file);
            return bytes_read;
        }
        if (bytes_read == 0) break; // EOF.

        sha256_update(&sha256_ctx, read_buffer, bytes_read);
        file_pos += bytes_read;

        for (int32_t i = 0; i < bytes_read; i++) {
            const uint8_t byte = read_buffer[i];

            // Count frames by their sync word.
            sync_window = (sync_window << 8) | byte;
            if (sync_window == MPI_FRAME_SYNC_WORD) {
                stats->frame_count++;
            }

            // Capture the rest of a time sync, after its prefix has been matched.
            if (time_sync_len > 0) {
                if (time_sync_len >= TIME_SYNC_MAX_LEN) {
                    // Too long; probably binary data that happened to match the prefix.
                    stats->malformed_time_sync_count++;
                    time_sync_len = 0;
                    continue;
                }
                time_sync_buf[time_sync_len++] = byte;
                if (byte == '}') {
                    time_sync_buf[time_sync_len] = '\0';
                    process_time_sync(time_sync_buf, stats);
                    time_sync_len = 0;
                }
                continue;
            }

            // Look for the start of a time sync. The prefix's first char ('{') appears only once
            // in it, so on a mismatch, the match can only restart at this byte.
            if (byte == (uint8_t)TIME_SYNC_PREFIX[prefix_match_len]) {
                prefix_match_len++;
            }
            else {
                prefix_match_len = (byte == (uint8_t)TIME_SYNC_PREFIX[0]) ? 1 : 0;
            }
            if (prefix_match_len == TIME_SYNC_PREFIX_LEN) {
                for (uint8_t j = 0; j < TIME_SYNC_PREFIX_LEN; j++) {
                    time_sync_buf[j] = TIME_SYNC_PREFIX[j];
                }
                time_sync_len = TIME_SYNC_PREFIX_LEN;
                prefix_match_len = 0;
            }
        }
    }

    // A time sync cut off by the end of the file.
    if (time_sync_len > 0) {
        stats->malformed_time_sync_count++;
    }

    const int32_t close_result = lfs_file_close(&LFS_filesystem, &file);
    if (close_result < 0) {
        return close_result;
    }

    if (file_pos != stats->size) {
        return 1; // File changed size while reading.
    }

    sha256_final(&sha256_ctx, stats->sha256);
    return 0;
}


/// @brief Main operation in this blob. Fills the response buffer with the analysis JSON.
/// @return 0 on success, non-zero error code on failure (with an error message in the response).
static uint8_t analyze_mpi_data(
    const char *file_path,
    char *response_buf, uint16_t response_buf_len
) {
    // Stored on the stack (not a global). ~200 bytes.
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

    const int32_t scan_result = scan_file(file_path, &stats);
    if (scan_result != 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: file scan failed (err=%ld)",
            BLOB_NAME, scan_result
        );
        return 22;
    }

    response_writer_t w = { .buf = response_buf, .size = response_buf_len, .pos = 0 };
    response_append(&w, "{\"action\":\"%s\",\"file\":\"%s\",\"sha256\":\"", BLOB_NAME, file_path);
    append_hex(&w, stats.sha256, sizeof(stats.sha256));
    response_append(
        &w,
        "\",\"size\":%lu,\"frame_count\":%lu,\"time_sync_count\":%lu,\"malformed_time_sync_count\":%lu",
        stats.size, stats.frame_count, stats.time_sync_count, stats.malformed_time_sync_count
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

    if (pos < args_str_len) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: unexpected args after file path: '%s'",
            BLOB_NAME, &args_str[pos]
        );
        return 136;
    }

    return analyze_mpi_data(arg0_file_path, response_buf, response_buf_len);
}
