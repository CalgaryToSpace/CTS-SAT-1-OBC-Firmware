#ifndef INCLUDE_GUARD__RANDOM_NUMBER_GENERATOR_H
#define INCLUDE_GUARD__RANDOM_NUMBER_GENERATOR_H

#include <stdint.h>

uint32_t CRYPTO_generate_random_uint32(uint32_t more_random_data);

void CRYPTO_random_fill_buffer(uint32_t more_random_data, uint8_t buffer[], uint32_t size);

#endif // INCLUDE_GUARD__RANDOM_NUMBER_GENERATOR_H
