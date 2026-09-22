// This is a blob (executable) that manages the GNSS receiver's power channel based on available
// power/sun, periodically samples "log bestxyzb once" from the GNSS receiver, stores good fixes
// into a ring of files in the LittleFS filesystem, periodically syncs the OBC clock to GNSS time
// (and pushes that time out to the EPS and the ADCS),
// downlinks a randomly-selected consecutive run of stored samples on every run, and schedules
// itself for the next run.
//
// Motivation: Collect a long rolling history of GNSS position/velocity fixes in non-volatile
// storage (surviving reboots), while autonomously duty-cycling the GNSS receiver so it only draws
// power when the satellite can afford it, and slowly send that history to the ground over many
// passes via random sampling.
//
// Args Format: <repeat_interval_ms>;<downlink_n>[;<flag1|flag2|...>]
// - repeat_interval_ms: 0 to run only once, or any positive number to run repeatedly at that
//   interval (clamped to a minimum of 1100ms).
// - downlink_n: Number of ADDITIONAL consecutive samples to downlink after the randomly-selected
//   starting sample. So a total of (1 + downlink_n) packets are sent per run, fewer if the
//   randomly-chosen start lands near the end of the chosen file.
// - flags: Optional. Vertical-bar-separated keywords, matched case-insensitively:
//     STOP       Permanently cancel this blob: set the persistent stop flag, cancel all pending
//                reruns, turn the GNSS channel off, and exit. Every later invocation exits
//                immediately (without sampling, downlinking, or rescheduling) until RESUME.
//     RESUME     Clear the persistent stop flag set by STOP, and run normally.
//     FAKE       Local/bench test mode: never touch the GNSS UART or the EPS, and synthesize
//                samples from the hardware RNG instead. Everything else (LittleFS storage,
//                downlink, rescheduling) behaves normally.
//     TRACK_MPI  Additionally force the GNSS on whenever the MPI is in active (sensing) mode,
//                regardless of sun/voltage -- except that the 14V hard floor still wins.
//                (This flag only governs powering the GNSS *on*. Downlink is suppressed during
//                MPI activity either way -- see note 8 below.)
//     NOEPS      Never command the EPS channel on or off; just sample if the channel happens to
//                already be on. For bench use and for handing power control back to the ground.
//
// Usage Example:
// After uplinking the blob as "blobs/gnss_bestxyzb_ring_v2.blob", run:
//  CTS1+exec_blob_from_fs(blobs/gnss_bestxyzb_ring_v2.blob,0,60000;5;TRACK_MPI)!
// To permanently stop it:
//  CTS1+exec_blob_from_fs(blobs/gnss_bestxyzb_ring_v2.blob,0,0;0;STOP)!
//
// Notes:
//  1. Always use "0" as the second argument to exec_blob_from_fs (i.e., always run with malloc).
//  2. This blob re-schedules itself at the specified interval, same mechanism/caveats as the
//     extended beacon blob (re-uplinking cancels any previously-scheduled rerun of this blob).
//  3. Unlike v1, this blob TURNS THE GNSS CHANNEL ON AND OFF ITSELF (see Power Policy below).
//     It no longer dies when it finds the GNSS powered off -- that's now an expected state.
//  4. SAFETY: The GNSS is never powered on while the battery is below
//     GNSS_POWER_HARD_FLOOR_MV (14000mV), and is actively turned off if the battery falls below
//     that, regardless of MPI state or sun.
//  5. If a GNSS query fails for any reason, this run skips storing a new sample but still
//     downlinks existing samples and reschedules normally, so transient GNSS comms errors
//     "self-heal" on the next run.
//  6. If GNSS firehose mode is activated, this blob skips collecting data samples while firehose
//     mode is active, but will resume after firehose mode is disabled.
//  7. Only good, non-empty fixes are stored: the solution status must be SOL_COMPUTED, the
//     position type must not be NONE, and the X/Y/Z position bytes must not be all-zero.
//  8. While the MPI is in active (sensing) mode, this blob still samples and stores to disk, but
//     sends NOTHING over the radio that run, so it doesn't compete with the science campaign.
//     Nothing is lost: the stored samples go down on a later run, once the MPI is idle. This
//     applies whether or not the TRACK_MPI flag was passed.
//  9. Likewise, no GNSS time sync (OBC clock set) is performed while the MPI is in active
//     (sensing) mode, even if one is due: stepping the clock mid-campaign would corrupt the
//     timestamps on the science data. The sync stays due and happens on the first run after the
//     MPI goes idle. Also not gated behind the TRACK_MPI flag.
// 10. Every successful GNSS time sync is pushed onward to the EPS (EPS_set_eps_time_based_on_obc_time())
//     and to the ADCS (ADCS_synchronize_unix_time()).
// 11. This blob never mounts the filesystem. If LittleFS is unmounted on entry, it logs CRITICAL
//     and exits with LFS_NOT_MOUNTED (7) without rescheduling. Not latched like STOP.
//
// --------------------------
//
// Power Policy (evaluated fresh on every run, in this order; first match wins):
//   1. battery < 14000mV                      -> OFF  (hard floor; overrides everything below)
//   2. TRACK_MPI flag set AND MPI is sensing   -> ON
//   3. battery >= 15000mV AND in sun           -> ON
//   4. eclipse OR battery < 14500mV            -> OFF
//   5. otherwise                               -> keep the channel in its current state (hysteresis)
// "In sun" means the sum of coarse sun sensors 1..6 is > 100; if the ADCS query fails, we
// conservatively treat it as eclipse (except on the bench, where RBF=BENCH steamrolls).
//
// --------------------------
//
// Implementation Details:
//
// Storage: Samples live in the LittleFS filesystem, under "gnss_ring/", as a ring of
// GNSS_RING_FILE_COUNT files ("gnss_ring/r0.bin" .. "gnss_ring/r9.bin"), each holding up to
// GNSS_RING_RECORDS_PER_FILE fixed-size records. When the current file fills, we advance to the
// next file index; after the last one we wrap back to index 0 and TRUNCATE it (rewrite from
// scratch), evicting the oldest data.
//
// Persistent RAM: This blob has NO .data/.bss of its own (enforced by the Makefile's build check
// -- see FATAL check), because each execution is freshly loaded into a transient malloc'd/MPI
// buffer and jumped into, so ordinary globals would reset every run. Instead, a small state block
// (the LittleFS write cursor, the stop flag, and counters -- NOT the sample data any more) lives
// at a fixed physical SRAM address (see `RING` in blob.ld) that is NOT used by the currently-
// running rc3 firmware image's .data/.bss, so its contents physically survive between this blob's
// separate executions. A magic number at the start of that memory detects true cold-start (e.g.,
// first ever run, or after a full power-cycle that clears SRAM) vs. already-initialized state.
// On cold-start we do NOT inspect the disk: the write cursor simply restarts at r0.bin/record 0,
// and each ring file is truncated as we first write to it, so the ring refills from scratch.
//
// Open file handle: The ring file we're currently appending to is left OPEN between executions --
// its lfs_file_t lives in the same persistent SRAM block. Appending a record is then just an
// lfs_file_write(), instead of the open/write/close per record that the LFS_* helpers do: no
// directory traversal and no cache-buffer malloc/free per sample. The handle is only trusted
// after confirming it's still linked into the mounted filesystem's open-file list (see
// ring_open_file_is_live()), and it's closed -- which is what commits its records to flash --
// when the file fills up, when the blob is STOPped, and when a run isn't rescheduling itself.
// So an unexpected reboot costs the records in the file currently being written (at most
// GNSS_RING_RECORDS_PER_FILE of them); every earlier file is already committed. That's an
// accepted trade: reboots are rare, and the ring holds far more than one file's worth.
//
// The file being written is never also read: it's excluded from the downlink candidates below, so
// there's only ever one handle open on it. Its samples become downlinkable once it fills and is
// closed.

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>

#define USE_HAL_DRIVER 1
#define STM32L4R5xx 1

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_def.h"

#include "telecommand_exec/telecommand_executor.h"
#include "telecommand_exec/telecommand_definitions.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "timekeeping/timekeeping.h"
#include "gnss_receiver/gnss_internal_drivers.h"
#include "gnss_receiver/gnss_time.h"
#include "eps_drivers/eps_types.h"
#include "eps_drivers/eps_channel_control.h"
#include "eps_drivers/eps_commands.h"
#include "eps_drivers/eps_time.h"
#include "comms_drivers/ax100_tx.h"
#include "comms_drivers/comms_tx.h"
#include "crypto/random_number_generator.h"
#include "obc_systems/external_led_and_rbf.h"
#include "obc_systems/adc_vbat_monitor.h"
#include "adcs_drivers/adcs_types.h"
#include "mpi/mpi_types.h"
#include "../lfs.h"
#include "littlefs/littlefs_helper.h"


typedef enum {
    LOG_SEVERITY_DEBUG = 1 << 0,
    LOG_SEVERITY_NORMAL = 1 << 1,
    LOG_SEVERITY_WARNING = 1 << 2,
    LOG_SEVERITY_ERROR = 1 << 3,
    LOG_SEVERITY_CRITICAL = 1 << 4,
} LOG_severity_enum_t;

static const uint32_t LOG_SYSTEM_GNSS = 1 << 3;
static const uint32_t LOG_SYSTEM_TELECOMMAND = 1 << 12;
static const uint32_t LOG_SINK_ALL = (1 << 4) - 1;

static const char ARG_DELIM = ';';
static const char FLAG_DELIM = '|';
static const char BLOB_NAME[] = "gnss_bestxyzb_ring_blob_v2";

// Extern variables from core FW:
extern const uint16_t UART_gnss_buffer_len;                      // Length of the GNSS response buffer
extern volatile uint8_t UART_gnss_buffer[];                      // Buffer for GNSS response
extern volatile uint16_t UART_gnss_buffer_write_idx;             // Write index for GNSS response buffer
extern volatile uint32_t UART_gnss_last_write_time_ms;           // Last write time in milliseconds for GNSS response

extern UART_HandleTypeDef *UART_gnss_port_handle;

extern volatile MPI_rx_mode_enum_t MPI_current_uart_rx_mode;


// Global variables defined in the firmware ELF (CTS-SAT-1_FW_rc3.elf).
// Note: TIME_uptime_ms() itself is not redeclared here -- it's already provided as a plain
// `inline` function by the included "timekeeping/timekeeping.h" (same as extended_beacon_blob).

// Note: strlen/strcmp/memset/memcpy are declared by <string.h>, which arrives transitively via
// littlefs's lfs_util.h, so (unlike in the other blobs) they must NOT be re-declared here.
extern int snprintf(char *buf, unsigned int size, const char *fmt, ...);
extern int strlen(const char *s);
extern int strcmp(const char *s1, const char *s2);
extern void *memset(void *s, int c, size_t n);
extern void *memcpy(void *__restrict dest, const void *__restrict src, size_t n);

