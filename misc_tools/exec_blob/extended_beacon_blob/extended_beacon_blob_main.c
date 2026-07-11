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
// After uplinking the blob as "blobs/extended_beacon_v1.blob", run:
// CTS1+exec_blob_from_fs(blobs/bulk_downlink_start_v2.blob,0,your_file.run;0;0)!

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "../lfs.h"

// Beacon v1 contents (includes).
#include "eps_drivers/eps_types.h"
#include "comms_drivers/rf_antenna_switch.h"
#include "timekeeping/timekeeping.h"
#include "telecommand_exec/telecommand_executor.h"
#include "mpi/mpi_types.h"
#include "mpi/mpi_command_handling.h"
#include "rtos_tasks/rtos_bootup_operation_fsm_task.h"
#include "gnss_receiver/gnss_internal_drivers.h"
#include "eps_drivers/eps_calculations.h"
#include "eps_drivers/eps_commands.h"
#include "stm32/stm32_reboot_reason.h"
#include "obc_systems/external_led_and_rbf.h"

#define LFS_MAX_PATH_LENGTH 200
#define COMMS_BEACON_FRIENDLY_MESSAGE_SIZE 42


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

// Beacon v1 contents (extern functions).
extern int32_t OBC_TEMP_SENSOR_get_temperature_cC();


extern int snprintf(char *buf, unsigned int size, const char *fmt, ...);
extern int strlen(const char *s);
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


#pragma pack(push, 1)

typedef struct {
    uint8_t packet_type; // COMMS_packet_type_enum_t - Always COMMS_PACKET_TYPE_BEACON_BASIC for this packet

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
    int16_t eps_battery_temperature_0_cC;
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

    char end_message[4]; // "END\0"
    
} COMMS_beacon_basic_packet_t;

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

static int8_t hex_to_int(char c) { // TODO: Remove this function.
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

static int16_t get_current_executing_tcmd_agenda_slot_num() {
    for (uint16_t i = 0; i < TCMD_AGENDA_SIZE; i++) {
        if (TCMD_agenda_is_valid[i] == TCMD_AGENDA_ENTRY_EXECUTING) {
            return i;
        }
    }
    return -1;
}

static uint8_t reexecute_current_blob_tcmd(uint32_t time_into_future_to_execute_ms) {
    const int16_t slot_num = get_current_executing_tcmd_agenda_slot_num();
    if (slot_num < 0) {
        return 163;
    }

    const char* args = TCMD_agenda[slot_num].args_str_no_parens;

    TCMD_parsed_tcmd_to_execute_t new_tcmd;
    memcpy(&new_tcmd, &TCMD_agenda[slot_num], sizeof(TCMD_parsed_tcmd_to_execute_t));

    new_tcmd.timestamp_sent = TIME_get_current_unix_epoch_time_ms();
    new_tcmd.timestamp_to_execute = TIME_get_current_unix_epoch_time_ms() + time_into_future_to_execute_ms;

    if (TCMD_add_tcmd_to_agenda(&new_tcmd) != 0) {
        return 164;
    }

    return 0;
}


/// @brief 
/// @param beacon_packet 
/// @note Based on the `COMMS_fill_beacon_basic_packet` function in the main firmware.
static void COMMS_fill_beacon_basic_packet_new(
    COMMS_beacon_basic_packet_t *beacon_packet
) {
    // Safety: Reset the packet to zero.
    memset(beacon_packet, 0, sizeof(COMMS_beacon_basic_packet_t));

    // Fill the packet with the current system state.
    beacon_packet->packet_type = 20; // FIXME: Ensure we want to make 20 = 0x14 the new code.
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
    memcpy(beacon_packet->end_message, "END", 4);
    
    // Try to fetch the EPS system status, and store it in the beacon packet if successful.
    {
        EPS_struct_system_status_t eps_status_data;
        if (EPS_CMD_get_system_status(&eps_status_data) == 0) {
            beacon_packet->eps_mode_enum = eps_status_data.mode;
            beacon_packet->eps_reset_cause_enum = eps_status_data.reset_cause;
            beacon_packet->eps_uptime_sec = eps_status_data.uptime_sec;
            beacon_packet->eps_error_code = eps_status_data.error_code;
        }
    }

    // Try to fetch the EPS battery data, and store it in the beacon packet if successful.
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
                eps_pbu_data.battery_pack_info_each_pack[0].battery_temperature_each_sensor_cC[0]
            );
            beacon_packet->eps_battery_temperature_1_cC = (
                eps_pbu_data.battery_pack_info_each_pack[0].battery_temperature_each_sensor_cC[1]
            );
        }
    }

    // Try to fetch the EPS PDU data, and store it in the beacon packet if successful.
    {
        EPS_struct_pdu_housekeeping_data_eng_t eps_pdu_data;
        if (EPS_CMD_get_pdu_housekeeping_data_eng(&eps_pdu_data) == 0) {
            beacon_packet->eps_enabled_channels_bitfield = (
                (eps_pdu_data.stat_ch_ext_on_bitfield << 16) | eps_pdu_data.stat_ch_on_bitfield
            );
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
    }

    // Try to fetch the EPS fault count, and store it in the beacon packet if successful.
    {
        // Get EPS total fault count.
        EPS_struct_pdu_overcurrent_fault_state_t eps_pdu_fault_data;
        if (EPS_CMD_get_pdu_overcurrent_fault_state(&eps_pdu_fault_data) == 0) {
            beacon_packet->eps_total_fault_count = EPS_calculate_total_fault_count(&eps_pdu_fault_data);
        }
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
    
    // TODO: Fill the packet and emit it.
    COMMS_beacon_basic_packet_t beacon_packet;
    COMMS_fill_beacon_basic_packet_new(&beacon_packet);

    if (beacon_interval_ms > 0) {
        const uint8_t reexec_result = reexecute_current_blob_tcmd(beacon_interval_ms);
        if (reexec_result != 0) {
            snprintf(
                response_buf, response_buf_len,
                "%s error: reexec failed (code %d)",
                BLOB_NAME,
                reexec_result
            );
            return reexec_result;
        }
    }

    snprintf(
        response_buf, response_buf_len,
        "%s success",
        BLOB_NAME
    );

    return 0;
}
