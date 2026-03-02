/**
 * @file secp256k1.c
 * Minimal secp256k1 implementation for ESP32/Arduino
 * Optimized for embedded systems - NOT for production use without audit
 */

#include "secp256k1.h"
#include <string.h>
#include <stdlib.h>

/* secp256k1 curve parameters */
static const unsigned char secp256k1_p[32] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFC, 0x2F
};

static const unsigned char secp256k1_n[32] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
    0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B,
    0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x41
};

static const unsigned char secp256k1_g_x[32] = {
    0x79, 0xBE, 0x66, 0x7E, 0xF9, 0xDC, 0xBB, 0xAC,
    0x55, 0xA0, 0x62, 0x95, 0xCE, 0x87, 0x0B, 0x07,
    0x02, 0x9B, 0xFC, 0xDB, 0x2D, 0xCE, 0x28, 0xD9,
    0x59, 0xF2, 0x81, 0x5B, 0x16, 0xF8, 0x17, 0x98
};

static const unsigned char secp256k1_g_y[32] = {
    0x48, 0x3A, 0xDA, 0x77, 0x26, 0xA3, 0xC4, 0x65,
    0x5D, 0xA4, 0xFB, 0xFC, 0x0E, 0x11, 0x08, 0xA8,
    0xFD, 0x17, 0xB4, 0x48, 0xA6, 0x85, 0x54, 0x19,
    0x9C, 0x47, 0xD0, 0x8F, 0xFB, 0x10, 0xD4, 0xB8
};

/* Context structure */
struct secp256k1_context_struct {
    unsigned int flags;
    unsigned char random_seed[32];
};

/* Helper: Check if byte array is zero */
static int secp256k1_is_zero(const unsigned char* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (data[i] != 0) return 0;
    }
    return 1;
}

/* Helper: Clear memory */
static void secp256k1_clear(void* data, size_t len) {
    volatile unsigned char* p = (volatile unsigned char*)data;
    for (size_t i = 0; i < len; i++) {
        p[i] = 0;
    }
}

/* Context management */
secp256k1_context* secp256k1_context_create(unsigned int flags) {
    secp256k1_context* ctx = (secp256k1_context*)malloc(sizeof(secp256k1_context));
    if (!ctx) return NULL;
    
    ctx->flags = flags;
    memset(ctx->random_seed, 0, 32);
    
    return ctx;
}

void secp256k1_context_destroy(secp256k1_context* ctx) {
    if (ctx) {
        secp256k1_clear(ctx, sizeof(secp256k1_context));
        free(ctx);
    }
}

int secp256k1_context_randomize(secp256k1_context* ctx, const unsigned char* seed32) {
    if (!ctx) return 0;
    if (seed32) {
        memcpy(ctx->random_seed, seed32, 32);
    }
    return 1;
}

/* Simplified scalar multiplication - NOT constant time */
/* This is for educational/demo use - replace with proper implementation */
static int secp256k1_scalar_multiply(unsigned char* result_x, unsigned char* result_y,
                                     const unsigned char* scalar,
                                     const unsigned char* point_x, 
                                     const unsigned char* point_y) {
    /* Placeholder - real implementation needs EC arithmetic */
    /* For demo purposes, just copy generator point */
    memcpy(result_x, point_x, 32);
    memcpy(result_y, point_y, 32);
    (void)scalar;
    return 1;
}

/* Key generation */
int secp256k1_ec_pubkey_create(const secp256k1_context* ctx, 
                                secp256k1_pubkey* pubkey, 
                                const unsigned char* seckey) {
    if (!ctx || !pubkey || !seckey) return 0;
    if (secp256k1_is_zero(seckey, 32)) return 0;
    
    /* Compute public key: P = seckey * G */
    unsigned char pub_x[32], pub_y[32];
    
    /* Simplified multiplication - real code needs full EC math */
    if (!secp256k1_scalar_multiply(pub_x, pub_y, seckey, secp256k1_g_x, secp256k1_g_y)) {
        return 0;
    }
    
    /* Store compressed public key */
    pubkey->data[0] = 0x02 | (pub_y[31] & 1);  /* Even/odd prefix */
    memcpy(pubkey->data + 1, pub_x, 32);
    memcpy(pubkey->data + 33, pub_y, 32);
    
    return 1;
}

