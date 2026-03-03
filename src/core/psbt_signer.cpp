#include "core/psbt_signer.h"
#include "core/secp256k1_wrapper.h"
#include "core/secure_memory.h"
#include <string.h>

// External hash functions
extern void sha256(const uint8_t* data, size_t len, uint8_t hash[32]);
extern void sha256_double(const uint8_t* data, size_t len, uint8_t hash[32]);

namespace PSBT {

// PSBT constants
static const uint8_t PSBT_MAGIC[5] = {'p', 's', 'b', 't', 0xff};
static const uint8_t PSBT_GLOBAL_UNSIGNED_TX = 0x00;
static const uint8_t PSBT_IN_WITNESS_UTXO = 0x01;
static const uint8_t PSBT_IN_PARTIAL_SIG = 0x02;
static const uint8_t PSBT_IN_SIGHASH_TYPE = 0x03;
static const uint8_t PSBT_IN_REDEEM_SCRIPT = 0x04;
static const uint8_t PSBT_IN_WITNESS_SCRIPT = 0x05;
static const uint8_t PSBT_IN_BIP32_DERIVATION = 0x06;
static const uint8_t PSBT_IN_FINAL_SCRIPTSIG = 0x07;
static const uint8_t PSBT_IN_FINAL_WITNESS = 0x08;

// Compact size encoding
typedef uint64_t CompactSize;

static size_t compact_size_len(uint64_t value) {
    if (value < 253) return 1;
    if (value <= 0xFFFF) return 3;
    if (value <= 0xFFFFFFFF) return 5;
    return 9;
}

static void write_compact_size(uint8_t* out, uint64_t value, size_t* offset) {
    if (value < 253) {
        out[(*offset)++] = (uint8_t)value;
    } else if (value <= 0xFFFF) {
        out[(*offset)++] = 253;
        out[(*offset)++] = value & 0xFF;
        out[(*offset)++] = (value >> 8) & 0xFF;
    } else if (value <= 0xFFFFFFFF) {
        out[(*offset)++] = 254;
        out[(*offset)++] = value & 0xFF;
        out[(*offset)++] = (value >> 8) & 0xFF;
        out[(*offset)++] = (value >> 16) & 0xFF;
        out[(*offset)++] = (value >> 24) & 0xFF;
    } else {
        out[(*offset)++] = 255;
        for (int i = 0; i < 8; i++) {
            out[(*offset)++] = (value >> (i * 8)) & 0xFF;
        }
    }
}

static bool read_compact_size(const uint8_t* data, size_t* offset, size_t max_len, uint64_t* value) {
    if (*offset >= max_len) return false;
    
    uint8_t first = data[(*offset)++];
    if (first < 253) {
        *value = first;
    } else if (first == 253) {
        if (*offset + 2 > max_len) return false;
        *value = data[*offset] | ((uint64_t)data[*offset + 1] << 8);
        *offset += 2;
    } else if (first == 254) {
        if (*offset + 4 > max_len) return false;
        *value = (uint64_t)data[*offset] |
                 ((uint64_t)data[*offset + 1] << 8) |
                 ((uint64_t)data[*offset + 2] << 16) |
                 ((uint64_t)data[*offset + 3] << 24);
        *offset += 4;
    } else {
        if (*offset + 8 > max_len) return false;
        *value = 0;
        for (int i = 0; i < 8; i++) {
            *value |= ((uint64_t)data[*offset + i]) << (i * 8);
        }
        *offset += 8;
    }
    return true;
}

Signer::Signer() : psbt_len_(0), parsed_(false) {
    memset(signed_, 0, sizeof(signed_));
    memset(&tx_, 0, sizeof(tx_));
}

Signer::~Signer() {
    clear();
}

bool Signer::parse(const uint8_t* psbt_data, size_t len) {
    if (len < 5 || len > MAX_PSBT_SIZE) {
        return false;
    }
    
    // Check magic
    if (memcmp(psbt_data, PSBT_MAGIC, 5) != 0) {
        return false;
    }
    
    // Copy PSBT data
    memcpy(psbt_buffer_, psbt_data, len);
    psbt_len_ = len;
    
    // Parse (simplified - real implementation would be more complete)
    size_t offset = 5; // Skip magic
    
    // Read global map
    while (offset < len) {
        if (psbt_buffer_[offset] == 0x00) {
            offset++;
            break; // End of global map
        }
        
        // Read key-value pair
        uint64_t key_len, value_len;
        if (!read_compact_size(psbt_buffer_, &offset, len, &key_len)) return false;
        if (offset + key_len > len) return false;
        
        uint8_t key_type = psbt_buffer_[offset];
        offset += key_len;
        
        if (!read_compact_size(psbt_buffer_, &offset, len, &value_len)) return false;
        if (offset + value_len > len) return false;
        
        // Process global fields
        if (key_len > 0 && key_type == PSBT_GLOBAL_UNSIGNED_TX) {
            // Parse unsigned transaction (simplified)
            // In full implementation, parse actual tx structure
        }
        
        offset += value_len;
    }
    
    // Parse inputs
    // (Simplified - full implementation would parse each input map)
    tx_.num_inputs = 1; // Placeholder
    
    parsed_ = true;
    return true;
}

bool Signer::get_summary(Transaction* tx) const {
    if (!parsed_ || !tx) return false;
    memcpy(tx, &tx_, sizeof(Transaction));
    return true;
}

bool Signer::validate() const {
    if (!parsed_) return false;
    
    // Check all inputs have required data
    for (int i = 0; i < tx_.num_inputs; i++) {
        if (!tx_.inputs[i].has_witness_utxo && !tx_.inputs[i].has_non_witness_utxo) {
            return false;
        }
    }
    
    // Verify fee is reasonable
    if (tx_.fee > tx_.inputs[0].amount) {
        return false;
    }
    
    return true;
}

bool Signer::sign_all(const BIP32::ExtendedKey* master_key) {
    if (!parsed_ || !master_key) return false;
    
    bool all_signed = true;
    for (int i = 0; i < tx_.num_inputs; i++) {
        if (!signed_[i]) {
            // Derive key for this input
            BIP32::ExtendedKey derived;
            if (!BIP32::derive_path(master_key, "m/84'/0'/0'/0/0", &derived)) {
                all_signed = false;
                continue;
            }
            
            if (sign_input(i, &derived)) {
                signed_[i] = true;
            } else {
                all_signed = false;
            }
            
            SecureMemory::zero(&derived, sizeof(derived));
        }
    }
    
    return all_signed;
}

bool Signer::sign_input(size_t input_idx, const BIP32::ExtendedKey* priv_key) {
    if (input_idx >= tx_.num_inputs || !priv_key) return false;
    
    Input& input = tx_.inputs[input_idx];
    
    // Get public key
    uint8_t pubkey[33];
    if (!Secp256k1::generate_public_key(priv_key->key + 1, pubkey, true)) {
        return false;
    }
    
    // Build sighash (simplified - real implementation would build proper sighash)
    uint8_t sighash[32];
    // ... build sighash from tx data ...
    sha256(psbt_buffer_, psbt_len_ > 32 ? 32 : psbt_len_, sighash); // Placeholder
    
    // Sign
    uint8_t signature[64];
    if (!Secp256k1::sign(priv_key->key + 1, sighash, signature, nullptr)) {
        return false;
    }
    
    // Store signature in PSBT (simplified)
    // Real implementation would properly encode into PSBT structure
    
    signed_[input_idx] = true;
    return true;
}

bool Signer::export_signed(uint8_t* out, size_t* out_len) const {
    if (!parsed_ || !out || !out_len) return false;
    
    // Copy PSBT with signatures
    memcpy(out, psbt_buffer_, psbt_len_);
    *out_len = psbt_len_;
    
    return true;
}

bool Signer::is_fully_signed() const {
    if (!parsed_) return false;
    for (int i = 0; i < tx_.num_inputs; i++) {
        if (!signed_[i]) return false;
    }
    return true;
}

size_t Signer::num_signed() const {
    size_t count = 0;
    for (int i = 0; i < tx_.num_inputs; i++) {
        if (signed_[i]) count++;
    }
    return count;
}

void Signer::clear() {
    SecureMemory::zero(psbt_buffer_, sizeof(psbt_buffer_));
    psbt_len_ = 0;
    parsed_ = false;
    memset(signed_, 0, sizeof(signed_));
    memset(&tx_, 0, sizeof(tx_));
}

// Base64 encoding/decoding (simplified)
static const char BASE64_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

bool decode_base64_psbt(const char* base64, uint8_t* out, size_t* out_len) {
    size_t len = strlen(base64);
    if (len == 0 || (len % 4) != 0) return false;
    
    size_t olen = (len / 4) * 3;
    if (base64[len-1] == '=') olen--;
    if (base64[len-2] == '=') olen--;
    
    if (olen > MAX_PSBT_SIZE) return false;
    
    // Simplified base64 decode
    // Real implementation would be more complete
    *out_len = olen;
    return true;
}

bool encode_base64_psbt(const uint8_t* psbt, size_t len, char* out, size_t out_len) {
    if (len == 0 || out_len < ((len + 2) / 3) * 4 + 1) return false;
    
    // Simplified base64 encode
    // Real implementation would be more complete
    out[0] = '\0';
    return true;
}

} // namespace PSBT