extern HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size, uint32_t Timeout);

extern void LOG_message(
    uint32_t source, LOG_severity_enum_t severity, uint32_t sink_mask,
    const char *fmt, ...
);

#define LOG(severity, fmt, ...) \
    LOG_message(LOG_SYSTEM_TELECOMMAND, severity, LOG_SINK_ALL, fmt, ##__VA_ARGS__)

extern void GNSS_set_uart_interrupt_state(uint8_t new_enabled);
extern uint8_t ADCS_get_raw_coarse_sun_sensor_1_to_6(ADCS_raw_coarse_sun_sensor_1_to_6_struct_t *output_struct);
extern uint8_t ADCS_synchronize_unix_time();

// MARK: Tunable Parameters

#define GNSS_POWER_HARD_FLOOR_MV 14000 // NEVER power the GNSS on below this. Turn it off if we fall below it.
#define GNSS_POWER_OFF_BELOW_MV 14500 // Below this (but above the hard floor), shed the GNSS.
#define GNSS_POWER_ON_ABOVE_MV 15000 // At/above this, and in sun, power the GNSS on.
#define GNSS_SUN_SENSOR_SUM_THRESHOLD 50 // Sum of coarse sun sensors 1..6 above which we're "in sun". Nominally 20-28 in eclipse and 50+ in sun.

// How often to re-sync the OBC clock from the GNSS, while the GNSS is powered on and healthy.
#define GNSS_TIME_SYNC_INTERVAL_MS 600000u // 10 minutes.

// Sentinel for the EPS/ADCS time-push statuses: no sync happened this run, so nothing was pushed.
// Can't collide with a real status, since those are single-byte error codes well below 0xFF.
#define TIME_PUSH_NOT_ATTEMPTED 0xFFu

// After switching the GNSS channel on, the receiver needs to boot before it can answer logs.
// We skip sampling on the run that powers it up, and sample from the next run onward.
#define GNSS_POWER_ON_SETTLE_MS 5000u

// MARK: Storage Layout

#define GNSS_RING_DIR "gnss_ring"
#define GNSS_RING_FILE_COUNT 10 // Number of files in the ring, before wrapping back to r0.bin.
#define GNSS_RING_RECORDS_PER_FILE 50 // Records per file, before advancing to the next file.
#define GNSS_SAMPLE_SIZE 144 // Record size. Each BESTXYZB is 144 bytes. Larger values add trailing padding.
#define GNSS_RING_PATH_MAX_LEN 32 // Enough for "gnss_ring/r<n>.bin".


// MARK: Error Enum

// All named status/error codes returned by this blob's functions, including the values
// `blob_main()` itself returns to the telecommand executor (0 = success).
typedef enum {
    BLOB_ERR_OK = 0,
    BLOB_ERR_GNSS_COMMS_FAILED = 1, // GNSS_send_cmd_get_response_NEW() failed.
    BLOB_ERR_BESTXYZB_SYNC_NOT_FOUND = 2, // Sync bytes (AA 44 12) not found in GNSS response.
    BLOB_ERR_EPS_QUERY_FAILED = 3, // EPS_CMD_get_pdu_housekeeping_data_eng() failed.
    BLOB_ERR_BAD_FIX = 4, // Fix was not SOL_COMPUTED, was position-type NONE, or was all-zero.
    BLOB_ERR_LFS_WRITE_FAILED = 5, // Writing/appending the sample record to LittleFS failed.
    BLOB_ERR_LFS_MKDIR_FAILED = 6, // Couldn't create/confirm the GNSS_RING_DIR directory.
    BLOB_ERR_LFS_NOT_MOUNTED = 7, // Filesystem wasn't mounted on entry; blob dies without rescheduling.
    BLOB_ERR_FIREHOSE_MODE_ACTIVE = 20, // Skipped sampling because GNSS firehose mode is active.
    BLOB_ERR_GNSS_POWERED_OFF = 50, // GNSS EPS channel is off this run; nothing sampled (not fatal).
    BLOB_ERR_GNSS_WARMING_UP = 51, // GNSS channel was just switched on; skipping sampling this run.
    BLOB_ERR_DOWNLINK_PARTIAL_FAILURE = 60, // At least one downlink packet failed to send.
    BLOB_ERR_NO_STORED_DATA = 61, // Nothing stored yet, so nothing to downlink.
    BLOB_ERR_DOWNLINK_SKIPPED_MPI_ACTIVE = 62, // Stored a sample, but stayed off the radio (MPI active).
    BLOB_ERR_PERMANENTLY_STOPPED = 70, // The persistent STOP flag is set; blob exits immediately.
    BLOB_ERR_MISSING_ARGS = 135, // One or more required args_str tokens were empty.
    BLOB_ERR_INVALID_INT_ARGS = 136, // One or more args_str tokens failed integer parsing.
    BLOB_ERR_CANCEL_RERUNS_FAILED = 137, // cancel_other_scheduled_reruns_of_this_blob() failed.
    BLOB_ERR_NO_EXECUTING_AGENDA_SLOT = 163, // Couldn't find our own agenda slot to reschedule.
    BLOB_ERR_AGENDA_ADD_FAILED = 164, // TCMD_add_tcmd_to_agenda() failed while rescheduling.
} GNSS_ring_blob_error_enum_t;

/// @brief Convert a GNSS_ring_blob_error_enum_t into a short human-readable name, for use in log
///     messages and output strings.
/// @return Static string; never NULL. "UNKNOWN_ERROR" for values not in the enum.
static const char *gnss_ring_blob_error_to_str(GNSS_ring_blob_error_enum_t err) {
    switch (err) {
        case BLOB_ERR_OK: return "OK";
        case BLOB_ERR_GNSS_COMMS_FAILED: return "GNSS_COMMS_FAILED";
        case BLOB_ERR_BESTXYZB_SYNC_NOT_FOUND: return "BESTXYZB_SYNC_NOT_FOUND";
        case BLOB_ERR_EPS_QUERY_FAILED: return "EPS_QUERY_FAILED";
        case BLOB_ERR_BAD_FIX: return "BAD_FIX";
        case BLOB_ERR_LFS_WRITE_FAILED: return "LFS_WRITE_FAILED";
        case BLOB_ERR_LFS_MKDIR_FAILED: return "LFS_MKDIR_FAILED";
        case BLOB_ERR_LFS_NOT_MOUNTED: return "LFS_NOT_MOUNTED";
        case BLOB_ERR_FIREHOSE_MODE_ACTIVE: return "FIREHOSE_MODE_ACTIVE";
        case BLOB_ERR_GNSS_POWERED_OFF: return "GNSS_POWERED_OFF";
        case BLOB_ERR_GNSS_WARMING_UP: return "GNSS_WARMING_UP";
        case BLOB_ERR_DOWNLINK_PARTIAL_FAILURE: return "DOWNLINK_PARTIAL_FAILURE";
        case BLOB_ERR_NO_STORED_DATA: return "NO_STORED_DATA";
        case BLOB_ERR_DOWNLINK_SKIPPED_MPI_ACTIVE: return "DOWNLINK_SKIPPED_MPI_ACTIVE";
        case BLOB_ERR_PERMANENTLY_STOPPED: return "PERMANENTLY_STOPPED";
        case BLOB_ERR_MISSING_ARGS: return "MISSING_ARGS";
        case BLOB_ERR_INVALID_INT_ARGS: return "INVALID_INT_ARGS";
        case BLOB_ERR_CANCEL_RERUNS_FAILED: return "CANCEL_RERUNS_FAILED";
        case BLOB_ERR_NO_EXECUTING_AGENDA_SLOT: return "NO_EXECUTING_AGENDA_SLOT";
        case BLOB_ERR_AGENDA_ADD_FAILED: return "AGENDA_ADD_FAILED";
    }
    return "UNKNOWN_ERROR";
}


// MARK: Persistent State

// See blob.ld for the memory reservation rationale.

// Defined by blob.ld: a NOLOAD symbol anchored at the start of the fixed `RING` memory region.
extern uint8_t gnss_ring_state_base[];

#define GNSS_RING_STATE_REGION_LEN 0x100u // Must match blob.ld's MEMORY.RING.LENGTH.
#define GNSS_RING_STATE_MAGIC 0xB35779C1u // Deliberately != v1's magic, so v1 state is treated as cold.

// NOT packed, deliberately: it holds a live lfs_file_t that littlefs accesses through an ordinary
// (alignment-assuming) pointer, and the struct is only ever read/written by this blob at a fixed,
// 4-byte-aligned base address, so there's nothing for packing to buy.
typedef struct {
    uint32_t magic; // GNSS_RING_STATE_MAGIC once initialized; anything else means cold/garbage SRAM.

    uint8_t is_permanently_stopped; // Set by the STOP flag; cleared by RESUME. Survives reboots-ish.
    uint8_t write_file_idx; // Ring file currently being appended to, 0..(GNSS_RING_FILE_COUNT-1).
    uint8_t write_record_idx; // Records already in that file, 0..GNSS_RING_RECORDS_PER_FILE.
    uint8_t has_wrapped; // 1 once we've cycled past the last file at least once (all files have data).

    uint8_t gnss_channel_is_on; // Our latest knowledge of the GNSS EPS channel state (1=on).
    uint8_t reserved_padding;

    uint16_t downlink_seq_num; // Next sequence number to stamp on a downlinked packet. Wraps at 65536.

    uint32_t gnss_fetch_failure_count; // GNSS comms + extraction failures, since cold-init.
    uint32_t bad_fix_skipped_count; // Fixes discarded for being invalid/empty, since cold-init.
    uint32_t stored_record_count; // Total records written to LittleFS, since cold-init.
    uint32_t last_time_sync_uptime_ms; // TIME_uptime_ms() at the last successful GNSS time sync.
    uint32_t time_sync_count; // Successful GNSS time syncs, since cold-init.
    uint32_t gnss_power_on_uptime_ms; // TIME_uptime_ms() when we last switched the channel on.

    // The ring file we're appending to, held open across executions. See ring_open_write_file().
    uint8_t open_file_is_valid; // 1 if we left `open_file` open; still verify it's live before use.
    uint8_t open_file_idx; // Ring file index that `open_file` refers to, when valid.
    lfs_file_t open_file; // Live littlefs handle; linked into LFS_filesystem.mlist while open.
} GNSS_ring_state_t;

_Static_assert(
    sizeof(GNSS_ring_state_t) <= GNSS_RING_STATE_REGION_LEN,
    "GNSS_ring_state_t must fit within the RING memory region reserved in blob.ld"
);

#define g_state ((GNSS_ring_state_t *)gnss_ring_state_base)


// MARK: Packet

#pragma pack(push, 1)
typedef struct {
    uint8_t packet_type; // COMMS_packet_type_enum_t - Always COMMS_PACKET_TYPE_GNSS_BESTXYZB_SAMPLE.

    uint16_t downlink_seq_num; // Sequence number of this downlinked packet (persisted, wraps at 65536).
    uint8_t file_idx; // Which ring file (0..GNSS_RING_FILE_COUNT-1) this sample was read from.
    uint16_t record_idx; // Which record within that file this sample was read from.

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

/// @brief Lowercase a single ASCII character. Non-letters pass through unchanged.
/// @note Hand-rolled because the firmware ELF doesn't export tolower() for us to link against.
static char to_lower_char(char c) {
    if ((c >= 'A') && (c <= 'Z')) {
        return (char)(c + ('a' - 'A'));
    }
    return c;
}

/// @brief Check whether `flags_str` contains `flag_name` as a whole, vertical-bar-separated token,
///     compared case-insensitively (so "track_mpi", "TRACK_MPI" and "Fake|Track_MPI" all match).
/// @details Whole-token matching (rather than a plain substring search) matters so that, e.g.,
///     a future "NOSTOP" flag would not accidentally trigger the "STOP" behaviour.
/// @return true if the flag is present.
static bool has_flag(const char *flags_str, const char *flag_name) {
    if (!flags_str || !flag_name) {
        return false;
    }

    uint16_t i = 0;
    while (flags_str[i] != '\0') {
        // Skip any leading delimiters/spaces before this token.
        while ((flags_str[i] == FLAG_DELIM) || (flags_str[i] == ' ')) {
            i++;
        }
        if (flags_str[i] == '\0') {
            break;
        }

        // Compare this token against flag_name, case-insensitively.
        uint16_t j = 0;
        while (
            (flags_str[i + j] != '\0')
            && (flags_str[i + j] != FLAG_DELIM)
            && (flag_name[j] != '\0')
            && (to_lower_char(flags_str[i + j]) == to_lower_char(flag_name[j]))
        ) {
            j++;
        }
        const bool token_ended = (flags_str[i + j] == '\0') || (flags_str[i + j] == FLAG_DELIM);
        if (token_ended && (flag_name[j] == '\0')) {
            return true;
        }

        // Advance to the next token.
        while ((flags_str[i] != '\0') && (flags_str[i] != FLAG_DELIM)) {
            i++;
        }
    }

    return false;
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
static GNSS_ring_blob_error_enum_t reschedule_current_blob_tcmd(uint32_t time_into_future_to_execute_ms) {
    const int16_t slot_num = get_current_executing_tcmd_agenda_slot_num();
    if (slot_num < 0) {
        return BLOB_ERR_NO_EXECUTING_AGENDA_SLOT;
    }

    TCMD_parsed_tcmd_to_execute_t new_tcmd;
    memcpy(&new_tcmd, &TCMD_agenda[slot_num], sizeof(TCMD_parsed_tcmd_to_execute_t));

    new_tcmd.timestamp_sent = TIME_get_current_unix_epoch_time_ms();
    new_tcmd.timestamp_to_execute = TIME_get_current_unix_epoch_time_ms() + time_into_future_to_execute_ms;

    if (TCMD_add_tcmd_to_agenda(&new_tcmd) != 0) {
        return BLOB_ERR_AGENDA_ADD_FAILED;
    }

    // Undo the counter increase in `TCMD_add_tcmd_to_agenda()`.
    TCMD_total_tcmd_queued_count--;

    return BLOB_ERR_OK;
}


// MARK: Power Management

/// @brief Check whether the GNSS EPS power channel is currently enabled, via the EPS PDU
///     housekeeping enabled-channels bitfield (NOT a GNSS-side query -- there is no such thing).
/// @param is_on_dest Set to 1 if the channel is enabled, 0 if disabled. Only meaningful if this
///     function returns BLOB_ERR_OK.
/// @return BLOB_ERR_OK on success (EPS query succeeded), BLOB_ERR_EPS_QUERY_FAILED
///     if the EPS query itself failed.
static GNSS_ring_blob_error_enum_t is_gnss_channel_powered_on(uint8_t *is_on_dest) {
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
            return BLOB_ERR_OK;
        }

        LOG(
            LOG_SEVERITY_WARNING,
            "%s: EPS_CMD_get_pdu_housekeeping_data_eng() -> %d",
            BLOB_NAME, eps_status
        );
        return BLOB_ERR_EPS_QUERY_FAILED;
    }

    // EPS_CHANNEL_3V3_GNSS == 8, which is < 16, so only stat_ch_on_bitfield is actually consulted;
    // stat_ch_ext_on_bitfield is passed through for channels >= 16 (not our case here).
    *is_on_dest = EPS_check_status_bit_of_channel(
        pdu_data.stat_ch_on_bitfield, pdu_data.stat_ch_ext_on_bitfield, EPS_CHANNEL_3V3_GNSS
    );
    return BLOB_ERR_OK;
}

/// @brief Determine whether the satellite is currently in sunlight, from the sum of coarse sun
///     sensors 1 through 6.
/// @details Conservative on failure: if the ADCS query fails we report "not in sun", so a dead or
///     powered-off ADCS can only ever cause us to leave the GNSS off, never to switch it on. The
///     bench (RBF=BENCH) is the exception, where we report "in sun" so the blob is testable
///     indoors without an ADCS.
/// @return true if in sun.
static bool is_in_sun() {
    ADCS_raw_coarse_sun_sensor_1_to_6_struct_t css;
    memset(&css, 0, sizeof(css));

    const uint8_t adcs_status = ADCS_get_raw_coarse_sun_sensor_1_to_6(&css);
    if (adcs_status != 0) {
        if (OBC_get_rbf_state() == OBC_RBF_STATE_BENCH) {
            return true; // Bench testing: pretend it's sunny.
        }
        LOG(
            LOG_SEVERITY_WARNING,
            "%s: ADCS_get_raw_coarse_sun_sensor_1_to_6() -> %d; assuming eclipse",
            BLOB_NAME, adcs_status
        );
        return false;
    }

    const uint16_t css_sum = (uint16_t)css.coarse_sun_sensor_1 + (uint16_t)css.coarse_sun_sensor_2
        + (uint16_t)css.coarse_sun_sensor_3 + (uint16_t)css.coarse_sun_sensor_4
        + (uint16_t)css.coarse_sun_sensor_5 + (uint16_t)css.coarse_sun_sensor_6;

    return (css_sum > GNSS_SUN_SENSOR_SUM_THRESHOLD);
}

/// @brief Whether the MPI is currently actively collecting science data.
static bool is_mpi_active() {
    return (MPI_current_uart_rx_mode == MPI_RX_MODE_SENSING_MODE);
}

typedef enum {
    GNSS_POWER_DECISION_KEEP = 0, // Hysteresis band: leave the channel however it already is.
    GNSS_POWER_DECISION_ON = 1,
    GNSS_POWER_DECISION_OFF = 2,
} GNSS_power_decision_enum_t;

/// @brief Apply the Power Policy (see the file header) to decide whether the GNSS channel should
///     be on, off, or left alone this run.
/// @param track_mpi Whether the TRACK_MPI flag was passed (MPI sensing mode forces the GNSS on).
/// @param vbatt_mV_dest Battery voltage read during the decision, for logging. Never NULL.
/// @param reason_dest Set to a short static string naming the rule that fired. Never NULL.
static GNSS_power_decision_enum_t decide_gnss_power(
    bool track_mpi, int16_t *vbatt_mV_dest, const char **reason_dest
) {
    const int16_t vbatt_mV = OBC_read_vbat_with_adc_mV();
    *vbatt_mV_dest = vbatt_mV;

    // Rule 1: Hard floor. This check comes first and has no exceptions -- not the MPI, not the sun.
    if (vbatt_mV < GNSS_POWER_HARD_FLOOR_MV) {
        *reason_dest = "vbatt<hard_floor";
        return GNSS_POWER_DECISION_OFF;
    }

    // Rule 2: The MPI outranks the sun/voltage rules below, so GNSS fixes accompany science data.
    if (track_mpi && is_mpi_active()) {
        *reason_dest = "mpi_active";
        return GNSS_POWER_DECISION_ON;
    }

    const bool in_sun = is_in_sun();

    // Rule 3: Plenty of charge and actively charging -> collect.
    if ((vbatt_mV >= GNSS_POWER_ON_ABOVE_MV) && in_sun) {
        *reason_dest = "vbatt_high+sun";
        return GNSS_POWER_DECISION_ON;
    }

    // Rule 4: Eclipse or a sagging battery -> shed the load.
    if (!in_sun) {
        *reason_dest = "eclipse";
        return GNSS_POWER_DECISION_OFF;
    }
    if (vbatt_mV < GNSS_POWER_OFF_BELOW_MV) {
        *reason_dest = "vbatt_low";
        return GNSS_POWER_DECISION_OFF;
    }

    // Rule 5: In the hysteresis band -- don't thrash the channel.
    *reason_dest = "hysteresis_hold";
    return GNSS_POWER_DECISION_KEEP;
}

/// @brief Drive the GNSS EPS channel to the state that decide_gnss_power() asked for, and record
///     the resulting state in persistent RAM.
/// @param decision What decide_gnss_power() returned.
/// @param no_eps If true, never command the EPS; just report the channel's existing state.
/// @param is_on_dest Set to the channel's state after this function runs (1=on).
/// @return BLOB_ERR_OK, or BLOB_ERR_EPS_QUERY_FAILED if we couldn't read the channel state.
static GNSS_ring_blob_error_enum_t apply_gnss_power_decision(
    GNSS_power_decision_enum_t decision, bool no_eps, uint8_t *is_on_dest
) {
    uint8_t is_on_now = 0;
    const GNSS_ring_blob_error_enum_t query_status = is_gnss_channel_powered_on(&is_on_now);
    if (query_status != BLOB_ERR_OK) {
        // Conservative: if we can't tell, report "off" and don't command anything. The next run
        // re-evaluates from scratch.
        *is_on_dest = 0;
        return query_status;
    }

    uint8_t want_on = is_on_now;
    if (decision == GNSS_POWER_DECISION_ON) {
        want_on = 1;
    }
    else if (decision == GNSS_POWER_DECISION_OFF) {
        want_on = 0;
    }

    if ((want_on != is_on_now) && (!no_eps)) {
        const uint8_t set_status = EPS_set_channel_enabled(EPS_CHANNEL_3V3_GNSS, want_on);
        if (set_status != 0) {
            LOG(
                LOG_SEVERITY_WARNING,
                "%s: EPS_set_channel_enabled(GNSS, %d) -> %d",
                BLOB_NAME, want_on, set_status
            );
            // Report the state we know we were in, since the change didn't take.
            *is_on_dest = is_on_now;
            g_state->gnss_channel_is_on = is_on_now;
            return BLOB_ERR_OK;
        }

        LOG(
            LOG_SEVERITY_NORMAL,
            "%s: GNSS channel switched %s",
            BLOB_NAME, want_on ? "ON" : "OFF"
        );
        if (want_on) {
            // Remember when it came up, so we can let the receiver boot before querying it.
            g_state->gnss_power_on_uptime_ms = TIME_uptime_ms();
        }
        is_on_now = want_on;
    }

    *is_on_dest = is_on_now;
    g_state->gnss_channel_is_on = is_on_now;
    return BLOB_ERR_OK;
}


// MARK: GNSS UART

const uint32_t GNSS_RX_TIMEOUT_BEFORE_FIRST_BYTE_MS = 800;

// Lots of commands pause in the middle (e.g., BESTXYZA) as it contemplates its position in the universe.
// GNSS takes time to respond, first section of log response ie <OK\n [COM1] is quick but the rest of
// the data response takes a while.
const uint32_t GNSS_RX_TIMEOUT_BETWEEN_BYTES_MS = 2500;

// MARK: GNSS Binary Frame Format
//
// NovAtel OEM7 binary log format (e.g., the body of a "log bestxyzb once" response), per the OEM7
// Commands and Logs Reference Manual, "Binary" message format:
//   Offset  Size  Field
//   0       3     Sync bytes: 0xAA 0x44 0x12
//   3       1     Header length (bytes), typically 28
//   4-7     4     (Message ID, Message type, Port address -- not needed for framing)
//   8       2     Message body length (bytes, little-endian uint16), NOT including header or CRC
//   ...           Rest of header, then the message body, then a trailing 4-byte CRC32.
// So the exact total frame length (from the sync bytes) is:
//   header_length + body_length + GNSS_BINARY_CRC_LEN
// This lets us detect end-of-message by byte count as soon as it's knowable, rather than only via
// the (slow, and binary-data-unsafe) ASCII "*CRC\r\n" heuristic below, which doesn't apply to binary
// responses at all.
#define GNSS_BINARY_SYNC_0 0xAAu
#define GNSS_BINARY_SYNC_1 0x44u
#define GNSS_BINARY_SYNC_2 0x12u
#define GNSS_BINARY_HEADER_LEN_OFFSET 3u // Offset (from sync) of the 1-byte header-length field.
#define GNSS_BINARY_BODY_LEN_OFFSET 8u // Offset (from sync) of the 2-byte (LE) body-length field.
#define GNSS_BINARY_MIN_BYTES_TO_READ_LENGTHS 10u // Bytes from sync needed to read both length fields.
#define GNSS_BINARY_CRC_LEN 4u // Trailing CRC32, appended after the header+body.

// BESTXYZ message body layout (offsets from the START OF THE BODY, i.e. past the header):
//   0   4  P-sol status (enum uint32, LE). 0 == SOL_COMPUTED; anything else is an unusable fix.
//   4   4  Position type (enum uint32, LE). 0 == NONE; anything else is some kind of real fix.
//   8   24 P-X, P-Y, P-Z (3x double, LE), in meters (ECEF).
#define GNSS_BESTXYZ_SOL_STATUS_OFFSET 0u
#define GNSS_BESTXYZ_POS_TYPE_OFFSET 4u
#define GNSS_BESTXYZ_POS_XYZ_OFFSET 8u
#define GNSS_BESTXYZ_POS_XYZ_LEN 24u // 3 doubles.
#define GNSS_BESTXYZ_MIN_BODY_LEN (GNSS_BESTXYZ_POS_XYZ_OFFSET + GNSS_BESTXYZ_POS_XYZ_LEN)
#define GNSS_SOL_STATUS_SOL_COMPUTED 0u
#define GNSS_POS_TYPE_NONE 0u

/// @brief Sends a log command to the GNSS, and receives the response.
/// @param cmd_buf Log command string to send to the GNSS, without EOL characters.
/// @param cmd_buf_len Exact length of the log command string.
/// @param rx_buf Buffer to store the response (not necessarily null terminated).
/// @param rx_buf_max_size Size of the response buffer.
/// @param rx_buf_len_dest Pointer to place to store the length of the response buffer (not necessarily null terminated).
/// @return 0 on success, >0 if error.
/// @note This function is intended for "once" log commands and control commands.
/// @note This function does not validate the response, as related to the request.
static uint8_t GNSS_send_cmd_get_response_when_firehose_storage_disabled_new(
    const char *cmd_buf, uint8_t cmd_buf_len,
    uint8_t rx_buf[],
    const uint16_t rx_buf_max_size,
    uint16_t* rx_buf_len_dest,
    uint8_t remove_null_bytes_in_middle
) {
    // Reset the GNSS UART interrupt variables
    GNSS_set_uart_interrupt_state(0); // Lock writing to the UART_gnss_buffer while we memset it
    for (uint16_t i = 0; i < UART_gnss_buffer_len; i++) {
        // Clear the buffer.
        // Can't use memset because UART_gnss_buffer is volatile.
        // Review comment: I think just setting the UART_gnss_buffer_write_idx to the start is good enough, but we'll keep this.
        UART_gnss_buffer[i] = 0;
    }

    // Make it start writing to the start of the buffer.
    UART_gnss_buffer_write_idx = 0;

    // TX TO GNSS
    const HAL_StatusTypeDef tx_status_1 = HAL_UART_Transmit(
        UART_gnss_port_handle,
        (uint8_t *)cmd_buf,
        cmd_buf_len,
        100
    );
    const HAL_StatusTypeDef tx_status_2 = HAL_UART_Transmit(
        UART_gnss_port_handle,
        (uint8_t *)"\r\n",
        3,
        100
    );

    if (tx_status_1 != HAL_OK || tx_status_2 != HAL_OK) {
        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "GNSS ERROR: tx_status != HAL_OK (%d, %d)",
            tx_status_1, tx_status_2
        );
        return 1;
    }

    GNSS_set_uart_interrupt_state(1);	// We are now expecting a response

    // RX FROM GNSS, into UART_gnss_buffer
    const uint32_t start_rx_time = TIME_uptime_ms();

    // State for the binary-frame fast-path end-of-message detection (see GNSS Binary Frame Format
    // above). `binary_sync_offset` is -1 until the 0xAA 0x44 0x12 sync is found in the buffer (it's
    // preceded by a short ASCII ack, e.g. "<OK\r\n[COM1]", for command-mode responses).
    int32_t binary_sync_offset = -1;
    uint8_t binary_lengths_parsed = 0;
    uint32_t binary_expected_total_len = 0; // Valid only once binary_lengths_parsed is set.

    while (1) {
        if ((UART_gnss_buffer_write_idx == 0)) {
            // Check if we've timed out (before the first byte)
            if ((TIME_uptime_ms() - start_rx_time) > GNSS_RX_TIMEOUT_BEFORE_FIRST_BYTE_MS) {
                LOG_message(
                    LOG_SYSTEM_GNSS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
                    "GNSS ERROR: Timeout before receiving any data"
                );

                // Disable the UART gnss channel
                GNSS_set_uart_interrupt_state(0);

                *rx_buf_len_dest = 0;

                return 2; // Error: Timeout before receiving any data.
            }
        }
        else { // thus, UART_gnss_buffer_write_idx > 0
            // Check if we've timed out (between bytes)
            const uint32_t cur_time = TIME_uptime_ms();
            // Note: Sometimes, because ISRs and C are fun, the UART_gnss_last_write_time_ms is
            // greater than `cur_time`. Thus, we must do a safety check that the time difference
            // is positive.
            if (
                (cur_time > UART_gnss_last_write_time_ms) // Important seemingly-obvious safety check.
                && ((cur_time - UART_gnss_last_write_time_ms) > GNSS_RX_TIMEOUT_BETWEEN_BYTES_MS)
            ) {
                // Non-fatal error. Parse what we've received.
                break;
            }

            // Fast path: detect a NovAtel OEM7 binary log frame (e.g. BESTXYZB) and compute its
            // exact expected length from the header, so we can stop the instant it's fully received
            // instead of waiting out GNSS_RX_TIMEOUT_BETWEEN_BYTES_MS on every binary response.
            if (binary_sync_offset < 0) {
                for (uint16_t i = 0; (i + 3) <= UART_gnss_buffer_write_idx; i++) {
                    if (
                        (UART_gnss_buffer[i] == GNSS_BINARY_SYNC_0)
                        && (UART_gnss_buffer[i + 1] == GNSS_BINARY_SYNC_1)
                        && (UART_gnss_buffer[i + 2] == GNSS_BINARY_SYNC_2)
                    ) {
                        binary_sync_offset = (int32_t)i;
                        break;
                    }
                }
            }

            if ((binary_sync_offset >= 0) && !binary_lengths_parsed) {
                const uint32_t lengths_ready_at = (uint32_t)binary_sync_offset + GNSS_BINARY_MIN_BYTES_TO_READ_LENGTHS;
                if (UART_gnss_buffer_write_idx >= lengths_ready_at) {
                    const uint8_t header_len = UART_gnss_buffer[binary_sync_offset + GNSS_BINARY_HEADER_LEN_OFFSET];
                    const uint16_t body_len = (uint16_t)(
                        (uint16_t)UART_gnss_buffer[binary_sync_offset + GNSS_BINARY_BODY_LEN_OFFSET]
                        | ((uint16_t)UART_gnss_buffer[binary_sync_offset + GNSS_BINARY_BODY_LEN_OFFSET + 1] << 8)
                    );
                    binary_expected_total_len = (uint32_t)binary_sync_offset + header_len + body_len + GNSS_BINARY_CRC_LEN;
                    binary_lengths_parsed = 1;
                }
            }

            if (binary_lengths_parsed && (UART_gnss_buffer_write_idx >= binary_expected_total_len)) {
                // Full binary frame (header + body + CRC) received.
                break;
            }

            // Fallback for ASCII/abbreviated-ASCII responses (this heuristic doesn't apply once a
            // binary frame has been detected above -- binary payload bytes could spuriously match it).
            // Check for "end of message" section:
            // (asterisk, followed by 8 CRC-ish hex chars, followed by a newline).
            // Critical to end communication as fast as possible, especially for time-related and
            // time-sensitive commands.
            // End of message example: ...,38000,VALID*5ea733a7[\r or \n]
            // Index offset from write_idx:   -10  -9  -8  -7  -6  -5  -4  -3  -2  -1
            // Character:                       *   H   H   H   H   H   H   H   H  \n|r
            if (
                (binary_sync_offset < 0) // Only relevant to non-binary (ASCII) responses.
                && (UART_gnss_buffer_write_idx > 12) // Semi-arbitrary minimum length (>10).
                && (
                    (UART_gnss_buffer[UART_gnss_buffer_write_idx - 1] == '\r')
                    || (UART_gnss_buffer[UART_gnss_buffer_write_idx - 1] == '\n')
                )
                && (UART_gnss_buffer[UART_gnss_buffer_write_idx - 10] == '*')
            ) {
                // Validate the 8 chars between '*' and EOL are all hex digits.
                uint8_t is_valid_hex = 1;
                for (int i = 2; i <= 9; i++) {
                    const uint8_t c = UART_gnss_buffer[UART_gnss_buffer_write_idx - i];
                    if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
                        is_valid_hex = 0;
                        break;
                    }
                }
                if (is_valid_hex) {
                    break;
                }
            }

            // Exit if we've received all the buffer can hold.
            if (UART_gnss_buffer_write_idx >= rx_buf_max_size) {
                break;
            }
        }
    }

    // End Receiving
    GNSS_set_uart_interrupt_state(0); // We are no longer expecting a response

    // Review comment: This next line doesn't seem necessary.
    UART_gnss_buffer[UART_gnss_buffer_write_idx] = '\0'; // Null-terminate the string.

    // Check that we've received what we're expecting.
    uint16_t bytes_received_count = 0; // Includes null bytes.
    if (UART_gnss_buffer_write_idx >= rx_buf_max_size) {
        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_WARNING, LOG_SINK_ALL,
            "GNSS: Received more data (>=%d bytes) than rx_buf_max_size (%d bytes)",
            UART_gnss_buffer_write_idx,
            rx_buf_max_size
        );

        bytes_received_count = rx_buf_max_size - 1;
        // No need to return here. We can still pass back the data we have.
    }
    else {
        bytes_received_count = UART_gnss_buffer_write_idx;
    }

    // Copy the log response from the UART gnss buffer to the rx_buf[] and clear the buffer.
    uint16_t dest_write_idx = 0;
    uint16_t remove_nulls_count = 0;
    for (uint16_t i = 0; i < bytes_received_count; i++) {
        if (remove_null_bytes_in_middle) {
            if (UART_gnss_buffer[i] == '\0') {
                remove_nulls_count++;
                continue;
            }
        }

        rx_buf[dest_write_idx++] = UART_gnss_buffer[i];
    }
    *rx_buf_len_dest = dest_write_idx;

    if (remove_nulls_count > 0) {
        LOG_message(
            LOG_SYSTEM_GNSS, LOG_SEVERITY_DEBUG, LOG_SINK_ALL,
            "GNSS: Removed %d null bytes from response, %d bytes remain",
            remove_nulls_count,
            dest_write_idx
        );
    }

    // Ensure the final output buffer (rx_buf) is null-terminated, no matter what.
    rx_buf[rx_buf_max_size - 1] = '\0';
    rx_buf[*rx_buf_len_dest] = '\0';

    return 0;
}


