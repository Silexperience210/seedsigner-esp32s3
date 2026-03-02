/**
 * @file psbt.h
 * PSBT (BIP174) Partially Signed Bitcoin Transaction handling
 */

#ifndef CORE_PSBT_H
#define CORE_PSBT_H

#include <Arduino.h>
#include <stdint.h>
#include "bip32.h"

namespace SeedSigner {
namespace Core {

// PSBT Global types
enum class PSBTGlobalType : uint8_t {
    UNSIGNED_TX = 0x00,
    XPUB = 0x01,
    VERSION = 0xFB,
    PROPRIETARY = 0xFC
};

// PSBT Input types
enum class PSBTInputType : uint8_t {
    NON_WITNESS_UTXO = 0x00,
    WITNESS_UTXO = 0x01,
    PARTIAL_SIG = 0x02,
    SIGHASH_TYPE = 0x03,
    REDEEM_SCRIPT = 0x04,
    WITNESS_SCRIPT = 0x05,
    BIP32_DERIVATION = 0x06,
    FINAL_SCRIPTSIG = 0x07,
    FINAL_SCRIPTWITNESS = 0x08,
    POR_COMMITMENT = 0x09,
    RIPEMD160 = 0x0A,
    SHA256 = 0x0B,
    HASH160 = 0x0C,
    HASH256 = 0x0D,
    PREV_TXID = 0x0E,
    PREV_VOUT = 0x0F,
    SEQUENCE = 0x10,
    REQUIRED_TIME_LOCKTIME = 0x11,
    REQUIRED_HEIGHT_LOCKTIME = 0x12,
    TAP_KEY_SIG = 0x13,
    TAP_SCRIPT_SIG = 0x14,
    TAP_LEAF_SCRIPT = 0x15,
    TAP_BIP32_DERIVATION = 0x16,
    TAP_INTERNAL_KEY = 0x17,
    TAP_MERKLE_ROOT = 0x18,
    PROPRIETARY = 0xFC
};

// PSBT Output types
enum class PSBTOutputType : uint8_t {
    REDEEM_SCRIPT = 0x00,
    WITNESS_SCRIPT = 0x01,
    BIP32_DERIVATION = 0x02,
    AMOUNT = 0x03,
    SCRIPT = 0x04,
    TAP_INTERNAL_KEY = 0x05,
    TAP_TREE = 0x06,
    TAP_BIP32_DERIVATION = 0x07,
    PROPRIETARY = 0xFC
};

// Transaction input
struct PSBTInput {
    uint8_t prev_txid[32];
    uint32_t prev_vout;
    uint64_t witness_utxo_value;
    uint8_t witness_utxo_script[128];
    size_t witness_utxo_script_len;
    uint8_t partial_sig[64];
    bool has_sig;
    uint8_t sighash_type;
    char bip32_path[128];
    bool has_bip32_derivation;
    uint8_t fingerprint[4];
    // Taproot fields
    uint8_t tap_key_sig[64];
    bool has_tap_key_sig;
};

// Transaction output
struct PSBTOutput {
    uint64_t amount;
    uint8_t script[128];
    size_t script_len;
    char bip32_path[128];
    bool has_bip32_derivation;
    uint8_t fingerprint[4];
    bool is_change;
};

// Parsed PSBT
struct PSBT {
    uint8_t version;
    uint8_t tx_version;
    uint8_t tx_locktime[4];
    
    PSBTInput inputs[10];      // Max 10 inputs
    uint8_t num_inputs;
    
    PSBTOutput outputs[10];    // Max 10 outputs
    uint8_t num_outputs;
    
    uint64_t total_input;
    uint64_t total_output;
    uint64_t fee;
    
    bool is_valid;
};

class PSBTProcessor {
public:
    PSBTProcessor();
    ~PSBTProcessor();
    
    // Parse PSBT from binary
    bool parse(const uint8_t* data, size_t len, PSBT* psbt);
    
    // Parse PSBT from base64
    bool parse_base64(const char* base64, PSBT* psbt);
    
    // Sign PSBT with given key
    bool sign(PSBT* psbt, const ExtendedKey* key);
    
    // Serialize PSBT to binary
    bool serialize(const PSBT* psbt, uint8_t* output, size_t* output_len);
    
    // Serialize to base64
    bool serialize_base64(const PSBT* psbt, char* output, size_t output_len);
    
    // Get human-readable summary
    void get_summary(const PSBT* psbt, char* output, size_t output_len);
    
    // Validate change address belongs to wallet
    bool validate_change(const PSBT* psbt, const ExtendedKey* xpub);
    
    bool self_test();
    
private:
    // Compact size unsigned integer (varint)
    size_t read_varint(const uint8_t* data, size_t offset, uint64_t* value);
    size_t write_varint(uint8_t* data, size_t offset, uint64_t value);
    
    // Base64 encoding/decoding
    size_t base64_decode(const char* input, uint8_t* output, size_t output_len);
    size_t base64_encode(const uint8_t* input, size_t input_len, 
                         char* output, size_t output_len);
    
    // Hash functions
    void sha256(const uint8_t* data, size_t len, uint8_t hash[32]);
    void hash256(const uint8_t* data, size_t len, uint8_t hash[32]);
};

} // namespace Core
} // namespace SeedSigner

#endif // CORE_PSBT_H
