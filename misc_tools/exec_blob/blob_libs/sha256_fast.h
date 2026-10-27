// Header-only SHA256 for blobs. Faster than the firmware's `sha256_update()` (B-Con
// crypto-algorithms, which copies byte by byte and is built at -Og), and small (~1.3 KB of code
// and tables).
//
// Usage:
//     #include "../blob_libs/sha256_fast.h"
//     sha256_fast_ctx_t ctx;
//     sha256_fast_init(&ctx);
//     sha256_fast_update(&ctx, data, len); // Repeat as needed.
//     sha256_fast_final(&ctx, hash);       // hash: uint8_t[32]
//
// Performance (Cortex-M4 Thumb-2, counted in an emulator, hashing 16 KiB):
//     Firmware's sha256_update() at -Og: 65.2 instructions/byte.
//     This, at -Os (with the hot loop at -O2): 37.2 instructions/byte.
//
// Notes:
// 1. Everything is `static`, so each blob gets its own copy, and unused functions are dropped.
// 2. Names are prefixed `sha256_fast_` to not clash with the firmware's `crypto/sha256.h`.
// 3. Hashes up to 4 GiB (the length is kept as a uint32 byte count).
// 4. memset/memcpy are declared here (not via <string.h>, as blobs are built with -nostdlib), and
//     resolve to the firmware's at link time (--just-symbols).

#ifndef BLOB_LIBS_SHA256_FAST_H
#define BLOB_LIBS_SHA256_FAST_H

#include <stdint.h>
#include <stddef.h>

extern void *memset(void *dest, int value, size_t len);
extern void *memcpy(void *dest, const void *src, size_t len);

typedef struct {
    uint32_t state[8];
    uint8_t block[64]; // Partial block, awaiting more data.
    uint32_t block_len;
    uint32_t total_len; // Bytes hashed so far.
} sha256_fast_ctx_t;

static const uint32_t SHA256_FAST_K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

/// @brief A uint32 that may be unaligned (the Cortex-M4 allows unaligned word loads).
typedef uint32_t __attribute__((aligned(1), may_alias)) sha256_fast_unaligned_u32_t;

#define SHA256_FAST_ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

/// @brief One SHA256 round. The callers rotate the variable names, instead of moving the values.
/// @note Σ1(e) = ROTR(e, 6) ^ ROTR(e, 11) ^ ROTR(e, 25) is factored as ROTR(e ^ ROTR(e, 5) ^
///     ROTR(e, 19), 6) (and likewise Σ0), which saves an instruction each.
#define SHA256_FAST_ROUND(a, b, c, d, e, f, g, h, i) do { \
    const uint32_t t1 = h + SHA256_FAST_ROTR(e ^ SHA256_FAST_ROTR(e, 5) ^ SHA256_FAST_ROTR(e, 19), 6) \
        + (g ^ (e & (f ^ g))) + SHA256_FAST_K[i] + w[i]; \
    d += t1; \
    h = t1 + SHA256_FAST_ROTR(a ^ SHA256_FAST_ROTR(a, 11) ^ SHA256_FAST_ROTR(a, 20), 2) \
        + ((a & b) | (c & (a | b))); \
} while (0)

/// @brief Hash 64-byte blocks into the state.
/// @note Built at -O2 (blobs are usually -Os), as it's the hot loop. ~850 bytes of code.
__attribute__((optimize("O2")))
static void sha256_fast_blocks(uint32_t state[8], const uint8_t *data, uint32_t block_count) {
    for (; block_count > 0; block_count--, data += 64) {
        uint32_t w[64]; // Message schedule.
        for (uint8_t i = 0; i < 16; i++) {
            w[i] = __builtin_bswap32(((const sha256_fast_unaligned_u32_t *)data)[i]);
        }
        for (uint8_t i = 16; i < 64; i++) {
            const uint32_t w2 = w[i - 2];
            const uint32_t w15 = w[i - 15];
            w[i] = (SHA256_FAST_ROTR(w2, 17) ^ SHA256_FAST_ROTR(w2, 19) ^ (w2 >> 10)) + w[i - 7]
                + (SHA256_FAST_ROTR(w15, 7) ^ SHA256_FAST_ROTR(w15, 18) ^ (w15 >> 3)) + w[i - 16];
        }

        uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
        uint32_t e = state[4], f = state[5], g = state[6], h = state[7];
        for (uint8_t i = 0; i < 64; i += 8) {
            SHA256_FAST_ROUND(a, b, c, d, e, f, g, h, i + 0);
            SHA256_FAST_ROUND(h, a, b, c, d, e, f, g, i + 1);
            SHA256_FAST_ROUND(g, h, a, b, c, d, e, f, i + 2);
            SHA256_FAST_ROUND(f, g, h, a, b, c, d, e, i + 3);
            SHA256_FAST_ROUND(e, f, g, h, a, b, c, d, i + 4);
            SHA256_FAST_ROUND(d, e, f, g, h, a, b, c, i + 5);
            SHA256_FAST_ROUND(c, d, e, f, g, h, a, b, i + 6);
            SHA256_FAST_ROUND(b, c, d, e, f, g, h, a, i + 7);
        }
        state[0] += a; state[1] += b; state[2] += c; state[3] += d;
        state[4] += e; state[5] += f; state[6] += g; state[7] += h;
    }
}

static void sha256_fast_init(sha256_fast_ctx_t *ctx) {
    static const uint32_t SHA256_FAST_INITIAL_STATE[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
    };
    memcpy(ctx->state, SHA256_FAST_INITIAL_STATE, sizeof(ctx->state));
    ctx->block_len = 0;
    ctx->total_len = 0;
}

static void sha256_fast_update(sha256_fast_ctx_t *ctx, const uint8_t *data, uint32_t len) {
    ctx->total_len += len;

    // Top up a partial block first.
    if (ctx->block_len > 0) {
        const uint32_t take = (len < 64 - ctx->block_len) ? len : 64 - ctx->block_len;
        memcpy(&ctx->block[ctx->block_len], data, take);
        ctx->block_len += take;
        data += take;
        len -= take;
        if (ctx->block_len < 64) return;
        sha256_fast_blocks(ctx->state, ctx->block, 1);
        ctx->block_len = 0;
    }

    // Hash whole blocks straight from `data` (no copy).
    sha256_fast_blocks(ctx->state, data, len / 64);
    data += len & ~63u;
    len &= 63;

    // Keep the remainder for next time.
    memcpy(ctx->block, data, len);
    ctx->block_len = len;
}

static void sha256_fast_final(sha256_fast_ctx_t *ctx, uint8_t hash[32]) {
    const uint32_t total_len = ctx->total_len;

    // 0x80, zeros, then the 64-bit big-endian length in bits (ending on a block boundary).
    uint8_t pad[72];
    const uint32_t pad_len = ((ctx->block_len < 56) ? 56 : 120) - ctx->block_len;
    memset(pad, 0, sizeof(pad));
    pad[0] = 0x80;
    const uint32_t bits_hi = total_len >> 29;
    const uint32_t bits_lo = total_len << 3;
    for (uint8_t i = 0; i < 4; i++) {
        pad[pad_len + i] = (uint8_t)(bits_hi >> (24 - 8 * i));
        pad[pad_len + 4 + i] = (uint8_t)(bits_lo >> (24 - 8 * i));
    }
    sha256_fast_update(ctx, pad, pad_len + 8);

    for (uint8_t i = 0; i < 32; i++) {
        hash[i] = (uint8_t)(ctx->state[i / 4] >> (24 - 8 * (i % 4)));
    }
}

#endif // BLOB_LIBS_SHA256_FAST_H
