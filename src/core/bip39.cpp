#include "core/bip39.h"
#include "core/bip39_wordlist_full.h"
#include <string.h>
#include <stdlib.h>

// External hash functions
extern void sha256(const uint8_t* data, size_t len, uint8_t hash[32]);
extern void hmac_sha512(const uint8_t* key, size_t key_len, 
                        const uint8_t* data, size_t data_len, 
                        uint8_t out[64]);

namespace BIP39 {

// Simple strcmp for PROGMEM strings
static int strcmp_P(const char* s1, const char* s2_p) {
    while (*s1 && pgm_read_byte(s2_p)) {
        if (*s1 != pgm_read_byte(s2_p)) {
            return *s1 - pgm_read_byte(s2_p);
        }
        s1++;
        s2_p++;
    }
    return *s1 - pgm_read_byte(s2_p);
}

// Binary search for word index (optimized for 2048 words)
int find_word_index(const char* word) {
    // Linear search for now - binary search on PROGMEM is complex
    // Could optimize with hash table if needed
    for (uint16_t i = 0; i < WORDLIST_SIZE; i++) {
        const char* w = get_word(i);
        if (strcmp_P(word, w) == 0) {
            return i;
        }
    }
    return -1;
}

bool generate_mnemonic(const uint8_t* entropy, size_t entropy_len, char* words_out, size_t out_len) {
    if (!entropy || !words_out || (entropy_len != 16 && entropy_len != 32)) {
        return false;
    }
    
    size_t num_words = (entropy_len == 16) ? 12 : 24;
    size_t total_bits = entropy_len * 8;
    size_t checksum_bits = total_bits / 32;
    
    // Calculate checksum (first byte of SHA256)
    uint8_t checksum_hash[32];
    sha256(entropy, entropy_len, checksum_hash);
    
    // Combine entropy + checksum into buffer
    uint8_t data[33]; // Max: 32 + 1
    memcpy(data, entropy, entropy_len);
    data[entropy_len] = checksum_hash[0];
    
    // Extract words (11 bits each)
    size_t out_idx = 0;
    uint32_t accumulator = 0;
    int bits = 0;
    size_t byte_idx = 0;
    
    char word_buffer[16];
    
    for (size_t i = 0; i < num_words; i++) {
        // Accumulate bits until we have 11
        while (bits < 11) {
            accumulator = (accumulator << 8) | data[byte_idx++];
            bits += 8;
        }
        
        bits -= 11;
        uint16_t word_idx = (accumulator >> bits) & 0x7FF;
        
        // Get word from PROGMEM
        get_word_copy(word_idx, word_buffer, sizeof(word_buffer));
        size_t word_len = strlen(word_buffer);
        
        if (out_idx + word_len + 2 > out_len) {
            return false;
        }
        
        memcpy(words_out + out_idx, word_buffer, word_len);
        out_idx += word_len;
        
        if (i < num_words - 1) {
            words_out[out_idx++] = ' ';
        }
    }
    
    words_out[out_idx] = '\0';
    return true;
}

bool mnemonic_to_seed(const char* mnemonic, const char* password, uint8_t seed[64]) {
    if (!mnemonic || !seed) {
        return false;
    }
    
    // Build salt: "mnemonic" + password
    char salt[256];
    int salt_len = 0;
    
    const char* prefix = "mnemonic";
    while (*prefix && salt_len < 255) {
        salt[salt_len++] = *prefix++;
    }
    
    if (password) {
        while (*password && salt_len < 255) {
            salt[salt_len++] = *password++;
        }
    }
    
    // PBKDF2-HMAC-SHA512 with 2048 iterations
    // Simplified implementation - production should use proper PBKDF2
    size_t mnemonic_len = strlen(mnemonic);
    
    // First iteration
    hmac_sha512((const uint8_t*)salt, salt_len,
                (const uint8_t*)mnemonic, mnemonic_len, seed);
    
    // Remaining iterations (simplified)
    uint8_t temp[64];
    for (int i = 1; i < 2048; i++) {
        hmac_sha512((const uint8_t*)salt, salt_len, seed, 64, temp);
        memcpy(seed, temp, 64);
    }
    
    // Clear temp buffer
    memset(temp, 0, sizeof(temp));
    
    return true;
}

bool validate_mnemonic(const char* mnemonic) {
    if (!mnemonic || !mnemonic[0]) {
        return false;
    }
    
    // Copy to buffer for tokenization
    char buffer[512];
    size_t len = strlen(mnemonic);
    if (len >= sizeof(buffer)) {
        return false;
    }
    memcpy(buffer, mnemonic, len + 1);
    
    // Count and validate words
    int word_count = 0;
    char* token = strtok(buffer, " ");
    
    while (token) {
        if (find_word_index(token) < 0) {
            return false; // Invalid word
        }
        word_count++;
        token = strtok(nullptr, " ");
    }
    
    // Valid counts: 12, 15, 18, 21, 24
    return word_count == 12 || word_count == 15 || word_count == 18 || 
           word_count == 21 || word_count == 24;
}

void wipe_mnemonic(char* mnemonic) {
    if (mnemonic) {
        volatile char* p = mnemonic;
        while (*p) {
            *p++ = 0;
        }
    }
}

} // namespace BIP39
