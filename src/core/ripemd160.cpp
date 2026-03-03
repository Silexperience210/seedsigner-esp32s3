#include <stdint.h>
#include <string.h>
#include <stddef.h>

// RIPEMD160 implementation pour Bitcoin
// Standard ISO/IEC 10118-3:2004

#define F(x, y, z) ((x) ^ (y) ^ (z))
#define G(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define H(x, y, z) (((x) | ~(y)) ^ (z))
#define I(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define J(x, y, z) ((x) ^ ((y) | ~(z)))

#define ROL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

// Round functions
#define R(a, b, c, d, e, f, k, r, s) \
    a = ROL(a + f + k + x[r], s) + e; \
    c = ROL(c, 10);

#define R1(a, b, c, d, e, r, s) R(a, b, c, d, e, F(b, c, d), 0, r, s)
#define R2(a, b, c, d, e, r, s) R(a, b, c, d, e, G(b, c, d), 0x5A827999, r, s)
#define R3(a, b, c, d, e, r, s) R(a, b, c, d, e, H(b, c, d), 0x6ED9EBA1, r, s)
#define R4(a, b, c, d, e, r, s) R(a, b, c, d, e, I(b, c, d), 0x8F1BBCDC, r, s)
#define R5(a, b, c, d, e, r, s) R(a, b, c, d, e, J(b, c, d), 0xA953FD4E, r, s)

#define RR1(a, b, c, d, e, r, s) R(a, b, c, d, e, F(b, c, d), 0x50A28BE6, r, s)
#define RR2(a, b, c, d, e, r, s) R(a, b, c, d, e, G(b, c, d), 0x5C4DD124, r, s)
#define RR3(a, b, c, d, e, r, s) R(a, b, c, d, e, H(b, c, d), 0x6D703EF3, r, s)
#define RR4(a, b, c, d, e, r, s) R(a, b, c, d, e, I(b, c, d), 0x7A6D76E9, r, s)
#define RR5(a, b, c, d, e, r, s) R(a, b, c, d, e, J(b, c, d), 0, r, s)

