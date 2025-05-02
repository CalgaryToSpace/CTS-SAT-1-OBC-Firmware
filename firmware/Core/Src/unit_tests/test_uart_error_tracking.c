#include "uart_handler/uart_handler.h"
#include "uart_handler/uart_error_tracking.h"
#include "unit_tests/unit_test_helpers.h"

#include <string.h>

uint8_t TEST_EXEC__UART_single_subsystem_error_info_to_json()
{
    char json_buffer[UART_ERROR_TRACKING_JSON_BUFFER_LEN / 4];

    UART_error_counts_single_subsystem_struct_t test_info = 
    {
        .parity_error_count = 1,
        .noise_error_count = 2,
        .frame_error_count = 3,
        .overrun_error_count = 4,
        .dma_transfer_error_count = 5,
        .receiver_timeout_error_count = 6
    };

    // Case 1: Valid buffer size
    uint8_t result = UART_single_subsystem_error_info_to_json(&test_info, json_buffer, sizeof(json_buffer));
    TEST_ASSERT(result == 0);

    const char *expected_json = 
    "{\"parity\":1,"
    "\"noise\":2,"
    "\"frame\":3,"
    "\"overrun\":4,"
    "\"dma_transfer\":5,"
    "\"receiver_timeout\":6}";

    TEST_ASSERT(strcmp(json_buffer, expected_json) == 0);

    // Case 2: Buffer too small
    char small_buffer[16] = {0};
    result = UART_single_subsystem_error_info_to_json(&test_info, small_buffer, sizeof(small_buffer));
    TEST_ASSERT(result == 1);             // Expect failure due to small buffer
    TEST_ASSERT(small_buffer[0] == '\0'); // Ensure buffer is empty

    return 0;
}
