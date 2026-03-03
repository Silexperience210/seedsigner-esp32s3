#pragma once

#include <stdint.h>
#include <stddef.h>
#include "core/bip32_secp256k1.h"

// PSBT (BIP174) Transaction Signer
// Simplified implementation for air-gapped signing

namespace PSBT {

// Maximum PSBT size (100KB)
constexpr size_t MAX_PSBT_SIZE = 100 * 1024;

// Transaction input info
struct Input {
    uint8_t tx_hash[32];      // Previous output tx hash
    uint32_t vout;            // Previous output index
    uint64_t amount;          // Amount in satoshis
    uint8_t scriptPubKey[34]; // Script (max P2WSH)
    size_t scriptPubKey_len;
    
    // PSBT fields
    bool has_witness_utxo;
    bool has_non_witness_utxo;
    bool has_redeem_script;
    bool has_witness_script;
    bool has_bip32_derivation;
    
    // Key path for signing
    uint32_t bip32_path[10];
    uint8_t path_depth;
};

// Transaction output info
struct Output {
    uint64_t amount;
    uint8_t script[34];
    size_t script_len;
    char address[65];  // Human-readable address
};

// Parsed PSBT transaction
struct Transaction {
    uint8_t version;
    Input inputs[10];      // Max 10 inputs
    Output outputs[10];    // Max 10 outputs
    uint8_t num_inputs;
    uint8_t num_outputs;
    uint32_t locktime;
    uint64_t fee;
};

// Signer class
class Signer {
public:
    Signer();
    ~Signer();
    
    // Parse PSBT from bytes
    bool parse(const uint8_t* psbt_data, size_t len);
    
    // Get transaction summary for display
    bool get_summary(Transaction* tx) const;
    
    // Validate transaction (check amounts, etc)
    bool validate() const;
    
    // Sign all inputs with key derived from master
    bool sign_all(const BIP32::ExtendedKey* master_key);
    
    // Sign specific input
    bool sign_input(size_t input_idx, const BIP32::ExtendedKey* priv_key);
    
    // Export signed PSBT
    bool export_signed(uint8_t* out, size_t* out_len) const;
    
    // Get signing status
    bool is_fully_signed() const;
    size_t num_signed() const;
    
    // Clear sensitive data
    void clear();
    
private:
    uint8_t psbt_buffer_[MAX_PSBT_SIZE];
    size_t psbt_len_;
    bool parsed_;
    bool signed_[10];
    Transaction tx_;
    
    // Parse helpers
    bool parse_compact_size(const uint8_t* data, size_t* offset, uint64_t* value);
    bool parse_input(const uint8_t* data, size_t* offset, Input* input);
    bool parse_output(const uint8_t* data, size_t* offset, Output* output);
};

// Utility functions
bool decode_base64_psbt(const char* base64, uint8_t* out, size_t* out_len);
bool encode_base64_psbt(const uint8_t* psbt, size_t len, char* out, size_t out_len);

} // namespace PSBT
