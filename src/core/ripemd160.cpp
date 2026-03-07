/**
 * @file ripemd160.cpp
 * Production-grade RIPEMD160 implementation
 * 
 * Used in Bitcoin for HASH160 = RIPEMD160(SHA256(data))
 */

#include "core/ripemd160.h"
#include <string.h>

namespace SeedSigner {
namespace Core {

// RIPEMD160 constants
static const uint32_t K1[5] = {0x00000000, 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xA953FD4E};
static const uint32_t K2[5] = {0x50A28BE6, 0x5C4DD124, 0x6D703EF3, 0x7A6D76E9, 0x00000000};

// Selection of message word
static const uint8_t r1[80] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    7, 4, 13, 1, 10, 6, 15, 3, 12, 0, 9, 5, 2, 14, 11, 8,
    3, 10, 14, 4, 9, 15, 8, 1, 2, 7, 0, 6, 13, 11, 5, 12,
    1, 9, 11, 10, 0, 8, 12, 4, 13, 3, 7, 15, 14, 5, 6, 2,
    4, 0, 5, 9, 7, 12, 2, 10, 14, 1, 3, 8, 11, 6, 15, 13
};

static const uint8_t r2[80] = {
    5, 14, 7, 0, 9, 2, 11, 4, 13, 6, 15, 8, 1, 10, 3, 12,
    6, 11, 3, 7, 0, 13, 5, 10, 14, 15, 8, 12, 4, 9, 1, 2,
    15, 5, 1, 3, 7, 14, 6, 9, 11, 8, 12, 2, 10, 0, 4, 13,
    8, 6, 4, 1, 3, 11, 15, 0, 5, 12, 2, 13, 9, 7, 10, 14,
    12, 15, 10, 4, 1, 5, 8, 7, 6, 2, 13, 14, 0, 3, 9, 11
};

// Amount of left rotate
static const uint8_t s1[80] = {
    11, 14, 15, 12, 5, 8, 7, 9, 11, 13, 14, 15, 6, 7, 9, 8,
    7, 6, 8, 13, 11, 9, 7, 15, 7, 12, 15, 9, 11, 7, 13, 12,
    11, 13, 6, 7, 14, 9, 13, 15, 14, 8, 13, 6, 5, 12, 7, 5,
    11, 12, 14, 15, 14, 15, 9, 8, 9, 14, 5, 6, 8, 6, 5, 12,
    9, 15, 5, 11, 6, 8, 13, 12, 5, 12, 13, 14, 11, 8, 5, 6
};

static const uint8_t s2[80] = {
    8, 9, 9, 11, 13, 15, 15, 5, 7, 7, 8, 11, 14, 14, 12, 6,
    9, 13, 15, 7, 12, 8, 9, 11, 7, 7, 12, 7, 6, 15, 13, 11,
    9, 7, 15, 11, 8, 6, 6, 14, 12, 13, 5, 14, 13, 13, 7, 5,
    15, 5, 8, 11, 14, 14, 6, 14, 6, 9, 12, 9, 12, 5, 15, 8,
    8, 5, 12, 9, 12, 5, 14, 6, 8, 13, 6, 5, 15, 13, 11, 11
};

// Nonlinear functions
#define F(x, y, z) ((x) ^ (y) ^ (z))
#define G(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define H(x, y, z) (((x) | ~(y)) ^ (z))
#define I(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define J(x, y, z) ((x) ^ ((y) | ~(z)))

// Rotate left
#define ROL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

// Round functions
#define ROUND1(a, b, c, d, e, f, k, r, s) { \
    a = ROL(a + f(b, c, d) + x[r] + k, s) + e; \
    c = ROL(c, 10); \
}

void RIPEMD160::process_block(uint32_t* h, const uint8_t* block) {
    uint32_t x[16];
    
    // Convert bytes to words (little endian)
    for (int i = 0; i < 16; i++) {
        x[i] = block[i * 4] | 
               ((uint32_t)block[i * 4 + 1] << 8) | 
               ((uint32_t)block[i * 4 + 2] << 16) | 
               ((uint32_t)block[i * 4 + 3] << 24);
    }
    
    // Working registers
    uint32_t a1 = h[0], b1 = h[1], c1 = h[2], d1 = h[3], e1 = h[4];
    uint32_t a2 = h[0], b2 = h[1], c2 = h[2], d2 = h[3], e2 = h[4];
    uint32_t t;
    
    // Rounds 1-5 (80 steps total)
    for (int i = 0; i < 80; i++) {
        // Left line
        t = ROL(a1 + ((i < 16) ? F(b1, c1, d1) : 
                      (i < 32) ? G(b1, c1, d1) :
                      (i < 48) ? H(b1, c1, d1) :
                      (i < 64) ? I(b1, c1, d1) : J(b1, c1, d1)) 
                 + x[r1[i]] + K1[i / 16], s1[i]) + e1;
        a1 = e1;
        e1 = d1;
        d1 = ROL(c1, 10);
        c1 = b1;
        b1 = t;
        
        // Right line
        t = ROL(a2 + ((i < 16) ? J(b2, c2, d2) :
                      (i < 32) ? I(b2, c2, d2) :
                      (i < 48) ? H(b2, c2, d2) :
                      (i < 64) ? G(b2, c2, d2) : F(b2, c2, d2))
                 + x[r2[i]] + K2[i / 16], s2[i]) + e2;
        a2 = e2;
        e2 = d2;
        d2 = ROL(c2, 10);
        c2 = b2;
        b2 = t;
    }
    
    // Combine results
    t = h[1] + c1 + d2;
    h[1] = h[2] + d1 + e2;
    h[2] = h[3] + e1 + a2;
    h[3] = h[4] + a1 + b2;
    h[4] = h[0] + b1 + c2;
    h[0] = t;
}

void RIPEMD160::hash(const uint8_t* data, size_t len, uint8_t out[20]) {
    // Initialize state
    uint32_t h[5] = {
        0x67452301,
        0xEFCDAB89,
        0x98BADCFE,
        0x10325476,
        0xC3D2E1F0
    };
    
    // Process full blocks
    size_t i = 0;
    while (i + 64 <= len) {
        process_block(h, data + i);
        i += 64;
    }
    
    // Final block with padding
    uint8_t block[64];
    size_t remaining = len - i;
    memcpy(block, data + i, remaining);
    
    // Padding
    block[remaining] = 0x80;
    if (remaining < 56) {
        // Padding fits in this block
        memset(block + remaining + 1, 0, 55 - remaining);
        // Append length (bits) as little endian 64-bit
        uint64_t bit_len = len * 8;
        for (int j = 0; j < 8; j++) {
            block[56 + j] = (bit_len >> (j * 8)) & 0xFF;
        }
        process_block(h, block);
    } else {
        // Need another block
        memset(block + remaining + 1, 0, 63 - remaining);
        process_block(h, block);
        
        // Second block with length
        memset(block, 0, 56);
        uint64_t bit_len = len * 8;
        for (int j = 0; j < 8; j++) {
            block[56 + j] = (bit_len >> (j * 8)) & 0xFF;
        }
        process_block(h, block);
    }
    
    // Output (little endian)
    for (int i = 0; i < 5; i++) {
        out[i * 4] = h[i] & 0xFF;
        out[i * 4 + 1] = (h[i] >> 8) & 0xFF;
        out[i * 4 + 2] = (h[i] >> 16) & 0xFF;
        out[i * 4 + 3] = (h[i] >> 24) & 0xFF;
    }
}

} // namespace Core
} // namespace SeedSigner
