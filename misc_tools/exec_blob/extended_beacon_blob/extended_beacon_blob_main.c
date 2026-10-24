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
// Args Format: <repeat_interval_ms>[;<history_count>]
// - repeat_interval_ms: 0 to run only once, or any positive number to run repeatedly at that
//   specified interval (clamped to a minimum of 2500ms).
// - history_count: Optional (default 0). Number of consecutive historical beacons to downlink per
//   run, after the live beacon. Fewer are sent if the randomly-chosen start lands near the end of
//   the chosen ring file.
//
// Usage Example:
// After uplinking the blob as "blobs/extended_beacon_v5.blob", run:
// CTS1+exec_blob_from_fs(blobs/extended_beacon_v5.blob,0,9000;5)!
//
// Notes:
//  1. Always use "0" as the second argument (i.e., always run with malloc).
//  2. If the ADCS fails to respond to the OBC, this blob hits the watchdog and crashes because
//      the ADCS communications each take about 3.5 seconds to time out. Thus, this blob cannot be
//      tested on a dev kit, and must be tested on the flatsat with the ADCS engg model computer.
//  3. This blob re-schedules itself at the specified interval. Each new scheduled telecommand gets
//      a tssent value of <interval_ms> after the beacon is sent.
//  4. If this blob is currently running in repeat mode, and you re-run it, it will first cancel
//      the existing repeat telecommand, and then re-schedule itself. That is, it is fine to send
//      a command to run this blob on every uplink pass, whether or not it's already running.
//  5. To stop the recurring rescheduling of this blob after starting it, you can use reboot, or
//      use `CTS1+agenda_delete_by_name(exec_blob_from_fs)`, or `CTS1+agenda_delete_all()`, or
//      `CTS1+exec_blob_from_fs(blobs/extended_beacon_v5.blob,0,0)!` (which will run one last time,
//      then cancel itself).
//  6. The beacon is always sent first. Storage and history downlink problems (e.g., filesystem
//      not mounted) never prevent the live beacon from being sent, nor the blob from rescheduling.
//  7. While the MPI is in active (sensing) mode, the live beacon is still sent and stored, but no
//      historical beacons are downlinked that run.
//  8. Nothing historical is downlinked until the first ring file fills (BEACON_RING_RECORDS_PER_FILE
//      beacons), because the file being written is never downlinked.
//  9. Historical beacons are sent with the same packet type, but with version string " S5\0"
//      (instead of " X5\0").
//
// ------ Implementation Notes ----------------
//
// Storage: Beacons live in LittleFS under "beacon_ring/", as a ring of BEACON_RING_FILE_COUNT
// files ("beacon_ring/r0.bin" .. "beacon_ring/r19.bin"), each holding BEACON_RING_RECORDS_PER_FILE
// fixed-size records (one raw COMMS_beacon_extended_packet_t each). When the current file fills,
// we advance to the next; after the last one we wrap back to r0 and truncate it, evicting the
// oldest data.
//
// Persistent RAM: Same approach as the GNSS ring blob. The write cursor lives in a small state
// block at a fixed SRAM address (see `BEACON_RING` in ../blobs_persistent_sram_regions.ld; separate
// from the GNSS ring blob's region, so both blobs can run at the same time). A magic number detects cold start,
// in which case the cursor restarts at r0/record 0.
//
// Open file handle: Same as the GNSS ring blob, the ring file we're currently appending to is left
// OPEN between executions (its lfs_file_t lives in the persistent state block), so storing a
// record is just an lfs_file_write(). Closes (which commit metadata to flash) are kept rare, to
// limit flash wear: the file is only closed when it fills up, or when the blob exits without
// rescheduling itself. The handle is only trusted after confirming it's still linked into the
// mounted filesystem's open-file list (see ring_open_file_is_live()).
//
// If the handle turns out to be stale (reboot, or filesystem unmount/remount), the uncommitted
// records in the current file are lost, and that file is restarted from record 0 (truncated).
// Unlike the GNSS ring blob, the rest of the ring is kept: every file behind the cursor was closed
// (committed) when it filled, so it's still valid on disk. On (re)opening a file, we seek to the
// cursor's exact offset (record_idx * record_size), so the file layout always matches the cursor.

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

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
#include "comms_drivers/ax100_hw.h"

typedef enum {
    LOG_SEVERITY_DEBUG = 1 << 0,
    LOG_SEVERITY_NORMAL = 1 << 1,
    LOG_SEVERITY_WARNING = 1 << 2,
    LOG_SEVERITY_ERROR = 1 << 3,
    LOG_SEVERITY_CRITICAL = 1 << 4,
} LOG_severity_enum_t;

static const uint32_t LOG_SYSTEM_UHF_RADIO = 1 << 1;
static const uint32_t LOG_SYSTEM_TELECOMMAND = 1 << 12;
static const uint32_t LOG_SINK_UHF_RADIO = 1 << 0;
static const uint32_t LOG_SINK_ALL = (1 << 4) - 1;

static const char ARG_DELIM = ';';
static const char BLOB_NAME[] = "extended_beacon_blob_v5";

// Global variables defined in the firmware ELF (CTS-SAT-1_FW_rc3.elf).
extern lfs_t LFS_filesystem;
extern volatile uint32_t TIME_uptime_ms_from_tim6;

