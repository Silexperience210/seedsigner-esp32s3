#pragma once

#include <stdint.h>
#include <stddef.h>

// QR Code Generator
// Minimal implementation for Bitcoin addresses and PSBT

namespace QR {

// QR Code capacity
constexpr size_t MAX_QR_SIZE = 2953;  // Version 40-L

// Error correction levels
enum class ECC {
    LOW = 0,      // ~7%
    MEDIUM = 1,   // ~15%
    QUARTILE = 2, // ~25%
    HIGH = 3      // ~30%
};

// QR Code data
struct Code {
    uint8_t version;      // 1-40
    ECC ecc_level;
    uint8_t width;        // Size in modules (17 + 4*version)
    uint8_t modules[177][177]; // Max size (177x177 for v40)
    
    void clear() {
        version = 0;
        width = 0;
        for (int i = 0; i < 177; i++) {
            for (int j = 0; j < 177; j++) {
                modules[i][j] = 0;
            }
        }
    }
};

// Generate QR code from data
bool generate(const uint8_t* data, size_t len, Code* qr, ECC ecc = ECC::MEDIUM);
bool generate_text(const char* text, Code* qr, ECC ecc = ECC::MEDIUM);

// Render to bitmap (1-bit per pixel)
// bitmap buffer must be at least ((width + 7) / 8) * width bytes
bool render_bitmap(const Code* qr, uint8_t* bitmap, size_t bitmap_size);

// Render to LVGL image buffer
#ifdef LVGL_H
bool render_to_lvgl(const Code* qr, lv_img_dsc_t* img);
#endif

// Get minimum QR version for data length
uint8_t get_min_version(size_t data_len, ECC ecc);

// Encode Bitcoin address as QR
bool encode_address(const char* address, Code* qr);

// Encode PSBT as multi-part UR (crypto-psbt)
// Returns number of parts needed
size_t encode_psbt_multipart(const uint8_t* psbt, size_t len, 
                              Code* parts, size_t max_parts,
                              ECC ecc = ECC::MEDIUM);

// Encode xpub as QR
bool encode_xpub(const char* xpub, Code* qr);

} // namespace QR
