// This is a blob (executable) that periodically requests a "log bestxyzb once" sample from the
// GNSS receiver, stores it into a persistent fixed-size in-memory ring buffer, downlinks
// some random samples from that buffer on every run, and schedules itself for the next run.
//
// Motivation: Collect a rolling history of GNSS position/velocity samples in RAM, and slowly send
// it down to the ground over many passes via random sampling, without needing a dedicated file.
//
// Args Format: <repeat_interval_ms>;<downlink_n>
// - repeat_interval_ms: 0 to run only once, or any positive number to run repeatedly at that
//   interval (clamped to a minimum of 1100ms).
// - downlink_n: Number of randomly-selected samples to downlink from the ring buffer.
//
// Usage Example:
// After uplinking the blob as "blobs/gnss_bestxyzb_ring_v1.blob", run:
//  CTS1+eps_set_channel_enabled(gnss,1)!
//  CTS1+exec_blob_from_fs(blobs/gnss_bestxyzb_ring_v1.blob,0,9000;5)!
//
// Notes:
//  1. Always use "0" as the second argument to exec_blob_from_fs (i.e., always run with malloc).
//  2. This blob re-schedules itself at the specified interval, same mechanism/caveats as the
//     extended beacon blob (re-uplinking cancels any previously-scheduled rerun of this blob).
//  3. SAFETY FEATURE: On each run, this blob checks whether the GNSS EPS power channel
//     (EPS_CHANNEL_3V3_GNSS) is enabled. If the GNSS power is disabled (e.g., EPS safety mode,
//     forgot to enable it before, or intentionally disabled it), this blob ends and does not
//     reschedule itself.
//  4. If a GNSS query fails for any reason, this run skips storing a new sample but still
//     downlinks existing samples and reschedules normally, so transient GNSS comms errors
//     "self-heal" on the next run.
//  5. If GNSS firehose mode is activated, this blob skips collecting data samples while firehose
//     mode is active, but will resume after firehose mode is disabled.

// --------------------------

// Implementation Details:
//
// Persistent storage: This blob has NO .data/.bss of its own (enforced by the Makefile's build
// check -- see FATAL check), because each execution is freshly loaded into a transient malloc'd/
// MPI buffer and jumped into, so ordinary globals would reset every run. Instead, the ring buffer
// lives at a fixed physical SRAM address (see `RING` in blob.ld) that is NOT used by the
// currently-running rc3 firmware image's .data/.bss, so its contents physically survive between
// this blob's separate executions, and are treated as this blob's only persistent state. A magic
// number at the start of that memory detects true cold-start (e.g., first ever run, or after a
// full power-cycle that clears SRAM) vs. an already-initialized buffer.


#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>

#include "telecommand_exec/telecommand_executor.h"
#include "telecommand_exec/telecommand_definitions.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "timekeeping/timekeeping.h"
#include "gnss_receiver/gnss_internal_drivers.h"
#include "eps_drivers/eps_types.h"
#include "eps_drivers/eps_channel_control.h"
#include "eps_drivers/eps_commands.h"
#include "comms_drivers/ax100_tx.h"
#include "comms_drivers/comms_tx.h"
#include "crypto/random_number_generator.h"
#include "obc_systems/external_led_and_rbf.h"

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
static const char BLOB_NAME[] = "gnss_bestxyzb_ring_blob_v1";

// Global variables defined in the firmware ELF (CTS-SAT-1_FW_rc3.elf).
// Note: TIME_uptime_ms() itself is not redeclared here -- it's already provided as a plain
// `inline` function by the included "timekeeping/timekeeping.h" (same as extended_beacon_blob).

extern int snprintf(char *buf, unsigned int size, const char *fmt, ...);
extern int strlen(const char *s);
extern int strcmp(const char *s1, const char *s2);
extern void *memset(void *s, int c, size_t n);
extern void *memcpy(void *__restrict dest, const void *__restrict src, size_t n);

extern void LOG_message(
    uint32_t source, LOG_severity_enum_t severity, uint32_t sink_mask,
    const char *fmt, ...
);

