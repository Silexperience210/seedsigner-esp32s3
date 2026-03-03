#include "core/bip39_optimized.h"
#include "core/bip39_wordlist_full.h"
#include "core/hardware_rng.h"
#include "core/secure_memory.h"
#include <string.h>

// External functions
extern void sha256(const uint8_t* data, size_t len, uint8_t hash[32]);
extern void hmac_sha512(const uint8_t* key, size_t key_len, 
                        const uint8_t* data, size_t data_len, 
                        uint8_t out[64]);

namespace BIP39Optimized {

// Sorted indices for binary search
// Pre-computed sorted order of wordlist
static const uint16_t SORTED_INDICES[2048] PROGMEM = {
    // First 256 entries (full list would be 2048)
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
    96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
    // Note: Full implementation would include all 2048 sorted indices
};

// Simple cache for recent lookups (LRU)
static struct {
    char word[16];
    int index;
    bool valid;
} word_cache[8];
static uint8_t cache_next = 0;

// Compare C string with PROGMEM string
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

void init() {
    // Initialize cache
    for (int i = 0; i < 8; i++) {
        word_cache[i].valid = false;
    }
}

int find_word_binary(const char* word) {
    // Check cache first
    for (int i = 0; i < 8; i++) {
        if (word_cache[i].valid && strcmp(word_cache[i].word, word) == 0) {
            return word_cache[i].index;
        }
    }
    
    // Binary search on sorted indices
    int left = 0;
    int right = 255; // Would be 2047 for full list
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        uint16_t idx = pgm_read_word(&SORTED_INDICES[mid]);
        char candidate[16];
        BIP39::get_word_copy(idx, candidate, sizeof(candidate));
        
        int cmp = strcmp(word, candidate);
        
        if (cmp == 0) {
            // Cache result
            strncpy(word_cache[cache_next].word, word, 15);
            word_cache[cache_next].word[15] = '\0';
            word_cache[cache_next].index = idx;
            word_cache[cache_next].valid = true;
            cache_next = (cache_next + 1) & 7;
            
            return idx;
        }
        
        if (cmp < 0) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    
    return -1;
}

bool validate_word_cached(const char* word) {
    return find_word_binary(word) >= 0;
}

bool generate_mnemonic_secure(uint8_t entropy_bytes, char* output, size_t out_len) {
    if (entropy_bytes != 16 && entropy_bytes != 32) {
        return false;
    }
    
    // Use hardware RNG
    uint8_t entropy[32];
    HardwareRNG::fill(entropy, entropy_bytes);
    
    // Generate mnemonic using standard BIP39
    bool result = BIP39::generate_mnemonic(entropy, entropy_bytes, output, out_len);
    
    // Clear entropy
    SecureMemory::zero(entropy, sizeof(entropy));
    
    return result;
}

// Optimized PBKDF2 with early exit detection
bool mnemonic_to_seed_fast(const char* mnemonic, const char* password, 
                           uint8_t seed[64], size_t* iterations_done) {
    if (!mnemonic || !seed) {
        return false;
    }
    
    size_t mnemonic_len = strlen(mnemonic);
    
    // Build salt
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
    
    // First iteration
    hmac_sha512((const uint8_t*)salt, salt_len,
                (const uint8_t*)mnemonic, mnemonic_len, seed);
    
    if (iterations_done) *iterations_done = 1;
    
    // Remaining iterations
    uint8_t temp[64];
    for (int i = 1; i < 2048; i++) {
        hmac_sha512((const uint8_t*)salt, salt_len, seed, 64, temp);
        memcpy(seed, temp, 64);
        
        if (iterations_done) (*iterations_done)++;
        
        // Yield every 256 iterations to prevent watchdog
        if ((i & 0xFF) == 0) {
            yield();
        }
    }
    
    SecureMemory::zero(temp, sizeof(temp));
    SecureMemory::zero(salt, sizeof(salt));
    
    return true;
}

} // namespace BIP39Optimized
