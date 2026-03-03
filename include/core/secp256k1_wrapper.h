/**
 * @file secp256k1_wrapper.h
 * C++ wrapper for secp256k1 library
 */

#ifndef CORE_SECP256K1_WRAPPER_H
#define CORE_SECP256K1_WRAPPER_H

#include <Arduino.h>
#include <stdint.h>
extern "C" {
#include "secp256k1.h"
}

namespace SeedSigner {
namespace Core {

class Secp256k1 {
public:
    Secp256k1();
    ~Secp256k1();
    
    bool init();
    void deinit();
    
    // Key generation
    bool generate_public_key(const uint8_t private_key[32], 
                             uint8_t public_key[33], 
                             bool compressed = true);
    
    // Signing
    bool sign(const uint8_t private_key[32],
              const uint8_t hash[32],
              uint8_t signature[64],
              uint8_t* recid = nullptr);
    
    // Verification
    bool verify(const uint8_t public_key[33],
                const uint8_t hash[32],
                const uint8_t signature[64]);
    
    // Point operations
    bool point_add(const uint8_t point_a[33], 
                   const uint8_t point_b[33],
                   uint8_t result[33]);
    bool scalar_multiply(const uint8_t scalar[32],
                         const uint8_t point[33],
                         uint8_t result[33]);
    
    // Seckey operations
    bool seckey_verify(const uint8_t seckey[32]);
    bool seckey_tweak_add(uint8_t seckey[32], const uint8_t tweak[32]);
    bool seckey_tweak_mul(uint8_t seckey[32], const uint8_t tweak[32]);
    
    // Hashing
    static void sha256(const uint8_t* data, size_t len, uint8_t hash[32]);
    static void hash256(const uint8_t* data, size_t len, uint8_t hash[32]);
    static void ripemd160(const uint8_t* data, size_t len, uint8_t hash[20]);
    
    // HASH160 = SHA256 + RIPEMD160 (used for Bitcoin addresses)
    static void hash160(const uint8_t* data, size_t len, uint8_t hash[20]);
    
    // Utility
    static void random_bytes(uint8_t* buf, size_t len);
    
private:
    secp256k1_context* m_ctx;
    bool m_initialized;
};

} // namespace Core
} // namespace SeedSigner

#endif // CORE_SECP256K1_WRAPPER_H
