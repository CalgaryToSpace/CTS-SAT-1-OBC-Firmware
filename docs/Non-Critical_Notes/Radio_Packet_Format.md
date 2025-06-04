# Radio Packet Format

## Packet Type Enum

The first byte of the packet (after the 4-byte CSP header) is the packet type. The packet type is an enum which indicates the type of packet being sent. The packet type is used to determine how to parse the rest of the packet.

### Byte 0: Enum packet type
    * `0x01` - beacon (basic telemetry, no peripherals)
    * `0x02` - beacon (peripheral telemetry)
    * `0x03` - log message
    * `0x04` - telecommand response
    * `0x10` - bulk file downlink

## Bytes 1-200 of the Packet, for each Packet Type

### Byte 1-200 of Basic Telemetry Beacon Packets (Packet Type `0x01`)

TODO: Fill in the format here. For now, see https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/338.


### Byte 1-200 of Peripheral Telemetry Beacon Packets (Packet Type `0x02`)

TODO: Fill in the format here. For now, see https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/338.

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

This packet type has sequence numbers.

* Byte 1: sequence number of file downlink packet (starting from 1)
* Byte 2: total number of file downlink packets (e.g., 1 if <183 bytes)
* Bytes 3,4,5,6: uint32_t of byte offset within file
* Bytes 7-200: Content of file


## Other Details

### Endianness

All multi-byte integers in these packets are in little-endian format.

For example, a uint16_t value of 1013 (0x03F5) would be sent as [0xF5, 0x03].

### Packet Length

The packet length is encoded by the AX100 in the ASM+GOLAY encoding scheme.

It is not managed nor encoded in the packets at the level handled by this firmware on the OBC.
