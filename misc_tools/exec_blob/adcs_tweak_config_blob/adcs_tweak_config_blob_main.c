// This is a blob (executable) that changes a single field inside an ADCS configuration frame.
//
// Motivation: Some ADCS configuration frames (e.g., the CubeSense config) have no dedicated
// set/get telecommand in the main firmware, and the whole frame is too long to set by a single
// telecommand. This blob does a read-modify-write of just one field, leaving every other byte as
// the ADCS had it.
//
// Args Format:
//  <command_id_set>;<command_id_get>;<parameters_length_bytes>;<field_to_update_offset_bits>;
//  <field_to_update_length_bits>;<field_to_update_new_value_encoding>;<field_to_update_new_value>
//
//  - command_id_set: ADCS telecommand ID to write the config frame (e.g., 25 = Set CubeSense Config).
//  - command_id_get: ADCS telemetry ID to read the config frame (e.g., 203 = Get CubeSense Config).
//  - parameters_length_bytes: Length of the full config frame, in bytes (1 to 512).
//  - field_to_update_offset_bits: Offset of the field within the frame. Must be a multiple of 8.
//  - field_to_update_length_bits: Length of the field. Must be a multiple of 8.
//  - field_to_update_new_value_encoding: One of:
//      - "int": Decimal or 0x-prefixed integer. Can be negative.
//      - "float": Decimal floating-point number (e.g., "-1.25e-3"). Field length must be 32 bits
//          (float) or 64 bits (double).
//      - "hex": Raw bytes, exactly 2 hex characters per byte of the field, in the order they appear
//          in the frame (i.e., little-endian for ADCS integers). Underscores are ignored. No "0x".
//  - field_to_update_new_value: The new value, in the given encoding.
//
// Description of Blob:
//  1. Reads the full existing config frame from the ADCS (using `command_id_get` arg).
//  2. Logs the full existing config frame as hex (for recovery, if needed).
//  3. Overwrites only the bytes of the target field with the new value.
//  4. Writes the full modified config frame to the ADCS (using `command_id_set` arg).
//  5. Reads the full config frame again, and confirms the target field holds the new value.
//  6. Sends a telecommand response with the old, new, and read-back values of the field.
//
// Notes:
//  1. This blob does NOT save the config to the ADCS's flash (ADCS_COMMAND_SAVE_CONFIG). The change
//      is lost on an ADCS reset unless the config is saved separately.
//
// Usage Example:
// After uplinking the blob as "blobs/adcs_tweak_config_v1.blob", run the following to set the
// CubeSense Cam1 exposure time to value 23:
// CTS1+exec_blob_from_fs(blobs/adcs_tweak_config_v1.blob,0,25;203;112;64;16;int;23)!
//
// The same thing, using the "hex" encoding (23 = 0x0017, which is "1700" little-endian):
// CTS1+exec_blob_from_fs(blobs/adcs_tweak_config_v1.blob,0,25;203;112;64;16;hex;1700)!

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>


typedef enum {
    LOG_SEVERITY_DEBUG = 1 << 0,
    LOG_SEVERITY_NORMAL = 1 << 1,
    LOG_SEVERITY_WARNING = 1 << 2,
    LOG_SEVERITY_ERROR = 1 << 3,
    LOG_SEVERITY_CRITICAL = 1 << 4,
} LOG_severity_enum_t;

static const uint32_t LOG_SYSTEM_ADCS = 1 << 7;
static const uint32_t LOG_SINK_ALL = (1 << 4) - 1;

static const char ARG_DELIM = ';';
static const char *BLOB_NAME = "adcs_tweak_config_v1";

// Largest config frame this blob will handle. The largest ADCS config frame (the full ADCS config,
// telemetry 206) is 504 bytes. The telecommand task stack is 8 KiB, and we hold two frames here
// plus one temporary frame-sized buffer inside the ADCS driver.
#define MAX_PARAMETERS_LENGTH_BYTES 512

// Largest field the "hex" encoding will write.
#define MAX_FIELD_LENGTH_BYTES 64

// Settling time between writing the config and reading it back.
#define SET_TO_VERIFY_DELAY_MS 100

// Bytes of the config frame per log message, when logging it as hex. Must fit within
// LOG_FORMATTED_MESSAGE_MAX_LENGTH (185) along with the prefix.
#define LOG_HEX_CHUNK_BYTES 64

static const uint8_t ADCS_INCLUDE_CHECKSUM = 1;

