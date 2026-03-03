#include "core/hardware_rng.h"
#include "core/secure_memory.h"
#include <Arduino.h>
#include <esp_random.h>
#include <esp_timer.h>
#include <soc/soc_caps.h>

// External hash function
extern void sha256(const uint8_t* data, size_t len, uint8_t hash[32]);

namespace HardwareRNG {

// Entropy pool state
static struct {
    uint8_t key[32];       // PRK for HKDF
    uint8_t seed[32];      // Current seed
    uint64_t counter;      // Increment for each generation
    bool initialized;
    size_t reseed_counter;
} pool;

void init() {
    pool.initialized = false;
    pool.counter = 0;
    pool.reseed_counter = 0;
    pool_init();
}

uint32_t get_uint32() {
    uint32_t val;
    fill(reinterpret_cast<uint8_t*>(&val), sizeof(val));
    return val;
}

void fill(uint8_t* buffer, size_t len) {
    if (!pool.initialized) {
        pool_init();
    }
    
    // Reseed after 1024 generations
    if (pool.reseed_counter > 1024) {
        pool_reseed();
    }
    
    pool_get_random(buffer, len);
    pool.reseed_counter++;
}

size_t gather_entropy(uint8_t* buffer, size_t len) {
    if (len < 32) return 0;
    
    // Collect from multiple sources
    uint32_t entropy[8];
    size_t idx = 0;
    
    // Source 1: Hardware TRNG (high quality, ~256 bits)
    for (int i = 0; i < 4; i++) {
        entropy[idx++] = from_trng();
    }
    
    // Source 2: Timing jitter (medium quality, ~64 bits)
    entropy[idx++] = from_timing_jitter();
    
    // Source 3: ADC noise if available
    #ifdef CONFIG_IDF_TARGET_ESP32S3
    entropy[idx++] = from_adc_noise();
    #endif
    
    // Mix with SHA256
    uint8_t hash[32];
    sha256(reinterpret_cast<uint8_t*>(entropy), idx * 4, hash);
    
    memcpy(buffer, hash, min(len, (size_t)32));
    
    // Clear intermediate data
    SecureMemory::zero(entropy, sizeof(entropy));
    
    return 32; // Estimated 256 bits of entropy
}

uint32_t from_trng() {
    // ESP32-S3 hardware RNG
    #ifdef CONFIG_IDF_TARGET_ESP32S3
    return esp_random();
    #else
    // Fallback for other platforms
    return random(0, 0xFFFFFFFF);
    #endif
}

uint32_t from_camera_noise() {
    // Placeholder for camera entropy gathering
    // Would read from GC0308 sensor in real implementation
    uint32_t noise = 0;
    
    // Simulate noise reading
    for (int i = 0; i < 8; i++) {
        noise ^= (esp_random() & 0xFF) << (i * 4);
    }
    
    return noise;
}

uint32_t from_timing_jitter() {
    // Measure timing jitter between interrupts
    static uint64_t last_time = 0;
    uint64_t now = esp_timer_get_time();
    uint64_t diff = now - last_time;
    last_time = now;
    
    // Extract entropy from timing variation
    return static_cast<uint32_t>(diff ^ (diff >> 32));
}

uint32_t from_adc_noise() {
    // Read floating ADC pin for noise
    // On ESP32-S3, ADC1 channel 0 (GPIO1) is commonly available
    #ifdef CONFIG_IDF_TARGET_ESP32S3
    // adc1_get_raw() would be used here with proper initialization
    return esp_random() ^ micros(); // Placeholder
    #else
    return micros();
    #endif
}

bool health_check() {
    // Verify TRNG is working
    uint32_t samples[16];
    for (int i = 0; i < 16; i++) {
        samples[i] = from_trng();
    }
    
    // Check for stuck bits (all same)
    uint32_t or_val = 0;
    uint32_t and_val = 0xFFFFFFFF;
    for (int i = 0; i < 16; i++) {
        or_val |= samples[i];
        and_val &= samples[i];
    }
    
    // If all bits are 0 or 1, RNG is stuck
    if (or_val == 0 || and_val == 0xFFFFFFFF) {
        return false;
    }
    
    return true;
}

void pool_init() {
    // Gather initial entropy
    uint8_t entropy[64];
    gather_entropy(entropy, 32);
    gather_entropy(entropy + 32, 32);
    
    // Initialize pool key with entropy
    sha256(entropy, 64, pool.key);
    memcpy(pool.seed, pool.key, 32);
    
    pool.counter = 0;
    pool.initialized = true;
    pool.reseed_counter = 0;
    
    SecureMemory::zero(entropy, sizeof(entropy));
}

void pool_reseed() {
    // Add fresh entropy and re-key
    uint8_t new_entropy[32];
    gather_entropy(new_entropy, 32);
    
    // HKDF-like reseed
    uint8_t combined[64];
    memcpy(combined, pool.key, 32);
    memcpy(combined + 32, new_entropy, 32);
    
    sha256(combined, 64, pool.key);
    memcpy(pool.seed, pool.key, 32);
    pool.counter = 0;
    pool.reseed_counter = 0;
    
    SecureMemory::zero(new_entropy, sizeof(new_entropy));
    SecureMemory::zero(combined, sizeof(combined));
}

void pool_get_random(uint8_t* out, size_t len) {
    // CTR-DRBG-like generation
    size_t offset = 0;
    
    while (offset < len) {
        // Increment counter
        for (int i = 0; i < 8; i++) {
            if (++pool.counter) break;
        }
        
        // Generate block: SHA256(seed || counter)
        uint8_t block_input[40]; // 32 + 8
        memcpy(block_input, pool.seed, 32);
        memcpy(block_input + 32, &pool.counter, 8);
        
        uint8_t block[32];
        sha256(block_input, 40, block);
        
        // Copy to output
        size_t to_copy = min(len - offset, (size_t)32);
        memcpy(out + offset, block, to_copy);
        offset += to_copy;
        
        SecureMemory::zero(block, sizeof(block));
        SecureMemory::zero(block_input, sizeof(block_input));
    }
}

} // namespace HardwareRNG
