// This is a blob (executable) that emits extended beacons with many extra peripheral fields.
//
// Motivation: The existing FrontierSat beacon is great, but lacks certain data (e.g., ADCS data
// and per-channel EPS data especially).
// This blob is a new feature that allows for sending additional data in the beacon packets.
//
// Args Format: repeat_interval_ms
// The repeat_interval_ms can be 0 to run only once, or any positive number to run repeatedly at
// that specified interval.
//
// Usage Example:
// After uplinking the blob as "blobs/extended_beacon_v2.blob", run:
// CTS1+exec_blob_from_fs(blobs/extended_beacon_v2.blob,0,9000)!
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
//      `CTS1+exec_blob_from_fs(blobs/extended_beacon_v2.blob,0,0)!` (which will run one last time,
//      then cancel itself).

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
static const char *BLOB_NAME = "extended_beacon_blob";

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

extern int32_t read_avg_temperature_cC_from_mpi_data_buffer(volatile uint8_t* large_buffer);
extern volatile MPI_buffer_state_enum_t MPI_buffer_one_state;
extern volatile MPI_buffer_state_enum_t MPI_buffer_two_state;
extern volatile uint32_t MPI_buffer_one_last_filled_uptime_ms;
extern volatile uint32_t MPI_buffer_two_last_filled_uptime_ms;

extern int snprintf(char *buf, unsigned int size, const char *fmt, ...);
extern int strlen(const char *s);
extern int strcasecmp(const char *s1, const char *s2);
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

#pragma pack(push, 1)

typedef struct {
    uint8_t packet_type; // COMMS_packet_type_enum_t - Always COMMS_PACKET_TYPE_BEACON_EXTENDED for this packet.

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
    // Note: Third battery temperature sensor doesn't work on our model.
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

    char end_message[4]; // "END\0" on basic packets; " X2\0" on extended packets (v2).

    // ====== END OF BASIC BEACON PACKET (DUPLICATED) ========
    // MARK: Extended Fields

    // Active oscillator frequency, in MHz. 16 MHz = internal clock (HSI, imprecise), 25 MHz = external (HSE, precise).
    uint8_t obc_active_oscillator_MHz;

    // Battery voltage, as measured by the ADC on the OBC. Very useful when EPS fails to report
    // its battery voltage, as it often tends to do (due to ISISpace bug).
    int16_t obc_adc_battery_voltage_mV;

    // Average temperature of the MPI, when active, at the most recent time it was active.
    // Clamped to -90C to +125C.
    // -99 = MPI inactive. -98 and -97 are error codes.
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
    uint16_t adcs_magnetic_field_x_T_en8;
    uint16_t adcs_magnetic_field_y_T_en8;
    uint16_t adcs_magnetic_field_z_T_en8;

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

} COMMS_beacon_extended_packet_t;

// Limit: sizeof(COMMS_beacon_extended_packet_t) <= 200
// Currently, sizeof(COMMS_beacon_extended_packet_t) = 198

#pragma pack(pop)


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

/// @brief Dig the current MPI temperature from the data buffer.
/// @note Based heavily on the inline logic in `rtos_mpi_tasks.c`.
/// @return Temperature in C, or -99 if MPI never active, or -98 if error calculating temperature.
/// @note Clamps at -90 if colder than -90C, or 125 if hotter than 125C.
static int8_t get_last_mpi_temperature_C() {
    int32_t last_mpi_temperature_cC = -99999;

    // Pick the buffer with the most recent data.
    if (MPI_buffer_one_last_filled_uptime_ms > MPI_buffer_two_last_filled_uptime_ms) {
        last_mpi_temperature_cC = read_avg_temperature_cC_from_mpi_data_buffer(
            MPI_science_buffer_one
        ) / 100;
    }
    else if (MPI_buffer_two_last_filled_uptime_ms > MPI_buffer_one_last_filled_uptime_ms) {
        last_mpi_temperature_cC = read_avg_temperature_cC_from_mpi_data_buffer(
            MPI_science_buffer_two
        ) / 100;
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

        if (strcasecmp(own_blob_file_name, other_blob_file_name) == 0) {
            // Main action: Cancel that entry!
            TCMD_agenda_is_valid[slot_num] = TCMD_AGENDA_ENTRY_INVALID;
            cancelled_count++;
        }
    }

    return cancelled_count;
}