// Functions defined in the firmware ELF (CTS-SAT-1_FW_rc3.elf).
extern int snprintf(char *buf, unsigned int size, const char *fmt, ...);
extern int strlen (const char *s);
extern double atof(const char *s);

extern void LOG_message(
    uint32_t source, LOG_severity_enum_t severity, uint32_t sink_mask,
    const char *fmt, ...
);

void HAL_Delay(uint32_t Delay);

// Returns 0 on success; 1-4 are the ADCS's TC error flags, 5 is an ACK timeout, others are HAL errors.
uint8_t ADCS_i2c_send_command_and_check(
    uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum
);
// Returns 0 on success; 4 is a checksum failure, others are HAL errors.
uint8_t ADCS_i2c_request_telemetry_and_check(
    uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum
);

#define LOG(severity, fmt, ...) \
    LOG_message(LOG_SYSTEM_ADCS, severity, LOG_SINK_ALL, fmt, ##__VA_ARGS__)

// MARK: Error Enum

// All named status/error codes returned by this blob's functions, including the values
// `blob_main()` itself returns to the telecommand executor (0 = success).
// For ADCS communication failures, the raw ADCS driver status is included in the response string.
typedef enum {
    BLOB_ERR_OK = 0,
    BLOB_ERR_MISSING_ARGS = 10, // Fewer than 7 args were given.
    BLOB_ERR_INVALID_INT_ARG = 11, // One of the numeric args isn't a valid integer.
    BLOB_ERR_INVALID_COMMAND_ID = 12, // A command ID is outside 0-255.
    BLOB_ERR_INVALID_PARAMETERS_LENGTH = 13, // parameters_length_bytes is 0 or too large.
    BLOB_ERR_FIELD_NOT_BYTE_ALIGNED = 14, // Field offset or length isn't a multiple of 8 bits.
    BLOB_ERR_FIELD_OUT_OF_RANGE = 15, // Field is empty, or extends past the end of the frame.
    BLOB_ERR_UNKNOWN_ENCODING = 20, // Encoding isn't "int", "float", or "hex".
    BLOB_ERR_INVALID_FIELD_LENGTH_FOR_ENCODING = 21, // e.g., "float" with a 16-bit field.
    BLOB_ERR_INVALID_VALUE = 22, // The value can't be parsed in the given encoding.
    BLOB_ERR_VALUE_DOES_NOT_FIT = 23, // The "int" value is too large for the field.
    BLOB_ERR_ADCS_GET_FAILED = 30, // Reading the existing config (command_id_get) failed.
    BLOB_ERR_ADCS_SET_FAILED = 31, // Writing the modified config (command_id_set) failed.
    BLOB_ERR_ADCS_VERIFY_GET_FAILED = 32, // Reading the config back (command_id_get) failed.
    BLOB_ERR_VERIFY_MISMATCH = 33, // The read-back field doesn't hold the new value.
} ADCS_tweak_config_blob_error_enum_t;

/// @brief Convert an ADCS_tweak_config_blob_error_enum_t into a short human-readable name, for use
///     in log messages and output strings.
/// @return Static string; never NULL. "UNKNOWN_ERROR" for values not in the enum.
static const char *adcs_tweak_config_blob_error_to_str(ADCS_tweak_config_blob_error_enum_t err) {
    switch (err) {
        case BLOB_ERR_OK: return "OK";
        case BLOB_ERR_MISSING_ARGS: return "MISSING_ARGS";
        case BLOB_ERR_INVALID_INT_ARG: return "INVALID_INT_ARG";
        case BLOB_ERR_INVALID_COMMAND_ID: return "INVALID_COMMAND_ID";
        case BLOB_ERR_INVALID_PARAMETERS_LENGTH: return "INVALID_PARAMETERS_LENGTH";
        case BLOB_ERR_FIELD_NOT_BYTE_ALIGNED: return "FIELD_NOT_BYTE_ALIGNED";
        case BLOB_ERR_FIELD_OUT_OF_RANGE: return "FIELD_OUT_OF_RANGE";
        case BLOB_ERR_UNKNOWN_ENCODING: return "UNKNOWN_ENCODING";
        case BLOB_ERR_INVALID_FIELD_LENGTH_FOR_ENCODING: return "INVALID_FIELD_LENGTH_FOR_ENCODING";
        case BLOB_ERR_INVALID_VALUE: return "INVALID_VALUE";
        case BLOB_ERR_VALUE_DOES_NOT_FIT: return "VALUE_DOES_NOT_FIT";
        case BLOB_ERR_ADCS_GET_FAILED: return "ADCS_GET_FAILED";
        case BLOB_ERR_ADCS_SET_FAILED: return "ADCS_SET_FAILED";
        case BLOB_ERR_ADCS_VERIFY_GET_FAILED: return "ADCS_VERIFY_GET_FAILED";
        case BLOB_ERR_VERIFY_MISMATCH: return "VERIFY_MISMATCH";
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

static bool str_equals(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return *a == *b;
}

/// @brief Parse a string into an unsigned integer, up to 0xFFFFFFFF.
/// @param s String to parse. Valid formats: "0x<digits>" or "<digits>". Underscores are ignored.
/// @param ok Set to true on success, false if invalid or larger than 0xFFFFFFFF.
/// @returns Parsed integer, or 0 if invalid.
static uint32_t parse_uint(const char *s, bool *ok) {
    uint64_t result = 0;
    bool hex = false;
    uint8_t i = 0;

    *ok = false;
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
        if (result > 0xFFFFFFFFULL) return 0; // overflow
        has_digits = true;
        i++;
    }

    if (!has_digits) return 0;
    *ok = true;
    return (uint32_t)result;
}

// MARK: Value Encoders

/// @brief Encode an "int" value into `out` as `len` little-endian bytes.
/// @param value_str Decimal or 0x-prefixed integer, optionally with a leading '-'.
/// @param len Field length in bytes (1 to 4).
/// @details Accepts any value that fits in the field as either a signed or an unsigned integer
///     (e.g., -128 to 255 for a 1-byte field), since the ADCS uses both.
static ADCS_tweak_config_blob_error_enum_t encode_int(
    const char *value_str, uint16_t len, uint8_t *out
) {
    if (len < 1 || len > 4) {
        return BLOB_ERR_INVALID_FIELD_LENGTH_FOR_ENCODING;
    }

    const bool negative = (value_str[0] == '-');
    bool ok;
    const uint32_t magnitude = parse_uint(negative ? &value_str[1] : value_str, &ok);
    if (!ok) {
        return BLOB_ERR_INVALID_VALUE;
    }

    const uint64_t unsigned_max = (1ULL << (8 * len)) - 1; // e.g., 255 for 1 byte.
    const uint64_t signed_min_magnitude = 1ULL << (8 * len - 1); // e.g., 128 for 1 byte.
    if (negative ? (magnitude > signed_min_magnitude) : (magnitude > unsigned_max)) {
        return BLOB_ERR_VALUE_DOES_NOT_FIT;
    }

    // Two's complement for negatives; truncation to `len` bytes happens below.
    const uint32_t value = negative ? (uint32_t)(0U - magnitude) : magnitude;
    for (uint16_t i = 0; i < len; i++) {
        out[i] = (uint8_t)(value >> (8 * i));
    }
    return BLOB_ERR_OK;
}

/// @brief Encode a "float" value into `out` as a little-endian float (4 bytes) or double (8 bytes).
/// @param value_str Decimal floating-point number, like "1.5", "-2", or "3.1e-4".
static ADCS_tweak_config_blob_error_enum_t encode_float(
    const char *value_str, uint16_t len, uint8_t *out
) {
    if (len != 4 && len != 8) {
        return BLOB_ERR_INVALID_FIELD_LENGTH_FOR_ENCODING;
    }

    // atof() silently returns 0.0 on garbage, so check the characters first.
    bool has_digits = false;
    for (uint16_t i = 0; value_str[i] != '\0'; i++) {
        const char c = value_str[i];
        if (c >= '0' && c <= '9') {
            has_digits = true;
        }
        else if (c != '.' && c != '-' && c != '+' && c != 'e' && c != 'E') {
            return BLOB_ERR_INVALID_VALUE;
        }
    }
    if (!has_digits) {
        return BLOB_ERR_INVALID_VALUE;
    }

    const double value = atof(value_str);

    // The STM32L4 is little-endian, so the in-memory bytes are already in ADCS order.
    if (len == 4) {
        union { float f; uint8_t b[4]; } u;
        u.f = (float)value;
        for (uint16_t i = 0; i < 4; i++) out[i] = u.b[i];
    }
    else {
        union { double d; uint8_t b[8]; } u;
        u.d = value;
        for (uint16_t i = 0; i < 8; i++) out[i] = u.b[i];
    }
    return BLOB_ERR_OK;
}

/// @brief Encode a "hex" value into `out`, as raw bytes in the order given.
/// @param value_str Exactly 2*len hex characters (underscores ignored), like "1700".
static ADCS_tweak_config_blob_error_enum_t encode_hex(
    const char *value_str, uint16_t len, uint8_t *out
) {
    if (len > MAX_FIELD_LENGTH_BYTES) {
        return BLOB_ERR_INVALID_FIELD_LENGTH_FOR_ENCODING;
    }

    uint16_t nibble_count = 0;
    for (uint16_t i = 0; value_str[i] != '\0'; i++) {
        if (value_str[i] == '_') continue;

        const int8_t d = hex_to_int(value_str[i]);
        if (d < 0 || nibble_count >= 2 * len) {
            return BLOB_ERR_INVALID_VALUE; // Bad char (including an "0x" prefix), or too long.
        }

        if (nibble_count % 2 == 0) {
            out[nibble_count / 2] = (uint8_t)(d << 4);
        } else {
            out[nibble_count / 2] |= (uint8_t)d;
        }
        nibble_count++;
    }

    if (nibble_count != 2 * len) {
        return BLOB_ERR_INVALID_VALUE; // Too short.
    }
    return BLOB_ERR_OK;
}

// MARK: Output Helpers

/// @brief Write `len` bytes as lowercase hex into `dest`. `dest` must hold at least 2*len+1 chars.
static void bytes_to_hex(const uint8_t *bytes, uint16_t len, char *dest) {
    static const char HEX_CHARS[] = "0123456789abcdef";
    for (uint16_t i = 0; i < len; i++) {
        dest[2 * i] = HEX_CHARS[bytes[i] >> 4];
        dest[2 * i + 1] = HEX_CHARS[bytes[i] & 0x0F];
    }
    dest[2 * len] = '\0';
}

/// @brief Log the full config frame as hex, split across multiple log messages.
static void log_config_hex(const char *label, uint8_t command_id_get, const uint8_t *config, uint16_t len) {
    char hex_str[2 * LOG_HEX_CHUNK_BYTES + 1];
    for (uint16_t start = 0; start < len; start += LOG_HEX_CHUNK_BYTES) {
        const uint16_t chunk_len = (len - start) < LOG_HEX_CHUNK_BYTES ? (len - start) : LOG_HEX_CHUNK_BYTES;
        bytes_to_hex(&config[start], chunk_len, hex_str);
        LOG(
            LOG_SEVERITY_NORMAL,
            "%s: %s config %u @%u: %s",
            BLOB_NAME, label, command_id_get, start, hex_str
        );
    }
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

    // MARK: Parse Args

    const uint16_t args_str_len = strlen(args_str);
    uint16_t pos = 0;

    char arg0_command_id_set[20];
    char arg1_command_id_get[20];
    char arg2_parameters_length_bytes[20];
    char arg3_field_offset_bits[20];
    char arg4_field_length_bits[20];
    char arg5_encoding[20];
    char arg6_value[2 * MAX_FIELD_LENGTH_BYTES + 40]; // Room for hex with underscores.

    pos = parse_token(args_str, pos, args_str_len, arg0_command_id_set,          sizeof(arg0_command_id_set));
    pos = parse_token(args_str, pos, args_str_len, arg1_command_id_get,          sizeof(arg1_command_id_get));
    pos = parse_token(args_str, pos, args_str_len, arg2_parameters_length_bytes, sizeof(arg2_parameters_length_bytes));
    pos = parse_token(args_str, pos, args_str_len, arg3_field_offset_bits,       sizeof(arg3_field_offset_bits));
    pos = parse_token(args_str, pos, args_str_len, arg4_field_length_bits,       sizeof(arg4_field_length_bits));
    pos = parse_token(args_str, pos, args_str_len, arg5_encoding,                sizeof(arg5_encoding));
    pos = parse_token(args_str, pos, args_str_len, arg6_value,                   sizeof(arg6_value));

    if (arg0_command_id_set[0] == '\0'
        || arg1_command_id_get[0] == '\0'
        || arg2_parameters_length_bytes[0] == '\0'
        || arg3_field_offset_bits[0] == '\0'
        || arg4_field_length_bits[0] == '\0'
        || arg5_encoding[0] == '\0'
        || arg6_value[0] == '\0'
    ) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: missing args! Need 7 args. (%s)",
            BLOB_NAME, adcs_tweak_config_blob_error_to_str(BLOB_ERR_MISSING_ARGS)
        );
        return BLOB_ERR_MISSING_ARGS;
    }

    bool ok0, ok1, ok2, ok3, ok4;
    const uint32_t command_id_set          = parse_uint(arg0_command_id_set,          &ok0);
    const uint32_t command_id_get          = parse_uint(arg1_command_id_get,          &ok1);
    const uint32_t parameters_length_bytes = parse_uint(arg2_parameters_length_bytes, &ok2);
    const uint32_t field_offset_bits       = parse_uint(arg3_field_offset_bits,       &ok3);
    const uint32_t field_length_bits       = parse_uint(arg4_field_length_bits,       &ok4);

    if (!ok0 || !ok1 || !ok2 || !ok3 || !ok4) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: invalid int args! (%s)",
            BLOB_NAME, adcs_tweak_config_blob_error_to_str(BLOB_ERR_INVALID_INT_ARG)
        );
        return BLOB_ERR_INVALID_INT_ARG;
    }

    if (command_id_set > 255 || command_id_get > 255) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: command IDs must be 0-255. Got set=%lu, get=%lu. (%s)",
            BLOB_NAME, command_id_set, command_id_get,
            adcs_tweak_config_blob_error_to_str(BLOB_ERR_INVALID_COMMAND_ID)
        );
        return BLOB_ERR_INVALID_COMMAND_ID;
    }

    if (parameters_length_bytes == 0 || parameters_length_bytes > MAX_PARAMETERS_LENGTH_BYTES) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: parameters_length_bytes must be 1-%d. Got %lu. (%s)",
            BLOB_NAME, MAX_PARAMETERS_LENGTH_BYTES, parameters_length_bytes,
            adcs_tweak_config_blob_error_to_str(BLOB_ERR_INVALID_PARAMETERS_LENGTH)
        );
        return BLOB_ERR_INVALID_PARAMETERS_LENGTH;
    }

    if ((field_offset_bits % 8 != 0) || (field_length_bits % 8 != 0)) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: field offset (%lu) and length (%lu) must be multiples of 8 bits. (%s)",
            BLOB_NAME, field_offset_bits, field_length_bits,
            adcs_tweak_config_blob_error_to_str(BLOB_ERR_FIELD_NOT_BYTE_ALIGNED)
        );
        return BLOB_ERR_FIELD_NOT_BYTE_ALIGNED;
    }

    const uint16_t field_offset_bytes = (uint16_t)(field_offset_bits / 8);
    const uint16_t field_length_bytes = (uint16_t)(field_length_bits / 8);

    // Compare in bits, so huge offsets can't wrap around when narrowed to uint16_t.
    if (field_length_bits == 0 || field_offset_bits + field_length_bits > parameters_length_bytes * 8) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: field [%lu, +%lu bits) must be non-empty and within the %lu-byte frame. (%s)",
            BLOB_NAME, field_offset_bits, field_length_bits, parameters_length_bytes,
            adcs_tweak_config_blob_error_to_str(BLOB_ERR_FIELD_OUT_OF_RANGE)
        );
        return BLOB_ERR_FIELD_OUT_OF_RANGE;
    }

    // MARK: Encode New Value

    uint8_t new_field_bytes[MAX_FIELD_LENGTH_BYTES];
    ADCS_tweak_config_blob_error_enum_t encode_err;
    if (str_equals(arg5_encoding, "int")) {
        encode_err = encode_int(arg6_value, field_length_bytes, new_field_bytes);
    }
    else if (str_equals(arg5_encoding, "float")) {
        encode_err = encode_float(arg6_value, field_length_bytes, new_field_bytes);
    }
    else if (str_equals(arg5_encoding, "hex")) {
        encode_err = encode_hex(arg6_value, field_length_bytes, new_field_bytes);
    }
    else {
        encode_err = BLOB_ERR_UNKNOWN_ENCODING;
    }

    if (encode_err != BLOB_ERR_OK) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: can't encode '%s' as %s in %u bytes. (%s)",
            BLOB_NAME, arg6_value, arg5_encoding, field_length_bytes,
            adcs_tweak_config_blob_error_to_str(encode_err)
        );
        return encode_err;
    }

    // MARK: Step 1: Get Existing Config

    uint8_t config[MAX_PARAMETERS_LENGTH_BYTES];
    const uint8_t get_status = ADCS_i2c_request_telemetry_and_check(
        (uint8_t)command_id_get, config, parameters_length_bytes, ADCS_INCLUDE_CHECKSUM
    );
    if (get_status != 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: getting config %lu -> %d. Nothing was changed. (%s)",
            BLOB_NAME, command_id_get, get_status,
            adcs_tweak_config_blob_error_to_str(BLOB_ERR_ADCS_GET_FAILED)
        );
        return BLOB_ERR_ADCS_GET_FAILED;
    }

    // Step 2: Log the full existing config, so it can be restored by hand if needed.
    log_config_hex("Old", (uint8_t)command_id_get, config, (uint16_t)parameters_length_bytes);

    // MARK: Step 3: Apply Change

    uint8_t old_field_bytes[MAX_FIELD_LENGTH_BYTES];
    for (uint16_t i = 0; i < field_length_bytes; i++) {
        old_field_bytes[i] = config[field_offset_bytes + i];
        config[field_offset_bytes + i] = new_field_bytes[i];
    }

    // MARK: Step 4: Set Modified Config

    const uint8_t set_status = ADCS_i2c_send_command_and_check(
        (uint8_t)command_id_set, config, parameters_length_bytes, ADCS_INCLUDE_CHECKSUM
    );
    if (set_status != 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: setting config %lu -> %d. Config state unknown; re-check it. (%s)",
            BLOB_NAME, command_id_set, set_status,
            adcs_tweak_config_blob_error_to_str(BLOB_ERR_ADCS_SET_FAILED)
        );
        return BLOB_ERR_ADCS_SET_FAILED;
    }

    HAL_Delay(SET_TO_VERIFY_DELAY_MS);

    // MARK: Step 5: Verify

    uint8_t readback[MAX_PARAMETERS_LENGTH_BYTES];
    const uint8_t verify_status = ADCS_i2c_request_telemetry_and_check(
        (uint8_t)command_id_get, readback, parameters_length_bytes, ADCS_INCLUDE_CHECKSUM
    );
    if (verify_status != 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: re-getting config %lu -> %d. Set succeeded, but not verified. (%s)",
            BLOB_NAME, command_id_get, verify_status,
            adcs_tweak_config_blob_error_to_str(BLOB_ERR_ADCS_VERIFY_GET_FAILED)
        );
        return BLOB_ERR_ADCS_VERIFY_GET_FAILED;
    }

    bool field_matches = true;
    for (uint16_t i = 0; i < field_length_bytes; i++) {
        if (readback[field_offset_bytes + i] != new_field_bytes[i]) {
            field_matches = false;
        }
    }

    // Bytes outside the target field which differ from what we wrote. Not an error by itself
    // (the ADCS may adjust some fields), but worth knowing about.
    uint16_t other_bytes_changed = 0;
    for (uint16_t i = 0; i < parameters_length_bytes; i++) {
        if (readback[i] != config[i] && (i < field_offset_bytes || i >= field_offset_bytes + field_length_bytes)) {
            other_bytes_changed++;
        }
    }
    if (other_bytes_changed > 0) {
        LOG(
            LOG_SEVERITY_WARNING,
            "%s: %u bytes outside the target field differ from what was written.",
            BLOB_NAME, other_bytes_changed
        );
        log_config_hex("Readback", (uint8_t)command_id_get, readback, (uint16_t)parameters_length_bytes);
    }

    // MARK: Step 6: Response

    char old_hex[2 * MAX_FIELD_LENGTH_BYTES + 1];
    char new_hex[2 * MAX_FIELD_LENGTH_BYTES + 1];
    char readback_hex[2 * MAX_FIELD_LENGTH_BYTES + 1];
    bytes_to_hex(old_field_bytes, field_length_bytes, old_hex);
    bytes_to_hex(new_field_bytes, field_length_bytes, new_hex);
    bytes_to_hex(&readback[field_offset_bytes], field_length_bytes, readback_hex);

    const ADCS_tweak_config_blob_error_enum_t result = field_matches ? BLOB_ERR_OK : BLOB_ERR_VERIFY_MISMATCH;

    // Format like JSON.
    snprintf(
        response_buf, response_buf_len,
        "{\"action\":\"%s\",\"result\":\"%s\",\"set_id\":%lu,\"get_id\":%lu,\"len\":%lu,"
        "\"field_offset_bytes\":%u,\"field_length_bytes\":%u,"
        "\"old\":\"%s\",\"new\":\"%s\",\"readback\":\"%s\",\"other_bytes_changed\":%u}",
        BLOB_NAME, adcs_tweak_config_blob_error_to_str(result),
        command_id_set, command_id_get, parameters_length_bytes,
        field_offset_bytes, field_length_bytes,
        old_hex, new_hex, readback_hex, other_bytes_changed
    );

    return result;
}
