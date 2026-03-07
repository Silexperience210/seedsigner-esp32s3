/**
 * @file secp256k1.h
 * Production-grade secp256k1 implementation for ESP32-S3
 * Optimized for embedded systems with constant-time operations
 * 
 * SECURITY WARNING: This implementation uses constant-time algorithms
 * to prevent timing side-channel attacks.
 */

#ifndef SECP256K1_H
#define SECP256K1_H

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Constants */
#define SECP256K1_PRIVATE_KEY_SIZE 32
#define SECP256K1_PUBLIC_KEY_SIZE 33
#define SECP256K1_PUBLIC_KEY_UNCOMPRESSED_SIZE 65
#define SECP256K1_SIGNATURE_SIZE 64
#define SECP256K1_SIGNATURE_DER_MAX_SIZE 72
#define SECP256K1_CURVE_ORDER_SIZE 32

/* Context flags */
#define SECP256K1_CONTEXT_VERIFY (1 << 0)
#define SECP256K1_CONTEXT_SIGN   (1 << 1)
#define SECP256K1_CONTEXT_NONE   0

/* Error codes */
typedef enum {
    SECP256K1_OK = 0,
    SECP256K1_ERROR_INVALID_CONTEXT = -1,
    SECP256K1_ERROR_INVALID_PRIVATE_KEY = -2,
    SECP256K1_ERROR_INVALID_PUBLIC_KEY = -3,
    SECP256K1_ERROR_INVALID_SIGNATURE = -4,
    SECP256K1_ERROR_INVALID_HASH = -5,
    SECP256K1_ERROR_BUFFER_TOO_SMALL = -6,
    SECP256K1_ERROR_INTERNAL = -7
} secp256k1_error_t;

/* Opaque context */
typedef struct secp256k1_context_struct secp256k1_context;

/* Public key structure */
typedef struct {
    uint8_t data[64];  /* X and Y coordinates (32 bytes each) */
} secp256k1_pubkey;

/* Signature structure */
typedef struct {
    uint8_t data[64];  /* r (32 bytes) || s (32 bytes) */
} secp256k1_ecdsa_signature;

/* Scalar (private key or nonce) */
typedef struct {
    uint8_t data[32];
} secp256k1_scalar;

/* Context management */
secp256k1_context* secp256k1_context_create(unsigned int flags);
void secp256k1_context_destroy(secp256k1_context* ctx);
int secp256k1_context_randomize(secp256k1_context* ctx, const uint8_t seed32[32]);

/* Key generation and validation */
int secp256k1_ec_pubkey_create(const secp256k1_context* ctx, 
                                secp256k1_pubkey* pubkey, 
                                const uint8_t seckey[32]);

int secp256k1_ec_seckey_verify(const uint8_t seckey[32]);
int secp256k1_ec_pubkey_verify(const secp256k1_pubkey* pubkey);

/* Private key operations - constant time */
int secp256k1_ec_privkey_tweak_add(uint8_t seckey[32], const uint8_t tweak[32]);
int secp256k1_ec_privkey_tweak_mul(uint8_t seckey[32], const uint8_t tweak[32]);
int secp256k1_ec_privkey_negate(uint8_t seckey[32]);

/* Public key operations - constant time */
int secp256k1_ec_pubkey_tweak_add(const secp256k1_context* ctx,
                                  secp256k1_pubkey* pubkey,
                                  const uint8_t tweak[32]);
int secp256k1_ec_pubkey_tweak_mul(const secp256k1_context* ctx,
                                  secp256k1_pubkey* pubkey,
                                  const uint8_t tweak[32]);
int secp256k1_ec_pubkey_negate(const secp256k1_context* ctx,
                               secp256k1_pubkey* pubkey);

/* Serialization */
int secp256k1_ec_pubkey_serialize(const secp256k1_context* ctx, 
                                  uint8_t* output, 
                                  size_t* outputlen, 
                                  const secp256k1_pubkey* pubkey, 
                                  unsigned int flags);
int secp256k1_ec_pubkey_parse(const secp256k1_context* ctx, 
                              secp256k1_pubkey* pubkey, 
                              const uint8_t* input, 
                              size_t inputlen);