// Beacon v1 contents (extern variables).
extern uint32_t AX100_uptime_at_last_received_kiss_tcmd_ms;
extern uint8_t LFS_is_lfs_mounted;
extern uint32_t COMMS_total_beacon_count_since_boot;
extern MPI_transceiver_state_enum_t MPI_current_transceiver_state;
extern MPI_reason_for_stopping_active_mode_enum_t MPI_last_reason_for_stopping_active_mode;
extern volatile uint8_t UART_gnss_uart_interrupt_enabled; // Flag to enable or disable the UART GNSS ISR
extern char COMMS_beacon_friendly_message_str[COMMS_BEACON_FRIENDLY_MESSAGE_SIZE];

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
extern uint8_t ADCS_get_raw_coarse_sun_sensor_1_to_6(ADCS_raw_coarse_sun_sensor_1_to_6_struct_t *output_struct);
extern uint8_t ADCS_get_raw_coarse_sun_sensor_7_to_10(ADCS_raw_coarse_sun_sensor_7_to_10_struct_t *output_struct);

extern volatile uint32_t MPI_buffer_one_last_filled_uptime_ms;
extern volatile uint32_t MPI_buffer_two_last_filled_uptime_ms;

// Beacon v5 contents (externs for the vendored AX100 downlink chain).
// hi2c1 is really an I2C_HandleTypeDef; only its address is used (via AX100_I2C_HANDLE), so it's
// declared opaquely to avoid pulling in the HAL headers. Likewise, HAL_StatusTypeDef is returned as
// a plain int (same ABI, as it's an enum).
extern uint8_t hi2c1[];
extern int HAL_I2C_Master_Transmit(void *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
extern void DEBUG_uart_print_str(const char *str);
extern void DEBUG_uart_print_mixed_array(const uint8_t *array, uint16_t array_len, const char *prefix);
extern uint32_t LOG_all_sinks_except(uint32_t exceptions);

extern int snprintf(char *buf, unsigned int size, const char *fmt, ...);
extern int strlen(const char *s);
extern int strcmp(const char *s1, const char *s2);
extern void *memset(void *s, int c, size_t n);
extern void *memcpy(void *__restrict dest, const void *__restrict src, size_t n);

extern void LOG_message(
    uint32_t source, LOG_severity_enum_t severity, uint32_t sink_mask,
    const char *fmt, ...
);


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

// MARK: Error Enum

// All named status/error codes returned by this blob's functions, including the values
// `blob_main()` itself returns to the telecommand executor (0 = success).
typedef enum {
    BLOB_ERR_OK = 0,
    BLOB_ERR_LFS_WRITE_FAILED = 5, // Writing the beacon record to LittleFS failed (beacon still sent).
    BLOB_ERR_LFS_MKDIR_FAILED = 6, // Couldn't create/confirm the BEACON_RING_DIR directory.
    BLOB_ERR_LFS_NOT_MOUNTED = 7, // Filesystem not mounted; skipped storage and history downlink.
    BLOB_ERR_DOWNLINK_FAILED = 60, // AX100_downlink_bytes_NEW() failed to send the beacon packet.
    BLOB_ERR_HISTORY_SEND_PARTIAL_FAILURE = 61, // At least one historical beacon failed to send.
    BLOB_ERR_PERIPHERAL_COMMS_FAILURES = 117, // Beacon was sent, but >=1 peripheral query failed.
    BLOB_ERR_MISSING_ARGS = 135, // One or more required args_str tokens were empty.
    BLOB_ERR_INVALID_INT_ARGS = 136, // One or more args_str tokens failed integer parsing.
    BLOB_ERR_CANCEL_RERUNS_FAILED = 137, // cancel_other_scheduled_reruns_of_this_blob() failed.
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
        case BLOB_ERR_CANCEL_RERUNS_FAILED: return "CANCEL_RERUNS_FAILED";
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

    // End with a null-terminated configurable friendly message.
    char friendly_message[COMMS_BEACON_FRIENDLY_MESSAGE_SIZE];

    // "END\0" on basic packets.
    // " X2\0" on extended packets (v2).
    // " X3\0" on extended packets (v3).
    // " X4\0" on extended packets (v4).
    // " X5\0" on extended packets (v5).
    // " S5\0" on ring-stored extended packets (v5), when downlinked as history.
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

    // The following fields are v5+.

    // Counts the distinct (tcmd_idx, tcmd_args) pairs in the pending queue, plus the
    // currently-executing tcmd (same as pending_queued_tcmd_count's +1). Equal to
    // pending_queued_tcmd_count when nothing is duplicated.
    // Useful to confirm that all uplinks were successful.
    uint8_t distinct_tcmds_pending;

    // Duration since time sync (clamped to max of 2^16 seconds = 18 hours)
    uint16_t duration_since_last_time_sync_sec;

    // Absolute speed of the fastest of the 3 reaction wheels.
    uint16_t adcs_fastest_wheel_speed_rpm;

    // Rotation angle of the quaternion error vector (commanded vs. estimated attitude,
    // Telemetry ID 163), 0 to 18000. Only meaningful in control modes with a commanded attitude.
    uint16_t adcs_pointing_error_cdeg;

    // Magnitude of the estimation innovation vector (Telemetry ID 162), in units of 0.0001.
    uint16_t adcs_innovation_vector_magnitude;

    // GNSS EPS channel (CH8) power, from the PDU running average.
    uint8_t gnss_power_consumed_cW;

    // Sum of ADCS current telemetry (IDs 172, 173, 198) times assumed rail voltages. See
    // calculate_adcs_total_power_cW() and the ADCS_*_SUPPLY_MV assumptions.
    // uint16, since peaks (~4.5 W per the CubeADCS ICD) exceed a uint8's 2.55 W.
    uint16_t adcs_total_power_consumed_cW;

} COMMS_beacon_extended_packet_t;

// Packet size (currently):
_Static_assert(sizeof(COMMS_beacon_extended_packet_t) == 210, "extended beacon packet size documented wrong here");

#pragma pack(pop)


// MARK: Ring Storage Layout

#define BEACON_RING_DIR "beacon_ring"
#define BEACON_RING_FILE_COUNT 20 // Number of files in the ring, before wrapping back to r0.bin.
#define BEACON_RING_RECORDS_PER_FILE 180 // Records per file. 20*180 = 3600 beacons = 9 hours at 9s.
#define BEACON_RING_RECORD_SIZE (sizeof(COMMS_beacon_extended_packet_t))
#define BEACON_RING_PATH_MAX_LEN 32 // Enough for "beacon_ring/r<n>.bin".


// MARK: Persistent State

// Defined by blob.ld: a NOLOAD symbol anchored at the start of the fixed `BEACON_RING` region.
extern uint8_t beacon_ring_state_base[];

#define BEACON_RING_STATE_REGION_LEN 0x100u // Must match BEACON_RING's LENGTH in ../blobs_persistent_sram_regions.ld.
#define BEACON_RING_STATE_MAGIC 0xBEAC0005u

// NOT packed, deliberately: it holds a live lfs_file_t that littlefs accesses through an ordinary
// (alignment-assuming) pointer. Same as GNSS_ring_state_t.
typedef struct {
    uint32_t magic; // BEACON_RING_STATE_MAGIC once initialized; anything else means cold/garbage SRAM.

    uint8_t write_file_idx; // Ring file currently being appended to, 0..(BEACON_RING_FILE_COUNT-1).
    uint8_t write_record_idx; // Records already in that file, 0..BEACON_RING_RECORDS_PER_FILE.
    uint8_t has_wrapped; // 1 once we've cycled past the last file at least once (all files have data).

    uint32_t stored_record_count; // Total records written to LittleFS, since cold-init.
    uint32_t store_failure_count; // Failed attempts to store a record, since cold-init.

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
///     The executing entry can't duplicate a pending one, as
///     cancel_other_scheduled_reruns_of_this_blob() has already removed this blob's other reruns.
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
            // Can't determine other blob file name; don't risk cancelling the wrong entries.
            // Could log a warning here.
            continue;
        }

        if (strcmp(own_blob_file_name, other_blob_file_name) == 0) {
            // Main action: Cancel that entry!
            TCMD_agenda_is_valid[slot_num] = TCMD_AGENDA_ENTRY_INVALID;
            cancelled_count++;
        }
    }

    return cancelled_count;
}

