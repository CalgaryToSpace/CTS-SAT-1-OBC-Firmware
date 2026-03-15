#include "mpi/mpi_types.h"

const char* MPI_rx_mode_enum_to_str(MPI_rx_mode_enum_t mode) {
    switch (mode) {
        case MPI_RX_MODE_COMMAND_MODE:
            return "COMMAND_MODE";
        case MPI_RX_MODE_SENSING_MODE:
            return "SENSING_MODE";
        case MPI_RX_MODE_NOT_LISTENING_TO_MPI:
            return "NOT_LISTENING_TO_MPI";
    }
    return "UNKNOWN_MODE";
}

const char* MPI_transceiver_state_enum_to_str(MPI_transceiver_state_enum_t state) {
    switch (state) {
        case MPI_TRANSCEIVER_MODE_INACTIVE:
            return "INACTIVE";
        case MPI_TRANSCEIVER_MODE_MOSI:
            return "MOSI";
        case MPI_TRANSCEIVER_MODE_MISO:
            return "MISO";
        case MPI_TRANSCEIVER_MODE_DUPLEX:
            return "DUPLEX";
    }
    return "UNKNOWN";
}
