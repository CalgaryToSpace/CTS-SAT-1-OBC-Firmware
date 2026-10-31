# Radio Packet Format

## Packet Type Enum

The first byte of the packet (after the 4-byte CSP header) is the packet type. The packet type is an enum which indicates the type of packet being sent. The packet type is used to determine how to parse the rest of the packet.

### Byte 0: Enum packet type
    * `0x01 = COMMS_PACKET_TYPE_BEACON_BASIC` - beacon (basic telemetry, no peripherals)
    * `0x03 = COMMS_PACKET_TYPE_LOG_MESSAGE` - log message
    * `0x04 = COMMS_PACKET_TYPE_TCMD_RESPONSE` - telecommand response
    * `0x10 = COMMS_PACKET_TYPE_BULK_FILE_DOWNLINK` - bulk file downlink
    * `0x20 = COMMS_PACKET_TYPE_BEACON_EXTENDED` - extended beacon (v2, v3, v4)
    * `0x21 = COMMS_PACKET_TYPE_BEACON_EXTENDED_V5PLUS` - extended beacon (v5+)
    * `0x30 = COMMS_PACKET_TYPE_GNSS_BESTXYZB_SAMPLE` - GNSS BESTXYZB sample (sent by `misc_tools/exec_blob/gnss_bestxyzb_ring_blob`)

## Bytes 1-200 of the Packet, for each Packet Type

### Byte 1-200 of Basic Telemetry Beacon Packets (Packet Type `0x01`)

See the beacon packet format (as a tightly-packed C struct) in [the firmware at `/comms_drivers/comms_tx.h`](https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/blob/main/firmware/Core/Inc/comms_drivers/comms_tx.h), in `COMMS_beacon_basic_packet_t`.

Relevant enum definitions:

```c
typedef enum {
    OBC_RBF_STATE_FLYING = 1,
    OBC_RBF_STATE_BENCH = 0,
} OBC_rbf_state_enum_t;

typedef enum {
    GNSS_RX_MODE_COMMAND_MODE,   // GNSS is in command mode.
    GNSS_RX_MODE_FIREHOSE_MODE,  // GNSS is in firehose data storage mode.
    GNSS_RX_MODE_DISABLED        // GNSS data is not being collected/stored.
} GNSS_rx_mode_enum_t;

typedef enum {
    MPI_RX_MODE_COMMAND_MODE,        // MPI is in command mode
    MPI_RX_MODE_SENSING_MODE,        // MPI is science data is being collected mode
    MPI_RX_MODE_NOT_LISTENING_TO_MPI // MPI may be sending science data, but it is not being collected by OBC.
} MPI_rx_mode_enum_t;

typedef enum {
    MPI_TRANSCEIVER_MODE_INACTIVE,
    MPI_TRANSCEIVER_MODE_MOSI,
    MPI_TRANSCEIVER_MODE_MISO,
    MPI_TRANSCEIVER_MODE_DUPLEX
} MPI_transceiver_state_enum_t;

typedef enum {
    MPI_REASON_FOR_STOPPING_NOT_SET,
    MPI_REASON_FOR_STOPPING_TEMPERATURE_EXCEEDED,
    MPI_REASON_FOR_STOPPING_TELECOMMAND,
    MPI_REASON_FOR_STOPPING_MAX_TIME_EXCEEDED,
    MPI_REASON_FOR_STOPPING_SELF_CHECK_DONE,
} MPI_reason_for_stopping_active_mode_enum_t;

typedef enum {
    TIME_SYNC_SOURCE_NONE = 0,
    TIME_SYNC_SOURCE_GNSS_UART,
    TIME_SYNC_SOURCE_GNSS_PPS,
    TIME_SYNC_SOURCE_TELECOMMAND_ABSOLUTE,
    TIME_SYNC_SOURCE_TELECOMMAND_CORRECTION,
    TIME_SYNC_SOURCE_EPS_RTC // Electrical Power System's Real-Time Clock (precise to 1 second only)
} TIME_sync_source_enum_t;

typedef enum {
    COMMS_RF_SWITCH_CONTROL_MODE_TOGGLE_BEFORE_EVERY_BEACON = 0, // Default.
    COMMS_RF_SWITCH_CONTROL_MODE_FORCE_ANT1 = 1,
    COMMS_RF_SWITCH_CONTROL_MODE_FORCE_ANT2 = 2,
    COMMS_RF_SWITCH_CONTROL_MODE_USE_ADCS_NORMAL = 3,
    COMMS_RF_SWITCH_CONTROL_MODE_USE_ADCS_FLIPPED = 4, // Redundant implementation that assumes the antennas are backwards.
    COMMS_RF_SWITCH_CONTROL_MODE_UNKNOWN = 255, // Used for error handling in string parser.
} COMMS_rf_switch_control_mode_enum_t;

typedef enum {
    // Entry point. Booted, and awaiting a trigger to deployment. Emulate a 60-minute countdown to antenna deployment.
    CTS1_OPERATION_STATE_BOOTED_AND_WAITING,

    // Intermediate state. Very short-lived nominally. Deploy deployable antennas here.
    CTS1_OPERATION_STATE_DEPLOYING,

    // Normal operation state, with radio transmission enabled (e.g., flying through space).
    CTS1_OPERATION_STATE_NOMINAL_WITH_RADIO_TX,

    // Normal operation state, with radio transmission disabled (e.g., during bench testing).
    CTS1_OPERATION_STATE_NOMINAL_WITHOUT_RADIO_TX
} CTS1_operation_state_enum_t;

typedef enum {
    STM32_RESET_CAUSE_UNKNOWN = 0,
    STM32_RESET_CAUSE_LOW_POWER_RESET,
    STM32_RESET_CAUSE_WINDOW_WATCHDOG_RESET,
    STM32_RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET,
    STM32_RESET_CAUSE_SOFTWARE_RESET,
    STM32_RESET_CAUSE_EXTERNAL_RESET_PIN_RESET,
    STM32_RESET_CAUSE_BROWNOUT_RESET,
    STM32_RESET_CAUSE_OPTION_BYTE_LOADER_RESET,
    STM32_RESET_CAUSE_FIREWALL_RESET,
} STM32_reset_cause_t;
```