/// @brief Enqueue a copy of the currently executing tcmd, with a new time into the future to execute.
/// @note This function avoids incrementing the "total telecommands" counter. Goal: Allow that counter to assess how many uplinked commands were successful.
static BLOB_ext_beacon_error_enum_t reschedule_current_blob_tcmd(uint32_t time_into_future_to_execute_ms) {
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

    // Added in v3+.
    // Undo the counter increase in `TCMD_add_tcmd_to_agenda()`.
    TCMD_total_tcmd_queued_count--;

    return BLOB_ERR_OK;
}

// MARK: Vendored AX100 TX

// VENDORING NOTE: The AX100_downlink_bytes() chain (comms_drivers/ax100_tx.c) is vendored here
// with a _NEW suffix, because the core FW version truncates every packet to
// AX100_DOWNLINK_MAX_BYTES (200), and this beacon is longer than that. Changes vs. the original:
//  1. AX100_downlink_bytes_NEW() has no length limit (no AX100_DOWNLINK_MAX_BYTES check/truncation).
//  2. The packet buffer is on the stack and sized to fit, instead of a static buffer (a blob can't
//     have .bss) of AX100_DOWNLINK_MAX_BYTES + AX100_CSP_HEADER_LENGTH_BYTES.
// Otherwise identical (same CSP header, same antenna-deployment inhibit, same logging).

static const uint32_t csp_priority = 3u << 30; // priority

static const uint32_t own_csp_addr = 1u << 25 ; // src address
static const uint32_t ground_station_csp_addr = 10u << 20; // dst address

static const uint32_t ground_station_csp_port = 10u << 14; // dst port
static const uint32_t own_csp_port = 10u << 8; // src port

static const uint32_t use_hmac = 0 << 3; // hmac
static const uint32_t use_xtea = 0 << 2; // xtea
static const uint32_t use_rdp = 0 << 1; // rdp
static const uint32_t use_crc = 0; // crc

static const uint32_t AX100_i2c_tx_timeout_ms = 100;

