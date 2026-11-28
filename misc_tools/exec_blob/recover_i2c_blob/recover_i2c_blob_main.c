// This is a blob (executable) that diagnoses and recovers a stuck I2C bus, without rebooting.
//
// Motivation: The flight firmware never resets an I2C peripheral after boot. If a bus glitch, a lost
// arbitration, or a HAL timeout mid-transfer (the HAL never sends a STOP) leaves the peripheral's
// BUSY flag set, every later transfer on that bus fails until the OBC reboots.
//
// Args Format: <bus>;<mode>
//  - bus: I2C bus number, 1-4 (1=stack/AX100/ADCS, 2=antenna A, 3=antenna B, 4=OBC temp sensor).
//  - mode: What to do to the bus. Required.
//      - INFO_ONLY: Read-only. Only reports the registers and scan.
//      - ENSURE_RECOVERED: If the bus is stuck, reset the peripheral. If it's still stuck (e.g., a line
//          is still held low), also do a bus clear (9 SCL pulses + STOP), then reset the peripheral again.
//      - FORCE_RECOVER: Reset the peripheral, even if it doesn't look stuck.
//      - FORCE_BUS_CLEAR: Do a bus clear, then reset the peripheral, even if it doesn't look stuck.
//
// Steps:
// 1. Scan the bus, then capture its registers and stuck reasons.
// 2. Do the recovery on the chosen bus (if the mode calls for it).
// 3. Scan the bus, then capture its registers and stuck reasons again.
//
// Response Format (JSON):
// {"action":"recover_i2c_v1","bus":"I2C1","mode":"ENSURE_RECOVERED",
//  "before":{"scan":{"majority_state":"BUSY"},"registers":{...},"stuck":"BUSY_FLAG"},
//  "recovery":{"peripheral_resets":1,"bus_clear_pulses":null,"init_status":0},
//  "after":{"scan":{"majority_state":"ERROR","0x05":"OK","0x57":"OK"},"registers":{...},"stuck":"NOT_STUCK"}}
//
// The registers entry: {"ISR":"0x...","CR1":"0x...","CR2":"0x...","hal_state":"0x20","hal_lock":false,"hal_err":"0x0","scl_level":1,"sda_level":1}
//  - ISR bit 15 = BUSY, bit 1 = TXIS. hal_state 0x20 = READY. scl_level/sda_level are the pin levels (1 = high/idle).
// The scan entry uses the same format as the `scan_i2c_bus` telecommand: the majority state, plus
//  every address with a different state.
// bus_clear_pulses is null if no bus clear was done, else the number of SCL pulses it took for SDA to go high.
//
// Return: 0 if the chosen bus isn't stuck at the end, 1 if it's still stuck. 135+ on argument errors.
// If the response doesn't fit in the response buffer, it's silently cut off (incomplete JSON).
//
// Usage Example:
// After uplinking the blob as "blobs/recover_i2c_v1.blob", run:
// CTS1+exec_blob_from_fs(blobs/recover_i2c_v1.blob,0,1;INFO_ONLY)!
// CTS1+exec_blob_from_fs(blobs/recover_i2c_v1.blob,0,1;ENSURE_RECOVERED)!


#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "telecommands/i2c_telecommand_defs.h"

#define USE_HAL_DRIVER 1
#define STM32L4R5xx 1

#include "main.h" // hi2c1-4 and the I2C pin definitions.

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
static const char *BLOB_NAME = "recover_i2c_v1";

#define BUS_CLEAR_MAX_PULSES 9

extern int snprintf(char *buf, unsigned int size, const char *fmt, ...);
extern int vsnprintf(char *buf, unsigned int size, const char *fmt, va_list args);
extern int strlen (const char *s);
extern int strcmp(const char *a, const char *b);

#if BLOB_ENABLE_LOGS
extern void LOG_message(
    uint32_t source, LOG_severity_enum_t severity, uint32_t sink_mask,
    const char *fmt, ...
);
#endif

// HAL_I2C_Init and HAL_GPIO_Init are declared by the HAL headers.
// Their definitions (and hi2c1-4) are resolved against the firmware ELF at link time.
// Note: HAL_I2C_DeInit is NOT in the flight firmware (removed by --gc-sections), so it can't be used.