static void ripemd160_transform(uint32_t state[5], const uint8_t block[64]) {
    uint32_t a1 = state[0], b1 = state[1], c1 = state[2], d1 = state[3], e1 = state[4];
    uint32_t a2 = state[0], b2 = state[1], c2 = state[2], d2 = state[3], e2 = state[4];
    
    uint32_t x[16];
    for (int i = 0; i < 16; i++) {
        x[i] = ((uint32_t)block[i*4]) | 
               ((uint32_t)block[i*4+1] << 8) | 
               ((uint32_t)block[i*4+2] << 16) | 
               ((uint32_t)block[i*4+3] << 24);
    }
    
    // Left line
    R1(a1, b1, c1, d1, e1, 0, 11);
    R1(e1, a1, b1, c1, d1, 1, 14);
    R1(d1, e1, a1, b1, c1, 2, 15);
    R1(c1, d1, e1, a1, b1, 3, 12);
    R1(b1, c1, d1, e1, a1, 4, 5);
    R1(a1, b1, c1, d1, e1, 5, 8);
    R1(e1, a1, b1, c1, d1, 6, 7);
    R1(d1, e1, a1, b1, c1, 7, 9);
    R1(c1, d1, e1, a1, b1, 8, 11);
    R1(b1, c1, d1, e1, a1, 9, 13);
    R1(a1, b1, c1, d1, e1, 10, 14);
    R1(e1, a1, b1, c1, d1, 11, 15);
    R1(d1, e1, a1, b1, c1, 12, 6);
    R1(c1, d1, e1, a1, b1, 13, 7);
    R1(b1, c1, d1, e1, a1, 14, 9);
    R1(a1, b1, c1, d1, e1, 15, 8);
    
    R2(e1, a1, b1, c1, d1, 7, 7);
    R2(d1, e1, a1, b1, c1, 4, 6);
    R2(c1, d1, e1, a1, b1, 13, 8);
    R2(b1, c1, d1, e1, a1, 1, 13);
    R2(a1, b1, c1, d1, e1, 10, 11);
    R2(e1, a1, b1, c1, d1, 6, 9);
    R2(d1, e1, a1, b1, c1, 15, 7);
    R2(c1, d1, e1, a1, b1, 3, 15);
    R2(b1, c1, d1, e1, a1, 12, 7);
    R2(a1, b1, c1, d1, e1, 0, 12);
    R2(e1, a1, b1, c1, d1, 9, 15);
    R2(d1, e1, a1, b1, c1, 5, 9);
    R2(c1, d1, e1, a1, b1, 2, 11);
    R2(b1, c1, d1, e1, a1, 14, 7);
    R2(a1, b1, c1, d1, e1, 11, 13);
    R2(e1, a1, b1, c1, d1, 8, 12);
    
    R3(d1, e1, a1, b1, c1, 3, 11);
    R3(c1, d1, e1, a1, b1, 10, 13);
    R3(b1, c1, d1, e1, a1, 14, 6);
    R3(a1, b1, c1, d1, e1, 4, 7);
    R3(e1, a1, b1, c1, d1, 9, 14);
    R3(d1, e1, a1, b1, c1, 15, 9);
    R3(c1, d1, e1, a1, b1, 8, 13);
    R3(b1, c1, d1, e1, a1, 1, 15);
    R3(a1, b1, c1, d1, e1, 2, 14);
    R3(e1, a1, b1, c1, d1, 7, 8);
    R3(d1, e1, a1, b1, c1, 0, 13);
    R3(c1, d1, e1, a1, b1, 6, 6);
    R3(b1, c1, d1, e1, a1, 13, 5);
    R3(a1, b1, c1, d1, e1, 11, 12);
    R3(e1, a1, b1, c1, d1, 5, 7);
    R3(d1, e1, a1, b1, c1, 12, 5);
    
    R4(c1, d1, e1, a1, b1, 1, 11);
    R4(b1, c1, d1, e1, a1, 9, 12);
    R4(a1, b1, c1, d1, e1, 11, 14);
    R4(e1, a1, b1, c1, d1, 10, 15);
    R4(d1, e1, a1, b1, c1, 0, 14);
    R4(c1, d1, e1, a1, b1, 8, 15);
    R4(b1, c1, d1, e1, a1, 12, 9);
    R4(a1, b1, c1, d1, e1, 4, 8);
    R4(e1, a1, b1, c1, d1, 13, 9);
    R4(d1, e1, a1, b1, c1, 3, 14);
    R4(c1, d1, e1, a1, b1, 7, 5);
    R4(b1, c1, d1, e1, a1, 15, 6);
    R4(a1, b1, c1, d1, e1, 14, 8);
    R4(e1, a1, b1, c1, d1, 5, 6);
    R4(d1, e1, a1, b1, c1, 6, 5);
    R4(c1, d1, e1, a1, b1, 2, 12);
    
    R5(b1, c1, d1, e1, a1, 4, 9);
    R5(a1, b1, c1, d1, e1, 0, 15);
    R5(e1, a1, b1, c1, d1, 5, 5);
    R5(d1, e1, a1, b1, c1, 9, 11);
    R5(c1, d1, e1, a1, b1, 7, 6);
    R5(b1, c1, d1, e1, a1, 12, 8);
    R5(a1, b1, c1, d1, e1, 2, 13);
    R5(e1, a1, b1, c1, d1, 10, 12);
    R5(d1, e1, a1, b1, c1, 14, 5);
    R5(c1, d1, e1, a1, b1, 1, 12);
    R5(b1, c1, d1, e1, a1, 3, 13);
    R5(a1, b1, c1, d1, e1, 8, 14);
    R5(e1, a1, b1, c1, d1, 11, 11);
    R5(d1, e1, a1, b1, c1, 6, 8);
    R5(c1, d1, e1, a1, b1, 15, 5);
    R5(b1, c1, d1, e1, a1, 13, 6);
    
    // Right line
    RR1(a2, b2, c2, d2, e2, 5, 8);
    RR1(e2, a2, b2, c2, d2, 14, 9);
    RR1(d2, e2, a2, b2, c2, 7, 9);
    RR1(c2, d2, e2, a2, b2, 0, 11);
    RR1(b2, c2, d2, e2, a2, 9, 13);
    RR1(a2, b2, c2, d2, e2, 2, 15);
    RR1(e2, a2, b2, c2, d2, 11, 15);
    RR1(d2, e2, a2, b2, c2, 4, 5);
    RR1(c2, d2, e2, a2, b2, 13, 7);
    RR1(b2, c2, d2, e2, a2, 6, 7);
    RR1(a2, b2, c2, d2, e2, 15, 8);
    RR1(e2, a2, b2, c2, d2, 8, 11);
    RR1(d2, e2, a2, b2, c2, 1, 14);
    RR1(c2, d2, e2, a2, b2, 10, 14);
    RR1(b2, c2, d2, e2, a2, 3, 12);
    RR1(a2, b2, c2, d2, e2, 12, 6);
    
    RR2(e2, a2, b2, c2, d2, 6, 9);
    RR2(d2, e2, a2, b2, c2, 11, 13);
    RR2(c2, d2, e2, a2, b2, 3, 15);
    RR2(b2, c2, d2, e2, a2, 7, 7);
    RR2(a2, b2, c2, d2, e2, 0, 12);
    RR2(e2, a2, b2, c2, d2, 13, 8);
    RR2(d2, e2, a2, b2, c2, 5, 9);
    RR2(c2, d2, e2, a2, b2, 10, 11);
    RR2(b2, c2, d2, e2, a2, 14, 7);
    RR2(a2, b2, c2, d2, e2, 15, 7);
    RR2(e2, a2, b2, c2, d2, 8, 12);
    RR2(d2, e2, a2, b2, c2, 12, 7);
    RR2(c2, d2, e2, a2, b2, 4, 6);
    RR2(b2, c2, d2, e2, a2, 9, 15);
    RR2(a2, b2, c2, d2, e2, 1, 13);
    RR2(e2, a2, b2, c2, d2, 2, 11);
    
    RR3(d2, e2, a2, b2, c2, 15, 9);
    RR3(c2, d2, e2, a2, b2, 5, 7);
    RR3(b2, c2, d2, e2, a2, 1, 15);
    RR3(a2, b2, c2, d2, e2, 3, 11);
    RR3(e2, a2, b2, c2, d2, 7, 8);
    RR3(d2, e2, a2, b2, c2, 14, 6);
    RR3(c2, d2, e2, a2, b2, 6, 6);
    RR3(b2, c2, d2, e2, a2, 9, 14);
    RR3(a2, b2, c2, d2, e2, 11, 12);
    RR3(e2, a2, b2, c2, d2, 8, 13);
    RR3(d2, e2, a2, b2, c2, 12, 5);
    RR3(c2, d2, e2, a2, b2, 2, 14);
    RR3(b2, c2, d2, e2, a2, 10, 13);
    RR3(a2, b2, c2, d2, e2, 0, 13);
    RR3(e2, a2, b2, c2, d2, 4, 7);
    RR3(d2, e2, a2, b2, c2, 13, 5);
    
    RR4(c2, d2, e2, a2, b2, 8, 15);
    RR4(b2, c2, d2, e2, a2, 6, 5);
    RR4(a2, b2, c2, d2, e2, 4, 8);
    RR4(e2, a2, b2, c2, d2, 1, 11);
    RR4(d2, e2, a2, b2, c2, 3, 14);
    RR4(c2, d2, e2, a2, b2, 11, 14);
    RR4(b2, c2, d2, e2, a2, 15, 6);
    RR4(a2, b2, c2, d2, e2, 0, 14);
    RR4(e2, a2, b2, c2, d2, 5, 6);
    RR4(d2, e2, a2, b2, c2, 12, 9);
    RR4(c2, d2, e2, a2, b2, 2, 12);
    RR4(b2, c2, d2, e2, a2, 13, 9);
    RR4(a2, b2, c2, d2, e2, 9, 12);
    RR4(e2, a2, b2, c2, d2, 7, 5);
    RR4(d2, e2, a2, b2, c2, 10, 15);
    RR4(c2, d2, e2, a2, b2, 14, 8);
    
    RR5(b2, c2, d2, e2, a2, 12, 8);
    RR5(a2, b2, c2, d2, e2, 15, 5);
    RR5(e2, a2, b2, c2, d2, 10, 12);
    RR5(d2, e2, a2, b2, c2, 4, 9);
    RR5(c2, d2, e2, a2, b2, 1, 12);
    RR5(b2, c2, d2, e2, a2, 5, 5);
    RR5(a2, b2, c2, d2, e2, 8, 14);
    RR5(e2, a2, b2, c2, d2, 7, 6);
    RR5(d2, e2, a2, b2, c2, 6, 8);
    RR5(c2, d2, e2, a2, b2, 2, 13);
    RR5(b2, c2, d2, e2, a2, 13, 6);
    RR5(a2, b2, c2, d2, e2, 14, 5);
    RR5(e2, a2, b2, c2, d2, 0, 15);
    RR5(d2, e2, a2, b2, c2, 3, 13);
    RR5(c2, d2, e2, a2, b2, 9, 11);
    RR5(b2, c2, d2, e2, a2, 11, 11);
    
    // Combine
    uint32_t t = state[1] + c1 + d2;
    state[1] = state[2] + d1 + e2;
    state[2] = state[3] + e1 + a2;
    state[3] = state[4] + a1 + b2;
    state[4] = state[0] + b1 + c2;
    state[0] = t;
}