static uint8_t reexecute_current_blob_tcmd(uint32_t time_into_future_to_execute_ms) {
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

    return 0;
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
    beacon_packet->packet_type = COMMS_PACKET_TYPE_BEACON_EXTENDED; // COMMS_PACKET_TYPE_BEACON_EXTENDED = 0x20 = 32
    memcpy(beacon_packet->satellite_name, "CTS1", 4);
    beacon_packet->active_rf_switch_antenna = COMMS_active_rf_switch_antenna; // 1 or 2
    beacon_packet->active_rf_switch_control_mode = COMMS_rf_switch_control_mode; // Enum
    beacon_packet->uptime_ms = TIME_uptime_ms();
    beacon_packet->duration_since_last_uplink_ms = TIME_uptime_ms() - AX100_uptime_at_last_received_kiss_tcmd_ms;
    beacon_packet->unix_epoch_time_ms = TIME_get_current_unix_epoch_time_ms();
    beacon_packet->last_time_sync_source_enum = TIME_last_synchronization_source;

    beacon_packet->is_fs_mounted = LFS_is_lfs_mounted;

    beacon_packet->total_tcmd_queued_count = TCMD_total_tcmd_queued_count;
    beacon_packet->pending_queued_tcmd_count = TCMD_get_agenda_used_slots_count();

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
    memcpy(beacon_packet->end_message, " X2", 4); // Extended beacon packet version.

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

    *peripheral_comms_error_count_dst = peripheral_comms_error_count;
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

    char arg0_beacon_interval_ms[20];

    pos = parse_token(args_str, pos, args_str_len, arg0_beacon_interval_ms, sizeof(arg0_beacon_interval_ms));
    
    if (arg0_beacon_interval_ms[0] == '\0') {
        // Missing args error.
        snprintf(
            response_buf, response_buf_len,
            "%s error: missing args!",
            BLOB_NAME
        );
        return 135;
    }

    bool arg0_beacon_interval_ms_ok;
    const int32_t beacon_interval_ms = parse_int(arg0_beacon_interval_ms, &arg0_beacon_interval_ms_ok);

    if (!arg0_beacon_interval_ms_ok) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: invalid int args!",
            BLOB_NAME
        );
        return 136;
    }
    
    // Cancel any other pending agenda entries that would re-run this same blob (e.g., a repeat
    // telecommand scheduled by a previous run of this blob), so re-uplinking this blob on every
    // pass doesn't stack up duplicate scheduled reruns.
    const int16_t cancel_result = cancel_other_scheduled_reruns_of_this_blob(get_current_executing_tcmd_agenda_slot_num());
    char cancel_msg[50];
    if (cancel_result < 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: cancel_other_scheduled_reruns_of_this_blob() -> %d",
            BLOB_NAME,
            cancel_result
        );
        return 137;
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

    // Downlink the beacon packet.
    const uint8_t tx_success = AX100_downlink_bytes(
        (uint8_t *)(&beacon_packet), 
        sizeof(COMMS_beacon_extended_packet_t)
    );
    if (tx_success != 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: downlink failed (AX100_downlink_bytes() -> %d)%s",
            BLOB_NAME,
            tx_success,
            cancel_msg
        );
        return tx_success;
    }

    if (beacon_interval_ms > 0) {
        const uint8_t reexec_result = reexecute_current_blob_tcmd(beacon_interval_ms);
        if (reexec_result != 0) {
            snprintf(
                response_buf, response_buf_len,
                "%s error: reexecute_current_blob_tcmd() -> %d%s",
                BLOB_NAME,
                reexec_result,
                cancel_msg
            );
            return reexec_result;
        }
    }

    if (peripheral_comms_error_count > 0) {
        snprintf(
            response_buf, response_buf_len,
            "%s error: peripheral comms error count: %d%s",
            BLOB_NAME,
            peripheral_comms_error_count,
            cancel_msg
        );
        return 117;
    }

    snprintf(
        response_buf, response_buf_len,
        "%s success%s",
        BLOB_NAME,
        cancel_msg
    );

    return 0;
}
