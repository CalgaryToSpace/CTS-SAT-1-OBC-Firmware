#include "comms_drivers/comms_tx.h"
#include "comms_drivers/beacon.h"

#include "comms_drivers/rf_antenna_switch.h"
#include "rtos_tasks/rtos_tasks_rx_telecommands.h"
#include "main.h"

#include "log/log.h"
#include "eps_drivers/eps_types.h"
#include "eps_drivers/eps_calculations.h"
#include "timekeeping/timekeeping.h"
#include "stm32/stm32_reboot_reason.h"
#include "eps_drivers/eps_commands.h"
#include "littlefs/littlefs_helper.h"
#include "transforms/arrays.h"
#include "self_checks/complete_self_check.h"
#include "obc_systems/external_led_and_rbf.h"
#include "obc_systems/obc_temperature_sensor.h"
#include "system/system_temperature.h"
#include "mpi/mpi_command_handling.h"
#include "mpi/mpi_types.h"
#include "uart_handler/uart_handler.h"
#include "rtos_tasks/rtos_bootup_operation_fsm_task.h"

#include "telecommands/system_telecommand_defs.h"
#include "telecommand_exec/telecommand_definitions.h"
#include "telecommand_exec/telecommand_executor.h"

#include <string.h>

extern uint32_t COMMS_total_beacon_count_since_boot;
extern MPI_transceiver_state_enum_t MPI_current_transceiver_state;
extern MPI_reason_for_stopping_active_mode MPI_last_reason_for_stopping_active_mode;


char COMMS_beacon_friendly_message_str[COMMS_BEACON_FRIENDLY_MESSAGE_SIZE] = "Hello from CalgaryToSpace FrontierSat";


void COMMS_fill_beacon_basic_packet(
    COMMS_beacon_basic_packet_t *beacon_packet
) {
    // Safety: Reset the packet to zero.
    memset(beacon_packet, 0, sizeof(COMMS_beacon_basic_packet_t));

    // Fill the packet with the current system state.
    beacon_packet->packet_type = COMMS_PACKET_TYPE_BEACON_BASIC;
    memcpy(beacon_packet->satellite_name, "CTS1", 4);
    beacon_packet->active_rf_switch_antenna = COMMS_active_rf_switch_antenna; // 1 or 2
    beacon_packet->active_rf_switch_control_mode = COMMS_rf_switch_control_mode; // Enum
    beacon_packet->uptime_ms = TIME_get_current_system_uptime_ms();
    beacon_packet->duration_since_last_uplink_ms = HAL_GetTick() - AX100_uptime_at_last_received_kiss_tcmd_ms;
    beacon_packet->unix_epoch_time_ms = TIME_get_current_unix_epoch_time_ms();
    beacon_packet->last_time_sync_source_enum = TIME_last_synchronization_source;

    beacon_packet->is_fs_mounted = LFS_is_lfs_mounted;

    beacon_packet->total_tcmd_queued_count = TCMD_total_tcmd_queued_count;

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
    beacon_packet->gnss_rx_mode_enum = 0; // FIXME: Fill during PR #567 review.

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
