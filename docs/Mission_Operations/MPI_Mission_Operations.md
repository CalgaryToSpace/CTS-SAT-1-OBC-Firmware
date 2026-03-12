# MPI Mission Operations

## Task: Record MPI data

### Procedure

1. Run the `mpi_enable_active_mode(<filename>)` telecommand.
    * This command enables the necessary EPS channels, and instructs the MPI to start sending science data frames.
    * Recommendation: Store the timestamp in the filename.
2. Run the `mpi_disable_active_mode()` telecommand to stop recording.
    * This command disables the EPS channels for the MPI.
3. Downlink the file.


## Task: Configure MPI, then record data (recommended way)

The MPI's configuration is volatile (that is, it is reset upon MPI reboot).

The MPI can be reconfigured with 1 or more configuration commands prior to data recording.

### Procedure

1. Power on the EPS channels for the MPI (`mpi_5v` and `mpi_12v`).
2. Use the `mpi_send_command_get_response` telecommand to set the configuration.
    * E.g., `mpi_send_command_get_response(54 43 15 00 02)`
3. Begin recording with the `mpi_enable_active_mode(<filename>)` telecommand.


## Task: Configure MPI, then record data (alternative way)

The MPI can be reconfigured with 1 or more configuration commands prior to data recording.

This approach uses the `uart_send_hex` telecommand instead of the `mpi_send_command_get_response` telecommand. It requires an extra step to set the transceiver correctly.

### Procedure

1. Power on the EPS channels for the MPI (`mpi_5v` and `mpi_12v`).
2. Use the `mpi_set_transceiver_mode` telecommand to set the MPI's UART transceiver to either MOSI or Duplex mode.
3. Use the `uart_send_hex(mpi,<command in hex>)` telecommand to set the configuration.
    * E.g., `uart_send_hex(mpi,54 43 15 00 02)`
4. Begin recording with the `mpi_enable_active_mode(<filename>)` telecommand.
