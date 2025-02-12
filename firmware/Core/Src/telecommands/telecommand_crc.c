#include "crc.h"
#include "log/log.h"
#include "telecommands/telecommand_args_helpers.h"
#include "telecommands/telecommand_types.h"
#include "main.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

// Function only works with Hex input messages.
uint8_t TCMDEXEC_crc(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel, char *response_output_buf,
                        uint16_t response_output_buf_len)
{
    uint8_t buffer[1024];
    uint16_t result_length = 0;

    const uint8_t parse_result = TCMD_extract_hex_array_arg(args_str, 0, buffer, sizeof(buffer), &result_length);

    if (parse_result != 0)
    {
        snprintf(response_output_buf, response_output_buf_len, "Could not parse arg 0 for: %s", args_str);
        return 1;
    }

    // Generate CRC32 checksum
    uint32_t crc_result = GEN_crc32_checksum(&hcrc1, buffer, result_length);

    snprintf(response_output_buf, response_output_buf_len, "The CRC result is %" PRIx32 ".\n", crc_result);

    return 0;
}
