#include "core/bip32_secp256k1.h"
#include "core/secp256k1_wrapper.h"
#include <string.h>
#include <stdio.h>

// Déclarations externes pour les fonctions de hachage
extern void sha256(const uint8_t* data, size_t len, uint8_t hash[32]);
extern void hmac_sha512(const uint8_t* key, size_t key_len, 
                        const uint8_t* data, size_t data_len, 
                        uint8_t out[64]);

namespace BIP32 {

// Version bytes
static const uint8_t VERSION_MAINNET_PRIVATE[4] = {0x04, 0x88, 0xAD, 0xE4};  // xprv
static const uint8_t VERSION_MAINNET_PUBLIC[4]  = {0x04, 0x88, 0xB2, 0x1E};  // xpub

// Base58 alphabet
static const char BASE58_CHARS[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

static void encode_base58(const uint8_t* data, size_t len, char* out, size_t* out_len) {
    // Count leading zeros
    size_t zeros = 0;
    while (zeros < len && data[zeros] == 0) zeros++;
    
    // Allocate temporary buffer (big enough for result)
    size_t size = (len - zeros) * 138 / 100 + 1;
    uint8_t* b58 = new uint8_t[size];
    memset(b58, 0, size);
    
    // Process bytes
    for (size_t i = zeros; i < len; i++) {
        uint32_t carry = data[i];
        for (size_t j = 0; j < size; j++) {
            carry += (uint32_t)b58[j] << 8;
            b58[j] = carry % 58;
            carry /= 58;
        }
    }
    
    // Skip leading zeros in b58
    size_t b58_zeros = 0;
    while (b58_zeros < size && b58[b58_zeros] == 0) b58_zeros++;
    
    // Build result
    size_t idx = 0;
    for (size_t i = 0; i < zeros; i++) {
        out[idx++] = '1';
    }
    for (size_t i = b58_zeros; i < size; i++) {
        out[idx++] = BASE58_CHARS[b58[size - 1 - i]];
    }
    out[idx] = '\0';
    *out_len = idx;
    
    delete[] b58;
}

bool init_from_seed(const uint8_t seed[SEED_SIZE], ExtendedKey* out_key) {
    if (!out_key) return false;
    
    // HMAC-SHA512(key="Bitcoin seed", data=seed)
    uint8_t I[64];
    const char* key = "Bitcoin seed";
    hmac_sha512((const uint8_t*)key, strlen(key), seed, SEED_SIZE, I);
    
    // I_L = master secret key (32 bytes)
    // I_R = master chain code (32 bytes)
    uint8_t* master_key = I;
    uint8_t* chain_code = I + 32;
    
    // Vérifier que master_key est valide (1 <= key < n)
    if (!Secp256k1::is_valid_seckey(master_key)) {
        return false;
    }
    
    // Initialize extended key
    memset(out_key, 0, sizeof(ExtendedKey));
    out_key->key[0] = 0x00;  // Private key prefix
    memcpy(out_key->key + 1, master_key, 32);
    memcpy(out_key->chain_code, chain_code, CHAIN_CODE_SIZE);
    out_key->depth = 0;
    out_key->child_number = 0;
    memset(out_key->fingerprint, 0, FINGERPRINT_SIZE);
    
    // Clear sensitive data
    memset(I, 0, sizeof(I));
    
    return true;
}

bool derive_child(const ExtendedKey* parent, uint32_t index, ExtendedKey* out_child) {
    if (!parent || !out_child) return false;
    
    bool is_hardened = (index & 0x80000000) != 0;
    
    // Prepare data for HMAC
    uint8_t data[37];
    if (is_hardened) {
        // Hardened: data = 0x00 || ser256(parent_key) || ser32(index)
        data[0] = 0x00;
        memcpy(data + 1, parent->key + 1, 32);
    } else {
        // Normal: data = serP(parent_pubkey) || ser32(index)
        uint8_t parent_pubkey[Secp256k1::PUBLIC_KEY_COMPRESSED_SIZE];
        if (parent->key[0] == 0x00) {
            // Private key - derive public key
            Secp256k1::generate_public_key(parent->key + 1, parent_pubkey, true);
        } else {
            // Public key - copy directly
            memcpy(parent_pubkey, parent->key, Secp256k1::PUBLIC_KEY_COMPRESSED_SIZE);
        }
        memcpy(data, parent_pubkey, Secp256k1::PUBLIC_KEY_COMPRESSED_SIZE);
    }
    
    // Append index (big endian)
    data[33] = (index >> 24) & 0xFF;
    data[34] = (index >> 16) & 0xFF;
    data[35] = (index >> 8) & 0xFF;
    data[36] = index & 0xFF;
    
    // HMAC-SHA512(parent_chain_code, data)
    uint8_t I[64];
    hmac_sha512(parent->chain_code, CHAIN_CODE_SIZE, data, 37, I);
    
    uint8_t* child_key = I;
    uint8_t* child_chain = I + 32;
    
    // Check for invalid derivation
    if (!Secp256k1::is_valid_seckey(child_key)) {
        // Try next index
        memset(I, 0, sizeof(I));
        return derive_child(parent, index + 1, out_child);
    }
    
    // Initialize child key
    memset(out_child, 0, sizeof(ExtendedKey));
    out_child->key[0] = 0x00;
    
    if (parent->key[0] == 0x00) {
        // Private parent: child_key = parse256(I_L) + parent_key
        memcpy(out_child->key + 1, parent->key + 1, 32);
        if (!Secp256k1::seckey_tweak_add(out_child->key + 1, child_key)) {
            memset(I, 0, sizeof(I));
            return false;
        }
    } else {
        // Public parent: use tweak_add on public key
        memcpy(out_child->key, parent->key, KEY_SIZE);
        if (!Secp256k1::pubkey_tweak_add(out_child->key, child_key)) {
            memset(I, 0, sizeof(I));
            return false;
        }
    }
    
    memcpy(out_child->chain_code, child_chain, CHAIN_CODE_SIZE);
    out_child->depth = parent->depth + 1;
    out_child->child_number = index;
    
    // Calculate fingerprint
    uint32_t fp = get_fingerprint(parent);
    out_child->fingerprint[0] = (fp >> 24) & 0xFF;
    out_child->fingerprint[1] = (fp >> 16) & 0xFF;
    out_child->fingerprint[2] = (fp >> 8) & 0xFF;
    out_child->fingerprint[3] = fp & 0xFF;
    
    // Clear sensitive data
    memset(I, 0, sizeof(I));
    
    return true;
}

bool derive_path(const ExtendedKey* master, const char* path, ExtendedKey* out_key) {
    if (!master || !path || !out_key) return false;
    
    // Copy master as starting point
    *out_key = *master;
    
    // Skip "m/" prefix if present
    const char* p = path;
    if (p[0] == 'm' && p[1] == '/') {
        p += 2;
    } else if (p[0] == '/') {
        p++;
    }
    
    // Parse each component
    char buffer[16];
    size_t buf_idx = 0;
    
    while (*p) {
        if (*p == '/') {
            buffer[buf_idx] = '\0';
            if (buf_idx > 0) {
                // Parse index
                bool hardened = false;
                if (buf_idx > 1 && (buffer[buf_idx-1] == '\'' || buffer[buf_idx-1] == 'h')) {
                    hardened = true;
                    buffer[buf_idx-1] = '\0';
                }
                
                uint32_t index = (uint32_t)atoi(buffer);
                if (hardened) index |= 0x80000000;
                
                ExtendedKey child;
                if (!derive_child(out_key, index, &child)) {
                    return false;
                }
                *out_key = child;
            }
            buf_idx = 0;
        } else {
            if (buf_idx < sizeof(buffer) - 1) {
                buffer[buf_idx++] = *p;
            }
        }
        p++;
    }
    
    // Process last component
    if (buf_idx > 0) {
        buffer[buf_idx] = '\0';
        bool hardened = false;
        if (buf_idx > 1 && (buffer[buf_idx-1] == '\'' || buffer[buf_idx-1] == 'h')) {
            hardened = true;
            buffer[buf_idx-1] = '\0';
        }
        
        uint32_t index = (uint32_t)atoi(buffer);
        if (hardened) index |= 0x80000000;
        
        ExtendedKey child;
        if (!derive_child(out_key, index, &child)) {
            return false;
        }
        *out_key = child;
    }
    
    return true;
}

bool get_public_key(const ExtendedKey* priv_key, ExtendedKey* out_pub_key) {
    if (!priv_key || !out_pub_key) return false;
    if (priv_key->key[0] != 0x00) return false;  // Already public
    
    *out_pub_key = *priv_key;
    
    uint8_t pubkey[Secp256k1::PUBLIC_KEY_COMPRESSED_SIZE];
    if (!Secp256k1::generate_public_key(priv_key->key + 1, pubkey, true)) {
        return false;
    }
    
    memcpy(out_pub_key->key, pubkey, Secp256k1::PUBLIC_KEY_COMPRESSED_SIZE);
    return true;
}

uint32_t get_fingerprint(const ExtendedKey* key) {
    if (!key) return 0;
    
    uint8_t pubkey[Secp256k1::PUBLIC_KEY_COMPRESSED_SIZE];
    
    if (key->key[0] == 0x00) {
        // Private key - derive public key
        Secp256k1::generate_public_key(key->key + 1, pubkey, true);
    } else {
        // Public key
        memcpy(pubkey, key->key, Secp256k1::PUBLIC_KEY_COMPRESSED_SIZE);
    }
    
    // HASH160(pubkey) = RIPEMD160(SHA256(pubkey))
    uint8_t hash160[20];
    Secp256k1::hash160(pubkey, Secp256k1::PUBLIC_KEY_COMPRESSED_SIZE, hash160);
    
    // Return first 4 bytes
    return ((uint32_t)hash160[0] << 24) | 
           ((uint32_t)hash160[1] << 16) | 
           ((uint32_t)hash160[2] << 8) | 
           (uint32_t)hash160[3];
}

bool serialize(const ExtendedKey* key, bool public_only, char* out, size_t out_len) {
    if (!key || !out || out_len < 113) return false;
    
    uint8_t serialized[78];
    
    // Version
    if (public_only || key->key[0] != 0x00) {
        memcpy(serialized, VERSION_MAINNET_PUBLIC, 4);
    } else {
        memcpy(serialized, VERSION_MAINNET_PRIVATE, 4);
    }
    
    // Depth
    serialized[4] = key->depth;
    
    // Fingerprint
    memcpy(serialized + 5, key->fingerprint, 4);
    
    // Child number
    serialized[9] = (key->child_number >> 24) & 0xFF;
    serialized[10] = (key->child_number >> 16) & 0xFF;
    serialized[11] = (key->child_number >> 8) & 0xFF;
    serialized[12] = key->child_number & 0xFF;
    
    // Chain code
    memcpy(serialized + 13, key->chain_code, 32);
    
    // Key data
    if (public_only || key->key[0] != 0x00) {
        // Public key
        memcpy(serialized + 45, key->key, 33);
    } else {
        // Private key (with 0x00 prefix)
        serialized[45] = 0x00;
        memcpy(serialized + 46, key->key + 1, 32);
    }
    
    // Calculate checksum (first 4 bytes of double SHA256)
    uint8_t hash1[32], hash2[32];
    sha256(serialized, 78, hash1);
    sha256(hash1, 32, hash2);
    
    // Append checksum
    uint8_t with_checksum[82];
    memcpy(with_checksum, serialized, 78);
    memcpy(with_checksum + 78, hash2, 4);
    
    // Base58 encode
    size_t b58_len;
    encode_base58(with_checksum, 82, out, &b58_len);
    
    return b58_len > 0;
}

void wipe_key(ExtendedKey* key) {
    if (key) {
        memset(key, 0, sizeof(ExtendedKey));
    }
}

} // namespace BIP32