### Bytes 1-200 of Log Message Packets (Packet Type `0x03`)

This packet type does not have sequence numbers. All log messages fit within (or are truncated to fit within) a single packet.

Plain text content, same as UART messages.

### Bytes 1-200 of Telecommand Response Packets (Packet Type `0x04`)

This packet type has sequence numbers.

* Bytes 1,2,3,4,5,6,7,8: uint64_t of tssent (unique ID of the associated uplinked telecommand)
* Byte 9: uint8_t of response code (0 for success)
* Byte 10,11: uint16_t of duration in ms
* Byte 12: sequence number of telecommand response (starting from 1)
* Byte 13: total number of telecommand response packets (e.g., 1 if <183 bytes)
* Bytes 14-200: Plain text content of the `response_output_buf`, null terminated


### Bytes 1-200 of Bulk File Downlink Packets (Packet Type `0x10`)

* Bytes 1,2,3,4: uint32_t of starting byte offset within file (index of Byte 5)
* Bytes 5-200: Content of file

### Bytes of Extended Beacon Packets in v2, v3, v4 (Packet Type `0x20`)

See the extended beacon packet format (as a tightly-packed C struct) in [the extended beacon blob, as of v4 (1c98525a)](https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/blob/1c98525a7b97f48ddef94b91df14d91b9fa3518a/misc_tools/exec_blob/extended_beacon_blob/extended_beacon_blob_main.c#L165), in `COMMS_beacon_extended_packet_t`.

The code is included as follows:

<details>

```c
#pragma pack(push, 1)

typedef struct {
    uint8_t packet_type; // COMMS_packet_type_enum_t - Always COMMS_PACKET_TYPE_BEACON_EXTENDED = 0x20 for this packet.

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

    // ADCS Estimated Attitude Angles (Telemetry ID 146).
    // These values are populated in Estimation Mode 3, 4, 5, and 6 only.
    int16_t adcs_estimated_roll_angle_cdeg;
    int16_t adcs_estimated_pitch_angle_cdeg;
    int16_t adcs_estimated_yaw_angle_cdeg;
} COMMS_beacon_extended_packet_t;

// Packet size (currently):
_Static_assert(sizeof(COMMS_beacon_extended_packet_t) == 198, "ext. beacon packet size documented wrong here");

#pragma pack(pop)
```

</details>

### Bytes of Extended Beacon Packets in v5+ (Packet Type `0x21`)

The v5+ extended beacon format places the friendly message field with more data fields.

Its format is defined in `COMMS_beacon_extended_packet_v5plus_t`.


### Bytes 1-end of GNSS BESTXYZB Sample Packets (Packet Type `0x30`)

Sent by `misc_tools/exec_blob/gnss_bestxyzb_ring_blob`, a blob (not built into the main firmware
image) that periodically samples `log bestxyzb once` from the GNSS receiver into an in-memory ring
buffer, and downlinks randomly-selected samples from that buffer on each run. See
`GNSS_bestxyzb_downlink_packet_t` in that blob's source for the authoritative struct definition.

* Bytes 1,2: uint16_t sequence number of this downlinked packet (persisted across the blob's runs; wraps at 65536)
* Bytes 3,4: uint16_t ring buffer position (0-299) this sample was read from
* Bytes 5-end: Raw NovAtel BESTXYZB binary log (144 bytes).


## Other Details

### Endianness

All multi-byte integers in these packets are in little-endian format.

For example, a uint16_t value of 1013 (0x03F5) would be sent as [0xF5, 0x03].

### Packet Length

The packet length is encoded by the AX100 in the ASM+GOLAY encoding scheme.

It is not managed nor encoded in the packets at the level handled by this firmware on the OBC.
