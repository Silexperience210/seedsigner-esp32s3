#pragma once

#include <stdint.h>
#include <stddef.h>

// GC0308 Camera Driver
// 0.3MP VGA camera for QR scanning and entropy gathering

namespace GC0308 {

// Resolution modes
enum class Resolution {
    VGA,      // 640x480
    QVGA,    // 320x240
    QQVGA    // 160x120
};

// Initialize camera
bool init(Resolution res = Resolution::QVGA);
void deinit();

// Capture frame
// buffer must be large enough for resolution (RGB565: width*height*2)
bool capture(uint8_t* buffer, size_t buffer_size);

// Capture grayscale (for QR scanning)
bool capture_gray(uint8_t* buffer, size_t buffer_size);

// Get frame for entropy extraction
// Returns pointer to internal buffer (do not free)
const uint8_t* get_frame_buffer();

// Entropy extraction from sensor noise
uint32_t extract_entropy();

// Auto exposure/gain
void set_exposure(uint16_t exposure);
void set_gain(uint8_t gain);

// Power management
void sleep();
void wake();

} // namespace GC0308