/// @brief Sends a log command to the GNSS, and receives the response.
/// @param cmd_buf Log command string to send to the GNSS, without EOL characters.
/// @param cmd_buf_len Exact length of the log command string.
/// @param rx_buf Buffer to store the response (not necessarily null terminated).
/// @param rx_buf_max_size Size of the response buffer.
/// @param rx_buf_len_dest Pointer to place to store the length of the response buffer (not necessarily null terminated).
/// @param remove_null_bytes_in_middle If non-zero, remove any null bytes in the middle of the response.
/// @return 0 on success, >0 if error.
/// @note This function is intended for "once" log commands and control commands.
/// @note This function does not validate the response, as related to the request.
/// @note This function properly handles interactions with the firehose file, if enabled.
uint8_t GNSS_send_cmd_get_response_NEW(
    const char *cmd_buf, uint8_t cmd_buf_len,
    uint8_t rx_buf[],
    const uint16_t rx_buf_max_size,
    uint16_t* rx_buf_len_dest,
    uint8_t remove_null_bytes_in_middle
) {
    const GNSS_rx_mode_enum_t rx_mode_at_start = GNSS_current_rx_mode;

    // We must first store any pending data in the UART_gnss_buffer to the file,
    // before clearing the buffer.
    // VENDORING NOTE: In this blob, this step is unnecessary because we exit early if in firehose mode.
    // if (rx_mode_at_start == GNSS_RX_MODE_FIREHOSE_MODE) {
    //     GNSS_subtask_store_firehose_data_to_file();
    // }

    GNSS_current_rx_mode = GNSS_RX_MODE_COMMAND_MODE;

    // This is the main action! The rest is a wrapper to handle interactions with firehose storage mode.
    const uint8_t ret = GNSS_send_cmd_get_response_when_firehose_storage_disabled_new(
        cmd_buf, cmd_buf_len, rx_buf, rx_buf_max_size, rx_buf_len_dest,
        remove_null_bytes_in_middle
    );

    // Reset back to the original RX mode.
    GNSS_current_rx_mode = rx_mode_at_start;

    // Write the data to the firehose file, or effectively discard it by resetting the buffer.
    if (rx_mode_at_start == GNSS_RX_MODE_FIREHOSE_MODE) {
        // You may be tempted to call GNSS_subtask_store_firehose_data_to_file() here, but you shouldn't.
        // This function must return ASAP in order to provide the data to caller quickly, in case the caller
        // is doing a time sync.

        // If we're in firehose mode, and the interrupt isn't currently enabled, we must ensure it's enabled.
        GNSS_set_uart_interrupt_state(1);
    }

    return ret;
}


