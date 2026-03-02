/**
 * @file secp256k1.h
 * Minimal secp256k1 implementation for ESP32/Arduino
 * Based on bitcoin-core/secp256k1, optimized for embedded
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

/* Context flags */
#define SECP256K1_CONTEXT_VERIFY (1 << 0)
#define SECP256K1_CONTEXT_SIGN   (1 << 1)
#define SECP256K1_CONTEXT_NONE   0

/* Opaque context */
typedef struct secp256k1_context_struct secp256k1_context;

/* Public key structure */
typedef struct {
    unsigned char data[64];
} secp256k1_pubkey;

/* Signature structure */
typedef struct {
    unsigned char data[64];
} secp256k1_ecdsa_signature;

/* Context management */
secp256k1_context* secp256k1_context_create(unsigned int flags);
void secp256k1_context_destroy(secp256k1_context* ctx);
int secp256k1_context_randomize(secp256k1_context* ctx, const unsigned char* seed32);

/* Key generation */
int secp256k1_ec_pubkey_create(const secp256k1_context* ctx, 
                                secp256k1_pubkey* pubkey, 
                                const unsigned char* seckey);

/* Private key operations */
int secp256k1_ec_seckey_verify(const secp256k1_context* ctx, 
                                const unsigned char* seckey);
int secp256k1_ec_privkey_tweak_add(unsigned char* seckey, 
                                   const unsigned char* tweak);
int secp256k1_ec_privkey_tweak_mul(unsigned char* seckey, 
                                   const unsigned char* tweak);

/* Public key operations */
int secp256k1_ec_pubkey_parse(const secp256k1_context* ctx, 
                              secp256k1_pubkey* pubkey, 
                              const unsigned char* input, 
                              size_t inputlen);
int secp256k1_ec_pubkey_serialize(const secp256k1_context* ctx, 
                                  unsigned char* output, 
                                  size_t* outputlen, 
                                  const secp256k1_pubkey* pubkey, 
                                  unsigned int flags);
int secp256k1_ec_pubkey_tweak_add(const secp256k1_context* ctx,
                                  secp256k1_pubkey* pubkey,
                                  const unsigned char* tweak);
int secp256k1_ec_pubkey_tweak_mul(const secp256k1_context* ctx,
                                  secp256k1_pubkey* pubkey,
                                  const unsigned char* tweak);

/* ECDSA Sign/Verify */
int secp256k1_ecdsa_sign(const secp256k1_context* ctx,
                         secp256k1_ecdsa_signature* signature,
                         const unsigned char* msg32,
                         const unsigned char* seckey,
                         void* noncefp,
                         const void* ndata);
int secp256k1_ecdsa_verify(const secp256k1_context* ctx,
                           const secp256k1_ecdsa_signature* signature,
                           const unsigned char* msg32,
                           const secp256k1_pubkey* pubkey);

/* Signature serialization */
int secp256k1_ecdsa_signature_parse_der(const secp256k1_context* ctx,
                                        secp256k1_ecdsa_signature* sig,
                                        const unsigned char* input,
                                        size_t inputlen);
int secp256k1_ecdsa_signature_parse_compact(const secp256k1_context* ctx,
                                            secp256k1_ecdsa_signature* sig,
                                            const unsigned char* input64);
int secp256k1_ecdsa_signature_serialize_der(const secp256k1_context* ctx,
                                            unsigned char* output,
                                            size_t* outputlen,
                                            const secp256k1_ecdsa_signature* sig);
int secp256k1_ecdsa_signature_serialize_compact(const secp256k1_context* ctx,
                                                unsigned char* output64,
                                                const secp256k1_ecdsa_signature* sig);
int secp256k1_ecdsa_signature_normalize(const secp256k1_context* ctx,
                                        secp256k1_ecdsa_signature* sigout,
                                        const secp256k1_ecdsa_signature* sigin);

/* Recovery ID */
int secp256k1_ecdsa_recoverable_signature_parse_compact(const secp256k1_context* ctx,
                                                        secp256k1_ecdsa_signature* sig,
                                                        const unsigned char* input64,
                                                        int recid);
int secp256k1_ecdsa_recoverable_signature_serialize_compact(const secp256k1_context* ctx,
                                                            unsigned char* output64,
                                                            int* recid,
                                                            const secp256k1_ecdsa_signature* sig);
int secp256k1_ecdsa_sign_recoverable(const secp256k1_context* ctx,
                                     secp256k1_ecdsa_signature* signature,
                                     const unsigned char* msg32,
                                     const unsigned char* seckey,
                                     void* noncefp,
                                     const void* ndata);
int secp256k1_ecdsa_recover(const secp256k1_context* ctx,
                            secp256k1_pubkey* pubkey,
                            const secp256k1_ecdsa_signature* signature,
                            const unsigned char* msg32);

/* Serialization flags */
#define SECP256K1_EC_COMPRESSED 1
#define SECP256K1_EC_UNCOMPRESSED 2

/* Hash functions */
void secp256k1_sha256_initialize(void* hash);
void secp256k1_sha256_write(void* hash, const unsigned char* data, size_t size);
void secp256k1_sha256_finalize(unsigned char* output32, void* hash);
void secp256k1_sha256(const unsigned char* input, size_t size, unsigned char* output32);

/* HMAC-SHA256 */
void secp256k1_hmac_sha256_initialize(void* hash, 
                                      const unsigned char* key, size_t keylen);
void secp256k1_hmac_sha256_write(void* hash, const unsigned char* data, size_t size);
void secp256k1_hmac_sha256_finalize(unsigned char* output32, void* hash);

/* Utility */
void secp256k1_rand_bytes(unsigned char* buf, size_t len);
int secp256k1_rand_int(int max);

#ifdef __cplusplus
}
#endif

#endif /* SECP256K1_H */
