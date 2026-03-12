# MPI Mission Operations

## Task: Record MPI data

### Procedure

1. Run the `mpi_enable_active_mode(<filename>)` telecommand.
    * This command enables the necessary EPS channels, and instructs the MPI to start sending science data frames.
    * Recommendation: Store the timestamp in the filename.
2. Run the `mpi_enable_active_mode()` telecommand to stop recording.
3. Downlink the file.


## Task: Configure MPI, then record data

The MPI's configuration is volatile (that is, it is reset upon MPI reboot).

The MPI can be reconfigured with 1 or more configuration commands prior to data recording.

### Procedure

1. Power on the EPS channels for the MPI (`mpi_5v` and `mpi_12v`).
2. Use the `mpi_send_command_get_response` telecommand to set the configuration.
    * E.g., `mpi_send_command_get_response(54 43 15 00 02)`
3. Begin recording with the `mpi_enable_active_mode(<filename>)` telecommand.
