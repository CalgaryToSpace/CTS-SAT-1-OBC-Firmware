#include "uart_handler/uart_handler.h"
#include "uart_handler/uart_error_tracking.h"
#include "unit_tests/unit_test_helpers.h"

uint8_t TEST_EXEC__UART_Error_tracking_get_tracking_struct_from_uart_instance()
{
    // Test the function with every valid UART Instance
    UART_Error_Info_t error_info_struct;
    uint8_t result = UART_Error_tracking_get_tracking_struct_from_uart_instance(UART_mpi_port_handle->Instance, &error_info_struct);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(&error_info_struct == &UART_Error_mpi_error_info);

    result = UART_Error_tracking_get_tracking_struct_from_uart_instance(UART_gps_port_handle->Instance, &error_info_struct);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(&error_info_struct == &UART_Error_gps_error_info);

    result = UART_Error_tracking_get_tracking_struct_from_uart_instance(UART_camera_port_handle->Instance, &error_info_struct);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(&error_info_struct == &UART_Error_camera_error_info);

    result = UART_Error_tracking_get_tracking_struct_from_uart_instance(UART_eps_port_handle->Instance, &error_info_struct);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(&error_info_struct == &UART_Error_eps_error_info);

    // Test the function with an invalid UART Instance
    USART_TypeDef *invalid_uart_instance = (USART_TypeDef *)0x12345678; // Invalid address
    result = UART_Error_tracking_get_tracking_struct_from_uart_instance(invalid_uart_instance, &error_info_struct);
    TEST_ASSERT(result == 1); // Expecting error
    
    // Ensure that the address is not equal to any of the valid UART instances 
    TEST_ASSERT(
        (&error_info_struct != &UART_Error_mpi_error_info) &&
        (&error_info_struct != &UART_Error_gps_error_info) &&
        (&error_info_struct != &UART_Error_camera_error_info) &&
        (&error_info_struct != &UART_Error_eps_error_info)
    )

    TEST_ASSERT(
        (error_info_struct.parity_error_count == 0) &&
        (error_info_struct.noise_error_count == 0) &&
        (error_info_struct.frame_error_count == 0) &&
        (error_info_struct.overrun_error_count == 0) &&
        (error_info_struct.dma_transfer_error_count == 0) &&
        (error_info_struct.receiver_timeout_error_count == 0)
    ); // Ensure that it is initialized to 0
    return 0;
}