// MARK: LittleFS Ring Storage

/// @brief Build the LittleFS path of ring file `file_idx` (e.g. "gnss_ring/r3.bin").
static void make_ring_file_path(uint8_t file_idx, char *path_dest, uint16_t path_dest_size) {
    snprintf(path_dest, path_dest_size, "%s/r%d.bin", GNSS_RING_DIR, (int)file_idx);
}

/// @brief Create the "gnss_ring/" directory if it doesn't already exist.
/// @return BLOB_ERR_OK if the directory exists (whether we just made it or not),
///     BLOB_ERR_LFS_MKDIR_FAILED otherwise.
static GNSS_ring_blob_error_enum_t ensure_ring_dir_exists() {
    const int mkdir_result = lfs_mkdir(&LFS_filesystem, GNSS_RING_DIR);
    if ((mkdir_result == 0) || (mkdir_result == LFS_ERR_EXIST)) {
        return BLOB_ERR_OK; // Already-exists is the normal case on every run but the first.
    }

    LOG(
        LOG_SEVERITY_ERROR,
        "%s: lfs_mkdir(%s) -> %d",
        BLOB_NAME, GNSS_RING_DIR, mkdir_result
    );
    return BLOB_ERR_LFS_MKDIR_FAILED;
}

/// @brief Number of whole records currently stored in ring file `file_idx`.
/// @return 0..GNSS_RING_RECORDS_PER_FILE. Returns 0 if the file is missing or unreadable.
static uint16_t ring_file_record_count(uint8_t file_idx) {
    char path[GNSS_RING_PATH_MAX_LEN];
    make_ring_file_path(file_idx, path, sizeof(path));

    const lfs_ssize_t size = LFS_file_size(path, 0); // 0 == don't log; a missing file is normal here.
    if (size <= 0) {
        return 0;
    }

    uint16_t count = (uint16_t)((uint32_t)size / GNSS_SAMPLE_SIZE);
    if (count > GNSS_RING_RECORDS_PER_FILE) {
        count = GNSS_RING_RECORDS_PER_FILE; // Defensive: ignore any over-long tail.
    }
    return count;
}

