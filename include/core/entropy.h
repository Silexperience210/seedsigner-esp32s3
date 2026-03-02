/**
 * @file entropy.h
 * Secure entropy collection for seed generation
 */

#ifndef CORE_ENTROPY_H
#define CORE_ENTROPY_H

#include <Arduino.h>
#include <stdint.h>

namespace SeedSigner {
namespace Core {

// Entropy sources
enum class EntropySource {
    HARDWARE_RNG = 0,    // ESP32-S3 hardware RNG
    DICE_D6,             // Physical dice rolls
    DICE_D20,            // D20 dice
    CAMERA_NOISE,        // Camera sensor noise
    TOUCH_TIMING,        // Touch screen timing
    ACCELEROMETER,       // IMU data
    COMBINED             // Multiple sources combined
};

// Entropy pool
struct EntropyPool {
    uint8_t data[64];    // 512 bits max
    size_t len;
    EntropySource sources[8];
    uint8_t num_sources;
};

class Entropy {
public:
    Entropy();
    ~Entropy();
    
    // Initialize entropy subsystem
    bool init();
    
    // Collect entropy from hardware RNG
    bool collect_hardware_rng(uint8_t* output, size_t len);
    
    // Add dice roll entropy
    // rolls: array of dice values (1-6 for D6)
    // num_rolls: number of rolls
    bool add_dice_entropy(const uint8_t* rolls, size_t num_rolls, bool is_d20 = false);
    
    // Collect entropy from camera noise
    // Requires camera driver initialized
    bool collect_camera_noise(uint8_t* output, size_t len);
    
    // Collect entropy from touch timing
    bool add_touch_timing_entropy(uint32_t timing_value);
    
    // Collect entropy from accelerometer
    bool add_accelerometer_entropy();
    
    // Get accumulated entropy
    bool get_entropy(uint8_t* output, size_t len, EntropySource source);
    
    // Mix multiple entropy sources
    bool mix_entropy(const EntropyPool* pool, uint8_t* output, size_t len);
    
    // Calculate Shannon entropy of buffer
    float calculate_shannon_entropy(const uint8_t* data, size_t len);
    
    // Estimate bits of entropy collected
    size_t estimate_entropy_bits();
    
    // Clear entropy pool
    void clear_pool();
    
    // Self-test
    bool self_test();
    
private:
    EntropyPool m_pool;
    bool m_initialized;
    
    // SHA-256 for mixing
    void sha256_mix(const uint8_t* input1, size_t len1,
                    const uint8_t* input2, size_t len2,
                    uint8_t output[32]);
    
    // Convert dice rolls to bytes (using base-6 encoding)
    void dice_to_bytes(const uint8_t* rolls, size_t num_rolls, 
                       uint8_t* output, size_t* output_len);
};

} // namespace Core
} // namespace SeedSigner

#endif // CORE_ENTROPY_H
