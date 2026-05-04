# Radio Packet Format

## Packet Type Enum

The first byte of the packet (after the 4-byte CSP header) is the packet type. The packet type is an enum which indicates the type of packet being sent. The packet type is used to determine how to parse the rest of the packet.

### Byte 0: Enum packet type
    * `0x01` - beacon (basic telemetry, no peripherals)
    * `0x03` - log message
    * `0x04` - telecommand response
    * `0x10` - bulk file downlink

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


## Other Details

### Endianness

All multi-byte integers in these packets are in little-endian format.

For example, a uint16_t value of 1013 (0x03F5) would be sent as [0xF5, 0x03].

### Packet Length

The packet length is encoded by the AX100 in the ASM+GOLAY encoding scheme.

It is not managed nor encoded in the packets at the level handled by this firmware on the OBC.
