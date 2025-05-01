#include "uart_handler/uart_handler.h"
#include "uart_handler/uart_error_tracking.h"
#include "unit_tests/unit_test_helpers.h"

#include <string.h>

uint8_t TEST_EXEC__UART_Error_tracking_get_tracking_struct_from_uart_instance()
{
    UART_Error_Info_t *error_info_struct = NULL;

    // Test with all valid UART instances
    uint8_t result = UART_Error_tracking_get_tracking_struct_from_uart_instance(UART_mpi_port_handle->Instance, &error_info_struct);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(error_info_struct == &UART_Error_mpi_error_info);

    result = UART_Error_tracking_get_tracking_struct_from_uart_instance(UART_gps_port_handle->Instance, &error_info_struct);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(error_info_struct == &UART_Error_gps_error_info);

    result = UART_Error_tracking_get_tracking_struct_from_uart_instance(UART_camera_port_handle->Instance, &error_info_struct);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(error_info_struct == &UART_Error_camera_error_info);

    result = UART_Error_tracking_get_tracking_struct_from_uart_instance(UART_eps_port_handle->Instance, &error_info_struct);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(error_info_struct == &UART_Error_eps_error_info);

    // Prepare a known pointer value before invalid call
    UART_Error_Info_t dummy_error_info;
    error_info_struct = &dummy_error_info;

    result = UART_Error_tracking_get_tracking_struct_from_uart_instance((USART_TypeDef *)0x12345678, &error_info_struct);
    TEST_ASSERT(result == 1);
    // The pointer should remain unchanged
    TEST_ASSERT(error_info_struct == &dummy_error_info);
    TEST_ASSERT_FALSE(
        error_info_struct == &UART_Error_mpi_error_info ||
        error_info_struct == &UART_Error_gps_error_info ||
        error_info_struct == &UART_Error_camera_error_info ||
        error_info_struct == &UART_Error_eps_error_info
    )

    return 0;
}


uint8_t TEST_EXEC__UART_Error_tracking_subsystem_error_info_to_json()
{
    char json_buffer[UART_ERROR_TRACKING_JSON_BUFFER_LEN / 4];

    UART_Error_Info_t test_info = 
    {
        .parity_error_count = 1,
        .noise_error_count = 2,
        .frame_error_count = 3,
        .overrun_error_count = 4,
        .dma_transfer_error_count = 5,
        .receiver_timeout_error_count = 6
    };

    // Case 1: Valid buffer size
    uint8_t result = UART_Error_tracking_subsystem_error_info_to_json(&test_info, json_buffer, sizeof(json_buffer));
    TEST_ASSERT(result == 0);

    const char *expected_json = 
    "{\"parity_error_count\":1,"
    "\"noise_error_count\":2,"
    "\"frame_error_count\":3,"
    "\"overrun_error_count\":4,"
    "\"dma_transfer_error_count\":5,"
    "\"receiver_timeout_error_count\":6}";

    TEST_ASSERT(strcmp(json_buffer, expected_json) == 0);

    // Case 2: Buffer too small
    char small_buffer[16] = {0};
    result = UART_Error_tracking_subsystem_error_info_to_json(&test_info, small_buffer, sizeof(small_buffer));
    TEST_ASSERT(result == 1);             // Expect failure due to small buffer
    TEST_ASSERT(small_buffer[0] == '\0'); // Ensure buffer is empty

    return 0;
}