/// @brief Write the csp header (4 bytes) to the beginning of the packet, then copy the rest of `data` into `destination`.
/// @param destination where the assembled packet will be stored
/// @param data data to place in the packet
/// @param data_len length of data
static void prepend_csp_header_NEW(uint8_t *destination, uint8_t *data, uint32_t data_len) {
    const uint32_t csp_header = (
        csp_priority | own_csp_addr | ground_station_csp_addr |
        ground_station_csp_port | own_csp_port | use_hmac | use_xtea | use_rdp | use_crc
    );

    destination[0] = (csp_header >> 24) & 0xFF;
    destination[1] = (csp_header >> 16) & 0xFF;
    destination[2] = (csp_header >> 8) & 0xFF;
    destination[3] = csp_header & 0xFF;

    // Copy data into packet.
    memcpy(&destination[AX100_CSP_HEADER_LENGTH_BYTES], data, data_len);
}

/// @brief Send a csp packet over i2c to the AX100 for downlink.
static uint8_t send_bytes_to_ax100_NEW(uint8_t *packet, uint16_t packet_size) {
    if (CTS1_operation_state != CTS1_OPERATION_STATE_NOMINAL_WITH_RADIO_TX) {
        // Recall: Do not transmit on the AX100 until the antenna is deployed.
        // The Bootup Operation FSM task will set the operation mode to NOMINAL_WITH_RADIO_TX
        // when the antenna is deployed.
        if (AX100_enable_downlink_inhibited_uart_logs) {
            DEBUG_uart_print_str("AX100 downlink inhibited because antenna not deployed.\n");
        }

        // Return success to avoid lots of errors. It's not really an error case, as this is expected during early ops.
        return 0;
    }

    const int status = HAL_I2C_Master_Transmit(
        AX100_I2C_HANDLE,
        AX100_I2C_ADDR << 1,
        packet,
        packet_size,
        AX100_i2c_tx_timeout_ms
    );

    if (status != 0) { // HAL_OK
        LOG_message(
            LOG_SYSTEM_UHF_RADIO, LOG_SEVERITY_ERROR, LOG_all_sinks_except(LOG_SINK_UHF_RADIO),
            "HAL I2C transmit error: %d", status
        );
        return status;
    }

    return 0;
}

/// @brief Send data to the ax100 for downlink. Same as AX100_downlink_bytes(), but without the
///     AX100_DOWNLINK_MAX_BYTES limit.
/// @param data pointer to the data to downlink
/// @param data_len length of the data
/// @return 0 on success, >0 on error/failure
static uint8_t AX100_downlink_bytes_NEW(uint8_t *data, uint16_t data_len) {
    uint8_t packet_buffer_including_csp_header[data_len + AX100_CSP_HEADER_LENGTH_BYTES];

    prepend_csp_header_NEW(packet_buffer_including_csp_header, data, data_len);

    // Any network layer (CSP) things should be done here (e.g., XTEA, CRC, etc.)

    // Debugging write to UART.
    if (AX100_enable_downlink_uart_logs) {
        DEBUG_uart_print_mixed_array(
            packet_buffer_including_csp_header, data_len + AX100_CSP_HEADER_LENGTH_BYTES,
            ((COMMS_active_rf_switch_antenna == 1) ? "AX100 Down [ANT1]" : "AX100 Down [ANT2]")
        );
    }

    return send_bytes_to_ax100_NEW(packet_buffer_including_csp_header, data_len + AX100_CSP_HEADER_LENGTH_BYTES);
}


// MARK: Fill Packet

