#include "crc.h"
#include <stdint.h>
#include "main.h"

#define CRC32_POLYNOMIAL 0x04C11DB7

CRC_HandleTypeDef hcrc;

int main(void)
{

    uint8_t input_arr[] = "Hello, World!";
    uint32_t crc_result;

    GEN_crc32_checksum(input_arr, sizeof(input_arr) - 1, &crc_result);
    DEBUG_uart_print_str("CRC32 Checksum: %08X\n", crc_result);
    /*
    while (1)
    {
    }
    */
}

void MX_CRC_Init(void)
{
    __HAL_RCC_CRC_CLK_ENABLE();

    hcrc.Instance = CRC;
    if (HAL_CRC_Init(&hcrc) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief Compute CRC32 checksum.
 * @param input_arr: Pointer to the input data array.
 * @param input_arr_len: Length of the input data array.
 * @param output_arr: Pointer to the output array where the CRC checksum will be stored (should be of length 1).
 */
void GEN_crc32_checksum(uint8_t *input_arr, uint32_t input_arr_len, uint32_t *output_arr)
{
    // Reset the CRC calculation unit
    __HAL_CRC_DR_RESET(&hcrc);

    // Compute the CRC32 checksum
    *output_arr = HAL_CRC_Calculate(&hcrc, (uint32_t *)input_arr, input_arr_len / 4);
}
