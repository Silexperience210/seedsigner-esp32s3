/**
 * @file bech32.h
 * Bech32 encoding for Bitcoin addresses (BIP173, BIP350)
 */

#ifndef CORE_BECH32_H
#define CORE_BECH32_H

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

namespace SeedSigner {
namespace Core {

// Bech32 encoding variants
enum Bech32Encoding {
    BECH32_ENCODING_BECH32 = 0,    // BIP173 - For v0 SegWit
    BECH32_ENCODING_BECH32M = 1    // BIP350 - For v1+ SegWit (Taproot)
};

// Bech32 codec class
class Bech32 {
public:
    /**
     * Encode data to Bech32 string
     * @param output Output buffer
     * @param output_len Output buffer size
     * @param hrp Human-readable part (e.g., "bc", "tb")
     * @param data 5-bit data values
     * @param data_len Number of data values
     * @param encoding BECH32 or BECH32M
     * @return true on success
     */
    static bool encode(char* output, size_t output_len,
                       const char* hrp,
                       const uint8_t* data, size_t data_len,
                       Bech32Encoding encoding = BECH32_ENCODING_BECH32);
    
    /**
     * Decode Bech32 string
     * @param input Bech32 encoded string
     * @param hrp_out Output buffer for HRP
     * @param hrp_len Size of HRP buffer
     * @param data_out Output buffer for 5-bit data
     * @param data_len Output: number of data values (excluding checksum)
     * @param encoding_out Output: encoding type detected
     * @return true on success
     */
    static bool decode(const char* input,
                       char* hrp_out, size_t hrp_len,
                       uint8_t* data_out, size_t* data_len,
                       Bech32Encoding* encoding_out = nullptr);
    
    /**
     * Convert between bit widths
     * @param out Output buffer
     * @param outlen Output: number of output values
     * @param outbits Output bit width (5 or 8)
     * @param in Input buffer
     * @param inlen Input length
     * @param inbits Input bit width (5 or 8)
     * @param pad Whether to pad with zeros
     * @return true on success
     */
    static bool convert_bits(uint8_t* out, size_t* outlen, int outbits,
                             const uint8_t* in, size_t inlen, int inbits,
                             bool pad);
    
    /**
     * Create SegWit address
     * @param output Output buffer for address
     * @param output_len Output buffer size
     * @param witness_version Witness version (0-16)
     * @param witness_program Witness program (script hash or key hash)
     * @param program_len Length of witness program
     * @param testnet Use testnet (tb) instead of mainnet (bc)
     * @return true on success
     */
    static bool create_segwit_address(char* output, size_t output_len,
                                       uint8_t witness_version,
                                       const uint8_t* witness_program, size_t program_len,
                                       bool testnet = false);
    
    /**
     * Decode SegWit address
     * @param address Bech32 address string
     * @param witness_version_out Output: witness version
     * @param witness_program_out Output: witness program
     * @param program_len Output: length of witness program
     * @param testnet_out Output: true if testnet address
     * @return true on success
     */
    static bool decode_segwit_address(const char* address,
                                       uint8_t* witness_version_out,
                                       uint8_t* witness_program_out, size_t* program_len,
                                       bool* testnet_out = nullptr);
    
    /**
     * Self-test with known vectors
     * @return true if all tests pass
     */
    static bool self_test();
};

} // namespace Core
} // namespace SeedSigner

#endif // CORE_BECH32_H
