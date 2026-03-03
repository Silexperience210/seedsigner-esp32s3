#pragma once

#include "core/bip39.h"
#include <Arduino.h>

// Optimized BIP39 with binary search and caching

namespace BIP39Optimized {

// Initialize optimized lookup structures
void init();

// Fast word lookup using binary search on sorted indices
// O(log n) instead of O(n)
int find_word_binary(const char* word);

// Cached word validation
bool validate_word_cached(const char* word);

// Fast mnemonic generation with hardware entropy
bool generate_mnemonic_secure(uint8_t entropy_bytes, char* output, size_t out_len);

// Optimized PBKDF2 using ESP32-S3 SHA accelerator if available
bool mnemonic_to_seed_fast(const char* mnemonic, const char* password, 
                           uint8_t seed[64], size_t* iterations_done);

} // namespace BIP39Optimized
