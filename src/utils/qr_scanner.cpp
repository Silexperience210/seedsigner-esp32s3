/**
 * @file qr_scanner.cpp
 * Production QR Code Scanner using quirc library
 * 
 * This requires the quirc library to be installed:
 * pio lib install "quirc"
 * 
 * Or add to platformio.ini:
 * lib_deps = 
 *     https://github.com/dlbeer/quirc.git
 */

#include "utils/qr_scanner.h"
#include <string.h>
#include <stdlib.h>

// Try to include quirc - if not available, use stub
#if __has_include(<quirc.h>)
    #include <quirc.h>
    #define HAS_QUIRC 1
#else
    #define HAS_QUIRC 0
    #warning "quirc library not found - QR scanner will use stub implementation"
#endif

namespace SeedSigner {
namespace Utils {

QRCodeScanner::QRCodeScanner() : m_initialized(false), m_scanning(false),
                                  m_has_result(false), m_animated_mode(false),
                                  m_expected_parts(0), m_received_parts(0),
                                  m_quirc(nullptr) {
    memset(m_result, 0, sizeof(m_result));
    memset(m_animation_buffer, 0, sizeof(m_animation_buffer));
    memset(m_part_received, 0, sizeof(m_part_received));
}

QRCodeScanner::~QRCodeScanner() {
    deinit();
}

bool QRCodeScanner::init() {
#if HAS_QUIRC
    m_quirc = quirc_new();
    if (!m_quirc) {
        return false;
    }
    
    // Resize for 320x240 (QVGA) - typical camera resolution
    if (quirc_resize(m_quirc, 320, 240) < 0) {
        quirc_destroy(m_quirc);
        m_quirc = nullptr;
        return false;
    }
#endif
    
    m_initialized = true;
    return true;
}

void QRCodeScanner::deinit() {
    stop();
    
#if HAS_QUIRC
    if (m_quirc) {
        quirc_destroy(m_quirc);
        m_quirc = nullptr;
    }
#endif
    
    m_initialized = false;
}

bool QRCodeScanner::start() {
    if (!m_initialized) return false;
    m_scanning = true;
    m_has_result = false;
    m_result[0] = '\0';
    return true;
}

bool QRCodeScanner::stop() {
    m_scanning = false;
    return true;
}

bool QRCodeScanner::is_scanning() const {
    return m_scanning;
}

bool QRCodeScanner::has_result() const {
    return m_has_result;
}

const char* QRCodeScanner::get_result() const {
    return m_has_result ? m_result : nullptr;
}

void QRCodeScanner::clear_result() {
    m_has_result = false;
    m_result[0] = '\0';
}

bool QRCodeScanner::process_frame(const uint8_t* grayscale, uint16_t width, uint16_t height) {
    if (!m_scanning || !grayscale || width == 0 || height == 0) return false;
    
#if HAS_QUIRC
    if (!m_quirc) return false;
    
    // Resize quirc buffer if needed
    int w, h;
    quirc_begin(m_quirc, &w, &h);
    
    if ((uint16_t)w != width || (uint16_t)h != height) {
        if (quirc_resize(m_quirc, width, height) < 0) {
            return false;
        }
        quirc_begin(m_quirc, &w, &h);
    }
    
    // Get image buffer
    uint8_t* image = quirc_begin(m_quirc, nullptr, nullptr);
    
    // Copy grayscale image
    memcpy(image, grayscale, width * height);
    
    quirc_end(m_quirc);
    
    // Get results
    int count = quirc_count(m_quirc);
    if (count == 0) {
        return false;
    }
    
    // Process first code found
    struct quirc_code code;
    struct quirc_data data;
    
    quirc_extract(m_quirc, 0, &code);
    
    if (quirc_decode(&code, &data) != QUIRC_SUCCESS) {
        return false;
    }
    
    // Copy result
    strncpy(m_result, (const char*)data.payload, sizeof(m_result) - 1);
    m_result[sizeof(m_result) - 1] = '\0';
    m_has_result = true;
    
    return true;
#else
    // Stub implementation - no actual QR decoding
    (void)grayscale;
    (void)width;
    (void)height;
    return false;
#endif
}

bool QRCodeScanner::start_animated_scan() {
    m_animated_mode = true;
    m_expected_parts = 0;
    m_received_parts = 0;
    memset(m_animation_buffer, 0, sizeof(m_animation_buffer));
    memset(m_part_received, 0, sizeof(m_part_received));
    return start();
}

bool QRCodeScanner::process_animated_frame() {
    if (!m_animated_mode || !m_has_result) return false;
    
    // Parse animated QR format (BIP370 UR format preferred)
    // Format: "ur:crypto-psbt/1-3/..." or "p1of3:data..."
    
    int part = 0, total = 0;
    const char* data_start = nullptr;
    
    // Try BIP370 UR format
    if (strncmp(m_result, "ur:crypto-psbt/", 15) == 0) {
        // Parse "ur:crypto-psbt/part-total/..."
        if (sscanf(m_result, "ur:crypto-psbt/%d-%d/", &part, &total) == 2) {
            data_start = strchr(m_result + 15, '/');
            if (data_start) data_start++;
        }
    }
    // Try legacy format
    else if (sscanf(m_result, "p%dof%d:", &part, &total) == 2) {
        data_start = strchr(m_result, ':');
        if (data_start) data_start++;
    }
    
    if (part <= 0 || total <= 0 || part > total || !data_start) {
        // Single QR - not animated
        strncpy(m_animation_buffer, m_result, sizeof(m_animation_buffer) - 1);
        m_animation_buffer[sizeof(m_animation_buffer) - 1] = '\0';
        m_expected_parts = 1;
        m_received_parts = 1;
        return true;
    }
    
    // Initialize on first part
    if (m_expected_parts == 0) {
        m_expected_parts = total;
        memset(m_part_received, 0, sizeof(m_part_received));
    }
    
    // Validate part number
    if (part > MAX_ANIMATED_PARTS) {
        return false;
    }
    
    // Store part if not already received
    if (!m_part_received[part - 1]) {
        m_part_received[part - 1] = 1;
        m_received_parts++;
        
        // Simple concatenation (for binary data, use proper buffer)
        size_t current_len = strlen(m_animation_buffer);
        size_t data_len = strlen(data_start);
        
        if (current_len + data_len < sizeof(m_animation_buffer) - 1) {
            memcpy(m_animation_buffer + current_len, data_start, data_len);
            m_animation_buffer[current_len + data_len] = '\0';
        }
    }
    
    m_has_result = false;  // Ready for next frame
    return true;
}

bool QRCodeScanner::is_animation_complete() const {
    return m_animated_mode && 
           m_expected_parts > 0 && 
           m_received_parts == m_expected_parts;
}

const char* QRCodeScanner::get_animated_result() const {
    if (!is_animation_complete()) return nullptr;
    return m_animation_buffer;
}

uint8_t QRCodeScanner::get_progress_percent() const {
    if (m_expected_parts == 0) return 0;
    return (m_received_parts * 100) / m_expected_parts;
}

void QRCodeScanner::reset_animation() {
    m_expected_parts = 0;
    m_received_parts = 0;
    memset(m_animation_buffer, 0, sizeof(m_animation_buffer));
    memset(m_part_received, 0, sizeof(m_part_received));
    m_has_result = false;
}

} // namespace Utils
} // namespace SeedSigner
