#include "uart_handler/uart_control.h"

#include "uart_handler/uart_handler.h" // For extern port handles.

#include <string.h>

/// @brief Set the STM32 UART peripheral's baud rate to a different value.
/// @param port_handle Pointer to the UART handle structure for the port to change the baud rate of.
/// @param new_baud_rate The new baud rate to set the STM32 UART peripheral to (in bits per second). Common values are 9600, 115200, 230400, etc.
/// @return 0 on success, >0 on error.
uint8_t UART_set_baud_rate(UART_HandleTypeDef *port_handle, uint32_t new_baud_rate) {
    if (new_baud_rate < 1200 || new_baud_rate > 921600) {
        return 1; // Invalid baud rate
    }

    // Deinitialize the UART port
    const HAL_StatusTypeDef de_init_status = HAL_UART_DeInit(port_handle);
    if (de_init_status != HAL_OK) {
        return 2; // Error deinitializing UART
    }

    // Set the new baud rate
    port_handle->Init.BaudRate = new_baud_rate;

    // Reinitialize the UART port with the new settings
    const HAL_StatusTypeDef init_status = HAL_UART_Init(port_handle);
    if (init_status != HAL_OK) {
        return 3; // Error reinitializing UART
    }

    return 0; // Success
}

/// @brief Get the UART port handle by its name.
/// @param port_name Case-insensitive name of the UART port to retrieve the handle for.
/// @return Pointer to the UART handle structure for the specified port name, or NULL if not found.
/// @warning This function MUST be checked for NULL return value before use.
UART_HandleTypeDef *UART_get_port_handle_by_name(const char port_name[]) {
    if (strcasecmp(port_name, "MPI") == 0) {
        return UART_mpi_port_handle;
    } else if (strcasecmp(port_name, "GNSS") == 0) {
        return UART_gnss_port_handle;
    } else if (strcasecmp(port_name, "CAMERA") == 0) {
        return UART_camera_port_handle;
    } else if (strcasecmp(port_name, "EPS") == 0) {
        return UART_eps_port_handle;
    } else if (strcasecmp(port_name, "AX100") == 0) {
        return UART_ax100_port_handle;
    } else if (strcasecmp(port_name, "DEBUG") == 0) {
        return UART_telecommand_port_handle;
    } else if (strcasecmp(port_name, "TELECOMMAND") == 0) {
        return UART_telecommand_port_handle;
    }

    return NULL; // Invalid port name
}
