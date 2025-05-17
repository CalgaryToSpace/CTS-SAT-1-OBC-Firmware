# UART Ports

Each UART port and associated subsystem behaves at least a little bit differently. This document captures these behaviours.

## UART1 - MPI

* High baud rate - 230400 baud
* DMA access

## UART2 - AX100

* Baud rate: 230400 baud
* Used exclusively to receive uplinks.
* AX100 sends data encoded with KISS encoding (escape/start/end special bytes).
* Interrupt-based.
* Messages can be received at any time.
* Note that, for simplicity of implementation, downlink is commanded over I2C to the AX100.

## UART3 - GPS

* Baud rate: 115200 baud
* Interrupt-based.
* Interrupt disabled by default. Very cautious about enabling it, as the GPS is known to spam null bytes if misconfigured.

## UART4 - Camera

* Variable baud rate. 115200 default, but decreased to 9600 by command right away to ease writing to LittleFS and avoid dropped data.
* DMA access
* Does not support receiving arbitrary data after transmitting arbitrary data (via the `CTS1+TCMDEXEC_uart_send_hex_get_response_hex` command).
    * Justification: Unnecessary complexity. Not a mission requirement. The camera doesn't really have a "command-mode" like all other UART peripherals do.

## UART5 - EPS

* Baud rate: 115200 baud
* Interrupt-based.
* Interrupt enabled by default, and is trusted.
* EPS only responds to commands, and does not send messages otherwise.

## LPUART1 - Debug/Umbilical

* Baud rate: 115200 baud
* Interrupt-based.
* Interrupt enabled by default, and is trusted. Always on.
