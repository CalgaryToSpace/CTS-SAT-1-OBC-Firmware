#ifndef INCLUDE_GUARD__COMMS_TX_H__
#define INCLUDE_GUARD__COMMS_TX_H__

#include "comms_drivers/ax100_tx.h"

/// @brief Packet types for the COMMS downlink packets.
/// @details This is the first byte (after the CSP header) of the downlink packets.
typedef enum {
    COMMS_PACKET_TYPE_BEACON_BASIC = 0x01,
    COMMS_PACKET_TYPE_BEACON_PERIPHERAL = 0x02, // Unused, currently.
    COMMS_PACKET_TYPE_LOG_MESSAGE = 0x03,
    COMMS_PACKET_TYPE_TCMD_RESPONSE = 0x04,
    COMMS_PACKET_TYPE_BULK_FILE_DOWNLINK = 0x10,
} COMMS_packet_type_enum_t;


// (AX100_DOWNLINK_MAX_BYTES - 1)
#define COMMS_LOG_MESSAGE_PACKET_MAX_DATA_BYTES_PER_PACKET 199

#if COMMS_LOG_MESSAGE_PACKET_MAX_DATA_BYTES_PER_PACKET != (AX100_DOWNLINK_MAX_BYTES - 1)
#error "COMMS_LOG_MESSAGE_PACKET_MAX_DATA_BYTES_PER_PACKET is incorrect"
#endif


// (AX100_DOWNLINK_MAX_BYTES - 1-8-1-2-1-1)
#define COMMS_TCMD_RESPONSE_PACKET_MAX_DATA_BYTES_PER_PACKET 186


#if COMMS_TCMD_RESPONSE_PACKET_MAX_DATA_BYTES_PER_PACKET != (AX100_DOWNLINK_MAX_BYTES - 1 - 8 - 1 - 2 - 1 - 1)
#error "COMMS_TCMD_RESPONSE_PACKET_MAX_DATA_BYTES_PER_PACKET is incorrect"
#endif

// (AX100_DOWNLINK_MAX_BYTES - 1-1-1-4)
#define COMMS_BULK_FILE_DOWNLINK_PACKET_MAX_DATA_BYTES_PER_PACKET 193

#if COMMS_BULK_FILE_DOWNLINK_PACKET_MAX_DATA_BYTES_PER_PACKET != (AX100_DOWNLINK_MAX_BYTES - 1 - 1 - 1 - 4)
#error "COMMS_BULK_FILE_DOWNLINK_PACKET_MAX_DATA_BYTES_PER_PACKET is incorrect"
#endif

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

    uint32_t total_beacon_count_since_boot;

    uint8_t eps_mode_enum; // 0=startup, 1=nominal, 2=safety, 3=emergency_low_power
    uint8_t eps_reset_cause_enum; // 0=power_on, 1=watchdog, 2=commanded, 3=control_system_reset, 4=emergency_low_power
    uint32_t eps_uptime_sec;
    uint16_t eps_error_code;
    uint16_t eps_battery_voltage_mV;
    uint8_t eps_battery_percent;
    // TODO: eps_battery_temperature
    int32_t eps_total_fault_count;
    uint32_t eps_enabled_channels_bitfield;
    int32_t eps_total_pcu_power_input_cW;
    int32_t eps_total_pcu_power_output_cW;
    int32_t eps_total_avg_pcu_power_input_cW;
    int32_t eps_total_avg_pcu_power_output_cW;

    uint8_t reboot_reason; // Enum: STM32_reset_cause_t
    
    uint8_t cts1_operation_state; // Enum: CTS1_operation_state_enum_t

    // TODO: obc_temperature

    uint8_t mpi_rx_mode_enum; // Enum: MPI_rx_mode_enum_t
    uint8_t mpi_transceiver_state_enum; // Enum: MPI_current_transceiver_state_enum_t

    uint8_t mpi_last_reason_for_stopping_enum; // Enum: MPI_reason_for_stopping_active_mode_enum_t
    
    uint8_t gnss_uart_interrupt_enabled;

    uint8_t gnss_rx_mode_enum; // Enum: GNSS_rx_mode_enum_t

    // TODO: Total log message counts (esp. for warning/error levels)

    
} COMMS_beacon_basic_packet_t;


typedef struct {
    uint8_t packet_type; // COMMS_packet_type_enum_t - Always COMMS_PACKET_TYPE_LOG_MESSAGE for this packet

    uint8_t data[COMMS_LOG_MESSAGE_PACKET_MAX_DATA_BYTES_PER_PACKET];
} COMMS_log_message_packet_t;


typedef struct {
    uint8_t packet_type; // COMMS_packet_type_enum_t - Always COMMS_PACKET_TYPE_TCMD_RESPONSE for this packet

    uint64_t ts_sent;        // 8 bytes
    uint8_t response_code;   // 1 byte
    uint16_t duration_ms;    // 2 bytes
    uint8_t response_seq_num; // 1 byte
    uint8_t response_max_seq_num;   // 1 byte

    uint8_t data[COMMS_TCMD_RESPONSE_PACKET_MAX_DATA_BYTES_PER_PACKET];
} COMMS_tcmd_response_packet_t;

typedef struct {
    uint8_t packet_type; // COMMS_packet_type_enum_t - Always COMMS_PACKET_TYPE_BULK_FILE_DOWNLINK for this packet

    uint8_t file_seq_num;   // 1 byte
    uint8_t file_max_seq_num;   // 1 byte

    uint32_t file_offset;   // 4 bytes

    uint8_t data[COMMS_BULK_FILE_DOWNLINK_PACKET_MAX_DATA_BYTES_PER_PACKET];
} COMMS_bulk_file_downlink_packet_t;

// Unit tests contain assertions like: assert(sizeof(COMMS_log_message_packet_t) == AX100_DOWNLINK_MAX_BYTES);

#pragma pack(pop)


uint8_t COMMS_downlink_tcmd_response(
    uint64_t ts_sent, 
    uint8_t response_code, 
    uint16_t duration_ms, 
    char *response,
    uint32_t response_len
);

uint8_t COMMS_downlink_log_message(const char log_message_str[]);

uint8_t COMMS_downlink_bulk_file_downlink(
    uint8_t file_seq_num,
    uint8_t file_max_seq_num,
    uint32_t file_offset,
    uint8_t data[],
    uint16_t data_len
);
uint8_t COMMS_downlink_beacon_basic_packet();

#endif // INCLUDE_GUARD__COMMS_TX_H__