void ripemd160(const uint8_t* data, size_t len, uint8_t out[20]) {
    uint32_t state[5] = {
        0x67452301,
        0xEFCDAB89,
        0x98BADCFE,
        0x10325476,
        0xC3D2E1F0
    };
    
    // Padding
    size_t total_bits = len * 8;
    size_t pad_len = (len % 64 < 56) ? (56 - len % 64) : (120 - len % 64);
    size_t total_len = len + pad_len + 8;
    
    uint8_t* padded = new uint8_t[total_len];
    memcpy(padded, data, len);
    padded[len] = 0x80;
    for (size_t i = len + 1; i < len + pad_len; i++) {
        padded[i] = 0;
    }
    
    // Length in bits (little endian)
    for (int i = 0; i < 8; i++) {
        padded[len + pad_len + i] = (total_bits >> (i * 8)) & 0xFF;
    }
    
    // Process blocks
    for (size_t i = 0; i < total_len; i += 64) {
        ripemd160_transform(state, padded + i);
    }
    
    delete[] padded;
    
    // Output
    for (int i = 0; i < 5; i++) {
        out[i*4] = state[i] & 0xFF;
        out[i*4+1] = (state[i] >> 8) & 0xFF;
        out[i*4+2] = (state[i] >> 16) & 0xFF;
        out[i*4+3] = (state[i] >> 24) & 0xFF;
    }
}
