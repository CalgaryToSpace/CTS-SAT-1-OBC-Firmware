#include "crypto/random_number_generator.h"

#include <string.h>


static uint32_t rng_state = 0x12345678;

/// @brief Generate a pseudo-random number. Not intended for cryptographic use.
/// @param more_random_data A seed input value, like HAL_GetTick().
/// @return A pseudo-random number.
uint32_t CRYPTO_generate_random_uint32(uint32_t more_random_data) {
    rng_state ^= more_random_data;

    uint32_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng_state = x;
    return x;
}


void CRYPTO_random_fill_buffer(uint32_t more_random_data, uint8_t buffer[], uint32_t size) {
    const uint32_t bytes_at_end = size % 4;

    uint32_t i = 0;

    for (i = 0; i < (size - bytes_at_end); i += 4) {
        uint32_t random_uint32 = CRYPTO_generate_random_uint32(more_random_data + i);
        memcpy(&buffer[i], &random_uint32, 4);
    }

    // Fill the last couple bytes past the size-divisible-by-4 boundary.
    for (; i < size; i++) {
        buffer[i] = (uint8_t)CRYPTO_generate_random_uint32(more_random_data + i);
    }
}
