#include "crc.h"
#include "log/log.h"
#include <inttypes.h>

#include <stdint.h>
// #include "main.h"

/**
 * @brief Compute CRC32 checksum.
 * @param hcrc: HandleTypeDef
 * @param input_arr: Pointer to the input data array.
 * @param input_arr_len: Length of the input data array.
 * @param output_arr: Pointer to the output array where the CRC checksum will be stored (should be of length 1).
 */
uint32_t GEN_crc32_checksum(CRC_HandleTypeDef *hcrc,  uint8_t *input_arr, uint32_t input_arr_len)
{
    uint8_t polynomial = hcrc->Init.DefaultPolynomialUse;
    uint8_t initial_value = hcrc->Init.DefaultInitValueUse;
    uint32_t input_inversion = hcrc->Init.InputDataInversionMode;
    uint32_t output_inversion = hcrc->Init.OutputDataInversionMode;
    uint32_t data_format = hcrc->InputDataFormat;
    uint32_t generating_polynomial = hcrc->Init.GeneratingPolynomial;


    // Log resister value for checksum
    LOG_message(LOG_SYSTEM_ALL, LOG_SEVERITY_ERROR, LOG_SINK_ALL, "The default polynomial is %lu \n The initial value is %lu \n The input inversion is %lu \n The output inversion is %lu \n The data format is %lu \n The generating polynomial is %lu", polynomial, initial_value, input_inversion, output_inversion, data_format, generating_polynomial);
    
    // Compute the CRC32 checksum
    // Internally clears data register aka. crc_dr
    uint32_t var = HAL_CRC_Calculate(hcrc, (uint32_t *)input_arr, input_arr_len);
    return var;
}
