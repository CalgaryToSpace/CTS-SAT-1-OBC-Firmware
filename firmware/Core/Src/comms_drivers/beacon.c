#include "comms_drivers/comms_tx.h"
#include "comms_drivers/beacon.h"

#include "comms_drivers/rf_antenna_switch.h"
#include "timekeeping/timekeeping.h"
#include "rtos_tasks/rtos_tasks_rx_telecommands.h"
#include "main.h"

#include <string.h>


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
    
    // TODO: Add more fields
}
