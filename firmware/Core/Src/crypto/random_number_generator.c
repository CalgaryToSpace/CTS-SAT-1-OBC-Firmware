#include "crypto/random_number_generator.h"

#include <string.h>


static uint32_t rng_state = 0x12345678;

static inline uint32_t xorshift32(void) {
    uint32_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng_state = x;
    return x;
}



/// @brief Generate a pseudo-random number. Not intended for cryptographic use.
/// @param more_random_data A seed input value, like HAL_GetTick().
/// @return A pseudo-random number.
uint32_t CRYPTO_generate_random_uint32(uint32_t more_random_data) {
    rng_state ^= more_random_data;
    return xorshift32();
}


/// @brief Fill a buffer with pseudo-random data.
/// @param more_random_data A seed input value, like HAL_GetTick().
/// @param buffer Destination buffer.
/// @param size Size of the buffer in bytes.
void CRYPTO_random_fill_buffer(uint32_t more_random_data, uint8_t buffer[], uint32_t size) {
    // Mix entropy once upfront.
    rng_state ^= more_random_data;

    uint32_t i = 0;

    // Fast path: direct 32-bit writes if buffer is word-aligned.
    if (((uintptr_t)buffer & 0x3) == 0) {
        uint32_t *p = (uint32_t *)buffer;
        for (; i + 4 <= size; i += 4) {
            *p++ = xorshift32();
        }
    }
    else {
        // Unaligned: memcpy is still needed but we avoid redundant RNG calls.
        for (; i + 4 <= size; i += 4) {
            uint32_t word = xorshift32();
            memcpy(&buffer[i], &word, 4);
        }
    }

    // Fill the last couple bytes past the size-divisible-by-4 boundary.
    if (i < size) {
        uint32_t tail = xorshift32();
        for (uint32_t shift = 0; i < size; i++, shift += 8) {
            buffer[i] = (uint8_t)(tail >> shift);
        }
    }
}