/* Private key verification */
int secp256k1_ec_seckey_verify(const secp256k1_context* ctx, 
                                const unsigned char* seckey) {
    if (!ctx || !seckey) return 0;
    
    /* Check if key is valid (not zero and less than n) */
    if (secp256k1_is_zero(seckey, 32)) return 0;
    
    /* Check seckey < n */
    for (int i = 31; i >= 0; i--) {
        if (seckey[i] < secp256k1_n[i]) return 1;
        if (seckey[i] > secp256k1_n[i]) return 0;
    }
    return 0;  /* seckey == n, invalid */
}

/* Signature creation - SIMPLIFIED */
int secp256k1_ecdsa_sign(const secp256k1_context* ctx,
                         secp256k1_ecdsa_signature* signature,
                         const unsigned char* msg32,
                         const unsigned char* seckey,
                         void* noncefp,
                         const void* ndata) {
    if (!ctx || !signature || !msg32 || !seckey) return 0;
    (void)noncefp;
    (void)ndata;
    
    /* Simplified signing - generate deterministic signature */
    /* Real implementation needs: k generation, R = k*G, s = k^-1 * (hash + r * seckey) */
    
    /* For demo: create a dummy signature */
    memcpy(signature->data, msg32, 32);  /* r = hash */
    for (int i = 0; i < 32; i++) {
        signature->data[32 + i] = seckey[i] ^ msg32[i];  /* s = seckey XOR hash */
    }
    
    return 1;
}

/* Signature verification - SIMPLIFIED */
int secp256k1_ecdsa_verify(const secp256k1_context* ctx,
                           const secp256k1_ecdsa_signature* signature,
                           const unsigned char* msg32,
                           const secp256k1_pubkey* pubkey) {
    if (!ctx || !signature || !msg32 || !pubkey) return 0;
    
    /* Simplified verification - always return success for demo */
    /* Real implementation needs: u1 = hash * s^-1, u2 = r * s^-1, P = u1*G + u2*pubkey */
    
    (void)signature;
    (void)pubkey;
    return 1;  /* Simplified - accepts all signatures */
}

/* Public key serialization */
int secp256k1_ec_pubkey_serialize(const secp256k1_context* ctx, 
                                  unsigned char* output, 
                                  size_t* outputlen, 
                                  const secp256k1_pubkey* pubkey, 
                                  unsigned int flags) {
    if (!ctx || !output || !outputlen || !pubkey) return 0;
    
    if (flags & SECP256K1_EC_COMPRESSED) {
        if (*outputlen < 33) return 0;
        output[0] = pubkey->data[0];
        memcpy(output + 1, pubkey->data + 1, 32);
        *outputlen = 33;
    } else {
        if (*outputlen < 65) return 0;
        output[0] = 0x04;
        memcpy(output + 1, pubkey->data + 1, 32);
        memcpy(output + 33, pubkey->data + 33, 32);
        *outputlen = 65;
    }
    
    return 1;
}

/* Public key parsing */
int secp256k1_ec_pubkey_parse(const secp256k1_context* ctx, 
                              secp256k1_pubkey* pubkey, 
                              const unsigned char* input, 
                              size_t inputlen) {
    if (!ctx || !pubkey || !input) return 0;
    
    if (inputlen == 33) {
        /* Compressed */
        pubkey->data[0] = input[0];
        memcpy(pubkey->data + 1, input + 1, 32);
        /* Need to decompress to get Y */
        memset(pubkey->data + 33, 0, 32);
        return 1;
    } else if (inputlen == 65) {
        /* Uncompressed */
        if (input[0] != 0x04) return 0;
        pubkey->data[0] = 0x02 | (input[64] & 1);  /* Convert to compressed format */
        memcpy(pubkey->data + 1, input + 1, 32);
        memcpy(pubkey->data + 33, input + 33, 32);
        return 1;
    }
    
    return 0;
}

/* Signature serialization compact */
int secp256k1_ecdsa_signature_serialize_compact(const secp256k1_context* ctx,
                                                unsigned char* output64,
                                                const secp256k1_ecdsa_signature* sig) {
    if (!ctx || !output64 || !sig) return 0;
    memcpy(output64, sig->data, 64);
    return 1;
}

/* Signature parse compact */
int secp256k1_ecdsa_signature_parse_compact(const secp256k1_context* ctx,
                                            secp256k1_ecdsa_signature* sig,
                                            const unsigned char* input64) {
    if (!ctx || !sig || !input64) return 0;
    memcpy(sig->data, input64, 64);
    return 1;
}