/* ECDSA Sign/Verify - constant time */
int secp256k1_ecdsa_sign(const secp256k1_context* ctx,
                         secp256k1_ecdsa_signature* signature,
                         const uint8_t msg32[32],
                         const uint8_t seckey[32],
                         const uint8_t* nonce32);

int secp256k1_ecdsa_verify(const secp256k1_context* ctx,
                           const secp256k1_ecdsa_signature* signature,
                           const uint8_t msg32[32],
                           const secp256k1_pubkey* pubkey);

/* Signature serialization */
int secp256k1_ecdsa_signature_parse_der(const secp256k1_context* ctx,
                                        secp256k1_ecdsa_signature* sig,
                                        const uint8_t* input,
                                        size_t inputlen);
int secp256k1_ecdsa_signature_serialize_der(const secp256k1_context* ctx,
                                            uint8_t* output,
                                            size_t* outputlen,
                                            const secp256k1_ecdsa_signature* sig);
int secp256k1_ecdsa_signature_parse_compact(const secp256k1_context* ctx,
                                            secp256k1_ecdsa_signature* sig,
                                            const uint8_t input64[64]);
int secp256k1_ecdsa_signature_serialize_compact(const secp256k1_context* ctx,
                                                uint8_t output64[64],
                                                const secp256k1_ecdsa_signature* sig);
int secp256k1_ecdsa_signature_normalize(const secp256k1_context* ctx,
                                        secp256k1_ecdsa_signature* sigout,
                                        const secp256k1_ecdsa_signature* sigin);

/* Recovery ID (for pubkey recovery from signature) */
int secp256k1_ecdsa_sign_recoverable(const secp256k1_context* ctx,
                                     secp256k1_ecdsa_signature* signature,
                                     uint8_t* recid,
                                     const uint8_t msg32[32],
                                     const uint8_t seckey[32],
                                     const uint8_t* nonce32);
int secp256k1_ecdsa_recover(const secp256k1_context* ctx,
                            secp256k1_pubkey* pubkey,
                            const secp256k1_ecdsa_signature* signature,
                            const uint8_t recid,
                            const uint8_t msg32[32]);

/* Serialization flags */
#define SECP256K1_EC_COMPRESSED 1
#define SECP256K1_EC_UNCOMPRESSED 2

/* Hash functions - SHA256 */
typedef struct {
    uint32_t state[8];
    uint64_t bitcount;
    uint8_t buffer[64];
    uint8_t bufferlen;
} secp256k1_sha256_t;

void secp256k1_sha256_initialize(secp256k1_sha256_t* hash);
void secp256k1_sha256_write(secp256k1_sha256_t* hash, const uint8_t* data, size_t size);
void secp256k1_sha256_finalize(uint8_t output32[32], secp256k1_sha256_t* hash);
void secp256k1_sha256(const uint8_t* input, size_t len, uint8_t output32[32]);

/* HMAC-SHA256 */
typedef struct {
    secp256k1_sha256_t inner;
    secp256k1_sha256_t outer;
} secp256k1_hmac_sha256_t;

void secp256k1_hmac_sha256_initialize(secp256k1_hmac_sha256_t* hash, 
                                      const uint8_t* key, size_t keylen);
void secp256k1_hmac_sha256_write(secp256k1_hmac_sha256_t* hash, 
                                 const uint8_t* data, size_t size);
void secp256k1_hmac_sha256_finalize(uint8_t output32[32], secp256k1_hmac_sha256_t* hash);

/* RFC 6979 deterministic nonce generation */
void secp256k1_rfc6979_hmac_sha256_generate_nonce(uint8_t nonce32[32],
                                                   const uint8_t key32[32],
                                                   const uint8_t msg32[32],
                                                   const uint8_t* extra,
                                                   size_t extralen);

/* Secure random bytes - uses ESP32 hardware RNG */
void secp256k1_rand_bytes(uint8_t* buf, size_t len);
int secp256k1_rand_int(int max);

/* Utility - constant time operations */
int secp256k1_memcmp_const(const void* s1, const void* s2, size_t n);
void secp256k1_memclear(void* ptr, size_t len);

/* Test vectors verification */
int secp256k1_self_test(void);

#ifdef __cplusplus
}
#endif

#endif /* SECP256K1_H */
