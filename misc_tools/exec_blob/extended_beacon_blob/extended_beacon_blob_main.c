// This is a blob (executable) that emits extended beacons with many extra peripheral fields.
//
// Motivation: The existing FrontierSat beacon is great, but lacks certain data (e.g., ADCS data
// and per-channel EPS data especially).
// This blob is a new feature that allows for sending additional data in the beacon packets.
//
// Since v5, every beacon is also stored into a ring of files in LittleFS, and each run downlinks
// a randomly-selected consecutive run of stored (historical) beacons.
// This lets the ground slowly reconstruct the full beacon history (including the parts
// of the orbit with no ground station coverage) over many random SatNOGS passes.
//
// Args Format: <repeat_interval_ms>[;<history_count>][;kwarg1=val;kwarg2=val]
// - repeat_interval_ms: 0 to run only once, or any positive number to run repeatedly at that
//   specified interval (clamped to a minimum of 2500ms).
// - history_count: Optional (default 0). Number of consecutive historical beacons to downlink per
//   run, after the live beacon (clamped to a maximum of 50). Fewer are sent if the randomly-chosen
//   start lands near the end of the chosen ring file. More are sent if the run reaches a beacon
//   stored while the MPI was sensing (see note 14).
// Supported kwargs:
// - mpi_interval_ms: Repeat interval while the MPI is in active (sensing) mode (default 1500,
//   clamped to a minimum of 1000). Only used if shorter than repeat_interval_ms. See note 13.
//
// Usage Example:
// After uplinking the blob as "blobs/extended_beacon_v6.blob", run:
// CTS1+exec_blob_from_fs(blobs/extended_beacon_v6.blob,0,9000;5)!
// With a 9 second interval, ~8h of beacons are kept in the ring buffer.
// To store beacons every 2 seconds (instead of the default 1.5 s) while the MPI is sensing:
// CTS1+exec_blob_from_fs(blobs/extended_beacon_v6.blob,0,9000;5;mpi_interval_ms=2000)!
//
// Notes:
//  1. Always use "0" as the second argument (i.e., always run with malloc).
//  2. (v5+) The ADCS is probed with a single request first. If it doesn't respond (e.g., dev kit,
//      or ADCS powered off), all ADCS requests are skipped and the ADCS fields are left as 0.
//      Previously, each ADCS request took ~3.5 s to fail, which hit the watchdog.
//  3. This blob re-schedules itself at the specified interval. Each new scheduled telecommand gets
//      a tssent value of <interval_ms> after the beacon is sent.
//  4. A run does NOT reschedule itself if another pending run of this blob family (any file named
//      `extended_beacon_v*`, so any version, with any args) is due within the next 2*<interval_ms>;
//      that upcoming run takes over instead. Pending
//      runs further out are left alone. So you can schedule several reconfigurations ahead of time
//      (e.g., different intervals at different times), and each one hands off to the next.
//      Caveat: re-running this blob immediately while it's already repeating does NOT replace the
//      running chain -- the new run sees the chain's next rerun and yields to it. To change the
//      settings immediately, first `CTS1+agenda_delete_by_name(exec_blob_from_fs)`.
//  5. To stop the recurring rescheduling of this blob after starting it, you can use reboot, or
//      use `CTS1+agenda_delete_by_name(exec_blob_from_fs)`, or `CTS1+agenda_delete_all()`, or
//      schedule a `CTS1+exec_blob_from_fs(blobs/extended_beacon_v6.blob,0,0)!` (run-once mode,
//      which doesn't reschedule) for when you want the chain to end.
//  6. The beacon is always sent first. Storage and history downlink problems (e.g., filesystem
//      not mounted) never prevent the live beacon from being sent, nor the blob from rescheduling.
//  7. While the MPI is in active (sensing) mode, the live beacon is still sent and stored, but no
//      historical beacons are downlinked that run.
//  8. Nothing historical is downlinked until the first ring file fills (BEACON_RING_RECORDS_PER_FILE
//      beacons), because the file being written is never downlinked.
//  9. Historical beacons are sent with the same packet type, but with version string " S6\0"
//      (instead of " X6\0"). (v5 used " S5\0" and " X5\0".)
// 10. (v5+) Beacons are stored in LittleFS under `beacon_ring/`, as a ring of `BEACON_RING_FILE_COUNT` = 20 files
//     (`beacon_ring/r0.bin` .. `beacon_ring/r19.bin`), each holding `BEACON_RING_RECORDS_PER_FILE` = 180
//     fixed-size records (one raw 198-byte beacon each). When the current file fills, the blob advances to
//     the next; after the last one it wraps back to `r0` and truncates it, evicting the oldest data.
// 11. (v5+) Any reboot (software reboot, watchdog reset, or power cycle), an unmounted filesystem,
//     or a filesystem remount/reformat resets the in-RAM ring state. Software reboots are detected
//     with a heap canary. (v6+) The ring is then recovered from the filesystem (via
//     `beacon_ring/cursor.bin`), so stored history survives reboots. The file being written at the
//     reboot is restarted (its records are lost). If anything about the stored files looks wrong,
//     they're all discarded, and the ring starts fresh at r0 with no history available.
// 12. (v6+) No-uplink safety net: once time since the last uplink exceeds
//     `STM32_system_reset_no_uplink_interval_sec` minus 5 minutes, each run (after sending and storing the
//     beacon) commits the ring file and sends `EPS_CMD_system_reset()`, power-cycling the whole satellite
//     before the firmware's native OBC-only reboot would fire. Cannot be disabled in this blob:
//     if that config is 0 (or <= 5 min), reset is performed immediately.
// 13. (v6+) MPI fast mode: while the MPI is in active (sensing) mode, the blob reschedules itself
//     every `mpi_interval_ms` (default 1500 ms) instead, so beacons are stored to the ring faster.
//     The live beacon is NOT downlinked on every fast run: only once roughly every
//     <repeat_interval_ms> (to keep the radio quiet for the MPI, and leave room for uplinks).
//     Note: the ring holds 3600 beacons, so at 1.5 s it only spans 90 minutes of MPI operation.
// 14. (v6+) If a history run reaches a stored beacon with `mpi_rx_mode_enum` = sensing, the run is
//     extended to 50 beacons total (or the end of that ring file, if sooner), even past history_count.
//     This prioritizes downlinking the beacons stored while the MPI was active.
//
// ------ Implementation Notes ----------------
//
// Storage: Beacons live in LittleFS under "beacon_ring/", as a ring of BEACON_RING_FILE_COUNT
// files ("beacon_ring/r0.bin" .. "beacon_ring/r19.bin"), each holding BEACON_RING_RECORDS_PER_FILE
// fixed-size records (one raw COMMS_beacon_extended_packet_v5plus_t each). When the current file fills,
// we advance to the next; after the last one we wrap back to r0 and truncate it, evicting the
// oldest data.
//
// Persistent RAM: Same approach as the GNSS ring blob. The write cursor lives in a small state
// block at a fixed SRAM address (see `BEACON_RING` in ../blobs_persistent_sram_regions.ld; separate
// from the GNSS ring blob's region, so both blobs can run at the same time). A magic number detects cold start,
// in which case the cursor is recovered from the filesystem (see below).
//
// Recovery (v6+): Each time a ring file is started (record 0), the cursor (file index + wrapped
// flag) is first written to "beacon_ring/cursor.bin". After any reset of the in-RAM state, the
// first run with the filesystem mounted reads it back, and checks that every file it says is full
// is exactly full and has valid-looking first/last records. Only then is the cursor restored (at
// record 0 of the file it names, which gets truncated). Any doubt means a fresh start at r0.
//
// Open file handle: Same as the GNSS ring blob, the ring file we're currently appending to is left
// OPEN between executions (its lfs_file_t lives in the persistent state block), so storing a
// record is just an lfs_file_write(). Closes (which commit metadata to flash) are kept rare, to
// limit flash wear: the file is only closed when it fills up, or when the blob exits without
// rescheduling itself. The handle is only trusted after confirming it's still linked into the
// mounted filesystem's open-file list (see ring_open_file_is_live()).
//
// Reboots: Same as the GNSS ring blob, a reboot that leaves the SRAM state block intact is
// detected with a heap canary (see detect_reboot_via_heap_canary()), independently of the
// open-file list check.
//
// Four things mean "start over", and all four take the same path (reset_to_fresh_state()), same
// as the GNSS ring blob: cold/garbage SRAM, a reboot caught by the heap canary, an unmounted
// filesystem, and a ring file handle the mounted filesystem doesn't recognise (remount/reformat
// without a reboot). Each resets the write cursor to r0.bin record 0 and zeroes the counters, so
// the ring refills from scratch, and the old files are excluded from downlink. In none of the
// four is the handle trusted, so it's dropped without being closed, and its uncommitted records
// are simply lost. Each then tries to recover the cursor from the filesystem (see Recovery above). Unlike the GNSS ring blob, an unmounted filesystem doesn't stop this blob: the
// live beacon is still sent, and the blob still reschedules itself.
//
// On (re)opening a part-full file (e.g., after a run-once exit closed it), we seek to the
// cursor's exact offset (record_idx * record_size), so the file layout always matches the cursor.

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>

#include "../lfs.h"

// Beacon v1 contents (includes).
#include "eps_drivers/eps_types.h"
#include "comms_drivers/rf_antenna_switch.h"
#include "timekeeping/timekeeping.h"
#include "telecommand_exec/telecommand_executor.h"
#include "telecommand_exec/telecommand_definitions.h"
#include "telecommand_exec/telecommand_args_helpers.h"
#include "mpi/mpi_types.h"
#include "mpi/mpi_command_handling.h"
#include "rtos_tasks/rtos_bootup_operation_fsm_task.h"
#include "gnss_receiver/gnss_internal_drivers.h"
#include "eps_drivers/eps_calculations.h"
#include "eps_drivers/eps_commands.h"
#include "stm32/stm32_reboot_reason.h"
#include "obc_systems/external_led_and_rbf.h"
#include "comms_drivers/ax100_tx.h"
#include "comms_drivers/comms_tx.h"

// Beacon v2 contents (includes).
#include "obc_systems/adc_vbat_monitor.h"
#include "adcs_drivers/adcs_types.h"
#include "mpi/mpi_types.h"

// Beacon v5 contents (includes).
#include "crypto/random_number_generator.h"
#include "crypto/sha256.h"
#include "comms_drivers/bulk_file_downlink.h"
#include "../blob_libs/eps_piu_fixed.h"
#include "../blob_libs/agenda_self_reschedule.h"

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
static const char BLOB_NAME[] = "extended_beacon_blob_v6";
static const char BLOB_FAMILY_PREFIX[] = "extended_beacon_v"; // Matches all versions' file names, for self-rescheduling handoff.

// Global variables defined in the firmware ELF (CTS-SAT-1_FW_rc3.elf).
extern lfs_t LFS_filesystem;
extern volatile uint32_t TIME_uptime_ms_from_tim6;

// Beacon v1 contents (extern variables).
extern uint32_t AX100_uptime_at_last_received_kiss_tcmd_ms;
extern uint32_t STM32_system_reset_no_uplink_interval_sec; // Firmware's no-uplink reboot threshold (config var).
extern void HAL_Delay(uint32_t Delay);
extern uint8_t LFS_is_lfs_mounted;
extern uint32_t COMMS_total_beacon_count_since_boot;
extern MPI_transceiver_state_enum_t MPI_current_transceiver_state;
extern MPI_reason_for_stopping_active_mode_enum_t MPI_last_reason_for_stopping_active_mode;
extern volatile uint8_t UART_gnss_uart_interrupt_enabled; // Flag to enable or disable the UART GNSS ISR

// Beacon v2 contents (extern variables).
extern uint32_t SystemCoreClock;
extern const uint16_t MPI_science_buffer_len;
extern volatile uint8_t MPI_science_buffer_one[];
extern volatile uint8_t MPI_science_buffer_two[];


// Beacon v1 contents (extern functions).
extern int32_t OBC_TEMP_SENSOR_get_temperature_cC();

/// Beacon v2 contents (extern functions).
// void DEBUG_uart_print_str(const char *str);
extern uint8_t ADCS_i2c_request_telemetry_and_check(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);
extern uint8_t ADCS_send_i2c_telemetry_request(uint8_t id, uint8_t* data, uint32_t data_length, uint8_t include_checksum);
extern uint8_t ADCS_get_raw_coarse_sun_sensor_1_to_6(ADCS_raw_coarse_sun_sensor_1_to_6_struct_t *output_struct);
extern uint8_t ADCS_get_raw_coarse_sun_sensor_7_to_10(ADCS_raw_coarse_sun_sensor_7_to_10_struct_t *output_struct);

extern volatile uint32_t MPI_buffer_one_last_filled_uptime_ms;
extern volatile uint32_t MPI_buffer_two_last_filled_uptime_ms;

extern int snprintf(char *buf, unsigned int size, const char *fmt, ...);
extern int strlen(const char *s);
extern int strcmp(const char *s1, const char *s2);
extern void *memset(void *s, int c, size_t n);
extern void *memcpy(void *__restrict dest, const void *__restrict src, size_t n);
extern int memcmp(const void *s1, const void *s2, size_t n);

extern void LOG_message(
    uint32_t source, LOG_severity_enum_t severity, uint32_t sink_mask,
    const char *fmt, ...
);

extern void *pvPortMalloc(size_t xWantedSize);

// lfs_file_open/size/seek/read/write/close are already declared in lfs.h;
// their definitions are resolved against the firmware ELF at link time.