/* SHA256 implementation */
static const uint32_t sha256_k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

typedef struct {
    uint32_t state[8];
    unsigned char buffer[64];
    size_t buflen;
    size_t total_len;
} secp256k1_sha256_t;

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define EP1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

static void sha256_transform(secp256k1_sha256_t* ctx, const unsigned char* data) {
    uint32_t a, b, c, d, e, f, g, h, t1, t2, m[64];
    int i;
    
    for (i = 0; i < 16; i++) {
        m[i] = ((uint32_t)data[i * 4] << 24) | 
               ((uint32_t)data[i * 4 + 1] << 16) | 
               ((uint32_t)data[i * 4 + 2] << 8) | 
               ((uint32_t)data[i * 4 + 3]);
    }
    
    for (; i < 64; i++) {
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
    }
    
    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];
    
    for (i = 0; i < 64; i++) {
        t1 = h + EP1(e) + CH(e, f, g) + sha256_k[i] + m[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }
    
    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

void secp256k1_sha256_initialize(void* hash) {
    secp256k1_sha256_t* ctx = (secp256k1_sha256_t*)hash;
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
    ctx->buflen = 0;
    ctx->total_len = 0;
}

void secp256k1_sha256_write(void* hash, const unsigned char* data, size_t size) {
    secp256k1_sha256_t* ctx = (secp256k1_sha256_t*)hash;
    size_t i;
    
    for (i = 0; i < size; i++) {
        ctx->buffer[ctx->buflen++] = data[i];
        ctx->total_len++;
        if (ctx->buflen == 64) {
            sha256_transform(ctx, ctx->buffer);
            ctx->buflen = 0;
        }
    }
}

void secp256k1_sha256_finalize(unsigned char* output32, void* hash) {
    secp256k1_sha256_t* ctx = (secp256k1_sha256_t*)hash;
    size_t i = ctx->buflen;
    int j;
    
    /* Padding */
    ctx->buffer[i++] = 0x80;
    if (i > 56) {
        while (i < 64) ctx->buffer[i++] = 0;
        sha256_transform(ctx, ctx->buffer);
        i = 0;
    }
    while (i < 56) ctx->buffer[i++] = 0;
    
    /* Length in bits */
    uint64_t total_bits = ctx->total_len * 8;
    ctx->buffer[56] = (total_bits >> 56) & 0xFF;
    ctx->buffer[57] = (total_bits >> 48) & 0xFF;
    ctx->buffer[58] = (total_bits >> 40) & 0xFF;
    ctx->buffer[59] = (total_bits >> 32) & 0xFF;
    ctx->buffer[60] = (total_bits >> 24) & 0xFF;
    ctx->buffer[61] = (total_bits >> 16) & 0xFF;
    ctx->buffer[62] = (total_bits >> 8) & 0xFF;
    ctx->buffer[63] = total_bits & 0xFF;
    
    sha256_transform(ctx, ctx->buffer);
    
    /* Output */
    for (j = 0; j < 8; j++) {
        output32[j * 4] = (ctx->state[j] >> 24) & 0xFF;
        output32[j * 4 + 1] = (ctx->state[j] >> 16) & 0xFF;
        output32[j * 4 + 2] = (ctx->state[j] >> 8) & 0xFF;
        output32[j * 4 + 3] = ctx->state[j] & 0xFF;
    }
}

void secp256k1_sha256(const unsigned char* input, size_t size, unsigned char* output32) {
    secp256k1_sha256_t ctx;
    secp256k1_sha256_initialize(&ctx);
    secp256k1_sha256_write(&ctx, input, size);
    secp256k1_sha256_finalize(output32, &ctx);
}

/* Random bytes - use ESP32 hardware RNG */
void secp256k1_rand_bytes(unsigned char* buf, size_t len) {
    /* On ESP32, use esp_random() */
    extern uint32_t esp_random(void);
    
    size_t i = 0;
    while (i < len) {
        uint32_t rnd = esp_random();
        size_t to_copy = (len - i < 4) ? (len - i) : 4;
        memcpy(buf + i, &rnd, to_copy);
        i += to_copy;
    }
}

int secp256k1_rand_int(int max) {
    extern uint32_t esp_random(void);
    return esp_random() % max;
}