/// @brief Is the persisted `open_file` handle still a live handle on the mounted filesystem?
/// @details The handle survives in SRAM between executions, but the filesystem state it points
///     into doesn't have to: an unmount/remount (or a firmware restart that happened to leave our
///     SRAM intact) frees its cache buffer and leaves us holding a dangling handle. littlefs links
///     every open file into `LFS_filesystem.mlist`, so walking that list is a cheap and
///     authoritative check that the currently-mounted filesystem still knows about our handle.
/// @return true only if the handle is safe to write to.
static bool ring_open_file_is_live() {
    if ((!g_state->open_file_is_valid) || (!LFS_is_lfs_mounted)) {
        return false;
    }

    for (struct lfs_mlist *entry = LFS_filesystem.mlist; entry != NULL; entry = entry->next) {
        if ((void *)entry == (void *)&g_state->open_file) {
            return true;
        }
    }
    return false; // Stale handle (remount, or something closed it for us); treat it as gone.
}

/// @brief Close the persisted write handle, committing its pending records, and mark it invalid.
/// @details Safe to call at any time: does nothing if no live handle is currently held.
static void ring_close_open_file() {
    if (ring_open_file_is_live()) {
        const int close_result = lfs_file_close(&LFS_filesystem, &g_state->open_file);
        if (close_result < 0) {
            LOG(
                LOG_SEVERITY_ERROR,
                "%s: lfs_file_close(r%d) -> %d",
                BLOB_NAME, g_state->open_file_idx, close_result
            );
        }
    }

    g_state->open_file_is_valid = 0;
}

/// @brief Make sure `g_state->open_file` is open on ring file `write_file_idx`, ready to append.
/// @details Reuses the handle left open by a previous execution whenever it's still live and
///     points at the right file, which is the common case: the open (a directory traversal plus a
///     cache-buffer allocation) then happens once per ring file, not once per sample.
/// @return BLOB_ERR_OK if a live handle is ready, BLOB_ERR_LFS_WRITE_FAILED otherwise.
static GNSS_ring_blob_error_enum_t ring_open_write_file() {
    if (ring_open_file_is_live()) {
        if (g_state->open_file_idx == g_state->write_file_idx) {
            return BLOB_ERR_OK; // Already open on the right file: nothing to do.
        }
        ring_close_open_file(); // We've rolled over to the next ring file; commit the old one.
    }
    else {
        g_state->open_file_is_valid = 0; // Stale handle from before a remount; just drop it.
    }

    char path[GNSS_RING_PATH_MAX_LEN];
    make_ring_file_path(g_state->write_file_idx, path, sizeof(path));

    // Starting a file at record 0 means creating it, or TRUNCATE-ing an old file we've now wrapped
    // around to -- that's how the ring evicts its oldest data. Otherwise we're resuming a file
    // that's partly full, so append to whatever is already on disk.
    const int open_flags = LFS_O_WRONLY | LFS_O_CREAT | (
        (g_state->write_record_idx == 0) ? LFS_O_TRUNC : LFS_O_APPEND
    );

    const int open_result = lfs_file_open(&LFS_filesystem, &g_state->open_file, path, open_flags);
    if (open_result < 0) {
        LOG(LOG_SEVERITY_ERROR, "%s: lfs_file_open(%s) -> %d", BLOB_NAME, path, open_result);
        return BLOB_ERR_LFS_WRITE_FAILED;
    }

    g_state->open_file_idx = g_state->write_file_idx;
    g_state->open_file_is_valid = 1;
    return BLOB_ERR_OK;
}

