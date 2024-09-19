#include "crc.h"
#include <stdint.h>
// #include "main.h"

#define CRC32_POLYNOMIAL 0x04C11DB7
CRC_HandleTypeDef hcrc1;

/**
 * @brief Compute CRC32 checksum.
 * @param input_arr: Pointer to the input data array.
 * @param input_arr_len: Length of the input data array.
 * @param output_arr: Pointer to the output array where the CRC checksum will be stored (should be of length 1).
 */
void GEN_crc32_checksum(CRC_HandleTypeDef *hcrc,  uint8_t *input_arr, uint32_t input_arr_len, int64_t *output_arr)
{
    // Reset the CRC calculation unit
    __HAL_CRC_DR_RESET(&hcrc1);

    // Compute the CRC32 checksum
    *output_arr = HAL_CRC_Calculate(&hcrc1, (uint32_t *)input_arr, input_arr_len / 4);
}
