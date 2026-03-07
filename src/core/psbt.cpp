/**
 * @file psbt.cpp
 * PSBT (BIP174) implementation for transaction signing
 */

#include "core/psbt.h"
#include "core/secp256k1_wrapper.h"
#include <string.h>
#include <mbedtls/sha256.h>

namespace SeedSigner {
namespace Core {

// Base64 alphabet
static const char BASE64_CHARS[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

PSBTProcessor::PSBTProcessor() {}

PSBTProcessor::~PSBTProcessor() {}

bool PSBTProcessor::parse(const uint8_t* data, size_t len, PSBT* psbt) {
    if (!data || !psbt || len < 5) return false;
    
    memset(psbt, 0, sizeof(PSBT));
    
    // Check magic bytes: 0x70736274 ("psbt")
    if (data[0] != 0x70 || data[1] != 0x73 || 
        data[2] != 0x62 || data[3] != 0x74) {
        return false;
    }
    
    psbt->version = data[4];
    
    // Parse global map (simplified - full implementation needed for production)
    size_t offset = 5;
    
    // Skip global map for now and look for inputs
    // In production, properly parse key-value pairs
    
    // For now, mark as invalid (needs full parser)
    psbt->is_valid = false;
    
    return false;  // Not yet fully implemented
}

bool PSBTProcessor::parse_base64(const char* base64, PSBT* psbt) {
    if (!base64 || !psbt) return false;
    
    size_t base64_len = strlen(base64);
    uint8_t* binary = (uint8_t*)malloc(base64_len);
    if (!binary) return false;
    
    size_t binary_len = base64_decode(base64, binary, base64_len);
    bool result = parse(binary, binary_len, psbt);
    
    free(binary);
    return result;
}

bool PSBTProcessor::sign(PSBT* psbt, const ExtendedKey* key) {
    if (!psbt || !key || !key->is_private) return false;
    
    Secp256k1 secp;
    if (!secp.init()) return false;
    
    // Sign each input that has a BIP32 derivation path
    for (int i = 0; i < psbt->num_inputs; i++) {
        PSBTInput& input = psbt->inputs[i];
        
        if (!input.has_bip32_derivation) continue;
        
        // Compute sighash (simplified)
        uint8_t sighash[32];
        // TODO: Compute proper sighash based on input type
        memset(sighash, 0, 32);
        
        // Sign
        uint8_t sig[64];
        uint8_t recid;
        if (secp.sign(key->key + 1, sighash, sig, &recid)) {
            memcpy(input.partial_sig, sig, 64);
            input.has_sig = true;
        }
    }
    
    secp.deinit();
    return true;
}

bool PSBTProcessor::serialize(const PSBT* psbt, uint8_t* output, size_t* output_len) {
    // TODO: Implement full serialization
    (void)psbt;
    (void)output;
    (void)output_len;
    return false;
}

bool PSBTProcessor::serialize_base64(const PSBT* psbt, char* output, size_t output_len) {
    if (!psbt || !output || output_len == 0) return false;
    
    uint8_t binary[4096];
    size_t binary_len = sizeof(binary);
    
    if (!serialize(psbt, binary, &binary_len)) return false;
    
    size_t encoded = base64_encode(binary, binary_len, output, output_len);
    return encoded > 0;
}

void PSBTProcessor::get_summary(const PSBT* psbt, char* output, size_t output_len) {
    if (!psbt || !output || output_len == 0) return;
    
    char buf[256];
    snprintf(buf, sizeof(buf), 
             "Inputs: %d, Outputs: %d\n"
             "Total Input: %llu sat\n"
             "Total Output: %llu sat\n"
             "Fee: %llu sat",
             psbt->num_inputs, psbt->num_outputs,
             psbt->total_input, psbt->total_output, psbt->fee);
    
    strncpy(output, buf, output_len - 1);
    output[output_len - 1] = '\0';
}

bool PSBTProcessor::validate_change(const PSBT* psbt, const ExtendedKey* xpub) {
    // TODO: Implement change address validation
    (void)psbt;
    (void)xpub;
    return true;  // Simplified
}

size_t PSBTProcessor::read_varint(const uint8_t* data, size_t offset, uint64_t* value) {
    if (!data || !value) return 0;
    
    uint8_t first = data[offset];
    if (first < 0xFD) {
        *value = first;
        return 1;
    } else if (first == 0xFD) {
        *value = data[offset + 1] | ((uint64_t)data[offset + 2] << 8);
        return 3;
    } else if (first == 0xFE) {
        *value = data[offset + 1] | ((uint64_t)data[offset + 2] << 8) |
                 ((uint64_t)data[offset + 3] << 16) | ((uint64_t)data[offset + 4] << 24);
        return 5;
    } else {
        *value = ((uint64_t)data[offset + 1]) | ((uint64_t)data[offset + 2] << 8) |
                 ((uint64_t)data[offset + 3] << 16) | ((uint64_t)data[offset + 4] << 24) |
                 ((uint64_t)data[offset + 5] << 32) | ((uint64_t)data[offset + 6] << 40) |
                 ((uint64_t)data[offset + 7] << 48) | ((uint64_t)data[offset + 8] << 56);
        return 9;
    }
}

size_t PSBTProcessor::write_varint(uint8_t* data, size_t offset, uint64_t value) {
    if (!data) return 0;
    
    if (value < 0xFD) {
        data[offset] = (uint8_t)value;
        return 1;
    } else if (value <= 0xFFFF) {
        data[offset] = 0xFD;
        data[offset + 1] = value & 0xFF;
        data[offset + 2] = (value >> 8) & 0xFF;
        return 3;
    } else if (value <= 0xFFFFFFFF) {
        data[offset] = 0xFE;
        data[offset + 1] = value & 0xFF;
        data[offset + 2] = (value >> 8) & 0xFF;
        data[offset + 3] = (value >> 16) & 0xFF;
        data[offset + 4] = (value >> 24) & 0xFF;
        return 5;
    } else {
        data[offset] = 0xFF;
        for (int i = 0; i < 8; i++) {
            data[offset + 1 + i] = (value >> (i * 8)) & 0xFF;
        }
        return 9;
    }
}

size_t PSBTProcessor::base64_decode(const char* input, uint8_t* output, size_t output_len) {
    if (!input || !output || output_len == 0) return 0;
    
    size_t input_len = strlen(input);
    size_t i, j;
    uint32_t a[4], b[3];
    size_t out_len = 0;
    
    for (i = 0; i < input_len; i += 4) {
        for (j = 0; j < 4; j++) {
            if (i + j < input_len) {
                char c = input[i + j];
                if (c >= 'A' && c <= 'Z') a[j] = c - 'A';
                else if (c >= 'a' && c <= 'z') a[j] = c - 'a' + 26;
                else if (c >= '0' && c <= '9') a[j] = c - '0' + 52;
                else if (c == '+') a[j] = 62;
                else if (c == '/') a[j] = 63;
                else if (c == '=') a[j] = 0;
                else return 0;  // Invalid char
            } else {
                a[j] = 0;
            }
        }
        
        b[0] = (a[0] << 2) | (a[1] >> 4);
        b[1] = (a[1] << 4) | (a[2] >> 2);
        b[2] = (a[2] << 6) | a[3];
        
        for (j = 0; j < 3 && out_len < output_len; j++) {
            output[out_len++] = b[j];
        }
    }
    
    return out_len;
}

size_t PSBTProcessor::base64_encode(const uint8_t* input, size_t input_len, 
                                    char* output, size_t output_len) {
    if (!input || !output || output_len == 0) return 0;
    
    size_t i, j;
    size_t out_len = 0;
    
    for (i = 0; i < input_len; i += 3) {
        uint8_t a[3];
        uint8_t b[4];
        
        for (j = 0; j < 3; j++) {
            a[j] = (i + j < input_len) ? input[i + j] : 0;
        }
        
        b[0] = (a[0] >> 2) & 0x3F;
        b[1] = ((a[0] << 4) | (a[1] >> 4)) & 0x3F;
        b[2] = ((a[1] << 2) | (a[2] >> 6)) & 0x3F;
        b[3] = a[2] & 0x3F;
        
        for (j = 0; j < 4 && out_len < output_len - 1; j++) {
            if (i + j * 0.75 >= input_len && j > 0) {
                output[out_len++] = '=';
            } else {
                output[out_len++] = BASE64_CHARS[b[j]];
            }
        }
    }
    
    output[out_len] = '\0';
    return out_len;
}

void PSBTProcessor::sha256(const uint8_t* data, size_t len, uint8_t hash[32]) {
    mbedtls_sha256(data, len, hash, 0);
}

void PSBTProcessor::hash256(const uint8_t* data, size_t len, uint8_t hash[32]) {
    uint8_t temp[32];
    sha256(data, len, temp);
    sha256(temp, 32, hash);
}

bool PSBTProcessor::self_test() {
    // Test Base64 encoding/decoding
    const char* test_str = "Hello World";
    char encoded[64];
    uint8_t decoded[64];
    
    size_t enc_len = base64_encode((const uint8_t*)test_str, strlen(test_str), 
                                   encoded, sizeof(encoded));
    if (enc_len == 0) return false;
    
    size_t dec_len = base64_decode(encoded, decoded, sizeof(decoded));
    if (dec_len != strlen(test_str)) return false;
    if (memcmp(decoded, test_str, strlen(test_str)) != 0) return false;
    
    return true;
}

} // namespace Core
} // namespace SeedSigner