/// @brief Append one sample record to the ring, advancing (and wrapping/truncating) as needed.
/// @details Writes straight through the persistent handle with lfs_file_write(). The file stays
///     open afterwards, and is only closed -- which is what commits its records -- once it fills
///     up.
/// @param sample The GNSS_SAMPLE_SIZE-byte record to store.
/// @return BLOB_ERR_OK on success, BLOB_ERR_LFS_WRITE_FAILED if LittleFS rejected the write.
static GNSS_ring_blob_error_enum_t store_sample_to_ring(const uint8_t sample[GNSS_SAMPLE_SIZE]) {
    const GNSS_ring_blob_error_enum_t open_status = ring_open_write_file();
    if (open_status != BLOB_ERR_OK) {
        return open_status;
    }

    const lfs_ssize_t write_result = lfs_file_write(
        &LFS_filesystem, &g_state->open_file, sample, GNSS_SAMPLE_SIZE
    );
    if (write_result != (lfs_ssize_t)GNSS_SAMPLE_SIZE) {
        LOG(
            LOG_SEVERITY_ERROR,
            "%s: lfs_file_write(r%d, record %d) -> %ld",
            BLOB_NAME, g_state->write_file_idx, g_state->write_record_idx, (long)write_result
        );
        // The handle's write position is now unknown; drop it so the next run reopens cleanly.
        ring_close_open_file();
        return BLOB_ERR_LFS_WRITE_FAILED;
    }

    g_state->stored_record_count++;
    g_state->write_record_idx++;

    if (g_state->write_record_idx >= GNSS_RING_RECORDS_PER_FILE) {
        // This file is full: advance the cursor and close the file (which commits its pending
        // records). The next stored sample opens the next file in the ring.
        g_state->write_record_idx = 0;
        g_state->write_file_idx++;
        if (g_state->write_file_idx >= GNSS_RING_FILE_COUNT) {
            g_state->write_file_idx = 0;
            g_state->has_wrapped = 1;
        }
        ring_close_open_file();
    }

    return BLOB_ERR_OK;
}


// MARK: GNSS Sampling

/// @brief Find the NovAtel binary log sync sequence (0xAA 0x44 0x12) in a GNSS command-mode
///     response, and copy up to GNSS_SAMPLE_SIZE bytes from there into a maybe-zero-padded,
///     GNSS_SAMPLE_SIZE-byte sample slot.
/// @details GNSS command-mode responses are prefixed with an ASCII acknowledgment (observed as
///     "<OK\n[COM1]"-style text) before the actual binary log, so byte 0 of the raw response is
///     NOT the start of the binary packet -- this must be located by scanning for the sync bytes.
/// @return BLOB_ERR_OK on success (sync found), BLOB_ERR_BESTXYZB_SYNC_NOT_FOUND
///     if the sync sequence was not found in the response.
static GNSS_ring_blob_error_enum_t extract_bestxyzb_binary(
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
            return BLOB_ERR_OK;
        }
    }

    return BLOB_ERR_BESTXYZB_SYNC_NOT_FOUND;
}

/// @brief Read a little-endian uint32 out of a byte buffer.
static uint32_t read_le_uint32(const uint8_t *buf) {
    return (uint32_t)buf[0]
        | ((uint32_t)buf[1] << 8)
        | ((uint32_t)buf[2] << 16)
        | ((uint32_t)buf[3] << 24);
}

/// @brief Decide whether an extracted BESTXYZB record is worth keeping.
/// @details We only store fixes that the receiver actually solved: the solution status must be
///     SOL_COMPUTED, the position type must not be NONE, and the 24 position bytes must not be
///     all zero (an "empty" fix). This keeps the ring full of usable data rather than of the
///     receiver's warm-up chatter, which matters a lot given we duty-cycle its power.
/// @return true if the fix should be stored.
static bool is_good_nonempty_fix(const uint8_t sample[GNSS_SAMPLE_SIZE]) {
    // The body begins after the variable-length header, whose length is byte 3 of the frame.
    const uint8_t header_len = sample[GNSS_BINARY_HEADER_LEN_OFFSET];
    if ((uint32_t)header_len + GNSS_BESTXYZ_MIN_BODY_LEN > GNSS_SAMPLE_SIZE) {
        return false; // Nonsense header length; can't trust this record.
    }

    const uint8_t *body = &sample[header_len];

    const uint32_t sol_status = read_le_uint32(&body[GNSS_BESTXYZ_SOL_STATUS_OFFSET]);
    if (sol_status != GNSS_SOL_STATUS_SOL_COMPUTED) {
        return false;
    }

    const uint32_t pos_type = read_le_uint32(&body[GNSS_BESTXYZ_POS_TYPE_OFFSET]);
    if (pos_type == GNSS_POS_TYPE_NONE) {
        return false;
    }

    // Reject an all-zero position, which a healthy-looking header can still carry.
    for (uint16_t i = 0; i < GNSS_BESTXYZ_POS_XYZ_LEN; i++) {
        if (body[GNSS_BESTXYZ_POS_XYZ_OFFSET + i] != 0) {
            return true;
        }
    }
    return false;
}

/// @brief Synthesize a plausible-looking BESTXYZB record from the hardware RNG, for FAKE mode.
/// @details The record is built to pass is_good_nonempty_fix(), so the whole storage/downlink
///     path can be exercised on the bench with no GNSS receiver attached (and no EPS).
static void generate_fake_sample(uint8_t out_sample[GNSS_SAMPLE_SIZE]) {
    memset(out_sample, 0, GNSS_SAMPLE_SIZE);

    // Header: sync bytes, a standard 28-byte header length, and a body length covering the fields
    // we care about.
    out_sample[0] = GNSS_BINARY_SYNC_0;
    out_sample[1] = GNSS_BINARY_SYNC_1;
    out_sample[2] = GNSS_BINARY_SYNC_2;
    const uint8_t header_len = 28;
    out_sample[GNSS_BINARY_HEADER_LEN_OFFSET] = header_len;
    const uint16_t body_len = GNSS_SAMPLE_SIZE - header_len - (uint16_t)GNSS_BINARY_CRC_LEN;
    out_sample[GNSS_BINARY_BODY_LEN_OFFSET] = (uint8_t)(body_len & 0xFF);
    out_sample[GNSS_BINARY_BODY_LEN_OFFSET + 1] = (uint8_t)((body_len >> 8) & 0xFF);

    uint8_t *body = &out_sample[header_len];

    // Solution status SOL_COMPUTED (0) and position type 16 (SINGLE), so it reads as a real fix.
    body[GNSS_BESTXYZ_POS_TYPE_OFFSET] = 16;

    // Random, definitely-non-zero position bytes.
    for (uint16_t i = 0; i < GNSS_BESTXYZ_POS_XYZ_LEN; i += 4) {
        const uint32_t rand_val = CRYPTO_generate_random_uint32(TIME_uptime_ms() + i);
        body[GNSS_BESTXYZ_POS_XYZ_OFFSET + i] = (uint8_t)(rand_val & 0xFF);
        body[GNSS_BESTXYZ_POS_XYZ_OFFSET + i + 1] = (uint8_t)((rand_val >> 8) & 0xFF);
        body[GNSS_BESTXYZ_POS_XYZ_OFFSET + i + 2] = (uint8_t)((rand_val >> 16) & 0xFF);
        // Keep the high byte small so the value reads as a sane-magnitude double, and ensure at
        // least one byte of each coordinate is non-zero.
        body[GNSS_BESTXYZ_POS_XYZ_OFFSET + i + 3] = (uint8_t)(0x40 | (rand_val >> 28));
    }
}

/// @brief Sample the GNSS (or the RNG, in FAKE mode) and, if the fix is good and non-empty, store
///     it as a new record in the LittleFS ring.
/// @param use_fake_data If true, don't touch the GNSS UART; synthesize the sample from the RNG.
/// @return BLOB_ERR_OK if a sample was stored, otherwise the reason it wasn't.
static GNSS_ring_blob_error_enum_t sample_and_store_bestxyzb(bool use_fake_data) {
    uint8_t sample[GNSS_SAMPLE_SIZE];

    if (use_fake_data) {
        generate_fake_sample(sample);
    }
    else {
        // Early exit condition: If in firehose mode, we can't do this.
        if (GNSS_current_rx_mode == GNSS_RX_MODE_FIREHOSE_MODE) {
            return BLOB_ERR_FIREHOSE_MODE_ACTIVE;
        }

        const char cmd[] = "log bestxyzb once\n";
        const uint16_t cmd_len = strlen(cmd);

        uint8_t rx_buf[256];
        uint16_t rx_buf_len = 0;
        memset(rx_buf, 0, sizeof(rx_buf));

        const uint8_t gnss_status = GNSS_send_cmd_get_response_NEW(
            cmd, cmd_len,
            rx_buf, sizeof(rx_buf),
            &rx_buf_len,
            0 // KEEP null bytes -- this is a binary response.
        );
        if (gnss_status != 0) {
            LOG(
                LOG_SEVERITY_WARNING,
                "%s: GNSS_send_cmd_get_response_NEW() -> %d (%s)",
                BLOB_NAME, gnss_status, gnss_ring_blob_error_to_str(BLOB_ERR_GNSS_COMMS_FAILED)
            );
            g_state->gnss_fetch_failure_count++;
            return BLOB_ERR_GNSS_COMMS_FAILED;
        }

        const GNSS_ring_blob_error_enum_t extract_status = extract_bestxyzb_binary(rx_buf, rx_buf_len, sample);
        if (extract_status != BLOB_ERR_OK) {
            LOG(
                LOG_SEVERITY_WARNING,
                "%s: BESTXYZB binary sync (AA 44 12) not found in %d-byte GNSS response (%s)",
                BLOB_NAME, rx_buf_len, gnss_ring_blob_error_to_str(extract_status)
            );
            g_state->gnss_fetch_failure_count++;
            return extract_status;
        }
    }

    // Only good, non-empty fixes earn a slot in the ring.
    if (!is_good_nonempty_fix(sample)) {
        g_state->bad_fix_skipped_count++;
        return BLOB_ERR_BAD_FIX;
    }

    return store_sample_to_ring(sample);
}