#define LOG(severity, fmt, ...) \
    LOG_message(LOG_SYSTEM_TELECOMMAND, severity, LOG_SINK_ALL, fmt, ##__VA_ARGS__)


typedef enum {
    MODE_INFO_ONLY,
    MODE_ENSURE_RECOVERED,
    MODE_FORCE_RECOVER,
    MODE_FORCE_BUS_CLEAR,
} recovery_mode_t;

/// @brief The handle and pins of one I2C bus.
typedef struct {
    I2C_HandleTypeDef *hi2c;
    GPIO_TypeDef *scl_port;
    uint16_t scl_pin;
    GPIO_TypeDef *sda_port;
    uint16_t sda_pin;
} i2c_bus_t;


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

/// @brief Fill `bus` with the handle and pins of I2C bus `bus_num` (1-4).
/// @return true on success, false if `bus_num` isn't 1-4.
/// @note A switch (not a const table) because a table of pointers to firmware globals would need
///     relocations, which blobs can't have.
static bool get_bus(uint8_t bus_num, i2c_bus_t *bus) {
    switch (bus_num) {
        case 1:
            bus->hi2c = &hi2c1;
            bus->scl_port = PIN_STACK_I2C1_SCL_GPIO_Port;
            bus->scl_pin = PIN_STACK_I2C1_SCL_Pin;
            bus->sda_port = PIN_STACK_I2C1_SDA_GPIO_Port;
            bus->sda_pin = PIN_STACK_I2C1_SDA_Pin;
            return true;
        case 2:
            bus->hi2c = &hi2c2;
            bus->scl_port = PIN_ANTENNA_A_I2C2_SCL_GPIO_Port;
            bus->scl_pin = PIN_ANTENNA_A_I2C2_SCL_Pin;
            bus->sda_port = PIN_ANTENNA_A_I2C2_SDA_GPIO_Port;
            bus->sda_pin = PIN_ANTENNA_A_I2C2_SDA_Pin;
            return true;
        case 3:
            bus->hi2c = &hi2c3;
            bus->scl_port = PIN_ANTENNA_B_I2C3_SCL_GPIO_Port;
            bus->scl_pin = PIN_ANTENNA_B_I2C3_SCL_Pin;
            bus->sda_port = PIN_ANTENNA_B_I2C3_SDA_GPIO_Port;
            bus->sda_pin = PIN_ANTENNA_B_I2C3_SDA_Pin;
            return true;
        case 4:
            bus->hi2c = &hi2c4;
            bus->scl_port = PIN_TEMP_SENSOR_I2C4_SCL_GPIO_Port;
            bus->scl_pin = PIN_TEMP_SENSOR_I2C4_SCL_Pin;
            bus->sda_port = PIN_TEMP_SENSOR_I2C4_SDA_GPIO_Port;
            bus->sda_pin = PIN_TEMP_SENSOR_I2C4_SDA_Pin;
            return true;
        default:
            return false;
    }
}

static inline uint8_t pin_level(GPIO_TypeDef *port, uint16_t pin) {
    return (port->IDR & pin) ? 1 : 0;
}

#define STUCK_BUSY_FLAG (1u << 0) // Peripheral's ISR BUSY flag is set.
#define STUCK_HAL_STATE (1u << 1) // HAL handle State isn't READY.
#define STUCK_HAL_LOCK (1u << 2)  // HAL handle is locked.
#define STUCK_SCL_LOW (1u << 3)   // SCL is held low (by the OBC or a device).
#define STUCK_SDA_LOW (1u << 4)   // SDA is held low (by the OBC or a device).

/// @brief Which ways the bus is stuck (a bitmask of STUCK_*). 0 if it isn't stuck.
/// @note Blobs run inside a telecommand, and tasks never yield mid-transfer (cooperative
///     scheduling), so no transfer is ever legitimately in progress here, and an idle bus has
///     both lines high (pulled up). A line held low means the bus is unusable, even if the
///     peripheral looks fine (e.g., a device holding SDA low doesn't set BUSY).
static uint8_t get_stuck_reasons(const i2c_bus_t *bus) {
    uint8_t reasons = 0;
    if (__HAL_I2C_GET_FLAG(bus->hi2c, I2C_FLAG_BUSY) != RESET) reasons |= STUCK_BUSY_FLAG;
    if (bus->hi2c->State != HAL_I2C_STATE_READY) reasons |= STUCK_HAL_STATE;
    if (bus->hi2c->Lock != HAL_UNLOCKED) reasons |= STUCK_HAL_LOCK;
    if (!pin_level(bus->scl_port, bus->scl_pin)) reasons |= STUCK_SCL_LOW;
    if (!pin_level(bus->sda_port, bus->sda_pin)) reasons |= STUCK_SDA_LOW;
    return reasons;
}

/// @brief Append `value` as a JSON number, or `null` if it's negative (i.e., not applicable).
static void append_int_or_null(response_writer_t *w, int16_t value) {
    if (value < 0) response_append(w, "null");
    else response_append(w, "%d", value);
}

static const char *stuck_reason_name(uint8_t reason) {
    switch (reason) {
        case STUCK_BUSY_FLAG: return "BUSY_FLAG";
        case STUCK_HAL_STATE: return "HAL_STATE";
        case STUCK_HAL_LOCK: return "HAL_LOCK";
        case STUCK_SCL_LOW: return "SCL_LOW";
        case STUCK_SDA_LOW: return "SDA_LOW";
    }
    return "UNKNOWN";
}

/// @brief Append the stuck reasons as a JSON string, e.g. `"BUSY_FLAG+SCL_LOW"`, or `"NOT_STUCK"`.
static void append_stuck_reasons(response_writer_t *w, uint8_t reasons) {
    if (reasons == 0) {
        response_append(w, "\"NOT_STUCK\"");
        return;
    }
    response_append(w, "\"");
    bool first = true;
    for (uint8_t reason_bit = STUCK_BUSY_FLAG; reason_bit <= STUCK_SDA_LOW; reason_bit <<= 1) {
        if (reasons & reason_bit) {
            response_append(w, "%s%s", first ? "" : "+", stuck_reason_name(reason_bit));
            first = false;
        }
    }
    response_append(w, "\"");
}

static const char *mode_name(recovery_mode_t mode) {
    switch (mode) {
        case MODE_INFO_ONLY: return "INFO_ONLY";
        case MODE_ENSURE_RECOVERED: return "ENSURE_RECOVERED";
        case MODE_FORCE_RECOVER: return "FORCE_RECOVER";
        case MODE_FORCE_BUS_CLEAR: return "FORCE_BUS_CLEAR";
    }
    return "UNKNOWN";
}

static void append_registers(response_writer_t *w, const i2c_bus_t *bus) {
    const I2C_HandleTypeDef *h = bus->hi2c;

    // true/false, or the raw number if it's neither HAL_LOCKED nor HAL_UNLOCKED (i.e., corrupted).
    char lock_str[6]; // Fits "false" and "255".
    if (h->Lock == HAL_LOCKED) snprintf(lock_str, sizeof(lock_str), "true");
    else if (h->Lock == HAL_UNLOCKED) snprintf(lock_str, sizeof(lock_str), "false");
    else snprintf(lock_str, sizeof(lock_str), "%u", h->Lock);

    response_append(
        w,
        "{\"ISR\":\"0x%08lx\",\"CR1\":\"0x%08lx\",\"CR2\":\"0x%08lx\","
        "\"hal_state\":\"0x%02x\",\"hal_lock\":%s,\"hal_err\":\"0x%lx\",\"scl_level\":%u,\"sda_level\":%u}",
        h->Instance->ISR, h->Instance->CR1, h->Instance->CR2,
        h->State, lock_str, h->ErrorCode,
        pin_level(bus->scl_port, bus->scl_pin), pin_level(bus->sda_port, bus->sda_pin)
    );
}

/// @brief Scan all 128 addresses with the firmware's `scan_i2c_bus` telecommand, and append its
///     result (`{"majority_state":"ERROR","0x10":"OK",...}`).
/// @note The telecommand writes from the start of the buffer it's given, so it gets the unused tail
///     of the response. If it's full, the scan is cut off, like the rest of the response.
static void append_scan(response_writer_t *w, uint8_t bus_num) {
    const char bus_arg[2] = { '0' + bus_num, '\0' };
    char *scan_buf = &w->buf[w->pos];

    // The `scan_i2c_bus` telecommand. Arg string: just the bus number as a string ("1"-"4").
    TCMDEXEC_scan_i2c_bus(bus_arg, scan_buf, w->size - w->pos);
    w->pos += strlen(scan_buf);
}

/// @brief Scan the bus, then append its registers and stuck reasons (read at the same moment).
/// @return The stuck reasons (bitmask of STUCK_*), as reported.
/// @note Scan first, so the registers and stuck reasons are a consistent snapshot of the bus as it
///     is after the scan. A stuck bus isn't touched by the scan (the HAL returns BUSY right away),
///     so its registers (e.g., CR2 from the abandoned transfer) are still intact.
static uint8_t append_bus_state(response_writer_t *w, uint8_t bus_num, const i2c_bus_t *bus) {
    response_append(w, "{\"scan\":");
    append_scan(w, bus_num);
    response_append(w, ",\"registers\":");
    append_registers(w, bus);
    const uint8_t stuck_reasons = get_stuck_reasons(bus);
    response_append(w, ",\"stuck\":");
    append_stuck_reasons(w, stuck_reasons);
    response_append(w, "}");
    return stuck_reasons;
}

/// @brief Reset the I2C peripheral through the RCC, and re-initialize it.
/// @return HAL_I2C_Init status.
/// @details The RCC reset clears every I2C register (including BUSY, and releases SCL/SDA).
///     Setting the handle to RESET makes HAL_I2C_Init also re-run HAL_I2C_MspInit (I2C clock
///     source, GPIO alternate function, IRQs) and unlock the handle. The resulting config matches
///     boot (CR1 reset value = analog filter on, no digital filter).
/// @note The usual way is HAL_I2C_DeInit + HAL_I2C_Init, but
///     HAL_I2C_DeInit and HAL_I2C_MspDeInit aren't in the flight firmware (removed by
///     --gc-sections). The RCC reset gives the same end state, and is more thorough (it resets
///     every register of the target I2C peripheral, not just the state machine register).
static HAL_StatusTypeDef reset_peripheral(uint8_t bus_num, const i2c_bus_t *bus) {
    switch (bus_num) {
        case 1: __HAL_RCC_I2C1_FORCE_RESET(); __HAL_RCC_I2C1_RELEASE_RESET(); break;
        case 2: __HAL_RCC_I2C2_FORCE_RESET(); __HAL_RCC_I2C2_RELEASE_RESET(); break;
        case 3: __HAL_RCC_I2C3_FORCE_RESET(); __HAL_RCC_I2C3_RELEASE_RESET(); break;
        case 4: __HAL_RCC_I2C4_FORCE_RESET(); __HAL_RCC_I2C4_RELEASE_RESET(); break;
        default: break;
    }

    bus->hi2c->State = HAL_I2C_STATE_RESET;
    return HAL_I2C_Init(bus->hi2c);
}

/// @brief Roughly 50-100 us at 16-25 MHz SYSCLK. Bus clear timing isn't critical (slow is fine).
static void delay_half_clock(void) {
    for (volatile uint32_t i = 0; i < 200; i++) {}
}

/// @brief Wait (bounded) for SCL to go high, in case a device is stretching the clock.
static void wait_scl_high(const i2c_bus_t *bus) {
    for (volatile uint32_t i = 0; i < 20000 && !pin_level(bus->scl_port, bus->scl_pin); i++) {}
}

/// @brief Bit-bang a bus clear: pulse SCL until a device holding SDA low lets go (max 9), then send a STOP.
/// @return Number of SCL pulses sent before SDA was high.
/// @note Leaves the pins as GPIO. reset_peripheral() must run after, to give them back to the I2C peripheral.
static uint8_t bus_clear(const i2c_bus_t *bus) {
    // Release both lines (high) before switching them from I2C to GPIO, so the switch doesn't glitch them.
    bus->scl_port->BSRR = bus->scl_pin;
    bus->sda_port->BSRR = bus->sda_pin;

    // Fields set one at a time (not `= {0}`), so the compiler doesn't emit a memset call.
    GPIO_InitTypeDef gpio;
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = 0;
    gpio.Pin = bus->scl_pin;
    HAL_GPIO_Init(bus->scl_port, &gpio);
    gpio.Pin = bus->sda_pin;
    HAL_GPIO_Init(bus->sda_port, &gpio);

    delay_half_clock();
    wait_scl_high(bus);

    uint8_t pulses = 0;
    while (pulses < BUS_CLEAR_MAX_PULSES && !pin_level(bus->sda_port, bus->sda_pin)) {
        bus->scl_port->BSRR = (uint32_t)bus->scl_pin << 16; // SCL low.
        delay_half_clock();
        bus->scl_port->BSRR = bus->scl_pin; // SCL high.
        wait_scl_high(bus);
        delay_half_clock();
        pulses++;
    }

    // STOP: SDA goes low-to-high while SCL is high.
    bus->scl_port->BSRR = (uint32_t)bus->scl_pin << 16; // SCL low.
    delay_half_clock();
    bus->sda_port->BSRR = (uint32_t)bus->sda_pin << 16; // SDA low.
    delay_half_clock();
    bus->scl_port->BSRR = bus->scl_pin; // SCL high.
    wait_scl_high(bus);
    delay_half_clock();
    bus->sda_port->BSRR = bus->sda_pin; // SDA high.
    delay_half_clock();

    return pulses;
}

static uint8_t fetch_states_and_do_recovery(
    uint8_t bus_num, recovery_mode_t mode,
    char *response_buf, uint16_t response_buf_len
) {
    i2c_bus_t bus;
    get_bus(bus_num, &bus);

    response_writer_t w = { .buf = response_buf, .size = response_buf_len, .pos = 0 };
    response_append(&w, "{\"action\":\"%s\",\"bus\":\"I2C%u\",\"mode\":\"%s\",\"before\":", BLOB_NAME, bus_num, mode_name(mode));
    const uint8_t stuck_before = append_bus_state(&w, bus_num, &bus);

    uint8_t peripheral_resets = 0;
    int16_t bus_clear_pulses = -1; // -1 = no bus clear (printed as null).
    int16_t init_status = -1; // -1 = HAL_I2C_Init not run (printed as null).

    if (mode == MODE_FORCE_RECOVER || (mode == MODE_ENSURE_RECOVERED && stuck_before)) {
        init_status = reset_peripheral(bus_num, &bus);
        peripheral_resets++;
    }

    if (
        mode == MODE_FORCE_BUS_CLEAR
        || (mode == MODE_ENSURE_RECOVERED && stuck_before && get_stuck_reasons(&bus))
    ) {
        bus_clear_pulses = bus_clear(&bus);
        init_status = reset_peripheral(bus_num, &bus);
        peripheral_resets++;
    }

    response_append(&w, ",\"recovery\":{\"peripheral_resets\":%u,\"bus_clear_pulses\":", peripheral_resets);
    append_int_or_null(&w, bus_clear_pulses);
    response_append(&w, ",\"init_status\":");
    append_int_or_null(&w, init_status);
    response_append(&w, "},\"after\":");
    const uint8_t stuck_after = append_bus_state(&w, bus_num, &bus);
    response_append(&w, "}");

    return stuck_after ? 1 : 0;
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

    char arg0_bus[4];
    pos = parse_token(args_str, pos, args_str_len, arg0_bus, sizeof(arg0_bus));

    if (arg0_bus[0] < '1' || arg0_bus[0] > '4' || arg0_bus[1] != '\0') {
        snprintf(
            response_buf, response_buf_len,
            "%s error: bus arg must be 1-4, got '%s'",
            BLOB_NAME, arg0_bus
        );
        return 135;
    }
    const uint8_t bus_num = arg0_bus[0] - '0';

    char arg1_mode[24];
    pos = parse_token(args_str, pos, args_str_len, arg1_mode, sizeof(arg1_mode));

    recovery_mode_t mode;
    if (strcmp(arg1_mode, "INFO_ONLY") == 0) {
        mode = MODE_INFO_ONLY;
    }
    else if (strcmp(arg1_mode, "ENSURE_RECOVERED") == 0) {
        mode = MODE_ENSURE_RECOVERED;
    }
    else if (strcmp(arg1_mode, "FORCE_RECOVER") == 0) {
        mode = MODE_FORCE_RECOVER;
    }
    else if (strcmp(arg1_mode, "FORCE_BUS_CLEAR") == 0) {
        mode = MODE_FORCE_BUS_CLEAR;
    }
    else {
        snprintf(
            response_buf, response_buf_len,
            "%s error: unknown mode '%s' (INFO_ONLY, ENSURE_RECOVERED, FORCE_RECOVER, FORCE_BUS_CLEAR)",
            BLOB_NAME, arg1_mode
        );
        return 136;
    }

    if (pos < args_str_len) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: too many args",
            BLOB_NAME
        );
        return 137;
    }

    return fetch_states_and_do_recovery(bus_num, mode, response_buf, response_buf_len);
}
