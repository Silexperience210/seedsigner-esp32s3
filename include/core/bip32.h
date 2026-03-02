/**
 * @file bip32.h
 * BIP32 Hierarchical Deterministic Wallets
 */

#ifndef CORE_BIP32_H
#define CORE_BIP32_H

#include <Arduino.h>
#include <stdint.h>

namespace SeedSigner {
namespace Core {

// Extended key structure (xprv/xpub)
struct ExtendedKey {
    uint8_t version[4];      // Mainnet: 0x0488B21E (pub) / 0x0488ADE4 (prv)
    uint8_t depth;           // 0 = master
    uint8_t fingerprint[4];  // Parent fingerprint
    uint8_t child_number[4]; // Child index
    uint8_t chain_code[32];  // Chain code
    uint8_t key[33];         // Public or private key (33 bytes)
    bool is_private;
};

// Derivation path component
struct DerivationPath {
    uint32_t indices[10];  // Support up to 10 levels
    uint8_t num_indices;
    bool hardened[10];
};

class BIP32 {
public:
    BIP32();
    ~BIP32();
    
    // Initialize from seed (64 bytes)
    bool init_from_seed(const uint8_t seed[64]);
    
    // Derive child key
    bool derive_child(const ExtendedKey* parent, uint32_t index, 
                      bool hardened, ExtendedKey* child);
    
    // Derive path (e.g., "m/84'/0'/0'/0/0")
    bool derive_path(const char* path, ExtendedKey* result);
    bool derive_path_indices(const DerivationPath* path, ExtendedKey* result);
    
    // Get master key
    const ExtendedKey* get_master_key() const { return &m_master; }
    
    // Get current derived key
    const ExtendedKey* get_current_key() const { return &m_current; }
    
    // Serialization
    bool serialize(const ExtendedKey* key, char* output, size_t output_len);
    bool deserialize(const char* xkey, ExtendedKey* key);
    
    // Public key from private
    bool get_public_key(const ExtendedKey* priv_key, ExtendedKey* pub_key);
    
    // Fingerprint calculation
    uint32_t get_fingerprint(const ExtendedKey* key);
    
    // Address generation
    bool get_address(const ExtendedKey* key, char* address, size_t addr_len);
    
    // Signing
    bool sign(const ExtendedKey* key, const uint8_t hash[32], 
              uint8_t signature[64], uint8_t* recid);
    
    // Clear sensitive data
    void clear();
    
    bool self_test();
    
private:
    ExtendedKey m_master;
    ExtendedKey m_current;
    bool m_initialized;
    
    // HMAC-SHA512
    void hmac_sha512(const uint8_t* key, size_t key_len,
                     const uint8_t* data, size_t data_len,
                     uint8_t output[64]);
    
    // CKD (Child Key Derivation)
    bool ckd_private(const ExtendedKey* parent, uint32_t index,
                     bool hardened, ExtendedKey* child);
    bool ckd_public(const ExtendedKey* parent, uint32_t index,
                    ExtendedKey* child);
    
    // Parse path string
    bool parse_path(const char* path, DerivationPath* result);
};

// Common derivation paths
#define BIP44_PURPOSE      44
#define BIP49_PURPOSE      49
#define BIP84_PURPOSE      84
#define BIP86_PURPOSE      86

#define COIN_TYPE_BITCOIN  0
#define COIN_TYPE_TESTNET  1

#define ACCOUNT_DEFAULT    0

#define CHANGE_EXTERNAL    0
#define CHANGE_INTERNAL    1

} // namespace Core
} // namespace SeedSigner

#endif // CORE_BIP32_H