/// @brief 
/// @param beacon_packet 
/// @note Based on the `COMMS_fill_beacon_basic_packet` function in the main firmware.
static void COMMS_fill_beacon_extended_packet(
    COMMS_beacon_extended_packet_t *beacon_packet, uint8_t *peripheral_comms_error_count_dst
) {
    // Safety: Reset the packet to zero.
    memset(beacon_packet, 0, sizeof(COMMS_beacon_extended_packet_t));

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

    // The destination is already zero-filled, so we only copy in the friendly message.
    memcpy(
        beacon_packet->friendly_message,
        COMMS_beacon_friendly_message_str,
        strlen(COMMS_beacon_friendly_message_str)
    );
    memcpy(beacon_packet->end_message, " X5", 4); // Extended beacon packet version.

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

    // Try to fetch the EPS battery data (INSTANTANEOUS), and store it in the beacon packet if successful.
    {
        EPS_struct_pbu_housekeeping_data_eng_t eps_pbu_data;
        if (EPS_CMD_get_pbu_housekeeping_data_eng(&eps_pbu_data) == 0) {
            beacon_packet->eps_battery_voltage_mV = (
                eps_pbu_data.battery_pack_info_each_pack[0].vip_bp_input.voltage_mV
            );
            beacon_packet->eps_battery_percent = (
                // Badly cast float to uint8. Adequate for the beacon.
                (uint8_t)EPS_convert_battery_voltage_to_percent(eps_pbu_data.battery_pack_info_each_pack[0])
            );
            beacon_packet->eps_battery_temperature_0_cC = (
                // Note: Original beacon in core FW has this value always pegged at "32767 cC".
                // Fixed in this blob though.
                eps_pbu_data.battery_pack_info_each_pack[0].battery_temperature_each_sensor_cC[2]
            );
            beacon_packet->eps_battery_temperature_1_cC = (
                eps_pbu_data.battery_pack_info_each_pack[0].battery_temperature_each_sensor_cC[1]
            );

            // Beacon v2 fields:
            beacon_packet->eps_battery_pack_status_bitfield = (
                eps_pbu_data.battery_pack_info_each_pack[0].bp_status_bitfield
            );
        }
        else {
            peripheral_comms_error_count++;
        }
    }

    // Try to fetch the EPS battery data (AVERAGE), and store it in the beacon packet if successful.
    {
        EPS_struct_pbu_housekeeping_data_eng_t eps_pbu_data;
        if (EPS_CMD_get_pbu_housekeeping_data_run_avg(&eps_pbu_data) == 0) {
            // Beacon v2 fields:
            beacon_packet->eps_total_avg_net_battery_power_cW = (
                eps_pbu_data.battery_pack_info_each_pack[0].vip_bp_input.power_cW
            );
        }
        else {
            peripheral_comms_error_count++;
        }
    }

    // Try to fetch the EPS PDU data (AVERAGE), and store it in the beacon packet if successful.
    {
        EPS_struct_pdu_housekeeping_data_eng_t eps_pdu_data;
        if (EPS_CMD_get_pdu_housekeeping_data_run_avg(&eps_pdu_data) == 0) {
            // Note: v1 beacon got this field from the instantaneous data; should be the exact same
            // value though, so we'll fetch it from the running average data in this v2 beacon.
            beacon_packet->eps_enabled_channels_bitfield = (
                (eps_pdu_data.stat_ch_ext_on_bitfield << 16) | eps_pdu_data.stat_ch_on_bitfield
            );

            // Beacon v2 fields:
            beacon_packet->eps_total_avg_power_distributed_cW = (
                eps_pdu_data.vip_total_input.power_cW
            );

            // Beacon v5 fields:
            beacon_packet->gnss_power_consumed_cW = clamp_power_cW_to_u8(
                eps_pdu_data.vip_each_channel[EPS_CHANNEL_3V3_GNSS].power_cW
            );
        }
        else {
            peripheral_comms_error_count++;
        }
    }

    // Try to fetch the EPS PCU data (INSTANTANEOUS), and store it in the beacon packet if successful.
    {
        EPS_struct_pcu_housekeeping_data_eng_t eps_pcu_data;
        if (EPS_CMD_get_pcu_housekeeping_data_eng(&eps_pcu_data) == 0) {
            beacon_packet->eps_total_pcu_power_input_cW = (
                EPS_calculate_total_pcu_power_input_cW(&eps_pcu_data)
            );
            beacon_packet->eps_total_pcu_power_output_cW = (
                EPS_calculate_total_pcu_power_output_cW(&eps_pcu_data)
            );

            // Beacon v2 fields:
            beacon_packet->eps_pcu_ch0_volt_in_mppt_mV = eps_pcu_data.conditioning_channel_info_each_channel[0].volt_in_mppt_mV;
            beacon_packet->eps_pcu_ch0_curr_in_mppt_mA = eps_pcu_data.conditioning_channel_info_each_channel[0].curr_in_mppt_mA;
            beacon_packet->eps_pcu_ch0_curr_ou_mppt_mA = eps_pcu_data.conditioning_channel_info_each_channel[0].curr_ou_mppt_mA;
            beacon_packet->eps_pcu_ch1_volt_in_mppt_mV = eps_pcu_data.conditioning_channel_info_each_channel[1].volt_in_mppt_mV;
            beacon_packet->eps_pcu_ch1_curr_in_mppt_mA = eps_pcu_data.conditioning_channel_info_each_channel[1].curr_in_mppt_mA;
            beacon_packet->eps_pcu_ch1_curr_ou_mppt_mA = eps_pcu_data.conditioning_channel_info_each_channel[1].curr_ou_mppt_mA;
            beacon_packet->eps_pcu_ch2_volt_in_mppt_mV = eps_pcu_data.conditioning_channel_info_each_channel[2].volt_in_mppt_mV;
            beacon_packet->eps_pcu_ch2_curr_in_mppt_mA = eps_pcu_data.conditioning_channel_info_each_channel[2].curr_in_mppt_mA;
            beacon_packet->eps_pcu_ch2_curr_ou_mppt_mA = eps_pcu_data.conditioning_channel_info_each_channel[2].curr_ou_mppt_mA;
            beacon_packet->eps_pcu_ch3_volt_in_mppt_mV = eps_pcu_data.conditioning_channel_info_each_channel[3].volt_in_mppt_mV;
            beacon_packet->eps_pcu_ch3_curr_in_mppt_mA = eps_pcu_data.conditioning_channel_info_each_channel[3].curr_in_mppt_mA;
            beacon_packet->eps_pcu_ch3_curr_ou_mppt_mA = eps_pcu_data.conditioning_channel_info_each_channel[3].curr_ou_mppt_mA;
        }
        else {
            peripheral_comms_error_count++;
        }
    }

    // Try to fetch the EPS PCU data (RUNNING AVERAGE), and store it in the beacon packet if successful.
    {
        EPS_struct_pcu_housekeeping_data_eng_t eps_pcu_data;
        if (EPS_CMD_get_pcu_housekeeping_data_run_avg(&eps_pcu_data) == 0) {
            beacon_packet->eps_total_avg_pcu_power_input_cW = (
                EPS_calculate_total_pcu_power_input_cW(&eps_pcu_data)
            );
            beacon_packet->eps_total_avg_pcu_power_output_cW = (
                EPS_calculate_total_pcu_power_output_cW(&eps_pcu_data)
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

    // Everything below is for the v2 beacon.

    // ADCS status data.
    {
        uint8_t data_received[6];

        const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(
            132, // ADCS_TELEMETRY_CUBEACP_ADCS_STATE
            data_received,
            sizeof(data_received),
            1 // ADCS_INCLUDE_CHECKSUM
        );

        if (tlm_status == 0) {
            memcpy(beacon_packet->adcs_current_state_1, data_received, sizeof(data_received));
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

    // ADCS magnetic field vector.
    // Raw values are int16, little-endian, in units of 10 nT (i.e. already in Telsla en8 scale).
    {
        uint8_t data_received[6];

        const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(
            151, // ADCS_TELEMETRY_CUBEACP_MAGNETIC_FIELD_VECTOR
            data_received,
            sizeof(data_received),
            1 // ADCS_INCLUDE_CHECKSUM
        );

        if (tlm_status == 0) {
            memcpy(&beacon_packet->adcs_magnetic_field_x_T_en8, &data_received[0], 2);
            memcpy(&beacon_packet->adcs_magnetic_field_y_T_en8, &data_received[2], 2);
            memcpy(&beacon_packet->adcs_magnetic_field_z_T_en8, &data_received[4], 2);
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
            int16_t x_cdeg_per_sec;
            int16_t y_cdeg_per_sec;
            int16_t z_cdeg_per_sec;
            memcpy(&x_cdeg_per_sec, &data_received[0], 2);
            memcpy(&y_cdeg_per_sec, &data_received[2], 2);
            memcpy(&z_cdeg_per_sec, &data_received[4], 2);

            beacon_packet->adcs_angular_rate_norm_cdeg_per_sec = integer_sqrt_u32(
                ((int32_t)x_cdeg_per_sec * (int32_t)x_cdeg_per_sec)
                + ((int32_t)y_cdeg_per_sec * (int32_t)y_cdeg_per_sec)
                + ((int32_t)z_cdeg_per_sec * (int32_t)z_cdeg_per_sec)
            );
        }
        else {
            peripheral_comms_error_count++;
        }
    }

    // ADCS estimated angular rates.
    {
        uint8_t data_received[6];

        const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(
            147, // ADCS_TELEMETRY_CUBEACP_ESTIMATED_ANGULAR_RATES
            data_received,
            sizeof(data_received),
            1 // ADCS_INCLUDE_CHECKSUM
        );

        if (tlm_status == 0) {
            memcpy(&beacon_packet->adcs_estimated_rate_x_cdeg_per_sec, &data_received[0], 2);
            memcpy(&beacon_packet->adcs_estimated_rate_y_cdeg_per_sec, &data_received[2], 2);
            memcpy(&beacon_packet->adcs_estimated_rate_z_cdeg_per_sec, &data_received[4], 2);
        }
        else {
            peripheral_comms_error_count++;
        }
    }

    // ADCS estimated attitude angles.
    {
        uint8_t data_received[6];

        const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(
            146, // ADCS_TELEMETRY_CUBEACP_ESTIMATED_ATTITUDE_ANGLES
            data_received,
            sizeof(data_received),
            1 // ADCS_INCLUDE_CHECKSUM
        );

        if (tlm_status == 0) {
            memcpy(&beacon_packet->adcs_estimated_roll_angle_cdeg, &data_received[0], 2);
            memcpy(&beacon_packet->adcs_estimated_pitch_angle_cdeg, &data_received[2], 2);
            memcpy(&beacon_packet->adcs_estimated_yaw_angle_cdeg, &data_received[4], 2);
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

    // ADCS wheel speeds: report the fastest wheel's absolute speed.
    {
        uint8_t data_received[6];

        const uint8_t tlm_status = ADCS_i2c_request_telemetry_and_check(
            156, // ADCS_TELEMETRY_CUBEACP_WHEEL_SPEED
            data_received,
            sizeof(data_received),
            1 // ADCS_INCLUDE_CHECKSUM
        );

        if (tlm_status == 0) {
            uint16_t fastest_rpm = 0;
            for (uint8_t i = 0; i < 3; i++) {
                const int16_t rpm = (int16_t)(data_received[(2 * i) + 1] << 8 | data_received[2 * i]); // TODO: There's a function for this.
                const uint16_t abs_rpm = (rpm < 0) ? (uint16_t)(-(int32_t)rpm) : (uint16_t)rpm;
                if (abs_rpm > fastest_rpm) {
                    fastest_rpm = abs_rpm;
                }
            }
            beacon_packet->adcs_fastest_wheel_speed_rpm = fastest_rpm;
        }
        else {
            peripheral_comms_error_count++;
        }
    }


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
            LOG(
                LOG_SEVERITY_ERROR,
                "%s: lfs_file_close(r%d) -> %d",
                BLOB_NAME, g_state->open_file_idx, close_result
            );
        }
    }

    g_state->open_file_is_valid = 0;
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
        LOG(LOG_SEVERITY_ERROR, "%s: lfs_mkdir(%s) -> %d", BLOB_NAME, BEACON_RING_DIR, mkdir_result);
        return BLOB_ERR_LFS_MKDIR_FAILED;
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
        LOG(LOG_SEVERITY_ERROR, "%s: lfs_file_open(%s) -> %d", BLOB_NAME, path, open_result);
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
        LOG(LOG_SEVERITY_ERROR, "%s: lfs_file_seek(%s) -> %ld", BLOB_NAME, path, (long)seek_result);
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
static BLOB_ext_beacon_error_enum_t store_beacon_to_ring(const COMMS_beacon_extended_packet_t *beacon_packet) {
    const BLOB_ext_beacon_error_enum_t open_status = ring_open_write_file();
    if (open_status != BLOB_ERR_OK) {
        return open_status;
    }

    const lfs_ssize_t write_result = lfs_file_write(
        &LFS_filesystem, &g_state->open_file, beacon_packet, BEACON_RING_RECORD_SIZE
    );
    if (write_result != (lfs_ssize_t)BEACON_RING_RECORD_SIZE) {
        LOG(
            LOG_SEVERITY_ERROR,
            "%s: lfs_file_write(r%d, record %d) -> %ld",
            BLOB_NAME, g_state->write_file_idx, g_state->write_record_idx, (long)write_result
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


// MARK: History Downlink

/// @brief Downlink a randomly-chosen run of consecutive stored beacons: pick a random full ring
///     file, pick a random record index within it, and send up to `history_count` consecutive
///     records from there (stopping at the end of the file).
/// @details Same selection logic as the GNSS ring blob. The file the write cursor is on is never
///     picked (it's partly full). Files left over from a previous campaign (a cold start zeroes
///     the cursor without erasing the disk) are excluded by counting only the files behind the
///     cursor until the ring has wrapped.
///
///     Records are stored exactly as generated (" X5\0"). Each is sent as-is, except that its
///     end_message is replaced with " S5\0" after reading it back, so the ground can tell it apart
///     from a live beacon.
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
    const uint16_t n_to_downlink = (history_count < available_count) ? history_count : available_count;

    char path[BEACON_RING_PATH_MAX_LEN];
    make_ring_file_path(chosen_file_idx, path, sizeof(path));

    lfs_file_t file;
    const int open_result = lfs_file_open(&LFS_filesystem, &file, path, LFS_O_RDONLY);
    if (open_result < 0) {
        // Same format string as in store_beacon_to_ring(), so it's stored once.
        LOG(LOG_SEVERITY_ERROR, "%s: lfs_file_open(%s) -> %d", BLOB_NAME, path, open_result);
        return n_to_downlink;
    }

    if (lfs_file_seek(&LFS_filesystem, &file, (lfs_soff_t)start_record_idx * BEACON_RING_RECORD_SIZE, LFS_SEEK_SET) < 0) {
        LOG(LOG_SEVERITY_WARNING, "%s: lfs_file_seek(%s) failed", BLOB_NAME, path);
        lfs_file_close(&LFS_filesystem, &file);
        return n_to_downlink;
    }

    uint16_t fail_count = 0;
    for (uint16_t i = 0; i < n_to_downlink; i++) {
        COMMS_beacon_extended_packet_t packet;
        const lfs_ssize_t read_len = lfs_file_read(&LFS_filesystem, &file, &packet, BEACON_RING_RECORD_SIZE);
        if (read_len != (lfs_ssize_t)BEACON_RING_RECORD_SIZE) {
            LOG(
                LOG_SEVERITY_WARNING,
                "%s: short read (%ld) at %s record %d",
                BLOB_NAME, (long)read_len, path, start_record_idx + i
            );
            fail_count += (uint16_t)(n_to_downlink - i); // The rest of the run is unreachable too.
            break;
        }

        // Stored as generated; mark it as a ring-stored (historical) beacon only now.
        memcpy(packet.end_message, " S5", 4);

        const uint8_t tx_status = AX100_downlink_bytes_NEW((uint8_t *)&packet, sizeof(packet));
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
    char arg1_history_count[20];

    pos = parse_token(args_str, pos, args_str_len, arg0_beacon_interval_ms, sizeof(arg0_beacon_interval_ms));
    parse_token(args_str, pos, args_str_len, arg1_history_count, sizeof(arg1_history_count)); // Optional; may be "".

    if (arg0_beacon_interval_ms[0] == '\0') {
        // Missing args error.
        snprintf(
            response_buf, response_buf_len,
            "%s error: missing args! (%s)",
            BLOB_NAME,
            ext_beacon_blob_error_to_str(BLOB_ERR_MISSING_ARGS)
        );
        return BLOB_ERR_MISSING_ARGS;
    }

    bool arg0_beacon_interval_ms_ok;
    int32_t beacon_interval_ms = parse_int(arg0_beacon_interval_ms, &arg0_beacon_interval_ms_ok);

    bool arg1_history_count_ok = true;
    int32_t history_count = 0;
    if (arg1_history_count[0] != '\0') {
        history_count = parse_int(arg1_history_count, &arg1_history_count_ok);
    }

    if (!arg0_beacon_interval_ms_ok || !arg1_history_count_ok) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: invalid int args! (%s)",
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

    // True cold-start: first ever run, or a power cycle that cleared SRAM. Nothing in the state
    // block can be trusted (including the open file handle), so restart the ring at r0.bin
    // record 0 (old files are excluded from downlink, and truncated as they're reached).
    if (g_state->magic != BEACON_RING_STATE_MAGIC) {
        memset(g_state, 0, sizeof(BEACON_ring_state_t));
        g_state->magic = BEACON_RING_STATE_MAGIC;
    }

    // A handle we believe we're holding, that the mounted filesystem doesn't know about, means a
    // reboot or an unmount/remount happened since the last run. The current file's uncommitted
    // records are lost, so restart that file from record 0. Files behind the cursor were committed
    // when they filled, so the rest of the ring is kept.
    if (g_state->open_file_is_valid && (!ring_open_file_is_live())) {
        LOG(
            LOG_SEVERITY_WARNING,
            "%s: stale ring file handle (r%d/%d); restarting that file",
            BLOB_NAME, g_state->open_file_idx, g_state->write_record_idx
        );
        g_state->open_file_is_valid = 0;
        g_state->write_record_idx = 0;
    }

    // Cancel any other pending agenda entries that would re-run this same blob (e.g., a repeat
    // telecommand scheduled by a previous run of this blob), so re-uplinking this blob on every
    // pass doesn't stack up duplicate scheduled reruns.
    const int16_t cancel_result = cancel_other_scheduled_reruns_of_this_blob(get_current_executing_tcmd_agenda_slot_num());
    char cancel_msg[50];
    if (cancel_result < 0) {
        ring_close_open_file(); // Not rescheduling; commit what we have.
        snprintf(
            response_buf, response_buf_len,
            "%s error: cancel_other_scheduled_reruns_of_this_blob() -> %d (%s)",
            BLOB_NAME,
            cancel_result,
            ext_beacon_blob_error_to_str(BLOB_ERR_CANCEL_RERUNS_FAILED)
        );
        return BLOB_ERR_CANCEL_RERUNS_FAILED;
    }
    else if (cancel_result > 0) {
        snprintf(
            cancel_msg,
            sizeof(cancel_msg),
            ", %d duplicate rerun(s) cancelled",
            cancel_result
        );
    }
    else {
        cancel_msg[0] = '\0';
    }

    // Fill the beacon packet.
    COMMS_beacon_extended_packet_t beacon_packet;
    uint8_t peripheral_comms_error_count = 0;
    COMMS_fill_beacon_extended_packet(&beacon_packet, &peripheral_comms_error_count);

    // Store the beacon into the ring. Never mounts the filesystem itself; just skips if unmounted.
    BLOB_ext_beacon_error_enum_t store_status = BLOB_ERR_LFS_NOT_MOUNTED;
    if (LFS_is_lfs_mounted) {
        store_status = store_beacon_to_ring(&beacon_packet);
    }
    if (store_status != BLOB_ERR_OK) {
        g_state->store_failure_count++;
    }

    // Downlink the beacon packet.
    const uint8_t tx_status = AX100_downlink_bytes_NEW(
        (uint8_t *)(&beacon_packet), 
        sizeof(COMMS_beacon_extended_packet_t)
    );
    if (tx_status != 0) {
        ring_close_open_file(); // Not rescheduling; commit what we have.
        snprintf(
            response_buf, response_buf_len,
            "%s error: downlink failed (AX100_downlink_bytes_NEW() -> %d) (%s)%s",
            BLOB_NAME,
            tx_status,
            ext_beacon_blob_error_to_str(BLOB_ERR_DOWNLINK_FAILED),
            cancel_msg
        );
        return BLOB_ERR_DOWNLINK_FAILED;
    }

    // Downlink a random consecutive run of historical beacons, unless the MPI is actively
    // collecting science data (stay off the radio as much as possible then).
    const bool skip_history_for_mpi = (MPI_current_uart_rx_mode == MPI_RX_MODE_SENSING_MODE);
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

    if (beacon_interval_ms > 0) {
        const BLOB_ext_beacon_error_enum_t reexec_result = reschedule_current_blob_tcmd(
            (uint32_t)beacon_interval_ms
        );
        if (reexec_result != BLOB_ERR_OK) {
            ring_close_open_file(); // No rerun is coming to close it; commit what we have.
            snprintf(
                response_buf, response_buf_len,
                "%s error: reschedule_current_blob_tcmd() -> %s%s%s",
                BLOB_NAME,
                ext_beacon_blob_error_to_str(reexec_result),
                ring_msg,
                cancel_msg
            );
            return reexec_result;
        }
    }
    else {
        ring_close_open_file(); // Run-once mode: no rerun is coming to close it.
    }

    if (peripheral_comms_error_count > 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: peripheral comms error count: %d (%s)%s%s",
            BLOB_NAME,
            peripheral_comms_error_count,
            ext_beacon_blob_error_to_str(BLOB_ERR_PERIPHERAL_COMMS_FAILURES),
            ring_msg,
            cancel_msg
        );
        return BLOB_ERR_PERIPHERAL_COMMS_FAILURES;
    }

    snprintf(
        response_buf, response_buf_len,
        "%s success%s%s",
        BLOB_NAME,
        ring_msg,
        cancel_msg
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
