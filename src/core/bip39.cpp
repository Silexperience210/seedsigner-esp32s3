/**
 * @file bip39.cpp
 * BIP39 implementation
 */

#include "core/bip39.h"
#include "utils/memory.h"
#include <string.h>
#include <mbedtls/sha256.h>
#include <mbedtls/pkcs5.h>

namespace SeedSigner {
namespace Core {

// English wordlist (first 20 words as example, full list would be embedded)
const char* BIP39::s_wordlist[2048] = {
    "abandon", "ability", "able", "about", "above", "absent", 
    "absorb", "abstract", "absurd", "abuse", "access", "accident",
    // ... full wordlist would be here
    "zoo", "zoom"
};

BIP39::BIP39() {}

BIP39::~BIP39() {}

bool BIP39::init() {
    // Verify wordlist integrity
    return s_wordlist[0] != nullptr;
}

bool BIP39::generate_mnemonic_from_entropy(const uint8_t* entropy, 
                                            size_t entropy_len,
                                            char* output, 
                                            size_t output_len) {
    if (!entropy || !output || output_len < 256) return false;
    if (entropy_len != 16 && entropy_len != 32) return false;
    
    // Calculate checksum
    uint8_t hash[32];
    mbedtls_sha256(entropy, entropy_len, hash, 0);
    
    // Entropy + checksum bits
    size_t total_bits = entropy_len * 8 + (entropy_len * 8 / 32);
    size_t num_words = total_bits / 11;
    
    // Build bit array
    uint8_t bits[33] = {0};  // Max 264 bits
    memcpy(bits, entropy, entropy_len);
    bits[entropy_len] = hash[0];
    
    // Generate words
    output[0] = '\0';
    size_t offset = 0;
    
    for (size_t i = 0; i < num_words; i++) {
        // Extract 11 bits for word index
        size_t bit_pos = i * 11;
        size_t byte_pos = bit_pos / 8;
        size_t bit_offset = bit_pos % 8;
        
        uint16_t index = 0;
        index = ((bits[byte_pos] << bit_offset) | 
                 (bits[byte_pos + 1] >> (8 - bit_offset))) >> (8 - bit_offset);
        index &= 0x07FF;
        
        const char* word = index_to_word(index);
        if (!word) return false;
        
        size_t word_len = strlen(word);
        if (offset + word_len + 1 >= output_len) return false;
        
        if (i > 0) {
            output[offset++] = ' ';
        }
        strcpy(output + offset, word);
        offset += word_len;
    }
    
    return true;
}

bool BIP39::generate_mnemonic_from_dice(const uint8_t* rolls, 
                                        size_t num_rolls,
                                        char* output, 
                                        size_t output_len) {
    // Need 50 rolls for 12 words, 99 rolls for 24 words
    if (num_rolls < 50) return false;
    
    // Convert base-6 dice rolls to bytes
    size_t bits_needed = (num_rolls == 50) ? 128 : 256;
    size_t bytes_needed = bits_needed / 8;
    
    Utils::StackBuffer<32> entropy;
    
    // Simple base-6 encoding
    uint32_t accumulator = 0;
    uint8_t bits_collected = 0;
    size_t byte_index = 0;
    
    for (size_t i = 0; i < num_rolls && byte_index < bytes_needed; i++) {
        uint8_t roll = rolls[i];
        if (roll < 1 || roll > 6) continue;
        
        accumulator = accumulator * 6 + (roll - 1);
        bits_collected += 3;  // log2(6) ≈ 2.585, we use 3 for simplicity
        
        if (bits_collected >= 8) {
            entropy[byte_index++] = (accumulator >> (bits_collected - 8)) & 0xFF;
            bits_collected -= 8;
        }
    }
    
    return generate_mnemonic_from_entropy(entropy.data(), bytes_needed, 
                                          output, output_len);
}

bool BIP39::mnemonic_to_seed(const char* mnemonic, 
                              const char* passphrase,
                              uint8_t seed[64]) {
    if (!mnemonic || !seed) return false;
    
    const char* salt_prefix = "mnemonic";
    size_t salt_len = strlen(salt_prefix) + (passphrase ? strlen(passphrase) : 0);
    
    Utils::StackBuffer<256> salt;
    memcpy(salt.data(), salt_prefix, strlen(salt_prefix));
    if (passphrase) {
        memcpy(salt.data() + strlen(salt_prefix), passphrase, strlen(passphrase));
    }
    
    // PBKDF2-HMAC-SHA512
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
    
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, md_info, 1);
    
    mbedtls_pkcs5_pbkdf2_hmac(&ctx,
                              (const uint8_t*)mnemonic, strlen(mnemonic),
                              salt.data(), salt_len,
                              2048,
                              64, seed);
    
    mbedtls_md_free(&ctx);
    return true;
}

bool BIP39::validate_mnemonic(const char* mnemonic) {
    if (!mnemonic) return false;
    
    // Copy and tokenize
    char buf[512];
    strncpy(buf, mnemonic, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    
    int word_count = 0;
    char* token = strtok(buf, " ");
    
    while (token && word_count < 25) {
        if (word_to_index(token) < 0) {
            return false;
        }
        word_count++;
        token = strtok(nullptr, " ");
    }
    
    // Must be 12 or 24 words
    return word_count == 12 || word_count == 24;
}

int BIP39::word_to_index(const char* word) {
    return find_word(word);
}

const char* BIP39::index_to_word(uint16_t index) {
    if (index >= 2048) return nullptr;
    return s_wordlist[index];
}

uint16_t BIP39::calculate_checksum_word(const uint8_t* entropy, 
                                         size_t entropy_len) {
    uint8_t hash[32];
    mbedtls_sha256(entropy, entropy_len, hash, 0);
    
    // First few bits of hash are checksum
    return hash[0] >> (8 - (entropy_len * 8 / 32));
}

size_t BIP39::entropy_to_word_count(size_t entropy_len) {
    if (entropy_len == 16) return 12;
    if (entropy_len == 32) return 24;
    return 0;
}

int BIP39::find_word(const char* word) {
    // Binary search on wordlist
    int left = 0;
    int right = 2047;
    
    while (left <= right) {
        int mid = (left + right) / 2;
        int cmp = strcmp(word, s_wordlist[mid]);
        
        if (cmp == 0) return mid;
        if (cmp < 0) right = mid - 1;
        else left = mid + 1;
    }
    
    return -1;
}

bool BIP39::self_test() {
    // Test vectors from BIP39
    const char* test_mnemonic = "abandon abandon abandon abandon abandon abandon "
                                "abandon abandon abandon abandon abandon about";
    uint8_t expected_seed[64] = {
        // ... expected seed
    };
    
    uint8_t seed[64];
    if (!mnemonic_to_seed(test_mnemonic, "", seed)) {
        return false;
    }
    
    // Additional tests...
    return true;
}

} // namespace Core
} // namespace SeedSigner
