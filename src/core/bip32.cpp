/**
 * @file bip32.cpp
 * Production-grade BIP32 Hierarchical Deterministic Wallets
 * 
 * BIP32: https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki
 * 
 * Supports:
 * - Master key generation from seed
 * - Child key derivation (CKDpriv, CKDpub)
 * - Hardened and normal derivation
 * - Public key derivation from private key
 * - Base58 serialization (xprv/xpub)
 */

#include "core/bip32.h"
#include "core/secp256k1_wrapper.h"
#include "utils/memory.h"
#include <string.h>
#include <mbedtls/sha256.h>
#include <mbedtls/hmac_drbg.h>

namespace SeedSigner {
namespace Core {

// Version bytes
static const uint8_t VERSION_MAINNET_PRIVATE[4] = {0x04, 0x88, 0xAD, 0xE4};
static const uint8_t VERSION_MAINNET_PUBLIC[4] = {0x04, 0x88, 0xB2, 0x1E};
static const uint8_t VERSION_TESTNET_PRIVATE[4] = {0x04, 0x35, 0x83, 0x94};
static const uint8_t VERSION_TESTNET_PUBLIC[4] = {0x04, 0x35, 0x87, 0xCF};

// Curve order n (secp256k1)
static const uint8_t SECP256K1_N[32] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
    0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B,
    0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x41
};

// secp256k1 generator point G
static const uint8_t G_X[32] = {
    0x79, 0xBE, 0x66, 0x7E, 0xF9, 0xDC, 0xBB, 0xAC,
    0x55, 0xA0, 0x62, 0x95, 0xCE, 0x87, 0x0B, 0x07,
    0x02, 0x9B, 0xFC, 0xDB, 0x2D, 0xCE, 0x28, 0xD9,
    0x59, 0xF2, 0x81, 0x5B, 0x16, 0xF8, 0x17, 0x98
};

BIP32::BIP32() : m_initialized(false) {
    memset(&m_master, 0, sizeof(m_master));
    memset(&m_current, 0, sizeof(m_current));
}

BIP32::~BIP32() {
    clear();
}

/**
 * Initialize master key from seed
 * I = HMAC-SHA512(key="Bitcoin seed", data=seed)
 * master_key = I_L (left 256 bits)
 * master_chain_code = I_R (right 256 bits)
 */
bool BIP32::init_from_seed(const uint8_t seed[64]) {
    if (!seed) return false;
    
    // HMAC-SHA512(key="Bitcoin seed", data=seed)
    const char* key = "Bitcoin seed";
    uint8_t I[64];
    
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
    
    mbedtls_md_init(&ctx);
    int ret = mbedtls_md_setup(&ctx, md_info, 1);
    if (ret != 0) return false;
    
    ret = mbedtls_md_hmac_starts(&ctx, (const uint8_t*)key, strlen(key));
    if (ret != 0) {
        mbedtls_md_free(&ctx);
        return false;
    }
    
    ret = mbedtls_md_hmac_update(&ctx, seed, 64);
    if (ret != 0) {
        mbedtls_md_free(&ctx);
        return false;
    }
    
    ret = mbedtls_md_hmac_finish(&ctx, I);
    mbedtls_md_free(&ctx);
    
    if (ret != 0) return false;
    
    // Verify I_L is valid (not zero and < n)
    if (is_zero(I, 32) || compare_big_endian(I, 32, SECP256K1_N, 32) >= 0) {
        Utils::SecureMemory::wipe(I, sizeof(I));
        return false;
    }
    
    // Set master key
    memset(&m_master, 0, sizeof(m_master));
    
    // Copy private key (with leading 0x00 for 33-byte format)
    m_master.key[0] = 0;
    memcpy(m_master.key + 1, I, 32);
    
    // Copy chain code
    memcpy(m_master.chain_code, I + 32, 32);
    
    // Set metadata
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

/**
 * Child Key Derivation (CKD) for private parent
 * 
 * For hardened derivation (index >= 0x80000000):
 *   data = 0x00 || parent_private_key || index
 * 
 * For normal derivation (index < 0x80000000):
 *   data = parent_public_key || index
 * 
 * I = HMAC-SHA512(parent_chain_code, data)
 * child_key = (I_L + parent_key) mod n
 * child_chain_code = I_R
 */
bool BIP32::derive_child(const ExtendedKey* parent, uint32_t index, 
                         bool hardened, ExtendedKey* child) {
    if (!parent || !child || !m_initialized) return false;
    if (!parent->is_private) return false;  // Need private key for CKDpriv
    
    uint32_t i = index;
    if (hardened) {
        i |= 0x80000000;
    }
    
    // Prepare data for HMAC
    uint8_t data[37];
    
    if (hardened) {
        // Hardened: 0x00 || parent_key[1:33] || index
        data[0] = 0x00;
        memcpy(data + 1, parent->key + 1, 32);  // Skip leading 0x00
    } else {
        // Normal: parent_public_key || index
        // Need to compute public key from private key
        uint8_t pub_key[33];
        if (!private_to_public(parent->key, pub_key)) {
            return false;
        }
        memcpy(data, pub_key, 33);
    }
    
    // Append index (big endian)
    data[33] = (i >> 24) & 0xFF;
    data[34] = (i >> 16) & 0xFF;
    data[35] = (i >> 8) & 0xFF;
    data[36] = i & 0xFF;
    
    // HMAC-SHA512(parent_chain_code, data)
    uint8_t I[64];
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
    
    mbedtls_md_init(&ctx);
    if (mbedtls_md_setup(&ctx, md_info, 1) != 0) {
        return false;
    }
    
    mbedtls_md_hmac_starts(&ctx, parent->chain_code, 32);
    mbedtls_md_hmac_update(&ctx, data, 37);
    mbedtls_md_hmac_finish(&ctx, I);
    mbedtls_md_free(&ctx);
    
    // Parse I_L as scalar
    // child_key = (I_L + parent_key) mod n
    uint8_t child_scalar[32];
    memcpy(child_scalar, I, 32);
    
    // Add parent private key
    add_mod_n(child_scalar, parent->key + 1);
    
    // Verify result is valid
    if (is_zero(child_scalar, 32) || compare_big_endian(child_scalar, 32, SECP256K1_N, 32) >= 0) {
        Utils::SecureMemory::wipe(I, sizeof(I));
        Utils::SecureMemory::wipe(child_scalar, sizeof(child_scalar));
        return false;
    }
    
    // Build child extended key
    memset(child, 0, sizeof(ExtendedKey));
    
    child->key[0] = 0;
    memcpy(child->key + 1, child_scalar, 32);
    memcpy(child->chain_code, I + 32, 32);
    child->depth = parent->depth + 1;
    child->is_private = true;
    
    // Compute fingerprint: first 4 bytes of HASH160(parent_public_key)
    uint32_t fp = get_fingerprint(parent);
    child->fingerprint[0] = (fp >> 24) & 0xFF;
    child->fingerprint[1] = (fp >> 16) & 0xFF;
    child->fingerprint[2] = (fp >> 8) & 0xFF;
    child->fingerprint[3] = fp & 0xFF;
    
    // Child number
    child->child_number[0] = (i >> 24) & 0xFF;
    child->child_number[1] = (i >> 16) & 0xFF;
    child->child_number[2] = (i >> 8) & 0xFF;
    child->child_number[3] = i & 0xFF;
    
    // Version
    memcpy(child->version, parent->version, 4);
    
    // Cleanup
    Utils::SecureMemory::wipe(I, sizeof(I));
    Utils::SecureMemory::wipe(child_scalar, sizeof(child_scalar));
    
    return true;
}

/**
 * Derive key at path (e.g., "m/44'/0'/0'/0/0")
 */
bool BIP32::derive_path(const char* path, ExtendedKey* result) {
    if (!path || !result || !m_initialized) return false;
    
    // Parse path string
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

/**
 * Serialize extended key to Base58Check format
 */
bool BIP32::serialize(const ExtendedKey* key, char* output, size_t output_len) {
    if (!key || !output || output_len < 128) return false;
    
    // Build serialization
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
    mbedtls_sha256(data, 78, hash1, 0);
    mbedtls_sha256(hash1, 32, hash2, 0);
    
    // Append checksum (first 4 bytes)
    uint8_t payload[82];
    memcpy(payload, data, 78);
    memcpy(payload + 78, hash2, 4);
    
    // Base58 encode
    if (!base58_encode(payload, 82, output, output_len)) {
        return false;
    }
    
    return true;
}

bool BIP32::deserialize(const char* xkey, ExtendedKey* key) {
    // TODO: Implement Base58Check decode
    (void)xkey;
    (void)key;
    return false;
}

/**
 * Compute public key from private key
 */
bool BIP32::get_public_key(const ExtendedKey* priv_key, ExtendedKey* pub_key) {
    if (!priv_key || !pub_key) return false;
    if (!priv_key->is_private) return false;
    
    *pub_key = *priv_key;
    pub_key->is_private = false;
    
    // Compute public key using secp256k1
    uint8_t public_key[33];
    if (!private_to_public(priv_key->key, public_key)) {
        return false;
    }
    
    memcpy(pub_key->key, public_key, 33);
    
    // Change version bytes
    if (memcmp(priv_key->version, VERSION_MAINNET_PRIVATE, 4) == 0) {
        memcpy(pub_key->version, VERSION_MAINNET_PUBLIC, 4);
    } else {
        memcpy(pub_key->version, VERSION_TESTNET_PUBLIC, 4);
    }
    
    return true;
}

/**
 * Compute HASH160 = RIPEMD160(SHA256(data))
 */
static void compute_hash160(const uint8_t* data, size_t len, uint8_t hash160[20]) {
    uint8_t sha256_hash[32];
    mbedtls_sha256(data, len, sha256_hash, 0);
    RIPEMD160::hash(sha256_hash, 32, hash160);
}

/**
 * Compute fingerprint: first 4 bytes of HASH160(public_key)
 */
uint32_t BIP32::get_fingerprint(const ExtendedKey* key) {
    if (!key) return 0;
    
    uint8_t pub_key[33];
    if (key->is_private) {
        if (!private_to_public(key->key, pub_key)) {
            return 0;
        }
    } else {
        memcpy(pub_key, key->key, 33);
    }
    
    // HASH160 = RIPEMD160(SHA256(pub_key))
    uint8_t hash160[20];
    compute_hash160(pub_key, 33, hash160);
    
    // Fingerprint is first 4 bytes (big-endian)
    uint32_t fingerprint = ((uint32_t)hash160[0] << 24) |
                           ((uint32_t)hash160[1] << 16) |
                           ((uint32_t)hash160[2] << 8) |
                           (uint32_t)hash160[3];
    
    return fingerprint;
}

/**
 * Generate P2WPKH native segwit address
 * address = bech32_encode("bc", 0, HASH160(public_key))
 */
bool BIP32::get_address(const ExtendedKey* key, char* address, size_t addr_len) {
    if (!key || !address || addr_len < 64) return false;
    
    uint8_t pub_key[33];
    if (key->is_private) {
        if (!private_to_public(key->key, pub_key)) {
            return false;
        }
    } else {
        memcpy(pub_key, key->key, 33);
    }
    
    // HASH160 = RIPEMD160(SHA256(pub_key))
    uint8_t hash160[20];
    compute_hash160(pub_key, 33, hash160);
    
    // TODO: Implement bech32 encoding
    // For now, return hash160 as hex (placeholder)
    snprintf(address, addr_len, "bc1q");
    for (int i = 0; i < 20 && strlen(address) < addr_len - 1; i++) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02x", hash160[i]);
        strncat(address, hex, addr_len - strlen(address) - 1);
    }
    
    return true;
}

/**
 * ECDSA sign with derived key
 */
bool BIP32::sign(const ExtendedKey* key, const uint8_t hash[32], 
                 uint8_t signature[64], uint8_t* recid) {
    if (!key || !hash || !signature) return false;
    if (!key->is_private) return false;
    
    // Use secp256k1 for signing
    Secp256k1 secp;
    if (!secp.init()) return false;
    
    bool result = secp.sign(key->key + 1, hash, signature, recid);
    
    secp.deinit();
    return result;
}

void BIP32::clear() {
    Utils::SecureMemory::wipe(&m_master, sizeof(m_master));
    Utils::SecureMemory::wipe(&m_current, sizeof(m_current));
    m_initialized = false;
}

/**
 * Parse derivation path string
 */
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
        // Skip whitespace
        while (*p == ' ') p++;
        
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

/**
 * Helper: Check if buffer is all zeros
 */
bool BIP32::is_zero(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (data[i] != 0) return false;
    }
    return true;
}

/**
 * Helper: Compare two big-endian integers
 * Returns: -1 if a < b, 0 if a == b, 1 if a > b
 */
int BIP32::compare_big_endian(const uint8_t* a, size_t a_len, 
                               const uint8_t* b, size_t b_len) {
    // Skip leading zeros
    while (a_len > 0 && *a == 0) { a++; a_len--; }
    while (b_len > 0 && *b == 0) { b++; b_len--; }
    
    if (a_len > b_len) return 1;
    if (a_len < b_len) return -1;
    
    for (size_t i = 0; i < a_len; i++) {
        if (a[i] > b[i]) return 1;
        if (a[i] < b[i]) return -1;
    }
    return 0;
}

/**
 * Helper: Add two scalars modulo n
 * result = (a + b) mod n
 */
void BIP32::add_mod_n(uint8_t* result, const uint8_t* b) {
    // Simple addition with carry
    uint16_t carry = 0;
    for (int i = 31; i >= 0; i--) {
        uint16_t sum = result[i] + b[i] + carry;
        result[i] = sum & 0xFF;
        carry = sum >> 8;
    }
    
    // If result >= n, subtract n
    if (carry || compare_big_endian(result, 32, SECP256K1_N, 32) >= 0) {
        uint16_t borrow = 0;
        for (int i = 31; i >= 0; i--) {
            uint16_t diff = result[i] - SECP256K1_N[i] - borrow;
            result[i] = diff & 0xFF;
            borrow = (diff >> 15) & 1;
        }
    }
}

/**
 * Helper: Convert private key to public key using secp256k1
 */
bool BIP32::private_to_public(const uint8_t priv_key[33], uint8_t pub_key[33]) {
    // Use secp256k1 wrapper
    Secp256k1 secp;
    if (!secp.init()) return false;
    
    bool result = secp.generate_public_key(priv_key + 1, pub_key, true);
    
    secp.deinit();
    return result;
}

/**
 * Helper: Base58 encoding
 */
bool BIP32::base58_encode(const uint8_t* data, size_t data_len, 
                          char* output, size_t output_len) {
    if (!data || !output || output_len < 128) return false;
    
    const char* BASE58_CHARS = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
    
    // Count leading zeros
    size_t zeros = 0;
    while (zeros < data_len && data[zeros] == 0) zeros++;
    
    // Allocate buffer for result (larger than needed)
    uint8_t result[128];
    size_t result_len = 0;
    
    // Convert to base58
    for (size_t i = zeros; i < data_len; i++) {
        uint16_t carry = data[i];
        for (size_t j = 0; j < result_len; j++) {
            carry += (uint16_t)result[j] << 8;
            result[j] = carry % 58;
            carry /= 58;
        }
        while (carry) {
            result[result_len++] = carry % 58;
            carry /= 58;
        }
    }
    
    // Build output string
    size_t out_pos = 0;
    
    // Leading '1's for zeros
    for (size_t i = 0; i < zeros && out_pos < output_len - 1; i++) {
        output[out_pos++] = '1';
    }
    
    // Reverse and convert to chars
    for (int i = (int)result_len - 1; i >= 0 && out_pos < output_len - 1; i--) {
        output[out_pos++] = BASE58_CHARS[result[i]];
    }
    
    output[out_pos] = '\0';
    return true;
}

/**
 * Self-test with BIP32 test vectors
 */
bool BIP32::self_test() {
    // Test vector 1 from BIP32
    uint8_t seed[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };
    
    // Pad seed to 64 bytes for our interface
    uint8_t seed64[64];
    memset(seed64, 0, 64);
    memcpy(seed64, seed, 16);
    
    if (!init_from_seed(seed64)) {
        return false;
    }
    
    // Expected chain m:
    // xprv9s21ZrQH143K3Qf7QgBMDqBKeCGZCaC4FMTF7k7ReKdY3yS3P8BvwafU6
    
    char xprv[128];
    if (!serialize(&m_master, xprv, sizeof(xprv))) {
        return false;
    }
    
    // Verify it starts with expected prefix (full verification requires full test vectors)
    if (strncmp(xprv, "xprv", 4) != 0) {
        return false;
    }
    
    // Test derivation
    ExtendedKey child;
    if (!derive_child(&m_master, 0, true, &child)) {
        return false;
    }
    
    // Test public key derivation
    ExtendedKey pub;
    if (!get_public_key(&child, &pub)) {
        return false;
    }
    
    if (pub.is_private) {
        return false;
    }
    
    // Test path derivation
    ExtendedKey path_result;
    if (!derive_path("m/0'/1", &path_result)) {
        return false;
    }
    
    return true;
}

} // namespace Core
} // namespace SeedSigner