/// @brief Re-sync the OBC clock from the GNSS, if it's been at least GNSS_TIME_SYNC_INTERVAL_MS
///     since the last successful sync, and then push the new time out to the EPS and the ADCS.
/// @param[out] eps_push_status_dest Result of EPS_set_eps_time_based_on_obc_time(), or
///     TIME_PUSH_NOT_ATTEMPTED if no sync happened this run. Never NULL.
/// @param[out] adcs_push_status_dest Result of ADCS_synchronize_unix_time(), or
///     TIME_PUSH_NOT_ATTEMPTED if no sync happened this run. Never NULL.
/// @return true if a sync was attempted AND succeeded. A failed push to the EPS or the ADCS does
///     NOT make this false: the OBC clock is correct either way, which is the sync's actual job.
static bool maybe_sync_time_from_gnss(uint8_t *eps_push_status_dest, uint8_t *adcs_push_status_dest) {
    *eps_push_status_dest = TIME_PUSH_NOT_ATTEMPTED;
    *adcs_push_status_dest = TIME_PUSH_NOT_ATTEMPTED;

    // Firehose mode owns the UART; don't interleave a time sync into it.
    if (GNSS_current_rx_mode == GNSS_RX_MODE_FIREHOSE_MODE) {
        return false;
    }

    // While the MPI is actively collecting science data, never step the OBC clock: MPI science
    // data is timestamped from that clock, so a jump mid-campaign would corrupt the timeline.
    // A due sync simply waits -- `last_time_sync_uptime_ms` isn't touched, so the sync stays due
    // and happens on the first run after the MPI goes idle. Like the downlink skip, this is
    // checked unconditionally, NOT gated behind the TRACK_MPI flag.
    if (is_mpi_active()) {
        return false;
    }

    const uint32_t now_ms = TIME_uptime_ms();
    if (
        (g_state->time_sync_count > 0)
        && ((now_ms - g_state->last_time_sync_uptime_ms) < GNSS_TIME_SYNC_INTERVAL_MS)
    ) {
        return false; // Synced recently enough.
    }

    const uint8_t sync_status = GNSS_set_obc_time_based_on_gnss_time_uart();
    if (sync_status != 0) {
        LOG(
            LOG_SEVERITY_WARNING,
            "%s: GNSS_set_obc_time_based_on_gnss_time_uart() -> %d",
            BLOB_NAME, sync_status
        );
        return false;
    }

    g_state->last_time_sync_uptime_ms = TIME_uptime_ms();
    g_state->time_sync_count++;

    // Propagate the freshly-disciplined OBC clock out to the two subsystems that keep clocks of
    // their own.
    //
    // The EPS push is what makes a GNSS sync stick: the firmware's background upkeep task
    // (subtask_sync_obc_time_based_on_eps_time(), every EPS_time_sync_period_sec, default 600s)
    // treats the EPS RTC as authoritative and sets the OBC clock BACK to it whenever the two
    // differ by more than EPS_max_time_deviation_for_sync_ms. Without this push, any correction we
    // make larger than that threshold gets reverted within ~10 minutes, the EPS RTC's drift is
    // never actually disciplined, and the two syncs fight each other forever.
    //
    // The ADCS push keeps the ADCS's own log/telemetry timestamps aligned with the OBC's, as well
    // as orbit propagation.
    //
    // Neither failure invalidates the sync -- the OBC clock is already set -- so we record the
    // statuses for the response string, log them, and carry on.
    *eps_push_status_dest = EPS_set_eps_time_based_on_obc_time();
    if (*eps_push_status_dest != 0) {
        LOG(
            LOG_SEVERITY_WARNING,
            "%s: EPS_set_eps_time_based_on_obc_time() -> %d",
            BLOB_NAME, *eps_push_status_dest
        );
    }

    *adcs_push_status_dest = ADCS_synchronize_unix_time();
    if (*adcs_push_status_dest != 0) {
        LOG(
            LOG_SEVERITY_WARNING,
            "%s: ADCS_synchronize_unix_time() -> %d",
            BLOB_NAME, *adcs_push_status_dest
        );
    }

    return true;
}


// MARK: Downlink

