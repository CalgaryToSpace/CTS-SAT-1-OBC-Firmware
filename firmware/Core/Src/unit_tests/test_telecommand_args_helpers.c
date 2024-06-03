#include "unit_tests/unit_test_helpers.h"
#include "telecommands/telecommand_args_helpers.h"
#include  "debug_tools/debug_uart.h"

#include <string.h>
#include <stdio.h>

uint8_t TEST_EXEC__TCMD_arg_base64_decode() {
    uint8_t result = 0;

    uint8_t base64_result[3] = {0};
    uint32_t base64_result_len = 3;

    char * telecommand1 = "CTS1+upload_mpi_firmware_page(AAAB)";
    result = TCMD_arg_base64_decode(telecommand1, strlen(telecommand1), 0, base64_result, &base64_result_len);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(base64_result[0] == 0);
    TEST_ASSERT(base64_result[1] == 0);
    TEST_ASSERT(base64_result[2] == 1);

    char * telecommand2 = "CTS1+upload_mpi_firmware_page(0,AAAB)";
    result = TCMD_arg_base64_decode(telecommand2, strlen(telecommand2), 1, base64_result, &base64_result_len);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(base64_result[0] == 0);
    TEST_ASSERT(base64_result[1] == 0);
    TEST_ASSERT(base64_result[2] == 1);
    
    char * telecommand3 = "CTS1+upload_mpi_firmware_page(0,AAAC,testfirmware.bin,42)";
    result = TCMD_arg_base64_decode(telecommand3, strlen(telecommand3), 1, base64_result, &base64_result_len);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(base64_result[0] == 0);
    TEST_ASSERT(base64_result[1] == 0);
    TEST_ASSERT(base64_result[2] == 2);
    
    return 0;
}


