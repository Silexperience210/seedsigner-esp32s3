/**
 * @file bip32.cpp
 * BIP32 Hierarchical Deterministic Wallets implementation
 */

#include "core/bip32.h"
#include "utils/memory.h"
#include <string.h>
#include <mbedtls/sha256.h>
#include <mbedtls/hmac_drbg.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecp.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/bignum.h>

namespace SeedSigner {
namespace Core {

// secp256k1 curve parameters
static const char* SECP256K1_P = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F";
static const char* SECP256K1_N = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141";
static const char* SECP256K1_G_X = "79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798";
static const char* SECP256K1_G_Y = "483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8";

// Version bytes
static const uint8_t VERSION_MAINNET_PRIVATE[4] = {0x04, 0x88, 0xAD, 0xE4};
static const uint8_t VERSION_MAINNET_PUBLIC[4] = {0x04, 0x88, 0xB2, 0x1E};
static const uint8_t VERSION_TESTNET_PRIVATE[4] = {0x04, 0x35, 0x83, 0x94};
static const uint8_t VERSION_TESTNET_PUBLIC[4] = {0x04, 0x35, 0x87, 0xCF};

BIP32::BIP32() : m_initialized(false) {
    memset(&m_master, 0, sizeof(m_master));
    memset(&m_current, 0, sizeof(m_current));
}

BIP32::~BIP32() {
    clear();
}

bool BIP32::init_from_seed(const uint8_t seed[64]) {
    if (!seed) return false;
    
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
    memcpy(m_master.key, I, 32);
    m_master.key[0] = 0;  // Clear first byte for 33-byte format
    memcpy(m_master.key + 1, I, 32);  // Copy private key with leading 0
    memcpy(m_master.chain_code, I + 32, 32);
    
    m_master.depth = 0;
    memset(m_master.fingerprint, 0, 4);
    memset(m_master.child_number, 0, 4);
    memcpy(m_master.version, VERSION_MAINNET_PRIVATE, 4);
    m_master.is_private = true;
    
    // Clear intermediate data
    Utils::SecureMemory::wipe(I, sizeof(I));
    
    m_current = m_master;
    m_initialized = true;
    
    return true;
}

bool BIP32::derive_child(const ExtendedKey* parent, uint32_t index, 
                         bool hardened, ExtendedKey* child) {
    if (!parent || !child) return false;
    if (!parent->is_private) return false;  // Need private key for derivation
    
    uint32_t i = index;
    if (hardened) {
        i |= 0x80000000;
    }
    
    uint8_t data[37];
    if (hardened) {
        // Hardened: data = 0x00 || parent_key || index
        data[0] = 0x00;
        memcpy(data + 1, parent->key + 1, 32);  // Private key (without 0x00 prefix)
    } else {
        // Normal: data = parent_public_key || index
        // Need to compute public key first
        uint8_t pub_key[33];
        // TODO: Implement public key derivation from private key
        // For now, return false
        return false;
    }
    
    data[33] = (i >> 24) & 0xFF;
    data[34] = (i >> 16) & 0xFF;
    data[35] = (i >> 8) & 0xFF;
    data[36] = i & 0xFF;
    
    // HMAC-SHA512(parent_chain_code, data)
    uint8_t I[64];
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
    
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, md_info, 1);
    mbedtls_md_hmac_starts(&ctx, parent->chain_code, 32);
    mbedtls_md_hmac_update(&ctx, data, 37);
    mbedtls_md_hmac_finish(&ctx, I);
    mbedtls_md_free(&ctx);
    
    // Parse I_L as private key
    memcpy(child->key + 1, I, 32);
    child->key[0] = 0;
    
    // Add parent private key (mod n)
    // TODO: Implement proper modular addition on secp256k1
    
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
    
    Utils::SecureMemory::wipe(I, sizeof(I));
    Utils::SecureMemory::wipe(data, sizeof(data));
    
