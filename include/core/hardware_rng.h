#pragma once

#include <stdint.h>
#include <stddef.h>
#include <esp_random.h>

// Hardware RNG for ESP32-S3
// Uses TRNG + camera noise + timing jitter

namespace HardwareRNG {

// Initialize RNG subsystem
void init();

// Get 32 bits of hardware entropy
uint32_t get_uint32();

// Fill buffer with hardware entropy
void fill(uint8_t* buffer, size_t len);

// Gather entropy from multiple sources
// Returns estimated entropy bits
size_t gather_entropy(uint8_t* buffer, size_t len);

// Entropy sources
uint32_t from_trng();           // ESP32-S3 hardware TRNG
uint32_t from_camera_noise();   // GC0308 sensor noise
uint32_t from_timing_jitter();  // Interrupt timing
uint32_t from_adc_noise();      // ADC floating pin

// Health check
bool health_check();

// Entropy pool management (HKDF-based)
void pool_init();
void pool_reseed();
void pool_get_random(uint8_t* out, size_t len);

} // namespace HardwareRNG