/// @brief Downlink a randomly-chosen run of consecutive stored samples: pick a random ring file
///     that holds data, pick a random record index within it, and send that record plus up to
///     `downlink_n` more consecutive records from the same file.
/// @details Consecutive (rather than v1's independent random picks) so the ground receives short
///     contiguous arcs of the orbit, which are far more useful for fitting than scattered points.
///     The run stops at the end of the file rather than continuing into the next one, because
///     adjacent ring files aren't necessarily adjacent in time once the ring has wrapped.
///
///     The ring file we currently hold open for appending is never a candidate: we never read a
///     file we're writing to. Its records are only partly committed to flash anyway, and they
///     become downlinkable as soon as it fills up and is closed.
/// @param downlink_n Number of ADDITIONAL records to send after the randomly-chosen first one.
/// @param sent_count_dest Set to the number of packets actually sent. Never NULL.
/// @return Number of downlink failures (0 = all succeeded).
static uint16_t downlink_consecutive_samples(uint16_t downlink_n, uint16_t *sent_count_dest) {
    *sent_count_dest = 0;

    // Collect the closed ring files that currently hold at least one record.
    const bool write_file_is_open = ring_open_file_is_live();
    uint8_t candidate_files[GNSS_RING_FILE_COUNT];
    uint16_t candidate_counts[GNSS_RING_FILE_COUNT];
    uint8_t candidate_total = 0;
    for (uint8_t file_idx = 0; file_idx < GNSS_RING_FILE_COUNT; file_idx++) {
        if (write_file_is_open && (file_idx == g_state->open_file_idx)) {
            continue; // Skip the file we're appending to; see this function's @details.
        }

        const uint16_t count = ring_file_record_count(file_idx);
        if (count > 0) {
            candidate_files[candidate_total] = file_idx;
            candidate_counts[candidate_total] = count;
            candidate_total++;
        }
    }

    if (candidate_total == 0) {
        return 0; // Nothing stored yet; caller reports BLOB_ERR_NO_STORED_DATA.
    }

    // Pick a random file that has data, then a random starting record within it.
    const uint32_t file_rand = CRYPTO_generate_random_uint32(TIME_uptime_ms());
    const uint8_t chosen_slot = (uint8_t)(file_rand % candidate_total);
    const uint8_t chosen_file_idx = candidate_files[chosen_slot];
    const uint16_t records_in_file = candidate_counts[chosen_slot];

    const uint32_t record_rand = CRYPTO_generate_random_uint32(TIME_uptime_ms() + 1);
    const uint16_t start_record_idx = (uint16_t)(record_rand % records_in_file);

    // Send the starting record plus up to `downlink_n` more, stopping at the end of the file.
    const uint32_t requested_total = 1u + (uint32_t)downlink_n;
    const uint32_t available_total = (uint32_t)(records_in_file - start_record_idx);
    const uint16_t n_to_downlink = (uint16_t)(
        (requested_total < available_total) ? requested_total : available_total
    );

    char path[GNSS_RING_PATH_MAX_LEN];
    make_ring_file_path(chosen_file_idx, path, sizeof(path));

    // Open once and read consecutively, rather than re-opening the file per record.
    lfs_file_t file;
    if (lfs_file_open(&LFS_filesystem, &file, path, LFS_O_RDONLY) < 0) {
        LOG(LOG_SEVERITY_WARNING, "%s: lfs_file_open(%s) failed", BLOB_NAME, path);
        return n_to_downlink;
    }

    if (lfs_file_seek(&LFS_filesystem, &file, (lfs_soff_t)start_record_idx * GNSS_SAMPLE_SIZE, LFS_SEEK_SET) < 0) {
        LOG(LOG_SEVERITY_WARNING, "%s: lfs_file_seek(%s) failed", BLOB_NAME, path);
        lfs_file_close(&LFS_filesystem, &file);
        return n_to_downlink;
    }

    uint16_t fail_count = 0;
    for (uint16_t i = 0; i < n_to_downlink; i++) {
        GNSS_bestxyzb_downlink_packet_t packet;
        packet.packet_type = COMMS_PACKET_TYPE_GNSS_BESTXYZB_SAMPLE;
        packet.downlink_seq_num = g_state->downlink_seq_num;
        packet.file_idx = chosen_file_idx;
        packet.record_idx = (uint16_t)(start_record_idx + i);

        const lfs_ssize_t read_len = lfs_file_read(
            &LFS_filesystem, &file, packet.bestxyzb_data, GNSS_SAMPLE_SIZE
        );
        if (read_len != GNSS_SAMPLE_SIZE) {
            LOG(
                LOG_SEVERITY_WARNING,
                "%s: short read (%ld) at %s record %d",
                BLOB_NAME, (long)read_len, path, packet.record_idx
            );
            fail_count += (uint16_t)(n_to_downlink - i); // The rest of the run is unreachable too.
            break;
        }

        g_state->downlink_seq_num++;

        const uint8_t tx_status = AX100_downlink_bytes((uint8_t *)&packet, sizeof(packet));
        if (tx_status != 0) {
            fail_count++;
        }
        else {
            (*sent_count_dest)++;
        }
    }

    lfs_file_close(&LFS_filesystem, &file);
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
    char arg2_flags[64];

    pos = parse_token(args_str, pos, args_str_len, arg0_repeat_interval_ms, sizeof(arg0_repeat_interval_ms));
    pos = parse_token(args_str, pos, args_str_len, arg1_downlink_n, sizeof(arg1_downlink_n));
    parse_token(args_str, pos, args_str_len, arg2_flags, sizeof(arg2_flags)); // Optional; may be "".

    const bool flag_stop = has_flag(arg2_flags, "STOP");
    const bool flag_resume = has_flag(arg2_flags, "RESUME");
    const bool flag_fake = has_flag(arg2_flags, "FAKE");
    const bool flag_track_mpi = has_flag(arg2_flags, "TRACK_MPI");
    const bool flag_no_eps = has_flag(arg2_flags, "NOEPS");

    if (arg0_repeat_interval_ms[0] == '\0' || arg1_downlink_n[0] == '\0') {
        snprintf(
            response_buf, response_buf_len, "%s error: missing args! (%s)",
            BLOB_NAME, gnss_ring_blob_error_to_str(BLOB_ERR_MISSING_ARGS)
        );
        return BLOB_ERR_MISSING_ARGS;
    }

    bool arg0_ok, arg1_ok;
    int32_t repeat_interval_ms = parse_int(arg0_repeat_interval_ms, &arg0_ok);
    const int32_t downlink_n = parse_int(arg1_downlink_n, &arg1_ok);

    if (!arg0_ok || !arg1_ok) {
        snprintf(
            response_buf, response_buf_len, "%s error: invalid int args! (%s)",
            BLOB_NAME, gnss_ring_blob_error_to_str(BLOB_ERR_INVALID_INT_ARGS)
        );
        return BLOB_ERR_INVALID_INT_ARGS;
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
            "%s error: cancel_other_scheduled_reruns_of_this_blob() -> %d (%s)",
            BLOB_NAME, cancel_result, gnss_ring_blob_error_to_str(BLOB_ERR_CANCEL_RERUNS_FAILED)
        );
        return BLOB_ERR_CANCEL_RERUNS_FAILED;
    }
    else if (cancel_result > 0) {
        snprintf(cancel_msg, sizeof(cancel_msg), ", %d duplicate rerun(s) cancelled", cancel_result);
    }
    else {
        cancel_msg[0] = '\0';
    }

    // The filesystem must already be mounted; this blob never mounts it. An unmounted filesystem
    // is a serious anomaly for a blob whose whole job is storage, and remounting underneath it
    // would both hide that and invalidate the ring file handle we hold in SRAM. Reruns were
    // cancelled above and we return without rescheduling, so the campaign simply stops. Not
    // latched into is_permanently_stopped: re-running the blob restarts it, no RESUME needed.
    if (!LFS_is_lfs_mounted) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: LittleFS not mounted (%s). Not rescheduling.%s",
            BLOB_NAME, gnss_ring_blob_error_to_str(BLOB_ERR_LFS_NOT_MOUNTED), cancel_msg
        );
        return BLOB_ERR_LFS_NOT_MOUNTED;
    }

    // Initialize the persistent state on true cold-start (first ever run, or after a full
    // power-cycle that cleared SRAM). Otherwise, its contents survive from the previous execution
    // of this blob. We deliberately do NOT measure the on-disk files to recover the write cursor:
    // a cold-started ring simply starts over at r0.bin/record 0, truncating each file as it first
    // writes to it. The memset also clears any stale `open_file` handle, which is essential --
    // after a power cycle it would point into a filesystem/heap that no longer exists.
    if (g_state->magic != GNSS_RING_STATE_MAGIC) {
        memset(g_state, 0, sizeof(GNSS_ring_state_t));
        g_state->magic = GNSS_RING_STATE_MAGIC;
    }

    // Make sure "gnss_ring/" exists before anything tries to read or write inside it.
    const GNSS_ring_blob_error_enum_t mkdir_status = ensure_ring_dir_exists();
    if (mkdir_status != BLOB_ERR_OK) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: couldn't create %s/ (%s)%s",
            BLOB_NAME, GNSS_RING_DIR, gnss_ring_blob_error_to_str(mkdir_status), cancel_msg
        );
        return mkdir_status;
    }

    // RESUME clears a previously-latched STOP. Checked before STOP so that passing both is a no-op
    // rather than an unstoppable blob.
    if (flag_resume) {
        g_state->is_permanently_stopped = 0;
    }

    // STOP: latch the persistent stop flag, shed the GNSS, and exit without rescheduling. Reruns
    // were already cancelled above, so this run is the last one.
    if (flag_stop) {
        g_state->is_permanently_stopped = 1;
        ring_close_open_file(); // Commit whatever is pending: nothing will reopen this file.
        if (!flag_no_eps) {
            EPS_set_channel_enabled(EPS_CHANNEL_3V3_GNSS, 0);
            g_state->gnss_channel_is_on = 0;
        }
        LOG(LOG_SEVERITY_NORMAL, "%s: STOP flag received; blob permanently stopped", BLOB_NAME);
        snprintf(
            response_buf, response_buf_len,
            "%s: STOP received. GNSS off, reruns cancelled, blob permanently stopped "
            "(pass RESUME to restart). stored_records=%lu%s",
            BLOB_NAME, (unsigned long)g_state->stored_record_count, cancel_msg
        );
        return BLOB_ERR_OK; // A requested stop is a success, not an error.
    }

    // A previously-latched STOP keeps us dead across invocations until someone passes RESUME.
    if (g_state->is_permanently_stopped) {
        snprintf(
            response_buf, response_buf_len,
            "%s: permanently stopped (%s); pass RESUME to restart. Not rescheduling.%s",
            BLOB_NAME, gnss_ring_blob_error_to_str(BLOB_ERR_PERMANENTLY_STOPPED), cancel_msg
        );
        return BLOB_ERR_PERMANENTLY_STOPPED;
    }

    // Decide and apply the GNSS power state for this run.
    // In FAKE mode we never touch the EPS or the GNSS at all, so the power policy is skipped.
    uint8_t gnss_is_on = 0;
    int16_t vbatt_mV = 0;
    const char *power_reason = "fake_mode";
    if (flag_fake) {
        gnss_is_on = 1; // Pretend, so the sampling path below runs.
    }
    else {
        const GNSS_power_decision_enum_t decision = decide_gnss_power(
            flag_track_mpi, &vbatt_mV, &power_reason
        );
        const GNSS_ring_blob_error_enum_t power_status = apply_gnss_power_decision(
            decision, flag_no_eps, &gnss_is_on
        );
        if (power_status != BLOB_ERR_OK) {
            // Couldn't talk to the EPS. Don't sample (we don't know if the GNSS even has power),
            // but keep the blob alive: the next run re-evaluates.
            gnss_is_on = 0;
        }
    }

    // Sample the GNSS, if it's powered and has had time to boot.
    GNSS_ring_blob_error_enum_t sample_status = BLOB_ERR_GNSS_POWERED_OFF;
    bool did_time_sync = false;
    uint8_t eps_time_push_status = TIME_PUSH_NOT_ATTEMPTED;
    uint8_t adcs_time_push_status = TIME_PUSH_NOT_ATTEMPTED;
    if (gnss_is_on) {
        const uint32_t ms_since_power_on = TIME_uptime_ms() - g_state->gnss_power_on_uptime_ms;
        if ((!flag_fake) && (ms_since_power_on < GNSS_POWER_ON_SETTLE_MS)) {
            // The receiver was just switched on and is still booting; give it until the next run.
            sample_status = BLOB_ERR_GNSS_WARMING_UP;
        }
        else {
            sample_status = sample_and_store_bestxyzb(flag_fake);

            // Keep the OBC clock disciplined while we have the receiver powered anyway.
            if (!flag_fake) {
                did_time_sync = maybe_sync_time_from_gnss(
                    &eps_time_push_status, &adcs_time_push_status
                );
            }
        }
    }

    // Downlink a random consecutive run of stored samples, regardless of whether the GNSS is
    // powered this run -- the history is on disk and is worth sending down either way.
    //
    // Exception: while the MPI is actively collecting science data, keep sampling and storing, but
    // stay off the radio entirely, so this blob's downlink doesn't compete with the science
    // campaign. The samples aren't lost -- they're on disk, and go down on a later run once the
    // MPI is idle. Note this is checked unconditionally, NOT gated behind the TRACK_MPI flag:
    // TRACK_MPI only governs whether MPI activity forces the GNSS *on*.
    const bool skip_downlink_for_mpi = is_mpi_active();
    uint16_t downlink_sent_count = 0;
    uint16_t downlink_fail_count = 0;
    if (!skip_downlink_for_mpi) {
        downlink_fail_count = downlink_consecutive_samples(
            (uint16_t)downlink_n, &downlink_sent_count
        );
    }

    if (repeat_interval_ms <= 0) {
        // One-shot run: no later execution will ever come back to flush or close the write file,
        // so commit it now rather than leaving the handle (and its cache buffer) dangling.
        ring_close_open_file();
    }
    else {
        const GNSS_ring_blob_error_enum_t reexec_result = reschedule_current_blob_tcmd((uint32_t)repeat_interval_ms);
        if (reexec_result != BLOB_ERR_OK) {
            ring_close_open_file(); // No rerun is coming to close it; commit what we have.
            snprintf(
                response_buf, response_buf_len,
                "%s error: reschedule_current_blob_tcmd() -> %s%s",
                BLOB_NAME, gnss_ring_blob_error_to_str(reexec_result), cancel_msg
            );
            return reexec_result;
        }
    }

    char downlink_msg[24];
    if (skip_downlink_for_mpi) {
        snprintf(downlink_msg, sizeof(downlink_msg), "skipped(mpi_active)");
    }
    else {
        snprintf(
            downlink_msg, sizeof(downlink_msg), "%d/%d",
            downlink_sent_count, downlink_sent_count + downlink_fail_count
        );
    }

    // On a sync run, report the status of each downstream time push (0 = accepted). On every
    // other run nothing was pushed, so this stays empty.
    char time_push_msg[40];
    if (did_time_sync) {
        snprintf(
            time_push_msg, sizeof(time_push_msg), ", eps_time=%d, adcs_time=%d",
            eps_time_push_status, adcs_time_push_status
        );
    }
    else {
        time_push_msg[0] = '\0';
    }

    snprintf(
        response_buf, response_buf_len,
        "%s: gnss=%s (%s, vbatt=%dmV), sample=%s, cursor=r%d/%d%s, stored=%lu, "
        "bad_fixes=%lu, fetch_fails=%lu, sync=%s(%lu)%s, sent=%s%s",
        BLOB_NAME,
        gnss_is_on ? "ON" : "OFF", power_reason, vbatt_mV,
        gnss_ring_blob_error_to_str(sample_status),
        g_state->write_file_idx, g_state->write_record_idx,
        g_state->has_wrapped ? " (wrapped)" : "",
        (unsigned long)g_state->stored_record_count,
        (unsigned long)g_state->bad_fix_skipped_count,
        (unsigned long)g_state->gnss_fetch_failure_count,
        did_time_sync ? "yes" : "no", (unsigned long)g_state->time_sync_count,
        time_push_msg,
        downlink_msg,
        cancel_msg
    );

    // Report the most useful non-fatal condition, in priority order. All of these still mean "the
    // blob ran fully and rescheduled itself", so none of them stop the collection campaign.
    if (downlink_fail_count > 0) {
        LOG(
            LOG_SEVERITY_WARNING,
            "%s: downlink_consecutive_samples() -> %d failures (%s)",
            BLOB_NAME, downlink_fail_count, gnss_ring_blob_error_to_str(BLOB_ERR_DOWNLINK_PARTIAL_FAILURE)
        );
        return BLOB_ERR_DOWNLINK_PARTIAL_FAILURE;
    }
    if (sample_status != BLOB_ERR_OK) {
        return 100 + sample_status; // Non-fatal: no sample stored this run, but we still ran fully.
    }
    if (skip_downlink_for_mpi) {
        return BLOB_ERR_DOWNLINK_SKIPPED_MPI_ACTIVE; // Deliberate radio silence, not a failure.
    }
    if ((downlink_sent_count == 0) && (g_state->stored_record_count == 0)) {
        return BLOB_ERR_NO_STORED_DATA; // Normal early in a campaign, before the first good fix.
    }

    return BLOB_ERR_OK;
}
