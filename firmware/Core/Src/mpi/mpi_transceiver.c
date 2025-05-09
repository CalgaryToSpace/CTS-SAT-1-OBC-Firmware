#include "main.h"

#include "mpi/mpi_types.h"

MPI_transceiver_state_enum_t MPI_current_transceiver_state = MPI_TRANSCEIVER_MODE_INACTIVE;
// TODO: Add a startup action to put it in INACTIVE mode.


/// @brief Sets the transceiver state.
/// @param state 
void MPI_set_transceiver_state(MPI_transceiver_state_enum_t state) {
    switch (state) {
        case MPI_TRANSCEIVER_MODE_INACTIVE:
            HAL_GPIO_WritePin(
                PIN_MPI_NEN_RX_MISO_OUT_GPIO_Port, PIN_MPI_NEN_RX_MISO_OUT_Pin, 1
            );
            HAL_GPIO_WritePin(
                PIN_MPI_EN_TX_MOSI_OUT_GPIO_Port, PIN_MPI_EN_TX_MOSI_OUT_Pin, 0
            );
            break;
        case MPI_TRANSCEIVER_MODE_MOSI:
            HAL_GPIO_WritePin(
                PIN_MPI_NEN_RX_MISO_OUT_GPIO_Port, PIN_MPI_NEN_RX_MISO_OUT_Pin, 1
            );
            HAL_GPIO_WritePin(
                PIN_MPI_EN_TX_MOSI_OUT_GPIO_Port, PIN_MPI_EN_TX_MOSI_OUT_Pin, 1
            );
            break;
        case MPI_TRANSCEIVER_MODE_MISO:
            HAL_GPIO_WritePin(
                PIN_MPI_NEN_RX_MISO_OUT_GPIO_Port, PIN_MPI_NEN_RX_MISO_OUT_Pin, 0
            );
            HAL_GPIO_WritePin(
                PIN_MPI_EN_TX_MOSI_OUT_GPIO_Port, PIN_MPI_EN_TX_MOSI_OUT_Pin, 0
            );
            break;
        case MPI_TRANSCEIVER_MODE_DUPLEX:
            HAL_GPIO_WritePin(
                PIN_MPI_NEN_RX_MISO_OUT_GPIO_Port, PIN_MPI_NEN_RX_MISO_OUT_Pin, 0
            );
            HAL_GPIO_WritePin(
                PIN_MPI_EN_TX_MOSI_OUT_GPIO_Port, PIN_MPI_EN_TX_MOSI_OUT_Pin, 1
            );
            break;
        default:
            break;
    }

    MPI_current_transceiver_state = state;
}
