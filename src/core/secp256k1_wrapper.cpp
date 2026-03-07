/**
 * @file secp256k1_wrapper.cpp
 * C++ wrapper implementation
 */

#include "core/secp256k1_wrapper.h"
#include "core/ripemd160.h"
#include "utils/memory.h"

namespace SeedSigner {
namespace Core {

Secp256k1::Secp256k1() : m_ctx(nullptr), m_initialized(false) {}

Secp256k1::~Secp256k1() {
    deinit();
}

bool Secp256k1::init() {
    if (m_initialized) return true;
    
    m_ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    if (!m_ctx) return false;
    
    // Randomize context for side-channel resistance
    uint8_t seed[32];
    secp256k1_rand_bytes(seed, 32);
    secp256k1_context_randomize(m_ctx, seed);
    Utils::SecureMemory::wipe(seed, 32);
    
    m_initialized = true;
    return true;
}

void Secp256k1::deinit() {
    if (m_ctx) {
        secp256k1_context_destroy(m_ctx);
        m_ctx = nullptr;
    }
    m_initialized = false;
}

bool Secp256k1::generate_public_key(const uint8_t private_key[32], 
                                    uint8_t public_key[33],
                                    bool compressed) {
    if (!m_initialized || !private_key || !public_key) return false;
    
    secp256k1_pubkey pubkey;
    if (!secp256k1_ec_pubkey_create(m_ctx, &pubkey, private_key)) {
        return false;
    }
    
    size_t len = compressed ? 33 : 65;
    unsigned int flags = compressed ? SECP256K1_EC_COMPRESSED : SECP256K1_EC_UNCOMPRESSED;
    
    if (!secp256k1_ec_pubkey_serialize(m_ctx, public_key, &len, &pubkey, flags)) {
        return false;
    }
    
    return true;
}

bool Secp256k1::sign(const uint8_t private_key[32],
                     const uint8_t hash[32],
                     uint8_t signature[64],
                     uint8_t* recid) {
    if (!m_initialized || !private_key || !hash || !signature) return false;
    
    secp256k1_ecdsa_signature sig;
    uint8_t recid_local = 0;
    
    if (recid) {
        // Use recoverable signature mode
        if (!secp256k1_ecdsa_sign_recoverable(m_ctx, &sig, &recid_local, hash, private_key, nullptr)) {
            return false;
        }
        *recid = recid_local;
    } else {
        if (!secp256k1_ecdsa_sign(m_ctx, &sig, hash, private_key, nullptr)) {
            return false;
        }
    }
    
    if (!secp256k1_ecdsa_signature_serialize_compact(m_ctx, signature, &sig)) {
        return false;
    }
    
    return true;
}

bool Secp256k1::verify(const uint8_t public_key[33],
                       const uint8_t hash[32],
                       const uint8_t signature[64]) {
    if (!m_initialized || !public_key || !hash || !signature) return false;
    
    secp256k1_pubkey pubkey;
    if (!secp256k1_ec_pubkey_parse(m_ctx, &pubkey, public_key, 33)) {
        return false;
    }
    
    secp256k1_ecdsa_signature sig;
    if (!secp256k1_ecdsa_signature_parse_compact(m_ctx, &sig, signature)) {
        return false;
    }
    
    return secp256k1_ecdsa_verify(m_ctx, &sig, hash, &pubkey) == 1;
}

bool Secp256k1::point_add(const uint8_t point_a[33], 
                          const uint8_t point_b[33],
                          uint8_t result[33]) {
    // Simplified - real implementation needs full EC point addition
    (void)point_a;
    (void)point_b;
    (void)result;
    return false;
}

bool Secp256k1::scalar_multiply(const uint8_t scalar[32],
                                const uint8_t point[33],
                                uint8_t result[33]) {
    // Simplified - real implementation needs EC scalar multiplication
    (void)scalar;
    (void)point;
    (void)result;
    return false;
}

bool Secp256k1::seckey_verify(const uint8_t seckey[32]) {
    if (!seckey) return false;
    return secp256k1_ec_seckey_verify(seckey) == 1;
}

bool Secp256k1::seckey_tweak_add(uint8_t seckey[32], const uint8_t tweak[32]) {
    if (!seckey || !tweak) return false;
    return secp256k1_ec_privkey_tweak_add(seckey, tweak) == 1;
}

bool Secp256k1::seckey_tweak_mul(uint8_t seckey[32], const uint8_t tweak[32]) {
    if (!seckey || !tweak) return false;
    return secp256k1_ec_privkey_tweak_mul(seckey, tweak) == 1;
}

void Secp256k1::sha256(const uint8_t* data, size_t len, uint8_t hash[32]) {
    if (!data || !hash) return;
    secp256k1_sha256(data, len, hash);
}

void Secp256k1::hash256(const uint8_t* data, size_t len, uint8_t hash[32]) {
    if (!data || !hash) return;
    // Double SHA256
    uint8_t temp[32];
    secp256k1_sha256(data, len, temp);
    secp256k1_sha256(temp, 32, hash);
}

void Secp256k1::ripemd160(const uint8_t* data, size_t len, uint8_t hash[20]) {
    RIPEMD160::hash(data, len, hash);
}

void Secp256k1::hash160(const uint8_t* data, size_t len, uint8_t hash[20]) {
    uint8_t sha256_hash[32];
    sha256(data, len, sha256_hash);
    ripemd160(sha256_hash, 32, hash);
}

void Secp256k1::random_bytes(uint8_t* buf, size_t len) {
    if (!buf) return;
    secp256k1_rand_bytes(buf, len);
}

} // namespace Core
} // namespace SeedSigner
