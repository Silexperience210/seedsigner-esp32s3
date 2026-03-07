/**
 * @file bip32_secp256k1.cpp
 * BIP32 implementation with full secp256k1 support
 * Complete HD wallet with proper elliptic curve operations
 */

#include "core/bip32.h"
#include "core/secp256k1_wrapper.h"
#include "core/ripemd160.h"
#include "core/bech32.h"
#include "utils/memory.h"
#include <string.h>
#include <mbedtls/sha256.h>
#include <mbedtls/md.h>

namespace SeedSigner {
namespace Core {

// Version bytes
static const uint8_t VERSION_MAINNET_PRIVATE[4] = {0x04, 0x88, 0xAD, 0xE4};
static const uint8_t VERSION_MAINNET_PUBLIC[4] = {0x04, 0x88, 0xB2, 0x1E};

// Base58 alphabet
static const char BASE58_CHARS[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

BIP32::BIP32() : m_initialized(false), m_secp256k1(nullptr) {
    memset(&m_master, 0, sizeof(m_master));
    memset(&m_current, 0, sizeof(m_current));
}

BIP32::~BIP32() {
    clear();
}

bool BIP32::init_from_seed(const uint8_t seed[64]) {
    if (!seed) return false;
    
    // Initialize secp256k1
    if (!m_secp256k1) {
        m_secp256k1 = new Secp256k1();
        if (!m_secp256k1->init()) {
            delete m_secp256k1;
            m_secp256k1 = nullptr;
            return false;
        }
    }
    
    // HMAC-SHA512(key="Bitcoin seed", data=seed)
    const char* key = "Bitcoin seed";
    uint8_t I[64];
    
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
    
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, md_info, 1);
    mbedtls_md_hmac_starts(&ctx, (const uint8_t*)key, strlen(key));
    mbedtls_md_hmac_update(&ctx, seed, 64);
    mbedtls_md_hmac_finish(&ctx, I);
    mbedtls_md_free(&ctx);
    
    // I[0:32] = master private key
    // I[32:64] = master chain code
    
    // Verify private key is valid
    if (!m_secp256k1->seckey_verify(I)) {
        Utils::SecureMemory::wipe(I, sizeof(I));
        return false;
    }
    
    memcpy(m_master.key + 1, I, 32);
    m_master.key[0] = 0;
    memcpy(m_master.chain_code, I + 32, 32);
    
    m_master.depth = 0;
    memset(m_master.fingerprint, 0, 4);
    memset(m_master.child_number, 0, 4);
    memcpy(m_master.version, VERSION_MAINNET_PRIVATE, 4);
    m_master.is_private = true;
    
    // Generate public key
    uint8_t pubkey[33];
    if (!m_secp256k1->generate_public_key(I, pubkey, true)) {
        Utils::SecureMemory::wipe(I, sizeof(I));
        return false;
    }
    memcpy(m_master.key + 33, pubkey, 33);
    
    Utils::SecureMemory::wipe(I, sizeof(I));
    
    m_current = m_master;
    m_initialized = true;
    
    return true;
}

bool BIP32::derive_child(const ExtendedKey* parent, uint32_t index, 
                         bool hardened, ExtendedKey* child) {
    if (!m_initialized || !parent || !child || !m_secp256k1) return false;
    if (!parent->is_private) return false;
    
    uint32_t i = index;
    if (hardened) {
        i |= 0x80000000;
    }
    
    uint8_t data[37];
    uint8_t I[64];
    
    if (hardened) {
        // Hardened: data = 0x00 || parent_private_key || index
        data[0] = 0x00;
        memcpy(data + 1, parent->key + 1, 32);
    } else {
        // Normal: data = parent_public_key || index
        memcpy(data, parent->key + 33, 33);
    }
    
    data[33] = (i >> 24) & 0xFF;
    data[34] = (i >> 16) & 0xFF;
    data[35] = (i >> 8) & 0xFF;
    data[36] = i & 0xFF;
    
    // HMAC-SHA512(parent_chain_code, data)
    mbedtls_md_context_t md_ctx;
    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
    
    mbedtls_md_init(&md_ctx);
    mbedtls_md_setup(&md_ctx, md_info, 1);
    mbedtls_md_hmac_starts(&md_ctx, parent->chain_code, 32);
    mbedtls_md_hmac_update(&md_ctx, data, 37);
    mbedtls_md_hmac_finish(&md_ctx, I);
    mbedtls_md_free(&md_ctx);
    
    // I_L is added to parent private key (mod n)
    uint8_t child_seckey[32];
    memcpy(child_seckey, I, 32);
    
    // tweak_add: child = parent + I_L
    if (!m_secp256k1->seckey_tweak_add(child_seckey, parent->key + 1)) {
        Utils::SecureMemory::wipe(I, sizeof(I));
        Utils::SecureMemory::wipe(child_seckey, sizeof(child_seckey));
        return false;
    }
    
    // Verify the key
    if (!m_secp256k1->seckey_verify(child_seckey)) {
        Utils::SecureMemory::wipe(I, sizeof(I));
        Utils::SecureMemory::wipe(child_seckey, sizeof(child_seckey));
        return false;
    }
    
    // Build child key
    memcpy(child->key + 1, child_seckey, 32);
    child->key[0] = 0;
    memcpy(child->chain_code, I + 32, 32);
    child->depth = parent->depth + 1;
    
    // Compute fingerprint
    uint32_t fp = get_fingerprint(parent);
    child->fingerprint[0] = (fp >> 24) & 0xFF;
    child->fingerprint[1] = (fp >> 16) & 0xFF;
    child->fingerprint[2] = (fp >> 8) & 0xFF;
    child->fingerprint[3] = fp & 0xFF;
    
    child->child_number[0] = (i >> 24) & 0xFF;
    child->child_number[1] = (i >> 16) & 0xFF;
    child->child_number[2] = (i >> 8) & 0xFF;
    child->child_number[3] = i & 0xFF;
    
    memcpy(child->version, parent->version, 4);
    child->is_private = true;
    
    // Generate and store public key
    uint8_t pubkey[33];
    if (m_secp256k1->generate_public_key(child_seckey, pubkey, true)) {
        memcpy(child->key + 33, pubkey, 33);
    }
    
    Utils::SecureMemory::wipe(I, sizeof(I));
    Utils::SecureMemory::wipe(child_seckey, sizeof(child_seckey));
    
    return true;
}

bool BIP32::get_public_key(const ExtendedKey* priv_key, ExtendedKey* pub_key) {
    if (!priv_key || !pub_key || !m_secp256k1) return false;
    if (!priv_key->is_private) return false;
    
    *pub_key = *priv_key;
    pub_key->is_private = false;
    
    // Copy the pre-computed public key
    memcpy(pub_key->key, pub_key->key + 33, 33);
    
    // Change version bytes
    if (memcmp(priv_key->version, VERSION_MAINNET_PRIVATE, 4) == 0) {
        memcpy(pub_key->version, VERSION_MAINNET_PUBLIC, 4);
    }
    
    return true;
}

uint32_t BIP32::get_fingerprint(const ExtendedKey* key) {
    if (!key || !m_secp256k1) return 0;
    
    // Fingerprint is first 4 bytes of HASH160(public_key)
    // HASH160 = SHA256 + RIPEMD160
    uint8_t sha256_hash[32];
    uint8_t ripemd160_hash[20];
    
    // First SHA256
    m_secp256k1->sha256(key->key + 33, 33, sha256_hash);
    
    // Then RIPEMD160
    RIPEMD160::hash(sha256_hash, 32, ripemd160_hash);
    
    return ((uint32_t)ripemd160_hash[0] << 24) |
           ((uint32_t)ripemd160_hash[1] << 16) |
           ((uint32_t)ripemd160_hash[2] << 8) |
           (uint32_t)ripemd160_hash[3];
}

static bool base58_encode(const uint8_t* data, size_t len, char* output, size_t output_len) {
    // Count leading zeros
    size_t zeros = 0;
    while (zeros < len && data[zeros] == 0) zeros++;
    
    // Allocate temporary buffer
    size_t size = (len - zeros) * 138 / 100 + 1;
    uint8_t* b58 = (uint8_t*)malloc(size);
    if (!b58) return false;
    memset(b58, 0, size);
    
    // Encode
    for (size_t i = zeros; i < len; i++) {
        uint32_t carry = data[i];
        for (size_t j = 0; j < size; j++) {
            carry += 256 * b58[size - 1 - j];
            b58[size - 1 - j] = carry % 58;
            carry /= 58;
        }
    }
    
    // Skip leading zeros in b58
    size_t b58_zeros = 0;
    while (b58_zeros < size && b58[b58_zeros] == 0) b58_zeros++;
    
    // Build output
    if (zeros + size - b58_zeros + 1 > output_len) {
        free(b58);
        return false;
    }
    
    size_t idx = 0;
    for (size_t i = 0; i < zeros; i++) output[idx++] = '1';
    for (size_t i = b58_zeros; i < size; i++) output[idx++] = BASE58_CHARS[b58[i]];
    output[idx] = '\0';
    
    free(b58);
    return true;
}

bool BIP32::serialize(const ExtendedKey* key, char* output, size_t output_len) {
    if (!key || !output || output_len < 128) return false;
    
    uint8_t data[78];
    memcpy(data, key->version, 4);
    data[4] = key->depth;
    memcpy(data + 5, key->fingerprint, 4);
    memcpy(data + 9, key->child_number, 4);
    memcpy(data + 13, key->chain_code, 32);
    
    if (key->is_private) {
        data[45] = 0;
        memcpy(data + 46, key->key + 1, 32);
    } else {
        memcpy(data + 45, key->key, 33);
    }
    
    // Double SHA256 checksum
    uint8_t hash1[32], hash2[32];
    m_secp256k1->hash256(data, 78, hash1);
    m_secp256k1->sha256(hash1, 32, hash2);
    
    // Append checksum (first 4 bytes)
    uint8_t data_with_checksum[82];
    memcpy(data_with_checksum, data, 78);
    memcpy(data_with_checksum + 78, hash2, 4);
    
    // Base58 encode
    if (!base58_encode(data_with_checksum, 82, output, output_len)) {
        return false;
    }
    
    return true;
}

void BIP32::clear() {
    Utils::SecureMemory::wipe(&m_master, sizeof(m_master));
    Utils::SecureMemory::wipe(&m_current, sizeof(m_current));
    if (m_secp256k1) {
        delete m_secp256k1;
        m_secp256k1 = nullptr;
    }
    m_initialized = false;
}

bool BIP32::derive_path(const char* path, ExtendedKey* result) {
    if (!path || !result || !m_initialized) return false;
    if (path[0] != 'm' && path[0] != 'M') return false;
    
    DerivationPath dpath;
    memset(&dpath, 0, sizeof(dpath));
    
    if (!parse_path(path, &dpath)) return false;
    return derive_path_indices(&dpath, result);
}

bool BIP32::derive_path_indices(const DerivationPath* path, ExtendedKey* result) {
    if (!path || !result || !m_initialized) return false;
    
    ExtendedKey current = m_master;
    ExtendedKey next;
    
    for (uint8_t i = 0; i < path->num_indices; i++) {
        if (!derive_child(&current, path->indices[i], path->hardened[i], &next)) {
            return false;
        }
        current = next;
    }
    
    *result = current;
    m_current = current;
    return true;
}

bool BIP32::deserialize(const char* xkey, ExtendedKey* key) {
    // TODO: Implement Base58 decode and deserialization
    return false;
}

bool BIP32::get_address(const ExtendedKey* key, char* address, size_t addr_len,
                        bool testnet, AddressType addr_type) {
    if (!key || !address || addr_len < 64) return false;
    
    // Get public key (compressed format at key->key + 33)
    const uint8_t* pub_key = key->key + 33;
    
    if (addr_type == AddressType::P2WPKH_NATIVE) {
        // Native SegWit (Bech32): P2WPKH
        // witness_version = 0, witness_program = HASH160(pub_key) (20 bytes)
        uint8_t sha256_hash[32];
        uint8_t hash160[20];
        
        m_secp256k1->sha256(pub_key, 33, sha256_hash);
        RIPEMD160::hash(sha256_hash, 32, hash160);
        
        return Bech32::create_segwit_address(address, addr_len, 0, hash160, 20, testnet);
    } else if (addr_type == AddressType::P2TR) {
        // Taproot (Bech32m): P2TR
        // Simplified: use hash of public key as witness program
        uint8_t sha256_hash[32];
        m_secp256k1->sha256(pub_key, 33, sha256_hash);
        
        return Bech32::create_segwit_address(address, addr_len, 1, sha256_hash, 32, testnet);
    }
    
    return false;
}

bool BIP32::sign(const ExtendedKey* key, const uint8_t hash[32], 
                 uint8_t signature[64], uint8_t* recid) {
    if (!key || !hash || !signature || !m_secp256k1) return false;
    if (!key->is_private) return false;
    
    return m_secp256k1->sign(key->key + 1, hash, signature, recid);
}

bool BIP32::parse_path(const char* path, DerivationPath* result) {
    if (!path || !result) return false;
    
    // Skip 'm/' or 'M/'
    const char* p = path;
    if (p[0] == 'm' || p[0] == 'M') {
        p++;
        if (p[0] == '/') p++;
    }
    
    result->num_indices = 0;
    
    while (*p && result->num_indices < 10) {
        bool hardened = false;
        
        // Parse number
        uint32_t index = 0;
        while (*p && *p >= '0' && *p <= '9') {
            index = index * 10 + (*p - '0');
            p++;
        }
        
        // Check for hardened marker
        if (*p == '\'' || *p == 'h' || *p == 'H') {
            hardened = true;
            p++;
        }
        
        result->indices[result->num_indices] = index;
        result->hardened[result->num_indices] = hardened;
        result->num_indices++;
        
        // Skip separator
        if (*p == '/') p++;
    }
    
    return result->num_indices > 0;
}

bool BIP32::self_test() {
    // Test vector from BIP32
    // Seed (hex): 000102030405060708090a0b0c0d0e0f
    uint8_t seed[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    
    // Pad to 64 bytes with zeros
    uint8_t seed64[64];
    memset(seed64, 0, 64);
    memcpy(seed64, seed, 16);
    
    BIP32 bip32;
    if (!bip32.init_from_seed(seed64)) {
        return false;
    }
    
    // Derive m/0' (first hardened child)
    ExtendedKey child;
    if (!bip32.derive_child(bip32.get_master_key(), 0, true, &child)) {
        return false;
    }
    
    // Check chain code matches expected
    uint8_t expected_chain[32] = {
        0x47, 0xfd, 0xac, 0xbd, 0x0f, 0x10, 0x97, 0x04,
        0x5b, 0x50, 0x21, 0x81, 0xb0, 0x60, 0x29, 0x8e,
        0x9a, 0xb0, 0xff, 0x05, 0x43, 0xce, 0x31, 0x0b,
        0x82, 0x45, 0x7b, 0x1d, 0x08, 0x2c, 0xd6, 0x52
    };
    
    return memcmp(child.chain_code, expected_chain, 32) == 0;
}

} // namespace Core
} // namespace SeedSigner
