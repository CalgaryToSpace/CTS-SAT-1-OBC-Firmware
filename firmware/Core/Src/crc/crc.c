#include "crc/crc.h"
#include "log/log.h"

#include <stdint.h>
#include "main.h"

/**
 * @brief Compute CRC32 checksum.
 * @param input_message: Pointer to the input data array.
 * @param input_message_len: Length of the input data array.
 */
uint32_t GEN_crc32_checksum(uint8_t *input_message, uint32_t input_message_len)
{
    uint32_t checksum = HAL_CRC_Calculate(&hcrc, (uint32_t *)input_message, input_message_len);
    LOG_message(LOG_SYSTEM_ALL, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "Checksum is %lu \n", checksum);
    return checksum;
}