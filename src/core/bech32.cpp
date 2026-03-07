/**
 * @file bech32.cpp
 * Bech32 encoding for Bitcoin addresses (BIP173)
 * 
 * Supports:
 * - Bech32: bc1q... (P2WPKH, P2WSH)
 * - Bech32m: bc1p... (P2TR - Taproot)
 */

#include "core/bech32.h"
#include <string.h>
#include <ctype.h>

namespace SeedSigner {
namespace Core {

// Bech32 charset
static const char BECH32_CHARSET[] = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

// Bech32 generator coefficients
static const uint32_t BECH32_GENERATOR[] = {
    0x3b6a57b2, 0x26508e6d, 0x1ea119fa, 0x3d4233dd, 0x2a1462b3
};

// Bech32m generator coefficients (for Taproot)
static const uint32_t BECH32M_GENERATOR[] = {
    0x0762a747, 0x0b61130f, 0x12a030a5, 0x25b92041, 0x2a2462b3
};

// Convert char to Bech32 value (5 bits)
static int8_t bech32_char_to_val(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= '0' && c <= '9') return c - '0' + 26;
    return -1;
}

// Compute Bech32 checksum polynomial
static uint32_t bech32_polymod(const uint8_t* values, size_t len) {
    uint32_t chk = 1;
    for (size_t i = 0; i < len; i++) {
        uint8_t b = chk >> 25;
        chk = ((chk & 0x1ffffff) << 5) ^ values[i];
        for (size_t j = 0; j < 5; j++) {
            chk ^= ((b >> j) & 1) ? BECH32_GENERATOR[j] : 0;
        }
    }
    return chk;
}

// Compute Bech32m checksum polynomial (for Taproot)
static uint32_t bech32m_polymod(const uint8_t* values, size_t len) {
    uint32_t chk = 1;
    for (size_t i = 0; i < len; i++) {
        uint8_t b = chk >> 25;
        chk = ((chk & 0x1ffffff) << 5) ^ values[i];
        for (size_t j = 0; j < 5; j++) {
            chk ^= ((b >> j) & 1) ? BECH32M_GENERATOR[j] : 0;
        }
    }
    return chk;
}

// Expand HRP for checksum computation
static void bech32_hrp_expand(const char* hrp, uint8_t* output) {
    size_t hrp_len = strlen(hrp);
    for (size_t i = 0; i < hrp_len; i++) {
        output[i] = hrp[i] >> 5;
    }
    output[hrp_len] = 0;
    for (size_t i = 0; i < hrp_len; i++) {
        output[hrp_len + 1 + i] = hrp[i] & 0x1f;
    }
}

// Verify checksum
static int bech32_verify_checksum(const char* hrp, const uint8_t* data, size_t data_len) {
    size_t hrp_len = strlen(hrp);
    uint8_t values[hrp_len * 2 + 3 + data_len];
    
    bech32_hrp_expand(hrp, values);
    memcpy(values + hrp_len * 2 + 1, data, data_len);
    
    uint32_t polymod = bech32_polymod(values, hrp_len * 2 + 1 + data_len);
    return polymod == 1 ? BECH32_ENCODING_BECH32 : (polymod == 0x2bc830a3 ? BECH32_ENCODING_BECH32M : -1);
}

// Create checksum
static void bech32_create_checksum(const char* hrp, const uint8_t* data, size_t data_len, 
                                    uint8_t* checksum, Bech32Encoding encoding) {
    size_t hrp_len = strlen(hrp);
    uint8_t values[hrp_len * 2 + 3 + data_len + 6];
    
    bech32_hrp_expand(hrp, values);
    memcpy(values + hrp_len * 2 + 1, data, data_len);
    memset(values + hrp_len * 2 + 1 + data_len, 0, 6);
    
    uint32_t polymod;
    if (encoding == BECH32_ENCODING_BECH32M) {
        polymod = bech32m_polymod(values, hrp_len * 2 + 1 + data_len + 6) ^ 1;
    } else {
        polymod = bech32_polymod(values, hrp_len * 2 + 1 + data_len + 6) ^ 1;
    }
    
    for (size_t i = 0; i < 6; i++) {
        checksum[i] = (polymod >> (5 * (5 - i))) & 0x1f;
    }
}

// Encode Bech32 address
bool Bech32::encode(char* output, size_t output_len,
                    const char* hrp,
                    const uint8_t* data, size_t data_len,
                    Bech32Encoding encoding) {
    if (!output || !hrp || !data) return false;
    
    size_t hrp_len = strlen(hrp);
    size_t total_len = hrp_len + 1 + data_len + 6;  // HRP + '1' + data + checksum
    
    if (output_len < total_len + 1) return false;
    
    // Check HRP validity (must be lowercase)
    for (size_t i = 0; i < hrp_len; i++) {
        if (hrp[i] < 33 || hrp[i] > 126) return false;
    }
    
    // Create checksum
    uint8_t checksum[6];
    bech32_create_checksum(hrp, data, data_len, checksum, encoding);
    
    // Build output
    strcpy(output, hrp);
    output[hrp_len] = '1';
    
    for (size_t i = 0; i < data_len; i++) {
        if (data[i] >= 32) return false;  // 5-bit values only
        output[hrp_len + 1 + i] = BECH32_CHARSET[data[i]];
    }
    
    for (size_t i = 0; i < 6; i++) {
        output[hrp_len + 1 + data_len + i] = BECH32_CHARSET[checksum[i]];
    }
    
    output[total_len] = '\0';
    return true;
}

// Decode Bech32 address
bool Bech32::decode(const char* input,
                    char* hrp_out, size_t hrp_len,
                    uint8_t* data_out, size_t* data_len,
                    Bech32Encoding* encoding_out) {
    if (!input || !hrp_out || !data_out || !data_len) return false;
    
    size_t input_len = strlen(input);
    if (input_len < 8) return false;  // Minimum: "a1qqqqqq"
    
    // Find separator '1'
    const char* sep = strchr(input, '1');
    if (!sep) return false;
    
    size_t hrp_len_actual = sep - input;
    if (hrp_len_actual >= hrp_len) return false;
    
    // Copy HRP
    strncpy(hrp_out, input, hrp_len_actual);
    hrp_out[hrp_len_actual] = '\0';
    
    // Convert to lowercase
    for (size_t i = 0; i < hrp_len_actual; i++) {
        hrp_out[i] = tolower(hrp_out[i]);
    }
    
    // Parse data part
    size_t data_part_len = input_len - hrp_len_actual - 1;
    if (data_part_len < 6) return false;  // At least checksum
    
    uint8_t data[data_part_len];
    for (size_t i = 0; i < data_part_len; i++) {
        char c = tolower(input[hrp_len_actual + 1 + i]);
        int val = bech32_char_to_val(c);
        if (val < 0) return false;
        data[i] = (uint8_t)val;
    }
    
    // Verify checksum
    int encoding = bech32_verify_checksum(hrp_out, data, data_part_len);
    if (encoding < 0) return false;
    
    if (encoding_out) *encoding_out = (Bech32Encoding)encoding;
    
    // Copy data (excluding checksum)
    *data_len = data_part_len - 6;
    memcpy(data_out, data, *data_len);
    
    return true;
}

// Convert bits (8-bit to 5-bit or vice versa)
bool Bech32::convert_bits(uint8_t* out, size_t* outlen, int outbits,
                          const uint8_t* in, size_t inlen, int inbits,
                          bool pad) {
    uint32_t val = 0;
    int bits = 0;
    size_t out_idx = 0;
    size_t max_out = *outlen;
    
    for (size_t i = 0; i < inlen; i++) {
        val = (val << inbits) | in[i];
        bits += inbits;
        
        while (bits >= outbits) {
            if (out_idx >= max_out) return false;
            bits -= outbits;
            out[out_idx++] = (val >> bits) & ((1 << outbits) - 1);
        }
    }
    
    if (pad && bits > 0) {
        if (out_idx >= max_out) return false;
        out[out_idx++] = (val << (outbits - bits)) & ((1 << outbits) - 1);
    }
    
    *outlen = out_idx;
    return true;
}

// Create SegWit address (P2WPKH, P2WSH, or P2TR)
bool Bech32::create_segwit_address(char* output, size_t output_len,
                                    uint8_t witness_version,
                                    const uint8_t* witness_program, size_t program_len,
                                    bool testnet) {
    if (!output || !witness_program) return false;
    
    const char* hrp = testnet ? "tb" : "bc";
    
    // Data = witness version + converted program
    uint8_t data[65];  // Max: version (1) + 64 bytes program (converted)
    size_t data_len = 0;
    
    // Witness version (0-16)
    data[0] = witness_version;
    
    // Convert witness program from 8 bits to 5 bits
    uint8_t converted[65];
    size_t converted_len = sizeof(converted);
    if (!convert_bits(converted, &converted_len, 5, witness_program, program_len, 8, true)) {
        return false;
    }
    
    memcpy(data + 1, converted, converted_len);
    data_len = 1 + converted_len;
    
    // Use Bech32m for Taproot (v1), Bech32 for v0
    Bech32Encoding encoding = (witness_version == 0) ? BECH32_ENCODING_BECH32 : BECH32_ENCODING_BECH32M;
    
    return encode(output, output_len, hrp, data, data_len, encoding);
}

// Decode SegWit address
bool Bech32::decode_segwit_address(const char* address,
                                    uint8_t* witness_version_out,
                                    uint8_t* witness_program_out, size_t* program_len,
                                    bool* testnet_out) {
    if (!address || !witness_version_out || !witness_program_out || !program_len) return false;
    
    char hrp[5];
    uint8_t data[65];
    size_t data_len;
    Bech32Encoding encoding;
    
    if (!decode(address, hrp, sizeof(hrp), data, &data_len, &encoding)) {
        return false;
    }
    
    // Check HRP
    bool testnet = false;
    if (strcmp(hrp, "bc") == 0) {
        testnet = false;
    } else if (strcmp(hrp, "tb") == 0) {
        testnet = true;
    } else {
        return false;
    }
    
    if (testnet_out) *testnet_out = testnet;
    
    // Minimum: version (1 byte converted = 2 bytes) + checksum
    if (data_len < 1) return false;
    
    // Witness version
    *witness_version_out = data[0];
    if (*witness_version_out > 16) return false;
    
    // Check encoding matches version
    if (*witness_version_out == 0 && encoding != BECH32_ENCODING_BECH32) return false;
    if (*witness_version_out == 1 && encoding != BECH32_ENCODING_BECH32M) return false;
    
    // Convert witness program from 5 bits to 8 bits
    return convert_bits(witness_program_out, program_len, 8, data + 1, data_len - 1, 5, false);
}

// Self-test with known vectors
bool Bech32::self_test() {
    // Test vector from BIP173
    const char* test_address = "bc1qar0srrr7xfkvy5l643lydnw9re59gtzzwf5mdq";
    
    uint8_t witness_version;
    uint8_t witness_program[40];
    size_t program_len;
    bool testnet;
    
    if (!decode_segwit_address(test_address, &witness_version, witness_program, &program_len, &testnet)) {
        return false;
    }
    
    // Verify it's v0, mainnet
    if (witness_version != 0) return false;
    if (testnet) return false;
    
    // Re-encode and verify
    char reencoded[100];
    if (!create_segwit_address(reencoded, sizeof(reencoded), witness_version, 
                               witness_program, program_len, testnet)) {
        return false;
    }
    
    return strcmp(reencoded, test_address) == 0;
}

} // namespace Core
} // namespace SeedSigner