#define LOG(severity, fmt, ...) \
    LOG_message(LOG_SYSTEM_TELECOMMAND, severity, LOG_SINK_ALL, fmt, ##__VA_ARGS__)


// MARK: Persistent Ring Buffer

// See blob.ld for the memory reservation rationale.

// Defined by blob.ld: a NOLOAD symbol anchored at the start of the fixed `RING` memory region.
extern uint8_t gnss_ring_buffer_base[];

#define GNSS_RING_BUFFER_REGION_LEN 0x9000u // Must match blob.ld's MEMORY.RING.LENGTH.

#define GNSS_SAMPLE_SIZE 144 // Slot size. Each BESTXYZB is 144 bytes. Value can be larger to add optional trailing padding.
#define GNSS_RING_BUFFER_CAPACITY 255
#define GNSS_RING_BUFFER_MAGIC 0xB35779C0u

#pragma pack(push, 1)
typedef struct {
    uint32_t magic; // GNSS_RING_BUFFER_MAGIC once initialized; anything else means cold/garbage SRAM.
    uint16_t write_idx; // Next slot to write, 0..(CAPACITY-1). Wraps around (circular eviction).
    uint16_t count; // Number of valid samples stored so far, capped at GNSS_RING_BUFFER_CAPACITY.
    uint32_t gnss_fetch_failure_count; // Total GNSS data fetch failures across all blob executions
        // (GNSS comms failures + BESTXYZB sync-not-found extraction failures), since cold-init.
    uint16_t downlink_seq_num; // Next sequence number to stamp on a downlinked GNSS_bestxyzb_downlink_packet_t.
        // Incremented (and wraps) per packet downlinked, persisted across this blob's executions.
    uint8_t samples[GNSS_RING_BUFFER_CAPACITY][GNSS_SAMPLE_SIZE];
} GNSS_ring_buffer_t;
#pragma pack(pop)

_Static_assert(
    sizeof(GNSS_ring_buffer_t) <= GNSS_RING_BUFFER_REGION_LEN,
    "GNSS_ring_buffer_t must fit within the RING memory region reserved in blob.ld"
);

#define g_ring ((GNSS_ring_buffer_t *)gnss_ring_buffer_base)


// MARK: Packet

#pragma pack(push, 1)
typedef struct {
    uint8_t packet_type; // COMMS_packet_type_enum_t - Always COMMS_PACKET_TYPE_GNSS_BESTXYZB_SAMPLE.

    uint16_t downlink_seq_num; // Sequence number of this downlinked packet (persisted, wraps at 65536).
    uint16_t ring_position; // Index (0..GNSS_RING_BUFFER_CAPACITY-1) within the ring buffer this sample came from.

    uint8_t bestxyzb_data[GNSS_SAMPLE_SIZE]; // Raw BESTXYZB binary log sample.
} GNSS_bestxyzb_downlink_packet_t;
#pragma pack(pop)

_Static_assert(
    sizeof(GNSS_bestxyzb_downlink_packet_t) <= AX100_DOWNLINK_MAX_BYTES,
    "GNSS_bestxyzb_downlink_packet_t must fit within a single AX100 downlink packet"
);


// MARK: Arg Parsing

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

/// @brief Parse a string into an integer.
/// @param s String to parse. Valid format: "<digits>". Underscores are ignored.
/// @returns Parsed integer, or 0 if invalid.
static int32_t parse_int(const char *s, bool *ok) {
    uint32_t result = 0;
    uint8_t i = 0;

    if (ok) *ok = false;
    if (!s || s[0] == '\0') return 0;

    bool has_digits = false;
    while (s[i] != '\0') {
        if (s[i] == '_') { i++; continue; } // skip delimiter

        if (s[i] < '0' || s[i] > '9') return 0; // invalid char
        result = result * 10 + (s[i] - '0');
        has_digits = true;
        i++;
    }

    if (!has_digits) return 0;
    if (ok) *ok = true;
    return (int32_t)result;
}


// MARK: Self-Rescheduling

// Note: Re-used from extended_beacon_blob_main.c

static int16_t get_current_executing_tcmd_agenda_slot_num() {
    for (uint16_t i = 0; i < TCMD_AGENDA_SIZE; i++) {
        if (TCMD_agenda_is_valid[i] == TCMD_AGENDA_ENTRY_EXECUTING) {
            return i;
        }
    }
    return -1;
}

/// @brief Cancel any other pending agenda entries that would re-run this same blob file, so that
/// re-uplinking this blob (whether to start, restart, or stop repeating) doesn't leave duplicate
/// scheduled reruns behind.
/// @param current_slot_num Agenda slot of the `exec_blob_from_fs` tcmd currently executing this blob.
/// @return Number of duplicate agenda entries cancelled. Negative if error.
static int16_t cancel_other_scheduled_reruns_of_this_blob(int16_t current_slot_num) {
    if (current_slot_num < 0) {
        return 0;
    }

    const uint8_t exec_blob_from_fs_tcmd_idx = TCMD_agenda[current_slot_num].tcmd_idx;

    char own_blob_file_name[TCMD_ARGS_STR_NO_PARENS_SIZE];
    const uint8_t parse_result = TCMD_extract_string_arg(
        TCMD_agenda[current_slot_num].args_str_no_parens, 0,
        own_blob_file_name, sizeof(own_blob_file_name)
    );
    if (parse_result != 0) {
        // Can't determine our own blob file name; don't risk cancelling the wrong entries.
        return -1;
    }

    uint16_t cancelled_count = 0;
    for (uint16_t slot_num = 0; slot_num < TCMD_AGENDA_SIZE; slot_num++) {
        if ((int16_t)slot_num == current_slot_num) {
            continue;
        }
        if (TCMD_agenda_is_valid[slot_num] != TCMD_AGENDA_ENTRY_VALID_AND_PENDING) {
            continue;
        }
        if (TCMD_agenda[slot_num].tcmd_idx != exec_blob_from_fs_tcmd_idx) {
            continue;
        }

        char other_blob_file_name[TCMD_ARGS_STR_NO_PARENS_SIZE];
        const uint8_t other_parse_result = TCMD_extract_string_arg(
            TCMD_agenda[slot_num].args_str_no_parens, 0,
            other_blob_file_name, sizeof(other_blob_file_name)
        );
        if (other_parse_result != 0) {
            continue;
        }

        if (strcmp(own_blob_file_name, other_blob_file_name) == 0) {
            TCMD_agenda_is_valid[slot_num] = TCMD_AGENDA_ENTRY_INVALID;
            cancelled_count++;
        }
    }

    return cancelled_count;
}

/// @brief Enqueue a copy of the currently executing tcmd, with a new time into the future to execute.
/// @note This function avoids incrementing the "total telecommands" counter. Goal: Allow that counter to assess how many uplinked commands were successful.
static uint8_t reschedule_current_blob_tcmd(uint32_t time_into_future_to_execute_ms) {
    const int16_t slot_num = get_current_executing_tcmd_agenda_slot_num();
    if (slot_num < 0) {
        return 163;
    }

    TCMD_parsed_tcmd_to_execute_t new_tcmd;
    memcpy(&new_tcmd, &TCMD_agenda[slot_num], sizeof(TCMD_parsed_tcmd_to_execute_t));

    new_tcmd.timestamp_sent = TIME_get_current_unix_epoch_time_ms();
    new_tcmd.timestamp_to_execute = TIME_get_current_unix_epoch_time_ms() + time_into_future_to_execute_ms;

    if (TCMD_add_tcmd_to_agenda(&new_tcmd) != 0) {
        return 164;
    }

    // Undo the counter increase in `TCMD_add_tcmd_to_agenda()`.
    TCMD_total_tcmd_queued_count--;

    return 0;
}


// MARK: GNSS Power Check

/// @brief Check whether the GNSS EPS power channel is currently enabled, via the EPS PDU
///     housekeeping enabled-channels bitfield (NOT a GNSS-side query -- there is no such thing).
/// @param is_on_dest Set to 1 if the channel is enabled, 0 if disabled. Only meaningful if this
///     function returns 0.
/// @return 0 on success (EPS query succeeded), non-zero if the EPS query itself failed.
static uint8_t is_gnss_channel_powered_on(uint8_t *is_on_dest) {
    *is_on_dest = 0;

    EPS_struct_pdu_housekeeping_data_eng_t pdu_data;
    const uint8_t eps_status = EPS_CMD_get_pdu_housekeeping_data_eng(&pdu_data);
    if (eps_status != 0) {
        // Special case for bench testing!
        if (OBC_get_rbf_state() == OBC_RBF_STATE_BENCH) {
            LOG(
                LOG_SEVERITY_WARNING,
                "%s: EPS query failed, but RBF=BENCH so steamroll",
                BLOB_NAME
            );

            *is_on_dest = 1;
            return 0;
        }

        LOG(
            LOG_SEVERITY_WARNING,
            "%s: EPS_CMD_get_pdu_housekeeping_data_eng() -> %d",
            BLOB_NAME, eps_status
        );
        return 1;
    }

    // EPS_CHANNEL_3V3_GNSS == 8, which is < 16, so only stat_ch_on_bitfield is actually consulted;
    // stat_ch_ext_on_bitfield is passed through for channels >= 16 (not our case here).
    *is_on_dest = EPS_check_status_bit_of_channel(
        pdu_data.stat_ch_on_bitfield, pdu_data.stat_ch_ext_on_bitfield, EPS_CHANNEL_3V3_GNSS
    );
    return 0;
}


// MARK: GNSS Sampling

/// @brief Find the NovAtel binary log sync sequence (0xAA 0x44 0x12) in a GNSS command-mode
///     response, and copy up to GNSS_BESTXYZB_BINARY_LEN bytes from there into a maybe-zero-padded,
///     GNSS_SAMPLE_SIZE-byte sample slot.
/// @details GNSS command-mode responses are prefixed with an ASCII acknowledgment (observed as
///     "<OK\n[COM1]"-style text) before the actual binary log, so byte 0 of the raw response is
///     NOT the start of the binary packet -- this must be located by scanning for the sync bytes.
/// @return 0 on success (sync found), 1 if the sync sequence was not found in the response.
static uint8_t extract_bestxyzb_binary(
    const uint8_t *raw_buf, uint16_t raw_buf_len,
    uint8_t out_sample[GNSS_SAMPLE_SIZE]
) {
    // Clear the sample slot. Important in case the message(s) are a bit smaller than the full slot size.
    memset(out_sample, 0, GNSS_SAMPLE_SIZE);

    // Scan for the sync sequence, then copy up to GNSS_SAMPLE_SIZE bytes from there into the slot.
    for (uint16_t i = 0; (i + 3) <= raw_buf_len; i++) {
        if (raw_buf[i] == 0xAA && raw_buf[i + 1] == 0x44 && raw_buf[i + 2] == 0x12) {
            const uint16_t available = raw_buf_len - i;
            const uint16_t copy_len = (available < GNSS_SAMPLE_SIZE) ? available : GNSS_SAMPLE_SIZE;
            memcpy(out_sample, &raw_buf[i], copy_len);
            return 0;
        }
    }

    return 1; // Sync sequence not found.
}

/// @brief Send "log bestxyzb once" to the GNSS and, on success, push the extracted binary log into
///     the persistent ring buffer (overwriting the oldest entry once full).
/// @return 0 on success (sample stored), non-zero on GNSS comms or extraction failure.
static uint8_t sample_and_store_bestxyzb() {
    // Early exit condition: If in firehose mode, we can't do this.
    if (GNSS_current_rx_mode == GNSS_RX_MODE_FIREHOSE_MODE) {
        return 20;
    }

    const char cmd[] = "log bestxyzb once\n";
    const uint16_t cmd_len = strlen(cmd);

    uint8_t rx_buf[256];
    uint16_t rx_buf_len = 0;
    memset(rx_buf, 0, sizeof(rx_buf));

    const uint8_t gnss_status = GNSS_send_cmd_get_response(
        cmd, cmd_len,
        rx_buf, sizeof(rx_buf),
        &rx_buf_len,
        0 // KEEP null bytes -- this is a binary response.
    );
    if (gnss_status != 0) {
        LOG(
            LOG_SEVERITY_WARNING,
            "%s: GNSS_send_cmd_get_response() -> %d",
            BLOB_NAME, gnss_status
        );
        g_ring->gnss_fetch_failure_count++;
        return 1;
    }

    uint8_t sample[GNSS_SAMPLE_SIZE];
    const uint8_t extract_status = extract_bestxyzb_binary(rx_buf, rx_buf_len, sample);
    if (extract_status != 0) {
        LOG(
            LOG_SEVERITY_WARNING,
            "%s: BESTXYZB binary sync (AA 44 12) not found in %d-byte GNSS response",
            BLOB_NAME, rx_buf_len
        );
        g_ring->gnss_fetch_failure_count++;
        return 2;
    }

    memcpy(g_ring->samples[g_ring->write_idx], sample, GNSS_SAMPLE_SIZE);
    g_ring->write_idx = (uint16_t)((g_ring->write_idx + 1) % GNSS_RING_BUFFER_CAPACITY);
    if (g_ring->count < GNSS_RING_BUFFER_CAPACITY) {
        g_ring->count++;
    }

    return 0;
}

/// @brief Downlink up to `downlink_n` randomly-selected (with replacement) samples currently in
///     the ring buffer, each wrapped in a GNSS_bestxyzb_downlink_packet_t
///     (COMMS_PACKET_TYPE_GNSS_BESTXYZB_SAMPLE).
/// @return Number of downlink failures (0 = all succeeded).
static uint16_t downlink_random_samples(uint16_t downlink_n) {
    const uint16_t available = g_ring->count;
    const uint16_t n_to_downlink = (downlink_n < available) ? downlink_n : available;

    uint16_t fail_count = 0;
    for (uint16_t i = 0; i < n_to_downlink; i++) {
        const uint32_t rand_val = CRYPTO_generate_random_uint32(TIME_uptime_ms() + i);
        const uint16_t idx = (uint16_t)(rand_val % available);

        GNSS_bestxyzb_downlink_packet_t packet;
        packet.packet_type = COMMS_PACKET_TYPE_GNSS_BESTXYZB_SAMPLE;
        packet.downlink_seq_num = g_ring->downlink_seq_num++;
        packet.ring_position = idx;
        memcpy(packet.bestxyzb_data, g_ring->samples[idx], GNSS_SAMPLE_SIZE);

        const uint8_t tx_status = AX100_downlink_bytes((uint8_t *)&packet, sizeof(packet));
        if (tx_status != 0) {
            fail_count++;
        }
    }

    return fail_count;
}


// MARK: Main

__attribute__((used, section(".text.entry")))
uint8_t blob_main(
    const char *args_str,
    char *response_buf, unsigned short response_buf_len
) {
    LOG(
        LOG_SEVERITY_DEBUG,
        "Blob (%s) args_str: '%s'",
        BLOB_NAME,
        args_str
    );

    const uint16_t args_str_len = strlen(args_str);
    uint16_t pos = 0;

    char arg0_repeat_interval_ms[20];
    char arg1_downlink_n[20];

    pos = parse_token(args_str, pos, args_str_len, arg0_repeat_interval_ms, sizeof(arg0_repeat_interval_ms));
    pos = parse_token(args_str, pos, args_str_len, arg1_downlink_n, sizeof(arg1_downlink_n));

    if (arg0_repeat_interval_ms[0] == '\0' || arg1_downlink_n[0] == '\0') {
        snprintf(response_buf, response_buf_len, "%s error: missing args!", BLOB_NAME);
        return 135;
    }

    bool arg0_ok, arg1_ok;
    int32_t repeat_interval_ms = parse_int(arg0_repeat_interval_ms, &arg0_ok);
    const int32_t downlink_n = parse_int(arg1_downlink_n, &arg1_ok);

    if (!arg0_ok || !arg1_ok) {
        snprintf(response_buf, response_buf_len, "%s error: invalid int args!", BLOB_NAME);
        return 136;
    }

    // Protect the minimum repeat interval, same as the extended beacon blob: too low of a value
    // would leave the satellite always transmitting, blocking uplink passes.
    if ((repeat_interval_ms != 0) && (repeat_interval_ms < 1100)) {
        repeat_interval_ms = 1100;
    }

    // Cancel any other pending agenda entries that would re-run this same blob, so re-uplinking on
    // every pass doesn't stack up duplicate scheduled reruns.
    const int16_t cancel_result = cancel_other_scheduled_reruns_of_this_blob(get_current_executing_tcmd_agenda_slot_num());
    char cancel_msg[50];
    if (cancel_result < 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: cancel_other_scheduled_reruns_of_this_blob() -> %d",
            BLOB_NAME, cancel_result
        );
        return 137;
    }
    else if (cancel_result > 0) {
        snprintf(cancel_msg, sizeof(cancel_msg), ", %d duplicate rerun(s) cancelled", cancel_result);
    }
    else {
        cancel_msg[0] = '\0';
    }

    // Initialize the persistent ring buffer on true cold-start (first ever run, or after a full
    // power-cycle that cleared SRAM). Otherwise, its contents survive from the previous execution
    // of this blob.
    if (g_ring->magic != GNSS_RING_BUFFER_MAGIC) {
        memset(g_ring, 0, sizeof(GNSS_ring_buffer_t));
        g_ring->magic = GNSS_RING_BUFFER_MAGIC;
    }

    // SAFETY: Check whether the GNSS EPS power channel is actually on.
    // If the channel is off (e.g., shed by EPS safety mode due to low power), or if the EPS query
    // itself fails (treated conservatively as "off"), this blob does NOT reschedule itself.
    // This behaviour ensures the recurring blob doesn't  keep running (and consuming downlink
    // budget) once GNSS has been deliberately powered down.
    uint8_t gnss_is_on = 0;
    const uint8_t power_check_status = is_gnss_channel_powered_on(&gnss_is_on);
    if ((power_check_status != 0) || (!gnss_is_on)) {
        snprintf(
            response_buf, response_buf_len,
            "%s: GNSS channel is off (or EPS query failed, status=%d); NOT rescheduling, blob dying. "
            "gnss_fetch_failures=%lu%s",
            BLOB_NAME, power_check_status, g_ring->gnss_fetch_failure_count, cancel_msg
        );
        return 50; // This is the intended safety shutdown path.
    }

    // GNSS channel is confirmed on: proceed with normal sampling/downlink.
    const uint8_t sample_status = sample_and_store_bestxyzb();
    const uint16_t downlink_fail_count = downlink_random_samples((uint16_t)downlink_n);

    if (repeat_interval_ms > 0) {
        const uint8_t reexec_result = reschedule_current_blob_tcmd((uint32_t)repeat_interval_ms);
        if (reexec_result != 0) {
            snprintf(
                response_buf, response_buf_len,
                "%s error: reschedule_current_blob_tcmd() -> %d%s",
                BLOB_NAME, reexec_result, cancel_msg
            );
            return reexec_result;
        }
    }

    snprintf(
        response_buf, response_buf_len,
        "%s: sample_status=%d, ring_count=%d/%d, downlink_n=%ld, downlink_fail_count=%d, "
        "gnss_fetch_failures=%lu%s",
        BLOB_NAME, sample_status, g_ring->count, GNSS_RING_BUFFER_CAPACITY,
        downlink_n, downlink_fail_count, g_ring->gnss_fetch_failure_count, cancel_msg
    );

    if (sample_status != 0) {
        return 100 + sample_status; // Non-fatal: sample wasn't stored this run, but we still ran fully.
    }
    if (downlink_fail_count > 0) {
        return 60; // Non-fatal: some downlinks failed.
    }

    return 0;
}
