#include "unit_tests/unit_test_helpers.h"
#include "telecommands/telecommand_args_helpers.h"
#include  "debug_tools/debug_uart.h"

#include <string.h>
#include <stdio.h>

uint8_t TEST_EXEC__TCMD_arg_base64_decode() {
    uint8_t result = 0;

    char * telecommand = "CTS1+somecommand(AAAB)";
    uint32_t telecommand_len = strlen(telecommand);

    uint8_t base64_result[3] = {0};
    uint32_t base64_result_len = 3;

    result = TCMD_arg_base64_decode(telecommand, telecommand_len, 0, base64_result, &base64_result_len);
    TEST_ASSERT(result == 0);
    TEST_ASSERT(base64_result[0] == 0);
    TEST_ASSERT(base64_result[1] == 0);
    TEST_ASSERT(base64_result[2] == 1);
    
    return 0;
}