#define LOG(severity, fmt, ...) \
    LOG_message(LOG_SYSTEM_TELECOMMAND, severity, LOG_SINK_ALL, fmt, ##__VA_ARGS__)

// static void DEBUG_UART(const char *fmt) {
//     #if 1
//     DEBUG_uart_print_str(fmt);
//     DEBUG_uart_print_str("\n");
//     #endif
// }

extern int vsnprintf(char *buf, unsigned int size, const char *fmt, va_list ap);

/// @brief LOG() with the "<BLOB_NAME>: " prefix, kept in one non-inlined place to save flash
///     (each inlined LOG() call site costs ~40 bytes).
__attribute__((noinline, format(printf, 2, 3)))
static void blob_log(LOG_severity_enum_t severity, const char *fmt, ...) {
    char msg[128];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
    LOG(severity, "%s: %s", BLOB_NAME, msg);
}

// MARK: Error Enum

// All named status/error codes returned by this blob's functions, including the values
// `blob_main()` itself returns to the telecommand executor (0 = success).
typedef enum {
    BLOB_ERR_OK = 0,
    BLOB_ERR_LFS_WRITE_FAILED = 5, // Writing the beacon record to LittleFS failed (beacon still sent).
    BLOB_ERR_LFS_MKDIR_FAILED = 6, // Couldn't create/confirm the BEACON_RING_DIR directory.
    BLOB_ERR_LFS_NOT_MOUNTED = 7, // Filesystem not mounted; skipped storage and history downlink.
    BLOB_ERR_DOWNLINK_FAILED = 60, // AX100_downlink_bytes() failed to send the beacon packet.
    BLOB_ERR_HISTORY_SEND_PARTIAL_FAILURE = 61, // At least one historical beacon failed to send.
    BLOB_ERR_PERIPHERAL_COMMS_FAILURES = 117, // Beacon was sent, but >=1 peripheral query failed.
    BLOB_ERR_MISSING_ARGS = 135, // One or more required args_str tokens were empty.
    BLOB_ERR_INVALID_INT_ARGS = 136, // One or more args_str tokens failed integer parsing.
    BLOB_ERR_INVALID_KWARGS = 137, // A kwarg was unknown, malformed, or not a positive integer.
    BLOB_ERR_NO_EXECUTING_AGENDA_SLOT = 163, // Couldn't find our own agenda slot to reschedule.
    BLOB_ERR_AGENDA_ADD_FAILED = 164, // TCMD_add_tcmd_to_agenda() failed while rescheduling.
} BLOB_ext_beacon_error_enum_t;

/// @brief Convert a BLOB_ext_beacon_error_enum_t into a short human-readable name, for use in log
///     messages and output strings.
/// @return Static string; never NULL. "UNKNOWN_ERROR" for values not in the enum.
static const char *ext_beacon_blob_error_to_str(BLOB_ext_beacon_error_enum_t err) {
    switch (err) {
        case BLOB_ERR_OK: return "OK";
        case BLOB_ERR_LFS_WRITE_FAILED: return "LFS_WRITE_FAILED";
        case BLOB_ERR_LFS_MKDIR_FAILED: return "LFS_MKDIR_FAILED";
        case BLOB_ERR_LFS_NOT_MOUNTED: return "LFS_NOT_MOUNTED";
        case BLOB_ERR_DOWNLINK_FAILED: return "DOWNLINK_FAILED";
        case BLOB_ERR_HISTORY_SEND_PARTIAL_FAILURE: return "HISTORY_SEND_PARTIAL_FAILURE";
        case BLOB_ERR_PERIPHERAL_COMMS_FAILURES: return "PERIPHERAL_COMMS_FAILURES";
        case BLOB_ERR_MISSING_ARGS: return "MISSING_ARGS";
        case BLOB_ERR_INVALID_INT_ARGS: return "INVALID_INT_ARGS";
        case BLOB_ERR_INVALID_KWARGS: return "INVALID_KWARGS";
        case BLOB_ERR_NO_EXECUTING_AGENDA_SLOT: return "NO_EXECUTING_AGENDA_SLOT";
        case BLOB_ERR_AGENDA_ADD_FAILED: return "AGENDA_ADD_FAILED";
        default: return "UNKNOWN_ERROR";
    }
}


#pragma pack(push, 1)

typedef struct {
    // COMMS_packet_type_enum_t
    // COMMS_PACKET_TYPE_BEACON_EXTENDED for this packet (on v2, v3, and v4).
    // COMMS_PACKET_TYPE_BEACON_EXTENDED_V5PLUS for this packet (v5+).
    uint8_t packet_type;

    char satellite_name[4]; // 4 bytes: "CTS1" :)

    uint8_t active_rf_switch_antenna; // Either 1 or 2.
    uint8_t active_rf_switch_control_mode; // Enum: COMMS_rf_switch_control_mode_enum_t
    uint32_t uptime_ms;

    uint32_t duration_since_last_uplink_ms;
    uint64_t unix_epoch_time_ms;
    uint8_t last_time_sync_source_enum; // Enum: TIME_sync_source_enum_t

    uint8_t is_fs_mounted;

    uint16_t total_tcmd_queued_count;
    uint16_t pending_queued_tcmd_count;

    uint32_t total_beacon_count_since_boot;

    uint8_t eps_mode_enum; // 0=startup, 1=nominal, 2=safety, 3=emergency_low_power
    uint8_t eps_reset_cause_enum; // 0=power_on, 1=watchdog, 2=commanded, 3=control_system_reset, 4=emergency_low_power
    uint32_t eps_uptime_sec;
    uint16_t eps_error_code;
    uint16_t eps_battery_voltage_mV;
    uint8_t eps_battery_percent;
    int16_t eps_battery_temperature_0_cC; // Note: Defective in core FW. Fixed in this blob.
    int16_t eps_battery_temperature_1_cC;
    // Note: Only 2 out of 3 battery temperature sensors work on our model.
    int32_t eps_total_fault_count;
    uint32_t eps_enabled_channels_bitfield;
    int32_t eps_total_pcu_power_input_cW;
    int32_t eps_total_pcu_power_output_cW;
    int32_t eps_total_avg_pcu_power_input_cW;
    int32_t eps_total_avg_pcu_power_output_cW;

    int32_t obc_temperature_cC;

    uint8_t reboot_reason; // Enum: STM32_reset_cause_t
    
    uint8_t cts1_operation_state; // Enum: CTS1_operation_state_enum_t
    uint8_t rbf_pin_state; // Enum: OBC_rbf_state_enum_t

    uint8_t mpi_rx_mode_enum; // Enum: MPI_rx_mode_enum_t
    uint8_t mpi_transceiver_state_enum; // Enum: MPI_transceiver_state_enum_t

    uint8_t mpi_last_reason_for_stopping_enum; // Enum: MPI_reason_for_stopping_active_mode_enum_t
    
    uint8_t gnss_uart_interrupt_enabled;

    uint8_t gnss_rx_mode_enum; // Enum: GNSS_rx_mode_enum_t

    // On BEACON_BASIC and BEACON_EXTENDED (v2, v3, v4), null-terminated configurable friendly message.
    // char friendly_message[COMMS_BEACON_FRIENDLY_MESSAGE_SIZE]; // COMMS_BEACON_FRIENDLY_MESSAGE_SIZE = 42

    // On BEACON_EXTENDED_B (v5+), we replace the friendly message with more data.

    // The following fields are v5+.

    // Counts the distinct (tcmd_idx, tcmd_args) pairs in the pending queue, plus the
    // currently-executing tcmd (same as pending_queued_tcmd_count's +1). Equal to
    // pending_queued_tcmd_count when nothing is duplicated.
    // Useful to confirm that all uplinks were successful.
    uint8_t distinct_tcmds_pending;

    // Duration since time sync (clamped to max of 2^16 seconds = 18 hours)
    uint16_t duration_since_last_time_sync_sec;

    // Measured speed of each of the 3 reaction wheels (Telemetry ID 156), in RPM (signed).
    int16_t adcs_wheel_speed_x_rpm;
    int16_t adcs_wheel_speed_y_rpm;
    int16_t adcs_wheel_speed_z_rpm;

    // Rotation angle of the quaternion error vector (commanded vs. estimated attitude,
    // Telemetry ID 163), 0 to 18000. Only meaningful in control modes with a commanded attitude.
    uint16_t adcs_pointing_error_cdeg;

    // ADCS estimation innovation vector (Telemetry ID 162), in units of 0.0001.
    // Only meaningful in EKF estimation modes.
    int16_t adcs_innovation_vector_x_en4;
    int16_t adcs_innovation_vector_y_en4;
    int16_t adcs_innovation_vector_z_en4;

    // ADCS EKF estimated rate sensor (gyro) bias (Telemetry ID 161), in units of 0.001 deg/s.
    // Only meaningful in EKF estimation modes.
    int16_t adcs_estimated_gyro_bias_x_mdeg_per_sec;
    int16_t adcs_estimated_gyro_bias_y_mdeg_per_sec;
    int16_t adcs_estimated_gyro_bias_z_mdeg_per_sec;

    // GNSS EPS channel (CH8) power, from the PIU running average.
    uint8_t gnss_power_consumed_cW;

    // Sum of the ADCS's current telemetry (Telemetry ID 195) times rail voltages: CubeSense 1 and 2,
    // CubeControl (3V3, 5V, Vbat), CubeWheels, and CubeStar. Excludes CubeComputer (not measured).
    // See calculate_adcs_total_power_cW().
    uint16_t adcs_total_power_consumed_cW;

    // Instantaneous solar panel power measurements for the 5th conditioning chain (CC5), which
    // only the PIU housekeeping reports. Continues eps_pcu_ch0..ch3 above (same units/exclusions).
    int16_t eps_pcu_ch4_volt_in_mppt_mV;
    int16_t eps_pcu_ch4_curr_in_mppt_mA;
    int16_t eps_pcu_ch4_curr_ou_mppt_mA;

    // First 3 bytes of SHA256(file path) of the file currently being bulk downlinked (state
    // DOWNLINKING), in digest order. All zeros otherwise (IDLE or PAUSED).
    // The path is hashed exactly as given in the bulk downlink telecommand (no null terminator).
    // Ground: hashlib.sha256(path.encode()).digest()[:3].
    // Useful for attributing bulk file downlink chunks to a specific file.
    uint8_t bulk_downlink_file_path_sha256_prefix[3];

    // Unused (always 0). Pads the v5+ fields to exactly the size of the friendly message they
    // replace, so end_message and all the fields after it keep their v2-v4 offsets.
    uint8_t reserved_v5[7];

    // End v5+ fields.

    // "END\0" on basic packets.
    // " X2\0" on extended packets (v2).
    // " X3\0" on extended packets (v3).
    // " X4\0" on extended packets (v4).
    // " X6\0" on extended packets (v6; " X5\0" in v5).
    // " S6\0" on ring-stored extended packets (v6; " S5\0" in v5), when downlinked as history.
    char end_message[4];

    // ====== END OF BASIC BEACON PACKET (DUPLICATED) ========
    // MARK: Extended Fields

    // Active oscillator frequency, in MHz. 16 MHz = internal clock (HSI, imprecise), 25 MHz = external (HSE, precise).
    uint8_t obc_active_oscillator_MHz;

    // Battery voltage, as measured by the ADC on the OBC. Very useful when EPS fails to report
    // its battery voltage, as it often tends to do (due to ISISpace bug).
    int16_t obc_adc_battery_voltage_mV;

    // Average temperature of the MPI, when active, at the most recent time it was active.
    // Clamped to -90C to +125C.
    // -99 = MPI inactive.
    // -98 = error calculating temperature.
    // -97 = stale (v5+): most recent MPI data was received more than 30 minutes ago.
    int8_t mpi_last_temperature_C;

    // Instantaneous solar panel power measurements (not averaged).
    // Note: Excluded the PCU output voltages, as they very closely match the rail/battery voltage.
    int16_t eps_pcu_ch0_volt_in_mppt_mV;
    int16_t eps_pcu_ch0_curr_in_mppt_mA;
    int16_t eps_pcu_ch0_curr_ou_mppt_mA;
    int16_t eps_pcu_ch1_volt_in_mppt_mV;
    int16_t eps_pcu_ch1_curr_in_mppt_mA;
    int16_t eps_pcu_ch1_curr_ou_mppt_mA;
    int16_t eps_pcu_ch2_volt_in_mppt_mV;
    int16_t eps_pcu_ch2_curr_in_mppt_mA;
    int16_t eps_pcu_ch2_curr_ou_mppt_mA;
    int16_t eps_pcu_ch3_volt_in_mppt_mV;
    int16_t eps_pcu_ch3_curr_in_mppt_mA;
    int16_t eps_pcu_ch3_curr_ou_mppt_mA;
    // Note: (v5+) ch4 is included above, a little ways away in the struct.

    // Battery pack status (EPS FW ICD Table 3-18).
    // Bits 0 (LSB) to 3 are per-cell under voltage flags. 4 to 7 are over voltage flags.
    // Bits 8 to 11 indicate balancing. Bit 12 (0x1000) indicates heater is active (valuable)!
    // Bit 15 (MSB) indicates battery pack is enabled.
    uint16_t eps_battery_pack_status_bitfield;

    // EPS power balance (running average).
    int16_t eps_total_avg_net_battery_power_cW;
    int16_t eps_total_avg_power_distributed_cW;

    // Includes run mode, attitude control mode, and attitude estimation mode, and a ton of
    // enabled/error bitfields. Splits apart into about 40 fields total.
    // See ADCS_pack_to_current_state_1_struct(...) for details on unpacking.
    uint8_t adcs_current_state_1[6];

    // Raw ADCS Coarse Sun Sensor (CSS) readings. Sensors 8 and 10 are unused and excluded.
    uint8_t adcs_raw_css_1;
    uint8_t adcs_raw_css_2;
    uint8_t adcs_raw_css_3;
    uint8_t adcs_raw_css_4;
    uint8_t adcs_raw_css_5;
    uint8_t adcs_raw_css_6;
    uint8_t adcs_raw_css_7;
    uint8_t adcs_raw_css_9;

    // ADCS Magnetic Field Vector (Telemetry ID 151).
    // ADCS calculates this value by applying calibration to the raw A/D readings.
    // Reference Frame: Satellite Body Frame.
    // Value_in_Teslas = Value_uint16 * 10^-8.
    // Value_in_uT     = Value_uint16 * 0.01.
    int16_t adcs_magnetic_field_x_T_en8;
    int16_t adcs_magnetic_field_y_T_en8;
    int16_t adcs_magnetic_field_z_T_en8;

    // ADCS MEMS Rate Sensor Angular Rate Magnitude (derrived from Telemetry ID 155).
    // This is the normalized magnitude of the MEMS rate sensor angular rates, i.e. sqrt(x^2 + y^2 + z^2).
    // Value in Deg/s = Value_uint16 * 0.01.
    uint16_t adcs_angular_rate_norm_cdeg_per_sec;

    // ADCS Estimated Angular Rates (Telemetry ID 147).
    // In Estimation Mode 1, these rates are the MEMS rate sensor values directly. Otherwise,
    // they are estimated from magnetometer and/or full-state calculations.
    // Value in Deg/s = Value_uint16 * 0.01.
    int16_t adcs_estimated_rate_x_cdeg_per_sec;
    int16_t adcs_estimated_rate_y_cdeg_per_sec;
    int16_t adcs_estimated_rate_z_cdeg_per_sec;

    // ADCS Estimated Quaternion (Telemetry ID 218).
    // These values are populated in Estimation Mode 3, 4, 5, and 6 only.
    // NOTE: Decided against quaternions. Will use angles instead, as angles will be easier to
    // directly compare to the commanded attitude settings.
    // int16_t adcs_estimated_quaternion_q1;
    // int16_t adcs_estimated_quaternion_q2;
    // int16_t adcs_estimated_quaternion_q3;

    // ADCS Estimated Attitude Angles (Telemetry ID 146).
    // These values are populated in Estimation Mode 3, 4, 5, and 6 only.
    int16_t adcs_estimated_roll_angle_cdeg;
    int16_t adcs_estimated_pitch_angle_cdeg;
    int16_t adcs_estimated_yaw_angle_cdeg;

} COMMS_beacon_extended_packet_v5plus_t;

// Packet size (currently): Same as v2-v4, as the v5+ fields exactly replace the friendly message.
_Static_assert(sizeof(COMMS_beacon_extended_packet_v5plus_t) == 198, "extended beacon packet size documented wrong here");

// Packet size limit: AX100_downlink_bytes() silently truncates anything longer.
_Static_assert(
    sizeof(COMMS_beacon_extended_packet_v5plus_t) <= AX100_DOWNLINK_MAX_BYTES,
    "extended beacon packet must fit in a single AX100 downlink packet"
);

// The v5+ fields must exactly fill the old friendly message slot (see reserved_v5), so that
// end_message and everything after it stay at their v2-v4 offsets. In v2-v4, end_message is
// at offset 126 (= offsetof(gnss_rx_mode_enum) + 1 + COMMS_BEACON_FRIENDLY_MESSAGE_SIZE).
_Static_assert(
    offsetof(COMMS_beacon_extended_packet_v5plus_t, end_message)
        == offsetof(COMMS_beacon_extended_packet_v5plus_t, gnss_rx_mode_enum) + 1 + COMMS_BEACON_FRIENDLY_MESSAGE_SIZE,
    "v5+ fields must exactly fill the old friendly message slot"
);
_Static_assert(
    offsetof(COMMS_beacon_extended_packet_v5plus_t, end_message) == 126,
    "end_message must stay at its v2-v4 offset"
);
_Static_assert(
    offsetof(COMMS_beacon_extended_packet_v5plus_t, end_message)
        == offsetof(COMMS_beacon_basic_packet_t, end_message),
    "end_message must be at the same offset as in the basic beacon (comms_tx.h)"
);

#pragma pack(pop)


// MARK: Ring Storage Layout

#define BEACON_RING_DIR "beacon_ring"
#define BEACON_RING_FILE_COUNT 20 // Number of files in the ring, before wrapping back to r0.bin.
#define BEACON_RING_RECORDS_PER_FILE 180 // Records per file. 20*180 = 3600 beacons = 9 hours at 9s.
#define BEACON_RING_RECORD_SIZE (sizeof(COMMS_beacon_extended_packet_v5plus_t))
#define BEACON_RING_PATH_MAX_LEN 32 // Enough for "beacon_ring/r<n>.bin".
#define BEACON_RING_CURSOR_PATH BEACON_RING_DIR "/cursor.bin"
#define BEACON_HISTORY_COUNT_MAX 50
#define BEACON_HISTORY_MPI_EXTENDED_MAX 50 // A run reaching an MPI-sensing beacon is extended to this many (note 14).
#define BEACON_MPI_INTERVAL_MS_DEFAULT 1500 // Default mpi_interval_ms kwarg.
#define BEACON_MPI_INTERVAL_MS_MIN 1000 // mpi_interval_ms kwarg is clamped to at least this. // Max historical beacons downlinked per run (history_count arg is clamped).


// On-disk copy of the write cursor (BEACON_RING_CURSOR_PATH), rewritten each time a ring file is
// started. Holds the layout too, so a layout change makes recovery discard the old files.
typedef struct {
    uint32_t magic; // BEACON_RING_CURSOR_MAGIC.
    uint16_t record_size; // BEACON_RING_RECORD_SIZE.
    uint16_t records_per_file; // BEACON_RING_RECORDS_PER_FILE.
    uint8_t file_count; // BEACON_RING_FILE_COUNT.
    uint8_t write_file_idx; // Ring file being written (always from record 0).
    uint8_t has_wrapped;
    uint8_t reserved; // Always 0.
} BEACON_ring_cursor_file_t;

_Static_assert(sizeof(BEACON_ring_cursor_file_t) == 12, "BEACON_ring_cursor_file_t should have no padding");

#define BEACON_RING_CURSOR_MAGIC 0xBEACC0D6u


// MARK: Persistent State

// Defined by blob.ld: a NOLOAD symbol anchored at the start of the fixed `BEACON_RING` region.
extern uint8_t beacon_ring_state_base[];

#define BEACON_RING_STATE_REGION_LEN 0x100u // Must match BEACON_RING's LENGTH in ../blobs_persistent_sram_regions.ld.
// Bumped whenever the layout changes (last: last_live_tx_uptime_ms), so older state is treated as cold.
#define BEACON_RING_STATE_MAGIC 0xBEAC0107u

// NOT packed, deliberately: it holds a live lfs_file_t that littlefs accesses through an ordinary
// (alignment-assuming) pointer. Same as GNSS_ring_state_t.
typedef struct {
    uint32_t magic; // BEACON_RING_STATE_MAGIC once initialized; anything else means cold/garbage SRAM.

    uint8_t write_file_idx; // Ring file currently being appended to, 0..(BEACON_RING_FILE_COUNT-1).
    uint8_t write_record_idx; // Records already in that file, 0..BEACON_RING_RECORDS_PER_FILE.
    uint8_t has_wrapped; // 1 once we've cycled past the last file at least once (all files have data).
    uint8_t needs_fs_recovery; // 1 after a reset, until a run with the filesystem mounted tries ring_recover_from_fs().

    uint32_t last_live_tx_uptime_ms; // Uptime of the last live beacon downlink (0 = none since reset). For MPI fast mode.

    uint32_t stored_record_count; // Total records written to LittleFS, since cold-init.
    uint32_t store_failure_count; // Failed attempts to store a record, since cold-init.

    // Heap allocation holding BEACON_REBOOT_CANARY_MAGIC, used to spot reboots that left this SRAM
    // block intact. NULL until the first allocation. See detect_reboot_via_heap_canary().
    uint32_t *reboot_canary;

    // The ring file we're appending to, held open across executions. See ring_open_write_file().
    uint8_t open_file_is_valid; // 1 if we left `open_file` open; still verify it's live before use.
    uint8_t open_file_idx; // Ring file index that `open_file` refers to, when valid.
    lfs_file_t open_file; // Live littlefs handle; linked into LFS_filesystem.mlist while open.
} BEACON_ring_state_t;

_Static_assert(
    sizeof(BEACON_ring_state_t) <= BEACON_RING_STATE_REGION_LEN,
    "BEACON_ring_state_t must fit within the BEACON_RING memory region reserved in blobs_persistent_sram_regions.ld"
);

_Static_assert(
    BEACON_RING_RECORDS_PER_FILE <= 255,
    "write_record_idx is a uint8_t"
);

#define g_state ((BEACON_ring_state_t *)beacon_ring_state_base)


// MARK: Soft-Reboot Detection

// Value written into the heap canary allocation. Anything else means the heap no longer holds what
// we left there.
#define BEACON_REBOOT_CANARY_MAGIC 0x5C0FFEE5u

/// @brief Detect a reboot that left this blob's fixed SRAM state block intact but reset the heap.
/// @details Same as in the GNSS ring blob. `magic` in the state block can't tell "previous run of
///     this blob" apart from "previous run, then a reboot": that block sits at a fixed SRAM
///     address specifically so it survives a software reset. The heap can tell them apart.
///     FreeRTOS allocates out of `ucHeap`, a static array that is re-initialised from scratch on
///     every boot, so an allocation we made before a reboot is no longer ours afterwards and its
///     contents aren't preserved. We keep one 4-byte allocation holding a magic value; if it no
///     longer reads back as our magic, something restarted underneath us.
///
///     Dereferencing the old pointer after a reboot is safe: it points into `ucHeap`, which is a
///     fixed static array, so the address is always mapped and readable whatever the heap has
///     since done with it. We deliberately never vPortFree() the old allocation -- the heap that
///     owned it no longer exists, and freeing it would corrupt the new heap's free list.
/// @return true if a reboot was detected this run. False on the very first allocation (there was
///     no previous boot to have left anything behind) and on every nominal run after it.
static bool detect_reboot_via_heap_canary() {
    uint32_t *canary = g_state->reboot_canary;
    const bool rebooted = (canary != NULL) && (*canary != BEACON_REBOOT_CANARY_MAGIC);

    // First run, or the old allocation belongs to a dead heap: abandon it (see @details) and
    // take a fresh one. A failed allocation leaves NULL, so the next run simply retries.
    if ((canary == NULL) || rebooted) {
        canary = (uint32_t *)pvPortMalloc(sizeof(uint32_t));
        if (canary != NULL) {
            *canary = BEACON_REBOOT_CANARY_MAGIC; // The only write: from here on we only read it.
        }
        g_state->reboot_canary = canary;
    }

    // One log line covers both notable events: a detected reboot, and/or a failed allocation.
    // Nominal runs (same boot, canary intact) stay silent.
    if (rebooted || (canary == NULL)) {
        blob_log(LOG_SEVERITY_WARNING, "heap canary: reboot=%d, alloc_ok=%d", rebooted, (canary != NULL)
        );
    }
    return rebooted;
}

/// @brief Reset the ring to its "nothing stored yet" state: the write cursor goes back to r0.bin
///     record 0, every counter goes to zero, and the ring refills from scratch (each file is
///     truncated as it's first written to).
/// @details This is the single path used by all four things that mean "start over" (see the
///     file header). Keeping them identical means there's only one "fresh start" behaviour to
///     reason about on the ground.
///
///     Wipes the open file handle WITHOUT closing it: in every caller it's untrusted (dead heap,
///     unmounted or remounted filesystem, or garbage SRAM), so its uncommitted records are lost.
/// @param may_keep_canary true when g_state holds values this blob actually wrote, so the heap
///     canary pointer is worth keeping (saving a re-allocation and another log line on the next
///     run). False for cold/garbage SRAM, where it can't be trusted and must not be dereferenced.
static void reset_to_fresh_state(bool may_keep_canary) {
    uint32_t *canary_to_keep = may_keep_canary ? g_state->reboot_canary : NULL;

    memset(g_state, 0, sizeof(BEACON_ring_state_t));
    g_state->magic = BEACON_RING_STATE_MAGIC;
    g_state->reboot_canary = canary_to_keep;
    g_state->needs_fs_recovery = 1;
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

/// @brief Parse a "key=value" token into a key and a positive integer value.
/// @param token The token. Modified in place (the '=' is replaced with a null terminator).
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


/// @brief Integer square root (Newton/Heron's method), overflow-safe.
/// Source: adapted from https://en.wikipedia.org/wiki/Integer_square_root
/// (CC BY-SA), "Newton's method" / Heron's method with integer-only ops.
static uint16_t integer_sqrt_u32(uint32_t value) {
    if (value == 0) {
        return 0;
    }
    if (value == UINT32_MAX) {
        // sqrt(2^32-1) floors to 2^16-1 exactly
        return UINT16_MAX;
    }

    uint32_t lo = 1;
    uint32_t hi = value;
    while (lo < hi) {
        hi = lo + ((hi - lo) / 2);   // midpoint without lo+hi overflow
        lo = value / hi;
    }
    return (uint16_t)hi;
}


/// @brief Scan an MPI data buffer, averaging all temperature reports in it.
/// @param large_buffer MPI data buffer input.
/// @return Average temperature in 100ths of a degree Celsius (cC). Returns special value -9999 on error.
/// @note The original version of this function in the CTS-SAT-1 firmware has a bug with negative temperatures
///      (https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/667).
///      This vendored version fixes the bug.
static int32_t read_avg_temperature_cC_from_mpi_data_buffer_NEW(
    volatile uint8_t* large_buffer
) {
    const uint8_t sync_pattern[4] = {0x0c, 0xff, 0xff, 0x0c};

    int64_t temp_sum_centi = 0;  // Use int64 to prevent overflow.
    uint32_t temp_count = 0;

    for (uint32_t i = 0; i + 7 < MPI_science_buffer_len; i++) {
        // Check sync pattern.
        if (large_buffer[i + 0] == sync_pattern[0] &&
            large_buffer[i + 1] == sync_pattern[1] &&
            large_buffer[i + 2] == sync_pattern[2] &&
            large_buffer[i + 3] == sync_pattern[3]
        ) {
            // Assumption: Negative values will be handled gracefully by C, just
            // by storing the value in a signed int.
            const int16_t raw_temp =
                ((int16_t)large_buffer[i + 6] << 8) |
                (int16_t)large_buffer[i + 7];

            // Convert to centi-Celsius (https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/462):
            // Celsius = raw_temp / 128.0
            // centi-Celsius = (raw_temp * 100) / 128
            const int32_t temp_centi = (raw_temp * 100) / 128;

            temp_sum_centi += temp_centi;
            temp_count++;
        }
    }

    if (temp_count == 0) {
        return -9999;
    }

    return (int32_t)(temp_sum_centi / temp_count);
}


// MPI temperature readings older than this are reported as stale (-97), not as a valid value.
#define MPI_TEMPERATURE_MAX_AGE_MS (30u * 60u * 1000u) // 30 minutes.

/// @brief Dig the current MPI temperature from the data buffer.
/// @note Based heavily on the inline logic in `rtos_mpi_tasks.c`.
/// @return Temperature in C, or -99 if MPI never active, or -98 if error calculating temperature,
///     or -97 if the most recent MPI data was received more than MPI_TEMPERATURE_MAX_AGE_MS ago.
/// @note Clamps at -90 if colder than -90C, or 125 if hotter than 125C.
static int8_t get_last_mpi_temperature_C() {
    int32_t last_mpi_temperature_cC = -99999;

    // Only report a valid value if the most recent MPI data is fresh.
    const uint32_t last_filled_uptime_ms = (
        (MPI_buffer_one_last_filled_uptime_ms > MPI_buffer_two_last_filled_uptime_ms)
        ? MPI_buffer_one_last_filled_uptime_ms
        : MPI_buffer_two_last_filled_uptime_ms
    );
    if ((last_filled_uptime_ms != 0) && ((TIME_uptime_ms() - last_filled_uptime_ms) > MPI_TEMPERATURE_MAX_AGE_MS)) {
        return -97; // Stale: last received too long ago.
    }

    // Pick the buffer with the most recent data.
    if (MPI_buffer_one_last_filled_uptime_ms > MPI_buffer_two_last_filled_uptime_ms) {
        last_mpi_temperature_cC = read_avg_temperature_cC_from_mpi_data_buffer_NEW(
            MPI_science_buffer_one
        );
    }
    else if (MPI_buffer_two_last_filled_uptime_ms > MPI_buffer_one_last_filled_uptime_ms) {
        last_mpi_temperature_cC = read_avg_temperature_cC_from_mpi_data_buffer_NEW(
            MPI_science_buffer_two
        );
    }
    else {
        return -99; // Neither has ever been filled.
    }

    // If we don't have a valid averaged temperature value available:
    if ((last_mpi_temperature_cC == -99999) || (last_mpi_temperature_cC == -9999)) {
        return -98; // Error calculating temperature.
    }

    // Divide by 100, and clamp the value to within -90C and 125C.
    const int32_t last_mpi_temperature_C = last_mpi_temperature_cC / 100;
    if (last_mpi_temperature_C <= -90) {
        return -90;
    }
    else if (last_mpi_temperature_C >= 125) {
        return 125;
    }
    
    return last_mpi_temperature_C;
}


// MARK: v5 Field Helpers


/// @brief Clamp a power reading into a uint8 cW beacon field (0 to 255).
static uint8_t clamp_power_cW_to_u8(int32_t power_cW) {
    if (power_cW < 0) {
        return 0;
    }
    if (power_cW > UINT8_MAX) {
        return UINT8_MAX;
    }
    return (uint8_t)power_cW;
}



// ADCS rail voltages used to convert the ADCS current telemetry (TLM 195) into power.
// Sources: CubeADCS ICD v7.5 and Firmware Reference Manual v7.4 (TLM 195, Table 154).
//  - CubeSense 1/2: 3.3 V supply (ICD Table 6).
//  - CubeControl: separate 3V3, 5V, and Vbat currents. Vbat is used e.g. for magnetometer
//    deployment (Commissioning Manual, sec. on deployment).
//  - CubeWheels: battery bus (ICD sec. 5.2.1: "The battery bus (pins H2-45 and H2-46) supplies power
//    to the CubeWheels."), so their currents use the measured battery voltage.
//  - CubeStar: 3.3 V (ICD CubeStar specifications).
// Deliberately NOT summed:
//  - Magnetorquer current: CubeControl drives the magnetorquers from its own supply (ICD 7.4), so
//    that current is already in the CubeControl rail currents. It's also reported in DN, not mA
//    (Health Check v7.18, sec. 6: mA = DN * k, with k = 0.51 for a CubeCoil, 1 for small CubeRods).
//  - CubeSense Cam SRAM currents: likely a sub-branch of the same 3.3 V supply (unconfirmed);
//    at most ~5.5 mA each (Health Check v7.18), so ~18 mW (tiny) each either way.
// Not measured by any telemetry, so not included: CubeComputer itself (~120 mW average, ICD).
#define ADCS_3V3_RAIL_MV 3300
#define ADCS_5V_RAIL_MV 5000
#define ADCS_NOMINAL_BATTERY_MV 14700 // Used if the battery voltage measurement fails.

// Byte offsets of the uint16 (little-endian) current channels in TLM 195 (Power and Temperature
// Measurements, 38 bytes). Firmware Reference Manual v7.4, Table 154.
#define ADCS_TLM195_LEN 38
#define ADCS_TLM195_CUBESENSE1_3V3_OFFSET 0 // mA = raw * 0.1
#define ADCS_TLM195_CUBESENSE2_3V3_OFFSET 4 // mA = raw * 0.1
#define ADCS_TLM195_CUBECONTROL_3V3_OFFSET 8 // mA = raw * 0.48828125
#define ADCS_TLM195_CUBECONTROL_5V_OFFSET 10 // mA = raw * 0.48828125
#define ADCS_TLM195_CUBECONTROL_VBAT_OFFSET 12 // mA = raw * 0.48828125
#define ADCS_TLM195_WHEEL1_OFFSET 14 // mA = raw * 0.01 (Wheel 2 and 3 follow, at +2 and +4).
#define ADCS_TLM195_CUBESTAR_OFFSET 20 // mA = raw * 0.01

/// @brief Read a little-endian int16 out of a byte buffer.
static int16_t read_le_int16(const uint8_t *buf) {
    return (int16_t)((uint16_t)buf[0] | ((uint16_t)buf[1] << 8));
}

/// @brief Read a little-endian uint16 out of a byte buffer.
static uint16_t read_le_uint16(const uint8_t *buf) {
    return (uint16_t)((uint16_t)buf[0] | ((uint16_t)buf[1] << 8));
}


/// @brief Compute the ADCS's total power draw from its current telemetry. See the notes on
///     ADCS_3V3_RAIL_MV for what's included and excluded.
/// @param tlm195_raw TLM 195 (Power and Temperature Measurements) raw frame.
/// @param battery_mV Battery voltage, for the CubeControl Vbat rail and the CubeWheels.
///     If the measurement failed (OBC_read_vbat_with_adc_mV() returns -9999, or any non-positive
///     value), ADCS_NOMINAL_BATTERY_MV is assumed instead.
/// @return Total power in cW, clamped to 0..65535.
static uint16_t calculate_adcs_total_power_cW(const uint8_t tlm195_raw[ADCS_TLM195_LEN], int16_t battery_mV) {
    const uint32_t vbatt_mV = (battery_mV > 0) ? (uint32_t)battery_mV : ADCS_NOMINAL_BATTERY_MV;
    #define RAW(offset) read_le_uint16(&tlm195_raw[offset])

    // Sum of (current in mA) * (voltage in mV) = power in uW. Computed in exact integer units of 0.01 uW (smaller than float code):
    // CubeSense (raw * 0.1 mA) and CubeStar (raw * 0.01 mA) at 3V3; CubeWheels (raw * 0.01 mA) at
    // Vbat; CubeControl (raw * 0.48828125 = raw * 125/256 mA) at 3V3, 5V, and Vbat.
    const uint64_t power_e2_uW =
        (10ull * ADCS_3V3_RAIL_MV * (RAW(ADCS_TLM195_CUBESENSE1_3V3_OFFSET) + RAW(ADCS_TLM195_CUBESENSE2_3V3_OFFSET)))
        + ((uint64_t)ADCS_3V3_RAIL_MV * RAW(ADCS_TLM195_CUBESTAR_OFFSET))
        + ((uint64_t)vbatt_mV * (
            RAW(ADCS_TLM195_WHEEL1_OFFSET) + RAW(ADCS_TLM195_WHEEL1_OFFSET + 2) + RAW(ADCS_TLM195_WHEEL1_OFFSET + 4)
        ))
        + ((12500ull * (
            ((uint64_t)ADCS_3V3_RAIL_MV * RAW(ADCS_TLM195_CUBECONTROL_3V3_OFFSET))
            + ((uint64_t)ADCS_5V_RAIL_MV * RAW(ADCS_TLM195_CUBECONTROL_5V_OFFSET))
            + ((uint64_t)vbatt_mV * RAW(ADCS_TLM195_CUBECONTROL_VBAT_OFFSET))
        )) / 256u);
    #undef RAW

    const uint64_t power_cW = (power_e2_uW + 500000u) / 1000000u; // 0.01 uW -> cW, rounded.
    return (power_cW > UINT16_MAX) ? UINT16_MAX : (uint16_t)power_cW;
}


/// @brief Magnitude of a 3-vector of little-endian int16 values (6 raw bytes), in the same units.
static uint16_t vector3_i16_norm(const uint8_t raw[6]) {
    uint32_t sum_sq = 0;
    for (uint8_t i = 0; i < 3; i++) {
        const int32_t v = read_le_int16(&raw[2 * i]);
        sum_sq += (uint32_t)(v * v); // Max 3 * 32768^2, which fits in uint32.
    }
    return integer_sqrt_u32(sum_sq);
}

/// @brief Convert the magnitude of a quaternion error vector (in units of 0.0001) to the
///     rotation angle it represents (2 * asin(|q_vec|)), in centidegrees (0 to 18000).
/// @note asin() approximation from Abramowitz & Stegun, Handbook of Mathematical Functions,
///     eq. 4.4.45 (|error| <= 5e-5 rad), since the firmware ELF has no libm to link against.
static uint16_t quaternion_error_vector_to_angle_cdeg(uint16_t q_vec_norm_e4) {
    if (q_vec_norm_e4 > 10000) {
        q_vec_norm_e4 = 10000; // Not a unit quaternion; clamp to 180 degrees.
    }

    const float x = (float)q_vec_norm_e4 / 10000.0f;
    const float sqrt_1_minus_x = (float)integer_sqrt_u32((uint32_t)(10000 - q_vec_norm_e4) * 10000u) / 10000.0f;
    const float asin_x_rad = 1.5707963f - sqrt_1_minus_x * (
        1.5707288f + x * (-0.2121144f + x * (0.0742610f + x * -0.0187293f))
    );

    const float angle_cdeg = 2.0f * asin_x_rad * (18000.0f / 3.14159265f);
    if (angle_cdeg <= 0.0f) {
        return 0;
    }
    return (uint16_t)(angle_cdeg + 0.5f);
}

/// @brief Fill `hash_dest` with the first 3 bytes of the SHA256 of the path of the file currently
///     being bulk downlinked, or with zeros if no bulk downlink is in progress.
/// @details Only the DOWNLINKING state counts. In IDLE (never started, or finished) and PAUSED,
///     zeros are reported, even though COMMS_bulk_file_downlink_file_path may still hold the
///     last file's name.
///     The path is hashed as stored, without its null terminator.
static void get_bulk_downlink_file_path_hash(uint8_t hash_dest[3]) {
    if ((COMMS_bulk_file_downlink_state != COMMS_BULK_FILE_DOWNLINK_STATE_DOWNLINKING)) {
        // If not currently downlinking, set to zero and finish.
        memset(hash_dest, 0, 3);
        return;
    }

    // Compute hash.
    uint8_t digest[32];
    CRYPT_compute_sha256_hash(
        (const uint8_t *)COMMS_bulk_file_downlink_file_path,
        strlen(COMMS_bulk_file_downlink_file_path),
        digest
    );
    memcpy(hash_dest, digest, 3);
}


/// @brief Is this agenda slot counted by count_distinct_pending_tcmds()? Pending entries, plus the
///     currently-executing one (i.e., this blob's own exec_blob_from_fs).
static bool is_agenda_slot_counted(uint16_t slot_num) {
    return (
        (TCMD_agenda_is_valid[slot_num] == TCMD_AGENDA_ENTRY_VALID_AND_PENDING)
        || (TCMD_agenda_is_valid[slot_num] == TCMD_AGENDA_ENTRY_EXECUTING)
    );
}

// Hash set used by count_distinct_pending_tcmds().
#define TCMD_HASH_SET_BUCKET_COUNT 1024 // Power of 2. 750 entries -> at most ~73% full.
#define TCMD_HASH_SET_EMPTY_SLOT_NUM 0xFFFF // slot_num value marking an unused bucket.

#pragma pack(push, 1)
typedef struct {
    uint16_t slot_num; // Agenda slot number of the entry in this bucket, or TCMD_HASH_SET_EMPTY_SLOT_NUM.
    uint16_t hash_check; // Upper 16 bits of the entry's hash, to skip most strcmp()s on unrelated entries.
} tcmd_hash_set_bucket_struct_t;
#pragma pack(pop)

_Static_assert(
    sizeof(tcmd_hash_set_bucket_struct_t) == 4,
    "tcmd_hash_set_bucket_struct_t should be tightly packed"
);
_Static_assert(
    TCMD_AGENDA_SIZE < TCMD_HASH_SET_BUCKET_COUNT,
    "Hash set must have more buckets than agenda slots, so it never fills up"
);
_Static_assert(
    TCMD_AGENDA_SIZE <= TCMD_HASH_SET_EMPTY_SLOT_NUM,
    "Agenda slot numbers must never equal TCMD_HASH_SET_EMPTY_SLOT_NUM"
);

/// @brief 32-bit FNV-1a hash of (tcmd_idx, args), so different telecommands with the same args
///     hash differently.
static uint32_t hash_agenda_entry(uint16_t slot_num) {
    uint32_t hash = 2166136261u; // FNV offset basis.
    hash = (hash ^ TCMD_agenda[slot_num].tcmd_idx) * 16777619u; // FNV prime.

    const char *args = TCMD_agenda[slot_num].args_str_no_parens;
    for (uint16_t i = 0; (i < TCMD_ARGS_STR_NO_PARENS_SIZE) && (args[i] != '\0'); i++) {
        hash = (hash ^ (uint8_t)args[i]) * 16777619u;
    }
    return hash;
}

/// @brief Count the distinct (tcmd_idx, args) pairs among the pending agenda entries, plus the
///     currently-executing one.
/// @details Includes the executing entry so that, when nothing is duplicated, this equals
///     pending_queued_tcmd_count (which is pending entries + 1 for the executing entry).
///     If the executing entry duplicates a pending one (e.g., a pre-scheduled run of this blob
///     with the same args), the pair is counted once.
///
///     Uses an open-addressing hash set (linear probing) on the stack, so it's O(n) expected
///     (n = agenda slots), instead of O(n^2) string compares. Still exact: every hash match is
///     confirmed with tcmd_idx and strcmp() before it's treated as a duplicate.
/// @note noinline, so the 4 KB table is only on the stack while this function runs, instead of
///     being folded into blob_main()'s frame (which is live during the deep EPS/LittleFS calls).
/// @return The count, capped at 255.
__attribute__((noinline))
static uint8_t count_distinct_pending_tcmds() {
    tcmd_hash_set_bucket_struct_t buckets[TCMD_HASH_SET_BUCKET_COUNT];
    for (uint16_t b = 0; b < TCMD_HASH_SET_BUCKET_COUNT; b++) {
        buckets[b].slot_num = TCMD_HASH_SET_EMPTY_SLOT_NUM;
        buckets[b].hash_check = 0;
    }

    uint16_t distinct_count = 0;
    for (uint16_t i = 0; i < TCMD_AGENDA_SIZE; i++) {
        if (!is_agenda_slot_counted(i)) {
            continue;
        }

        const uint32_t hash = hash_agenda_entry(i);
        const uint16_t hash_check = (uint16_t)(hash >> 16);
        uint16_t bucket_idx = (uint16_t)(hash & (TCMD_HASH_SET_BUCKET_COUNT - 1));

        // Only count the first occurrence of each pair. Probe until we find it, or an empty bucket.
        bool seen_before = false;
        while (buckets[bucket_idx].slot_num != TCMD_HASH_SET_EMPTY_SLOT_NUM) {
            const tcmd_hash_set_bucket_struct_t *bucket = &buckets[bucket_idx];
            if (
                (bucket->hash_check == hash_check)
                && (TCMD_agenda[bucket->slot_num].tcmd_idx == TCMD_agenda[i].tcmd_idx)
                && (strcmp(TCMD_agenda[bucket->slot_num].args_str_no_parens, TCMD_agenda[i].args_str_no_parens) == 0)
            ) {
                seen_before = true;
                break;
            }
            bucket_idx = (uint16_t)((bucket_idx + 1) & (TCMD_HASH_SET_BUCKET_COUNT - 1));
        }

        if (!seen_before) {
            buckets[bucket_idx].slot_num = i;
            buckets[bucket_idx].hash_check = hash_check;
            distinct_count++;
            if (distinct_count >= UINT8_MAX) {
                return UINT8_MAX;
            }
        }
    }
    return (uint8_t)distinct_count;
}


/// @brief Map a negative BLOB_reschedule_self_unless_upcoming() result to this blob's error enum.
static BLOB_ext_beacon_error_enum_t reschedule_err_to_blob_err(int8_t reschedule_result) {
    switch (reschedule_result) {
        case BLOB_RESCHEDULE_ERR_AGENDA_ADD_FAILED: return BLOB_ERR_AGENDA_ADD_FAILED;
        default: return BLOB_ERR_NO_EXECUTING_AGENDA_SLOT;
    }
}

// MARK: Fill Packet

/// @brief Check whether the ADCS responds on I2C at all, using a single telemetry request.
/// @details ADCS_i2c_request_telemetry_and_check() stalls for a very long time if ADCS is absent,
///         so we add a basic bypass here in such a case.
static bool is_adcs_responding() {
    uint8_t data_received[6];
    const uint8_t result = ADCS_send_i2c_telemetry_request(
        132, // ADCS_TELEMETRY_CUBEACP_ADCS_STATE
        data_received,
        sizeof(data_received),
        0 // ADCS_NO_CHECKSUM
    );
    return result == 0;
}

// ADCS telemetry whose 6 raw bytes are copied straight into the beacon: {telemetry ID, beacon offset}.
// This table structure is used to decrease the size of the implementation.
#define ADCS_COPY(id, field) { id, offsetof(COMMS_beacon_extended_packet_v5plus_t, field) }
static const uint8_t ADCS_COPY_TLMS[][2] = {
    // ADCS status data.
    ADCS_COPY(132, adcs_current_state_1), // ADCS_TELEMETRY_CUBEACP_ADCS_STATE

    // ADCS magnetic field vector (x, y, z).
    // Raw values are int16, little-endian, in units of 10 nT (i.e. already in Telsla en8 scale).
    ADCS_COPY(151, adcs_magnetic_field_x_T_en8), // ADCS_TELEMETRY_CUBEACP_MAGNETIC_FIELD_VECTOR

    // ADCS estimated angular rates (x, y, z rates).
    ADCS_COPY(147, adcs_estimated_rate_x_cdeg_per_sec), // ADCS_TELEMETRY_CUBEACP_ESTIMATED_ANGULAR_RATES

    // ADCS estimated attitude angles (roll, pitch, yaw).
    ADCS_COPY(146, adcs_estimated_roll_angle_cdeg), // ADCS_TELEMETRY_CUBEACP_ESTIMATED_ATTITUDE_ANGLES

    // ADCS measured wheel speeds, in RPM.
    ADCS_COPY(156, adcs_wheel_speed_x_rpm), // ADCS_TELEMETRY_CUBEACP_WHEEL_SPEED

    // ADCS estimation innovation vector, in the raw units of 0.0001.
    ADCS_COPY(162, adcs_innovation_vector_x_en4), // ADCS_TELEMETRY_CUBEACP_ESTIMATION_INNOVATION_VECTOR

    // ADCS EKF estimated gyro bias, in the raw units of 0.001 deg/s.
    ADCS_COPY(161, adcs_estimated_gyro_bias_x_mdeg_per_sec), // ADCS_TELEMETRY_CUBEACP_ESTIMATED_GYRO_BIAS
};
_Static_assert(sizeof(COMMS_beacon_extended_packet_v5plus_t) <= 255, "offsets stored as uint8_t");

/// @brief Fill all the ADCS fields of the beacon packet (v2 and v5+).
/// @details Skips every ADCS request if the ADCS doesn't respond to a single probe (e.g., dev kit
///     with no ADCS, or ADCS powered off), leaving the ADCS fields zeroed. See is_adcs_responding().
/// @note Must run after obc_adc_battery_voltage_mV is filled (used for the ADCS power calculation).
/// @return Number of failed ADCS requests (1 if the ADCS didn't respond at all).
static uint8_t fill_beacon_adcs_fields(COMMS_beacon_extended_packet_v5plus_t *beacon_packet) {
    if (!is_adcs_responding()) {
        return 1;
    }

    uint8_t peripheral_comms_error_count = 0;
    uint8_t data_received[ADCS_TLM195_LEN];

    for (uint8_t i = 0; i < sizeof(ADCS_COPY_TLMS) / sizeof(ADCS_COPY_TLMS[0]); i++) {
        const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(
            ADCS_COPY_TLMS[i][0], // Telemetry ID (ADCS_TELEMETRY_CUBEACP_...).
            data_received,
            6,  // All are 3 elements long, 16-bit elements.
            1 // ADCS_INCLUDE_CHECKSUM
        );

        if (tlm_status == 0) {
            // x, y, z (int16 each) land in the 3 consecutive beacon fields starting at the offset.
            memcpy((uint8_t *)beacon_packet + ADCS_COPY_TLMS[i][1], data_received, 6);
        }
        else {
            peripheral_comms_error_count++;
        }
    }

    // ADCS CSS (coarse sun sensor) raw data.
    {
        ADCS_raw_coarse_sun_sensor_1_to_6_struct_t output_struct_1_to_6;
        ADCS_raw_coarse_sun_sensor_7_to_10_struct_t output_struct_7_to_10;

        const uint8_t status_1_to_6 = ADCS_get_raw_coarse_sun_sensor_1_to_6(&output_struct_1_to_6);
        const uint8_t status_7_to_10 = ADCS_get_raw_coarse_sun_sensor_7_to_10(&output_struct_7_to_10);

        if ((status_1_to_6 == 0) && (status_7_to_10 == 0)) {
            beacon_packet->adcs_raw_css_1 = output_struct_1_to_6.coarse_sun_sensor_1;
            beacon_packet->adcs_raw_css_2 = output_struct_1_to_6.coarse_sun_sensor_2;
            beacon_packet->adcs_raw_css_3 = output_struct_1_to_6.coarse_sun_sensor_3;
            beacon_packet->adcs_raw_css_4 = output_struct_1_to_6.coarse_sun_sensor_4;
            beacon_packet->adcs_raw_css_5 = output_struct_1_to_6.coarse_sun_sensor_5;
            beacon_packet->adcs_raw_css_6 = output_struct_1_to_6.coarse_sun_sensor_6;
            beacon_packet->adcs_raw_css_7 = output_struct_7_to_10.coarse_sun_sensor_7;
            beacon_packet->adcs_raw_css_9 = output_struct_7_to_10.coarse_sun_sensor_9;
            // Skip 8 and 10 (unused).
        }
        else {
            peripheral_comms_error_count++;
        }
    }

    // ADCS MEMS rate sensor angular rate magnitude.
    {
        uint8_t data_received[6];

        const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(
            155, // ADCS_TELEMETRY_CUBEACP_RATE_SENSOR_RATES
            data_received,
            sizeof(data_received),
            1 // ADCS_INCLUDE_CHECKSUM
        );

        if (tlm_status == 0) {
            // Note: data_received is 3x i16 (x, y, z), each in cdeg/s.
            beacon_packet->adcs_angular_rate_norm_cdeg_per_sec = vector3_i16_norm(data_received);
        }
        else {
            peripheral_comms_error_count++;
        }
    }

    // ADCS pointing error: the rotation angle of the quaternion error vector (commanded vs.
    // estimated attitude). Only meaningful in control modes with a commanded attitude.
    {
        uint8_t data_received[6];

        const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(
            163, // ADCS_TELEMETRY_CUBEACP_QUATERNION_ERROR_VECTOR
            data_received,
            sizeof(data_received),
            1 // ADCS_INCLUDE_CHECKSUM
        );

        if (tlm_status == 0) {
            beacon_packet->adcs_pointing_error_cdeg = quaternion_error_vector_to_angle_cdeg(
                vector3_i16_norm(data_received)
            );
        }
        else {
            peripheral_comms_error_count++;
        }
    }

    // ADCS total power, from the ADCS's own current measurements (TLM 195), each times its rail
    // voltage. See calculate_adcs_total_power_cW().
    {
        uint8_t data_received[ADCS_TLM195_LEN];

        const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(
            195, // ADCS_TELEMETRY_CUBEACP_POWER_AND_TEMPERATURE_MEASUREMENTS
            data_received,
            sizeof(data_received),
            1 // ADCS_INCLUDE_CHECKSUM
        );

        if (tlm_status == 0) {
            beacon_packet->adcs_total_power_consumed_cW = calculate_adcs_total_power_cW(
                data_received, beacon_packet->obc_adc_battery_voltage_mV
            );
        }
        else {
            peripheral_comms_error_count++;
        }
    }

    return peripheral_comms_error_count;
}


/// @brief 
/// @param beacon_packet 
/// @note Based on the `COMMS_fill_beacon_basic_packet` function in the main firmware.
static void COMMS_fill_beacon_extended_packet_v5plus(
    COMMS_beacon_extended_packet_v5plus_t *beacon_packet, uint8_t *peripheral_comms_error_count_dst
) {
    // Safety: Reset the packet to zero.
    memset(beacon_packet, 0, sizeof(COMMS_beacon_extended_packet_v5plus_t));

    uint8_t peripheral_comms_error_count = 0;

    // Fill the packet with the current system state.
    beacon_packet->packet_type = COMMS_PACKET_TYPE_BEACON_EXTENDED_V5PLUS; // 0x21 = 33
    memcpy(beacon_packet->satellite_name, "CTS1", 4);
    beacon_packet->active_rf_switch_antenna = COMMS_active_rf_switch_antenna; // 1 or 2
    beacon_packet->active_rf_switch_control_mode = COMMS_rf_switch_control_mode; // Enum
    beacon_packet->uptime_ms = TIME_uptime_ms();
    beacon_packet->duration_since_last_uplink_ms = TIME_uptime_ms() - AX100_uptime_at_last_received_kiss_tcmd_ms;
    beacon_packet->unix_epoch_time_ms = TIME_get_current_unix_epoch_time_ms();
    beacon_packet->last_time_sync_source_enum = TIME_last_synchronization_source;

    beacon_packet->is_fs_mounted = LFS_is_lfs_mounted;

    beacon_packet->total_tcmd_queued_count = TCMD_total_tcmd_queued_count;
    beacon_packet->pending_queued_tcmd_count = TCMD_get_agenda_used_slots_count() + 1; // Since v4, add 1 for the currently executing tcmd. Prevents bouncing between basic and extended beacons.

    beacon_packet->total_beacon_count_since_boot = COMMS_total_beacon_count_since_boot;

    beacon_packet->reboot_reason = STM32_get_reset_cause();

    // EPS fields here (EPS is the only peripheral in the basic beacon).
    // Set the default here, then conditionally fill them below if the EPS is successful.
    // This section is all placeholders for EPS communication errors.
    beacon_packet->eps_mode_enum = 255;
    beacon_packet->eps_reset_cause_enum = 255;
    beacon_packet->eps_uptime_sec = 9999;
    beacon_packet->eps_error_code = 9999;
    beacon_packet->eps_battery_voltage_mV = 0;
    beacon_packet->eps_battery_percent = 0;
    beacon_packet->eps_battery_temperature_0_cC = -9999;
    beacon_packet->eps_battery_temperature_1_cC = -9999;
    beacon_packet->eps_total_fault_count = -1;
    beacon_packet->eps_enabled_channels_bitfield = 0;
    beacon_packet->eps_total_pcu_power_input_cW = -99999;
    beacon_packet->eps_total_pcu_power_output_cW = -99999;
    beacon_packet->eps_total_avg_pcu_power_input_cW = -99999;
    beacon_packet->eps_total_avg_pcu_power_output_cW = -99999;

    beacon_packet->obc_temperature_cC = OBC_TEMP_SENSOR_get_temperature_cC();

    beacon_packet->cts1_operation_state = CTS1_operation_state;
    beacon_packet->rbf_pin_state = OBC_get_rbf_state();

    beacon_packet->mpi_rx_mode_enum = MPI_current_uart_rx_mode;
    beacon_packet->mpi_transceiver_state_enum = MPI_current_transceiver_state;
    beacon_packet->mpi_last_reason_for_stopping_enum = MPI_last_reason_for_stopping_active_mode;

    beacon_packet->gnss_uart_interrupt_enabled = UART_gnss_uart_interrupt_enabled;
    beacon_packet->gnss_rx_mode_enum = GNSS_current_rx_mode;

    // Note: v5+ has no friendly message (its slot holds the v5+ fields instead).
    memcpy(beacon_packet->end_message, " X6", 4); // Extended beacon packet version.

    // Set the extended beacon packet fields.

    beacon_packet->obc_active_oscillator_MHz = SystemCoreClock / 1000000;

    beacon_packet->obc_adc_battery_voltage_mV = OBC_read_vbat_with_adc_mV();

    beacon_packet->mpi_last_temperature_C = get_last_mpi_temperature_C();

    beacon_packet->eps_pcu_ch0_volt_in_mppt_mV = -9999;
    beacon_packet->eps_pcu_ch0_curr_in_mppt_mA = -9999;
    beacon_packet->eps_pcu_ch0_curr_ou_mppt_mA = -9999;
    beacon_packet->eps_pcu_ch1_volt_in_mppt_mV = -9999;
    beacon_packet->eps_pcu_ch1_curr_in_mppt_mA = -9999;
    beacon_packet->eps_pcu_ch1_curr_ou_mppt_mA = -9999;
    beacon_packet->eps_pcu_ch2_volt_in_mppt_mV = -9999;
    beacon_packet->eps_pcu_ch2_curr_in_mppt_mA = -9999;
    beacon_packet->eps_pcu_ch2_curr_ou_mppt_mA = -9999;
    beacon_packet->eps_pcu_ch3_volt_in_mppt_mV = -9999;
    beacon_packet->eps_pcu_ch3_curr_in_mppt_mA = -9999;
    beacon_packet->eps_pcu_ch3_curr_ou_mppt_mA = -9999;
    beacon_packet->eps_pcu_ch4_volt_in_mppt_mV = -9999;
    beacon_packet->eps_pcu_ch4_curr_in_mppt_mA = -9999;
    beacon_packet->eps_pcu_ch4_curr_ou_mppt_mA = -9999;

    beacon_packet->eps_battery_pack_status_bitfield = 0xFFFF;

    beacon_packet->eps_total_avg_net_battery_power_cW = -9999;
    beacon_packet->eps_total_avg_power_distributed_cW = -9999;

    
    // Try to fetch the EPS system status, and store it in the beacon packet if successful.
    {
        EPS_struct_system_status_t eps_status_data;
        if (EPS_CMD_get_system_status(&eps_status_data) == 0) {
            beacon_packet->eps_mode_enum = eps_status_data.mode;
            beacon_packet->eps_reset_cause_enum = eps_status_data.reset_cause;
            beacon_packet->eps_uptime_sec = eps_status_data.uptime_sec;
            beacon_packet->eps_error_code = eps_status_data.error_code;
        }
        else {
            peripheral_comms_error_count++;
        }
    }

    // Try to fetch the EPS PIU data (INSTANTANEOUS), and store the solar panel (conditioning
    // chain) values in the beacon packet if successful.
    // Uses the repaired PIU reader, as the firmware's misplaces the conditioning chains.
    // The PIU is used instead of the PCU (0x72), as the PCU reply only has CC1-CC4 (of 5).
    // Also provides the battery fields, instead of the PBU (0x62), to save an EPS transaction.
    {
        EPS_struct_piu_housekeeping_data_eng_FIXED_t eps_piu_data;
        if (EPS_CMD_get_piu_housekeeping_data_eng_FIXED(&eps_piu_data) == 0) {
            // Beacon v1 fields (v5+: includes CC5 now):
            beacon_packet->eps_total_pcu_power_input_cW = (
                EPS_calculate_total_pcu_power_input_cW_FIXED(&eps_piu_data)
            );
            beacon_packet->eps_total_pcu_power_output_cW = (
                EPS_calculate_total_pcu_power_output_cW_FIXED(&eps_piu_data)
            );

            // Battery fields. Same scale as EPS_convert_battery_voltage_to_percent() (12.4 V = 0%,
            // 16.0 V = 100%), but in integer math. Clamped at 0%, but not at 100%, same as before.
            const int32_t battery_voltage_mV = eps_piu_data.vip_batt_input.voltage_mV;
            const int32_t battery_percent = ((battery_voltage_mV - 12400) * 100) / (16000 - 12400);
            beacon_packet->eps_battery_voltage_mV = (uint16_t)battery_voltage_mV;
            beacon_packet->eps_battery_percent = (battery_percent < 0) ? 0 : (uint8_t)battery_percent;
            beacon_packet->eps_battery_temperature_0_cC = eps_piu_data.battery_temp3_cC;
            beacon_packet->eps_battery_temperature_1_cC = eps_piu_data.battery_temp2_cC;

            // Beacon v2 fields:
            beacon_packet->eps_battery_pack_status_bitfield = eps_piu_data.battery_status_bitfield;

            const EPS_conditioning_channel_short_datatype_eng_t *cc0 = EPS_piu_get_conditioning_channel_FIXED(&eps_piu_data, 0);
            const EPS_conditioning_channel_short_datatype_eng_t *cc1 = EPS_piu_get_conditioning_channel_FIXED(&eps_piu_data, 1);
            const EPS_conditioning_channel_short_datatype_eng_t *cc2 = EPS_piu_get_conditioning_channel_FIXED(&eps_piu_data, 2);
            const EPS_conditioning_channel_short_datatype_eng_t *cc3 = EPS_piu_get_conditioning_channel_FIXED(&eps_piu_data, 3);
            const EPS_conditioning_channel_short_datatype_eng_t *cc4 = EPS_piu_get_conditioning_channel_FIXED(&eps_piu_data, 4);

            // Beacon v2 fields:
            beacon_packet->eps_pcu_ch0_volt_in_mppt_mV = cc0->volt_in_mppt_mV;
            beacon_packet->eps_pcu_ch0_curr_in_mppt_mA = cc0->curr_in_mppt_mA;
            beacon_packet->eps_pcu_ch0_curr_ou_mppt_mA = cc0->curr_ou_mppt_mA;
            beacon_packet->eps_pcu_ch1_volt_in_mppt_mV = cc1->volt_in_mppt_mV;
            beacon_packet->eps_pcu_ch1_curr_in_mppt_mA = cc1->curr_in_mppt_mA;
            beacon_packet->eps_pcu_ch1_curr_ou_mppt_mA = cc1->curr_ou_mppt_mA;
            beacon_packet->eps_pcu_ch2_volt_in_mppt_mV = cc2->volt_in_mppt_mV;
            beacon_packet->eps_pcu_ch2_curr_in_mppt_mA = cc2->curr_in_mppt_mA;
            beacon_packet->eps_pcu_ch2_curr_ou_mppt_mA = cc2->curr_ou_mppt_mA;
            beacon_packet->eps_pcu_ch3_volt_in_mppt_mV = cc3->volt_in_mppt_mV;
            beacon_packet->eps_pcu_ch3_curr_in_mppt_mA = cc3->curr_in_mppt_mA;
            beacon_packet->eps_pcu_ch3_curr_ou_mppt_mA = cc3->curr_ou_mppt_mA;

            // Beacon v5+ fields:
            beacon_packet->eps_pcu_ch4_volt_in_mppt_mV = cc4->volt_in_mppt_mV;
            beacon_packet->eps_pcu_ch4_curr_in_mppt_mA = cc4->curr_in_mppt_mA;
            beacon_packet->eps_pcu_ch4_curr_ou_mppt_mA = cc4->curr_ou_mppt_mA;
        }
        else {
            peripheral_comms_error_count++;
        }
    }

    // Try to fetch the EPS PIU data (RUNNING AVERAGE), and store it in the beacon packet if successful.
    // The PIU is used instead of the PCU (0x74), as the PCU reply only has CC1-CC4 (of 5).
    {
        EPS_struct_piu_housekeeping_data_eng_FIXED_t eps_piu_data;
        if (EPS_CMD_get_piu_housekeeping_data_run_avg_FIXED(&eps_piu_data) == 0) {
            // Beacon v1 fields (v5+: includes CC5):
            beacon_packet->eps_total_avg_pcu_power_input_cW = (
                EPS_calculate_total_pcu_power_input_cW_FIXED(&eps_piu_data)
            );
            beacon_packet->eps_total_avg_pcu_power_output_cW = (
                EPS_calculate_total_pcu_power_output_cW_FIXED(&eps_piu_data)
            );

            beacon_packet->eps_enabled_channels_bitfield = (
                ((uint32_t)eps_piu_data.stat_ch_ext_on_bitfield << 16) | eps_piu_data.stat_ch_on_bitfield
            );

            // Beacon v2 fields:
            beacon_packet->eps_total_avg_net_battery_power_cW = eps_piu_data.vip_batt_input.power_cW;
            beacon_packet->eps_total_avg_power_distributed_cW = eps_piu_data.vip_dist_input.power_cW;

            // Beacon v5 fields:
            beacon_packet->gnss_power_consumed_cW = clamp_power_cW_to_u8(
                EPS_piu_get_channel_vip_FIXED(&eps_piu_data, EPS_CHANNEL_3V3_GNSS)->power_cW
            );
        }
        else {
            peripheral_comms_error_count++;
        }
    }

    // Try to fetch the EPS fault count, and store it in the beacon packet if successful.
    {
        // Get EPS total fault count.
        EPS_struct_pdu_overcurrent_fault_state_t eps_pdu_fault_data;
        if (EPS_CMD_get_pdu_overcurrent_fault_state(&eps_pdu_fault_data) == 0) {
            beacon_packet->eps_total_fault_count = EPS_calculate_total_fault_count(&eps_pdu_fault_data);
        }
        else {
            peripheral_comms_error_count++;
        }
    }

    // Everything below is for the v5+ beacon.

    beacon_packet->distinct_tcmds_pending = count_distinct_pending_tcmds();

    {
        const uint32_t sec_since_sync = (TIME_uptime_ms() - TIME_system_uptime_at_last_time_resync_ms) / 1000;
        beacon_packet->duration_since_last_time_sync_sec = (
            (sec_since_sync > UINT16_MAX) ? UINT16_MAX : (uint16_t)sec_since_sync
        );
    }

    get_bulk_downlink_file_path_hash(beacon_packet->bulk_downlink_file_path_sha256_prefix);

    // ADCS fields (v2 and v5+).
    peripheral_comms_error_count += fill_beacon_adcs_fields(beacon_packet);

    *peripheral_comms_error_count_dst = peripheral_comms_error_count;
}


// MARK: LittleFS Ring Storage

/// @brief Build the LittleFS path of ring file `file_idx` (e.g. "beacon_ring/r3.bin").
static void make_ring_file_path(uint8_t file_idx, char *path_dest, uint16_t path_dest_size) {
    snprintf(path_dest, path_dest_size, "%s/r%d.bin", BEACON_RING_DIR, (int)file_idx);
}

/// @brief Is the persisted `open_file` handle still a live handle on the mounted filesystem?
/// @details Same as in the GNSS ring blob: littlefs links every open file into
///     `LFS_filesystem.mlist`, so walking that list is a cheap and authoritative check that the
///     currently-mounted filesystem still knows about our handle (it won't after a reboot or a
///     remount).
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
    return false;
}

/// @brief Close the persisted write handle, committing its pending records, and mark it invalid.
/// @details Safe to call at any time: does nothing if no live handle is currently held.
static void ring_close_open_file() {
    if (ring_open_file_is_live()) {
        const int close_result = lfs_file_close(&LFS_filesystem, &g_state->open_file);
        if (close_result < 0) {
            blob_log(LOG_SEVERITY_ERROR, "lfs_file_close(r%d) -> %d", g_state->open_file_idx, close_result
            );
        }
    }

    g_state->open_file_is_valid = 0;
}

/// @brief Write the current cursor to BEACON_RING_CURSOR_PATH (see BEACON_ring_cursor_file_t).
/// @details A failed write may leave the file empty/short, which makes recovery discard the ring
///     (fail-safe), rather than leave a stale cursor behind.
static BLOB_ext_beacon_error_enum_t ring_write_cursor_file() {
    BEACON_ring_cursor_file_t cursor;
    memset(&cursor, 0, sizeof(cursor));
    cursor.magic = BEACON_RING_CURSOR_MAGIC;
    cursor.record_size = BEACON_RING_RECORD_SIZE;
    cursor.records_per_file = BEACON_RING_RECORDS_PER_FILE;
    cursor.file_count = BEACON_RING_FILE_COUNT;
    cursor.write_file_idx = g_state->write_file_idx;
    cursor.has_wrapped = g_state->has_wrapped;

    lfs_file_t file;
    int result = lfs_file_open(
        &LFS_filesystem, &file, BEACON_RING_CURSOR_PATH, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC
    );
    if (result >= 0) {
        const lfs_ssize_t write_len = lfs_file_write(&LFS_filesystem, &file, &cursor, sizeof(cursor));
        const int close_result = lfs_file_close(&LFS_filesystem, &file);
        result = (write_len != (lfs_ssize_t)sizeof(cursor)) ? -1 : close_result;
    }
    if (result < 0) {
        blob_log(LOG_SEVERITY_ERROR, "writing %s -> %d", BEACON_RING_CURSOR_PATH, result);
        return BLOB_ERR_LFS_WRITE_FAILED;
    }
    return BLOB_ERR_OK;
}

/// @brief Make sure `g_state->open_file` is open on ring file `write_file_idx`, positioned at the
///     cursor, ready to write.
/// @details Reuses the handle left open by a previous execution whenever it's still live, which is
///     the common case: the open happens once per ring file, not once per record.
/// @return BLOB_ERR_OK if a live handle is ready, or the reason it isn't.
static BLOB_ext_beacon_error_enum_t ring_open_write_file() {
    if (ring_open_file_is_live()) {
        if (g_state->open_file_idx == g_state->write_file_idx) {
            return BLOB_ERR_OK; // Already open on the right file: nothing to do.
        }
        ring_close_open_file(); // Shouldn't happen (closed when full), but commit it if so.
    }
    g_state->open_file_is_valid = 0;

    const int mkdir_result = lfs_mkdir(&LFS_filesystem, BEACON_RING_DIR);
    if ((mkdir_result != 0) && (mkdir_result != LFS_ERR_EXIST)) {
        blob_log(LOG_SEVERITY_ERROR, "lfs_mkdir(%s) -> %d", BEACON_RING_DIR, mkdir_result);
        return BLOB_ERR_LFS_MKDIR_FAILED;
    }

    // Starting a new file: persist the cursor first, so a reboot from here on recovers to this file.
    if (g_state->write_record_idx == 0) {
        const BLOB_ext_beacon_error_enum_t cursor_status = ring_write_cursor_file();
        if (cursor_status != BLOB_ERR_OK) {
            return cursor_status;
        }
    }

    char path[BEACON_RING_PATH_MAX_LEN];
    make_ring_file_path(g_state->write_file_idx, path, sizeof(path));

    // Starting a file at record 0 means creating it, or TRUNCATE-ing an old file we've now wrapped
    // around to -- that's how the ring evicts its oldest data.
    const int open_flags = LFS_O_WRONLY | LFS_O_CREAT | (
        (g_state->write_record_idx == 0) ? LFS_O_TRUNC : 0
    );

    const int open_result = lfs_file_open(&LFS_filesystem, &g_state->open_file, path, open_flags);
    if (open_result < 0) {
        blob_log(LOG_SEVERITY_ERROR, "lfs_file_open(%s) -> %d", path, open_result);
        return BLOB_ERR_LFS_WRITE_FAILED;
    }
    g_state->open_file_idx = g_state->write_file_idx;
    g_state->open_file_is_valid = 1;

    // Resuming a partly-full file (e.g., after a run-once exit closed it): seek to the cursor's
    // exact slot (littlefs zero-fills any gap), so the file layout always matches the cursor.
    const lfs_soff_t seek_result = lfs_file_seek(
        &LFS_filesystem, &g_state->open_file,
        (lfs_soff_t)g_state->write_record_idx * BEACON_RING_RECORD_SIZE, LFS_SEEK_SET
    );
    if (seek_result < 0) {
        blob_log(LOG_SEVERITY_ERROR, "lfs_file_seek(%s) -> %ld", path, (long)seek_result);
        ring_close_open_file();
        return BLOB_ERR_LFS_WRITE_FAILED;
    }

    return BLOB_ERR_OK;
}

/// @brief Append one beacon record to the ring, advancing (and wrapping/truncating) as needed.
/// @details Writes straight through the persistent handle with lfs_file_write(). The file stays
///     open afterwards, and is only closed -- which is what commits its records -- once it fills
///     up (or when the blob exits without rescheduling).
/// @return BLOB_ERR_OK on success, or the reason the record wasn't stored.
static BLOB_ext_beacon_error_enum_t store_beacon_to_ring(const COMMS_beacon_extended_packet_v5plus_t *beacon_packet) {
    const BLOB_ext_beacon_error_enum_t open_status = ring_open_write_file();
    if (open_status != BLOB_ERR_OK) {
        return open_status;
    }

    const lfs_ssize_t write_result = lfs_file_write(
        &LFS_filesystem, &g_state->open_file, beacon_packet, BEACON_RING_RECORD_SIZE
    );
    if (write_result != (lfs_ssize_t)BEACON_RING_RECORD_SIZE) {
        blob_log(LOG_SEVERITY_ERROR, "lfs_file_write(r%d, record %d) -> %ld", g_state->write_file_idx, g_state->write_record_idx, (long)write_result
        );
        // The handle's write position is now unknown; drop it so the next run reopens (and
        // re-seeks) cleanly.
        ring_close_open_file();
        return BLOB_ERR_LFS_WRITE_FAILED;
    }

    g_state->stored_record_count++;
    g_state->write_record_idx++;

    if (g_state->write_record_idx >= BEACON_RING_RECORDS_PER_FILE) {
        // This file is full: advance the cursor and close the file (which commits its records).
        // The next record truncates/creates the next file.
        g_state->write_record_idx = 0;
        g_state->write_file_idx++;
        if (g_state->write_file_idx >= BEACON_RING_FILE_COUNT) {
            g_state->write_file_idx = 0;
            g_state->has_wrapped = 1;
        }
        ring_close_open_file();
    }

    return BLOB_ERR_OK;
}


// MARK: Ring Recovery

/// @brief Does a record read back from the ring look like a beacon this version stored?
static bool ring_record_looks_valid(const COMMS_beacon_extended_packet_v5plus_t *record) {
    return (
        (record->packet_type == COMMS_PACKET_TYPE_BEACON_EXTENDED_V5PLUS)
        && (memcmp(record->satellite_name, "CTS1", 4) == 0)
        && (memcmp(record->end_message, " X6", 4) == 0)
    );
}

/// @brief Is ring file `file_idx` exactly full, with valid-looking first and last records?
/// @note Spot-checks only 2 records: reading all of them for every file would take too long.
static bool ring_file_is_full_and_valid(uint8_t file_idx) {
    char path[BEACON_RING_PATH_MAX_LEN];
    make_ring_file_path(file_idx, path, sizeof(path));

    lfs_file_t file;
    if (lfs_file_open(&LFS_filesystem, &file, path, LFS_O_RDONLY) < 0) {
        blob_log(LOG_SEVERITY_WARNING, "recovery: can't open %s", path);
        return false;
    }

    bool ok = (
        lfs_file_size(&LFS_filesystem, &file)
        == (lfs_soff_t)(BEACON_RING_RECORDS_PER_FILE * BEACON_RING_RECORD_SIZE)
    );
    const uint16_t check_record_idxs[2] = {0, BEACON_RING_RECORDS_PER_FILE - 1};
    for (uint8_t i = 0; ok && (i < 2); i++) {
        COMMS_beacon_extended_packet_v5plus_t record;
        ok = (
            (lfs_file_seek(&LFS_filesystem, &file, (lfs_soff_t)check_record_idxs[i] * BEACON_RING_RECORD_SIZE, LFS_SEEK_SET) >= 0)
            && (lfs_file_read(&LFS_filesystem, &file, &record, BEACON_RING_RECORD_SIZE) == (lfs_ssize_t)BEACON_RING_RECORD_SIZE)
            && ring_record_looks_valid(&record)
        );
    }
    lfs_file_close(&LFS_filesystem, &file);

    if (!ok) {
        blob_log(LOG_SEVERITY_WARNING, "recovery: %s is not full/valid", path);
    }
    return ok;
}

/// @brief After a reset of the in-RAM state, restore the write cursor from BEACON_RING_CURSOR_PATH,
///     if it and every file it says is full look good.
/// @details Leaves g_state untouched (fresh: r0, record 0, not wrapped, no history) on any doubt.
///     On success, the cursor restarts at record 0 of the file that was being written (its
///     uncommitted or partial records are discarded when it's truncated).
/// @return true if the cursor was restored.
static bool ring_recover_from_fs() {
    BEACON_ring_cursor_file_t cursor;
    memset(&cursor, 0, sizeof(cursor));

    lfs_file_t file;
    if (lfs_file_open(&LFS_filesystem, &file, BEACON_RING_CURSOR_PATH, LFS_O_RDONLY) < 0) {
        return false; // Never written (e.g., first ever run, or reformatted).
    }
    const lfs_soff_t file_size = lfs_file_size(&LFS_filesystem, &file);
    const lfs_ssize_t read_len = lfs_file_read(&LFS_filesystem, &file, &cursor, sizeof(cursor));
    lfs_file_close(&LFS_filesystem, &file);

    if (
        (file_size != (lfs_soff_t)sizeof(cursor))
        || (read_len != (lfs_ssize_t)sizeof(cursor))
        || (cursor.magic != BEACON_RING_CURSOR_MAGIC)
        || (cursor.record_size != BEACON_RING_RECORD_SIZE)
        || (cursor.records_per_file != BEACON_RING_RECORDS_PER_FILE)
        || (cursor.file_count != BEACON_RING_FILE_COUNT)
        || (cursor.write_file_idx >= BEACON_RING_FILE_COUNT)
        || (cursor.has_wrapped > 1)
        || (cursor.reserved != 0)
    ) {
        blob_log(LOG_SEVERITY_WARNING, "recovery: %s is invalid", BEACON_RING_CURSOR_PATH);
        return false;
    }

    // Same set of files downlink_history_beacons() will read from.
    const uint8_t full_file_count = cursor.has_wrapped ? (BEACON_RING_FILE_COUNT - 1) : cursor.write_file_idx;
    for (uint8_t steps_back = 1; steps_back <= full_file_count; steps_back++) {
        const uint8_t file_idx = (uint8_t)(
            (cursor.write_file_idx + BEACON_RING_FILE_COUNT - steps_back) % BEACON_RING_FILE_COUNT
        );
        if (!ring_file_is_full_and_valid(file_idx)) {
            return false;
        }
    }

    g_state->write_file_idx = cursor.write_file_idx;
    g_state->write_record_idx = 0;
    g_state->has_wrapped = cursor.has_wrapped;
    return true;
}


// MARK: History Downlink

/// @brief Downlink a randomly-chosen run of consecutive stored beacons: pick a random full ring
///     file, pick a random record index within it, and send up to `history_count` consecutive
///     records from there (stopping at the end of the file).
/// @details Same selection logic as the GNSS ring blob. The file the write cursor is on is never
///     picked (it's partly full). Files left over from a previous campaign (a cold start zeroes
///     the cursor without erasing the disk) are excluded by counting only the files behind the
///     cursor until the ring has wrapped.
///
///     Records are stored exactly as generated (" X6\0"). Each is sent as-is, except that its
///     end_message is replaced with " S6\0" after reading it back, so the ground can tell it apart
///     from a live beacon.
///
///     If any record sent has mpi_rx_mode_enum = sensing, the run is extended to
///     BEACON_HISTORY_MPI_EXTENDED_MAX records (or the end of the file, if sooner), to prioritize
///     the beacons stored while the MPI was active.
/// @param history_count Number of records to send (0 = none).
/// @param sent_count_dest Set to the number of packets actually sent. Never NULL.
/// @return Number of downlink failures (0 = all succeeded).
static uint16_t downlink_history_beacons(uint16_t history_count, uint16_t *sent_count_dest) {
    *sent_count_dest = 0;

    // Every file behind the cursor holds exactly BEACON_RING_RECORDS_PER_FILE records.
    const uint8_t full_file_count = g_state->has_wrapped
        ? (BEACON_RING_FILE_COUNT - 1) // Everything except the file the cursor is on.
        : g_state->write_file_idx;     // Only what we've filled since the last cold start.

    if ((history_count == 0) || (full_file_count == 0)) {
        return 0;
    }

    // 1 step back = the most recently filled file; full_file_count steps back = the oldest one.
    const uint32_t file_rand = CRYPTO_generate_random_uint32(TIME_uptime_ms());
    const uint8_t steps_back = (uint8_t)(1u + (file_rand % full_file_count));
    const uint8_t chosen_file_idx = (uint8_t)(
        (g_state->write_file_idx + BEACON_RING_FILE_COUNT - steps_back) % BEACON_RING_FILE_COUNT
    );

    const uint32_t record_rand = CRYPTO_generate_random_uint32(TIME_uptime_ms() + 1);
    const uint16_t start_record_idx = (uint16_t)(record_rand % BEACON_RING_RECORDS_PER_FILE);

    const uint16_t available_count = (uint16_t)(BEACON_RING_RECORDS_PER_FILE - start_record_idx);
    uint16_t n_to_downlink = (history_count < available_count) ? history_count : available_count;

    char path[BEACON_RING_PATH_MAX_LEN];
    make_ring_file_path(chosen_file_idx, path, sizeof(path));

    lfs_file_t file;
    const int open_result = lfs_file_open(&LFS_filesystem, &file, path, LFS_O_RDONLY);
    if (open_result < 0) {
        blob_log(LOG_SEVERITY_ERROR, "lfs_file_open(%s) -> %d", path, open_result);
        return n_to_downlink;
    }

    const lfs_soff_t seek_result = lfs_file_seek(
        &LFS_filesystem, &file, (lfs_soff_t)start_record_idx * BEACON_RING_RECORD_SIZE, LFS_SEEK_SET
    );
    if (seek_result < 0) {
        blob_log(LOG_SEVERITY_WARNING, "lfs_file_seek(%s) -> %ld", path, (long)seek_result);
        lfs_file_close(&LFS_filesystem, &file);
        return n_to_downlink;
    }

    uint16_t fail_count = 0;
    for (uint16_t i = 0; i < n_to_downlink; i++) {
        COMMS_beacon_extended_packet_v5plus_t packet;
        const lfs_ssize_t read_len = lfs_file_read(&LFS_filesystem, &file, &packet, BEACON_RING_RECORD_SIZE);
        if (read_len != (lfs_ssize_t)BEACON_RING_RECORD_SIZE) {
            blob_log(LOG_SEVERITY_WARNING, "short read (%ld) at %s record %d", (long)read_len, path, start_record_idx + i
            );
            fail_count += (uint16_t)(n_to_downlink - i); // The rest of the run is unreachable too.
            break;
        }

        // Recovery only spot-checks each file's first/last record; never re-broadcast garbage.
        if (!ring_record_looks_valid(&packet)) {
            blob_log(LOG_SEVERITY_WARNING, "invalid record at %s record %d", path, start_record_idx + i);
            fail_count++;
            continue;
        }

        // Stored while the MPI was sensing: prioritize these, by extending the run (capped).
        if (packet.mpi_rx_mode_enum == MPI_RX_MODE_SENSING_MODE) {
            const uint16_t extended_count = (available_count < BEACON_HISTORY_MPI_EXTENDED_MAX)
                ? available_count : BEACON_HISTORY_MPI_EXTENDED_MAX;
            if (extended_count > n_to_downlink) {
                n_to_downlink = extended_count; // Never shortens a longer history_count run.
            }
        }

        // Stored as generated; mark it as a ring-stored (historical) beacon only now.
        memcpy(packet.end_message, " S6", 4);

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


// MARK: No-Uplink EPS Reset
// Purpose: On the first "no uplink" reboot, do an EPS reboot. Then, after the reboot,
// the main OBC firmware will take over and just do OBC-only reboots.

// Fire this long before the firmware's own no-uplink OBC reboot, so we power-cycle everything instead.
#define NO_UPLINK_EPS_RESET_MARGIN_SEC 300

/// @brief If no uplink has been received for (STM32_system_reset_no_uplink_interval_sec - 5 min),
///     commit the ring file and command an EPS system reset (power-cycles the whole satellite,
///     not just the OBC like the firmware's native no-uplink reboot).
/// @details Deliberately has no "disabled" case: a threshold of 0 (or <= the margin) triggers the
///     reset immediately, so the config can't be used to bypass this. Only returns if the reset failed.
static void reset_eps_if_no_recent_uplink() {
    const uint32_t threshold_sec = STM32_system_reset_no_uplink_interval_sec;
    // Saturate at 0 (instead of underflowing), so a 0/small threshold resets right away.
    const uint32_t trigger_sec = (threshold_sec > NO_UPLINK_EPS_RESET_MARGIN_SEC)
        ? (threshold_sec - NO_UPLINK_EPS_RESET_MARGIN_SEC)
        : 0;

    const uint32_t time_since_last_uplink_sec = (
        (TIME_uptime_ms() - AX100_uptime_at_last_received_kiss_tcmd_ms) / 1000
    );
    if (time_since_last_uplink_sec < trigger_sec) {
        return;  // Happy path: Just leave this function.
    }

    // We could do `ring_close_open_file();` to commit beacons, but not really necessary,
    // and just adds complexity. Continue.
    blob_log(
        LOG_SEVERITY_CRITICAL,
        "no uplink for %lu sec (native reboot at %lu sec); EPS system reset",
        (unsigned long)time_since_last_uplink_sec, (unsigned long)threshold_sec
    );
    HAL_Delay(1000); // Give time for the log to be sent.

    const uint8_t eps_result = EPS_CMD_system_reset();
    blob_log(LOG_SEVERITY_ERROR, "EPS_CMD_system_reset() -> %d; continuing", eps_result);
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

    const uint16_t args_str_len = strlen(args_str);
    uint16_t pos = 0;

    char arg0_beacon_interval_ms[20];
    pos = parse_token(args_str, pos, args_str_len, arg0_beacon_interval_ms, sizeof(arg0_beacon_interval_ms));

    if (arg0_beacon_interval_ms[0] == '\0') {
        // Missing args error.
        snprintf(
            response_buf, response_buf_len,
            "%s error: %s",
            BLOB_NAME,
            ext_beacon_blob_error_to_str(BLOB_ERR_MISSING_ARGS)
        );
        return BLOB_ERR_MISSING_ARGS;
    }

    bool arg0_beacon_interval_ms_ok;
    int32_t beacon_interval_ms = parse_int(arg0_beacon_interval_ms, &arg0_beacon_interval_ms_ok);

    // Remaining tokens: the optional positional history_count, then kwargs (key=value).
    bool arg1_history_count_ok = true;
    int32_t history_count = 0;
    uint32_t mpi_interval_ms = BEACON_MPI_INTERVAL_MS_DEFAULT;
    for (uint8_t arg_num = 1; pos < args_str_len; arg_num++) {
        char token[40];
        pos = parse_token(args_str, pos, args_str_len, token, sizeof(token));

        const char *key;
        uint32_t value;
        if (parse_kwarg(token, &key, &value) && (strcmp(key, "mpi_interval_ms") == 0)) {
            mpi_interval_ms = value;
        }
        else if ((arg_num == 1) && (token[0] != '\0')) {
            // Also catches a malformed kwarg here (parse_kwarg() cut it at the '=').
            history_count = parse_int(token, &arg1_history_count_ok);
        }
        else if (token[0] != '\0') { // Tolerate empty tokens (e.g., trailing ';').
            snprintf(
                response_buf, response_buf_len,
                "%s error: %s '%s'",
                BLOB_NAME, ext_beacon_blob_error_to_str(BLOB_ERR_INVALID_KWARGS), token
            );
            return BLOB_ERR_INVALID_KWARGS;
        }
    }

    if (!arg0_beacon_interval_ms_ok || !arg1_history_count_ok) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: %s",
            BLOB_NAME,
            ext_beacon_blob_error_to_str(BLOB_ERR_INVALID_INT_ARGS)
        );
        return BLOB_ERR_INVALID_INT_ARGS;
    }

    // Protect the minimum repeat interval! Too low of value would make it so we can't uplink to
    // the satellite because it'd always be transmitting, which would be super non-ideal.
    if ((beacon_interval_ms != 0) && (beacon_interval_ms < 2500)) {
        beacon_interval_ms = 2500;
    }

    // Likewise, cap the history burst, so one run can't flood the AX100 / hog the radio.
    if (history_count > BEACON_HISTORY_COUNT_MAX) {
        history_count = BEACON_HISTORY_COUNT_MAX;
    }

    // Leave the telecommand task time for everything else (each run takes a while to fill the beacon).
    if (mpi_interval_ms < BEACON_MPI_INTERVAL_MS_MIN) {
        mpi_interval_ms = BEACON_MPI_INTERVAL_MS_MIN;
    }

    // True cold-start: first ever run, or a power cycle that cleared SRAM. Nothing in the state
    // block can be trusted, including the open file handle and the canary pointer, so reset
    // without dereferencing either. The cursor is recovered from the filesystem later (before
    // storing), via needs_fs_recovery.
    if (g_state->magic != BEACON_RING_STATE_MAGIC) {
        reset_to_fresh_state(false);
    }

    // Now that the state block is known-good, check whether the heap restarted under us since the
    // previous run. If it did, the lfs_file_t we persisted points at a cache buffer from a heap
    // that no longer exists, and we can't trust the cursor to match what's on disk either -- so
    // start the ring over, exactly as a cold start would.
    if (detect_reboot_via_heap_canary()) {
        reset_to_fresh_state(true);
    }

    // This blob never mounts the filesystem itself. While it's unmounted, our cursor's claim about
    // what's on disk is no longer worth anything, so start over (on every such run), rather than
    // appending into the unknown once it's remounted. Unlike the GNSS ring blob, we keep going:
    // the live beacon is still sent and the blob still reschedules; storage and history are just
    // skipped below.
    if (!LFS_is_lfs_mounted) {
        reset_to_fresh_state(true);
    }

    // A handle we believe we're holding, that the currently-mounted filesystem doesn't know
    // about, means the filesystem was unmounted/remounted or reformatted under us WITHOUT a
    // reboot -- so the heap canary above saw nothing wrong. Our cursor no longer describes what's
    // on disk, so start over. Note the `open_file_is_valid` guard: ring_open_file_is_live() also
    // returns false in the perfectly normal case where we simply aren't holding a handle, and
    // that must not trigger a reset.
    if (g_state->open_file_is_valid && (!ring_open_file_is_live())) {
        blob_log(LOG_SEVERITY_WARNING, "stale ring file handle; resetting");
        reset_to_fresh_state(true);
    }

    // Fill the beacon packet.
    COMMS_beacon_extended_packet_v5plus_t beacon_packet;
    uint8_t peripheral_comms_error_count = 0;
    COMMS_fill_beacon_extended_packet_v5plus(&beacon_packet, &peripheral_comms_error_count);

    // Snapshot once, so the whole run agrees on the MPI state.
    const bool mpi_active = (MPI_current_uart_rx_mode == MPI_RX_MODE_SENSING_MODE);

    // MPI fast mode (note 13): rerun every mpi_interval_ms, to store beacons faster.
    const bool mpi_fast_mode = (
        (beacon_interval_ms > 0) && mpi_active && (mpi_interval_ms < (uint32_t)beacon_interval_ms)
    );

    // Fast runs only add stored beacons; the live beacon keeps (roughly) its normal cadence, so the
    // radio stays quiet for the MPI and uplinks still get through. Sent on the first fast run at
    // least (beacon_interval_ms - mpi_interval_ms) after the last one, so it's never sent less often.
    const uint32_t now_ms = TIME_uptime_ms();
    const bool send_live_beacon = (
        (!mpi_fast_mode)
        || (g_state->last_live_tx_uptime_ms == 0)
        || ((now_ms - g_state->last_live_tx_uptime_ms) >= ((uint32_t)beacon_interval_ms - mpi_interval_ms))
    );

    // Downlink the beacon packet first, so the filesystem write doesn't add latency to the live beacon.
    uint8_t tx_status = 0;
    if (send_live_beacon) {
        tx_status = AX100_downlink_bytes(
            (uint8_t *)(&beacon_packet),
            sizeof(COMMS_beacon_extended_packet_v5plus_t)
        );
        g_state->last_live_tx_uptime_ms = now_ms;
    }

    // Safety net, checked after the beacon is out.
    reset_eps_if_no_recent_uplink();

    // Then store it into the ring (even if the downlink failed). Never mounts the filesystem
    // itself; just skips if unmounted.
    BLOB_ext_beacon_error_enum_t store_status = BLOB_ERR_LFS_NOT_MOUNTED;
    if (LFS_is_lfs_mounted) {
        // First mounted run since a reset: try to pick up where the stored ring left off.
        if (g_state->needs_fs_recovery) {
            g_state->needs_fs_recovery = 0; // One attempt only; a failure just means a fresh start.
            const bool recovered = ring_recover_from_fs();
            blob_log(LOG_SEVERITY_NORMAL, "ring recovery: %s (cursor=r%d%s)", recovered ? "restored" : "fresh start",
                g_state->write_file_idx, g_state->has_wrapped ? ", wrapped" : ""
            );
        }
        store_status = store_beacon_to_ring(&beacon_packet);
    }
    if (store_status != BLOB_ERR_OK) {
        g_state->store_failure_count++;
    }

    if (tx_status != 0) {
        ring_close_open_file(); // Not rescheduling; commit what we have.
        snprintf(
            response_buf, response_buf_len,
            "%s error: %s (%d)%s%s", // %d = AX100_downlink_bytes() result. Likely will never see this error message.
            BLOB_NAME,
            ext_beacon_blob_error_to_str(BLOB_ERR_DOWNLINK_FAILED),
            tx_status,
            "", ""
        );
        return BLOB_ERR_DOWNLINK_FAILED;
    }

    // Downlink a random consecutive run of historical beacons, unless the MPI is actively
    // collecting science data (stay off the radio as much as possible then).
    const bool skip_history_for_mpi = mpi_active;
    uint16_t history_sent_count = 0;
    uint16_t history_fail_count = 0;
    if (LFS_is_lfs_mounted && !skip_history_for_mpi) {
        history_fail_count = downlink_history_beacons((uint16_t)history_count, &history_sent_count);
    }

    // Ring summary, appended to every response below.
    char ring_msg[110];
    snprintf(
        ring_msg, sizeof(ring_msg),
        ", store=%s, cursor=r%d/%d%s, stored=%lu, store_fails=%lu, history_sent=%d/%d%s",
        ext_beacon_blob_error_to_str(store_status),
        g_state->write_file_idx, g_state->write_record_idx,
        g_state->has_wrapped ? " (wrapped)" : "",
        (unsigned long)g_state->stored_record_count,
        (unsigned long)g_state->store_failure_count,
        history_sent_count, history_sent_count + history_fail_count,
        skip_history_for_mpi ? " (mpi_active)" : ""
    );

    // Reschedule, unless another run of this blob (e.g., a scheduled reconfiguration) is due
    // within 2 intervals, in which case that run takes over.
    const char *resched_msg = "";
    if (mpi_fast_mode) {
        resched_msg = send_live_beacon ? ", mpi_fast" : ", mpi_fast (live skipped)";
    }
    if (beacon_interval_ms > 0) {
        const uint32_t next_interval_ms = mpi_fast_mode ? mpi_interval_ms : (uint32_t)beacon_interval_ms;
        const int8_t resched_result = BLOB_reschedule_self_unless_upcoming(next_interval_ms, BLOB_FAMILY_PREFIX);
        if (resched_result < 0) {
            const BLOB_ext_beacon_error_enum_t reexec_result = reschedule_err_to_blob_err(resched_result);
            ring_close_open_file(); // No rerun is coming to close it; commit what we have.
            snprintf(
                response_buf, response_buf_len,
                "%s error: %s%s",
                BLOB_NAME,
                ext_beacon_blob_error_to_str(reexec_result),
                ring_msg
            );
            return reexec_result;
        }
        if (resched_result == BLOB_RESCHEDULE_SKIPPED_UPCOMING) {
            // The upcoming run of this same blob keeps using the open ring file.
            resched_msg = ", not rescheduled (upcoming run takes over)";
        }
    }
    else {
        ring_close_open_file(); // Run-once mode: no rerun is coming to close it.
    }

    if (peripheral_comms_error_count > 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: %s (%d)%s%s", // %d = peripheral comms error count.
            BLOB_NAME,
            ext_beacon_blob_error_to_str(BLOB_ERR_PERIPHERAL_COMMS_FAILURES),
            peripheral_comms_error_count,
            ring_msg,
            resched_msg
        );
        return BLOB_ERR_PERIPHERAL_COMMS_FAILURES;
    }

    snprintf(
        response_buf, response_buf_len,
        "%s success%s%s",
        BLOB_NAME,
        ring_msg,
        resched_msg
    );

    // Storage/history problems are non-fatal (the beacon was sent and the blob rescheduled), but
    // are still reported via the return code.
    if (store_status != BLOB_ERR_OK) {
        return store_status;
    }
    if (history_fail_count > 0) {
        return BLOB_ERR_HISTORY_SEND_PARTIAL_FAILURE;
    }

    return BLOB_ERR_OK;
}