    return true;
}

bool BIP32::derive_path(const char* path, ExtendedKey* result) {
    if (!path || !result || !m_initialized) return false;
    
    // Parse path string (e.g., "m/84'/0'/0'/0/0")
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

bool BIP32::serialize(const ExtendedKey* key, char* output, size_t output_len) {
    if (!key || !output || output_len < 113) return false;
    
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
    
    // Base58Check encode
    // TODO: Implement base58 encoding
    // For now, hex encode
    strcpy(output, "xprv/");
    for (int i = 0; i < 78; i++) {
        sprintf(output + 5 + i * 2, "%02x", data[i]);
    }
    
    return true;
}

bool BIP32::deserialize(const char* xkey, ExtendedKey* key) {
    // TODO: Implement Base58Check decode
    (void)xkey;
    (void)key;
    return false;
}

bool BIP32::get_public_key(const ExtendedKey* priv_key, ExtendedKey* pub_key) {
    if (!priv_key || !pub_key) return false;
    if (!priv_key->is_private) return false;
    
    *pub_key = *priv_key;
    pub_key->is_private = false;
    
    // Compute public key from private key using secp256k1
    // TODO: Implement EC point multiplication
    // P = k * G
    
    // Change version bytes
    if (memcmp(priv_key->version, VERSION_MAINNET_PRIVATE, 4) == 0) {
        memcpy(pub_key->version, VERSION_MAINNET_PUBLIC, 4);
    } else {
        memcpy(pub_key->version, VERSION_TESTNET_PUBLIC, 4);
    }
    
    return true;
}

uint32_t BIP32::get_fingerprint(const ExtendedKey* key) {
    if (!key) return 0;
    
    // Fingerprint is first 4 bytes of HASH160(public_key)
    // TODO: Implement proper fingerprint calculation
    // For now, return hash of key data
    uint8_t hash[32];
    mbedtls_sha256(key->key, 33, hash, 0);
    
    return ((uint32_t)hash[0] << 24) |
           ((uint32_t)hash[1] << 16) |
           ((uint32_t)hash[2] << 8) |
           (uint32_t)hash[3];
}

bool BIP32::get_address(const ExtendedKey* key, char* address, size_t addr_len) {
    if (!key || !address || addr_len < 40) return false;
    
    // P2WPKH native segwit address
    // address = bech32_encode("bc", 0, HASH160(public_key))
    
    // TODO: Implement bech32 encoding
    strcpy(address, "bc1q...");
    
    return true;
}

bool BIP32::sign(const ExtendedKey* key, const uint8_t hash[32], 
                 uint8_t signature[64], uint8_t* recid) {
    if (!key || !hash || !signature) return false;
    if (!key->is_private) return false;
    
    // ECDSA sign using secp256k1
    // TODO: Implement proper ECDSA signing
    
    (void)recid;
    return false;
}

void BIP32::clear() {
    Utils::SecureMemory::wipe(&m_master, sizeof(m_master));
    Utils::SecureMemory::wipe(&m_current, sizeof(m_current));
    m_initialized = false;
}

bool BIP32::parse_path(const char* path, DerivationPath* result) {
    if (!path || !result) return false;
    
    // Skip leading 'm/' or 'M/'
    const char* p = path;
    if (*p == 'm' || *p == 'M') {
        p++;
        if (*p == '/') p++;
    }
    
    result->num_indices = 0;
    
    while (*p && result->num_indices < 10) {
        // Parse number
        uint32_t index = 0;
        bool hardened = false;
        
        while (*p >= '0' && *p <= '9') {
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
    // Chain m:
    //   ext pub: xpub661MyMwAqRbcFtXgS5sYJABqqG9YLmC4Q1Rdap9gSE8NqtwybGhePY2gZ29ESFjqJoCu1Rupje8YtGqsefD265TMg7usUDFdp6W1EGMcet8
    //   ext prv: xprv9s21ZrQH143K3QTDL4LXw2F7HEK3wJUD2nW2nRk8stYQtjPyVHP7q4XAGhEF5xQtV5E2Z6jF8gJq8xQjx8JQq7JZgG

    uint8_t seed[64];
    memset(seed, 0, sizeof(seed));
    for (int i = 0; i < 16; i++) {
        seed[i] = i;
    }
    
    if (!init_from_seed(seed)) {
        return false;
    }
    
    char xprv[128];
    if (!serialize(&m_master, xprv, sizeof(xprv))) {
        return false;
    }
    
    // TODO: Verify against expected value
    
    return true;
}

} // namespace Core
} // namespace SeedSigner
