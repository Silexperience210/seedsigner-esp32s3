/**
 * @file qr_code.cpp
 * QR Code generation and scanning
 */

#include "utils/qr_code.h"
#include <qrcode.h>
#include <stdlib.h>

namespace SeedSigner {
namespace Utils {

// QR Code Generator
QRCodeGenerator::QRCodeGenerator() {}

bool QRCodeGenerator::generate(const char* data, QRCode* qr, QRErrorLevel ec) {
    if (!data || !qr) return false;
    
    // Map error level
    uint8_t ecc = 0;  // ECC_LOW
    switch (ec) {
        case QRErrorLevel::MEDIUM: ecc = 1; break;
        case QRErrorLevel::QUARTILE: ecc = 2; break;
        case QRErrorLevel::HIGH: ecc = 3; break;
        default: ecc = 0; break;
    }
    
    // Determine version needed
    uint8_t version = estimate_version(data, ec);
    if (version == 0 || version > QR_MAX_VERSION) return false;
    
    // Create QR code
    QRCode qrc;
    uint8_t qrcodeData[qrcode_getBufferSize(version)];
    
    qrcode_initText(&qrc, qrcodeData, version, ecc, data);
    
    // Copy to our structure
    qr->version = version;
    qr->ec_level = ec;
    qr->size = qrc.size;
    strncpy(qr->data, data, sizeof(qr->data) - 1);
    qr->data[sizeof(qr->data) - 1] = '\0';
    qr->data_len = strlen(data);
    
    // Copy modules
    for (int y = 0; y < qrc.size && y < QR_MAX_MODULES; y++) {
        for (int x = 0; x < qrc.size && x < QR_MAX_MODULES; x++) {
            qr->modules[y][x] = qrcode_getModule(&qrc, x, y) ? 1 : 0;
        }
    }
    
    return true;
}

uint8_t QRCodeGenerator::estimate_version(const char* data, QRErrorLevel ec) {
    size_t len = strlen(data);
    
    // Capacity table (Alphanumeric mode approximation)
    // Version: capacity for each ECC level
    const uint16_t capacity[QR_MAX_VERSION][4] = {
        {25, 20, 16, 10},      // V1
        {47, 38, 29, 20},      // V2
        {77, 61, 47, 35},      // V3
        {114, 90, 67, 50},     // V4
        {154, 122, 87, 64},    // V5
        // ... more versions
        {3706, 2945, 2201, 1620},  // V20
    };
    
    uint8_t ecc_idx = (uint8_t)ec;
    
    for (uint8_t v = 1; v <= QR_MAX_VERSION; v++) {
        uint8_t idx = (v <= 5) ? (v - 1) : 4;  // Simplified
        if (capacity[idx][ecc_idx] >= len) {
            return v;
        }
    }
    
    return 0;  // Too large
}

bool QRCodeGenerator::render_bitmap(const QRCode* qr, uint8_t* bitmap, 
                                     uint16_t scale, uint16_t* width, uint16_t* height) {
    if (!qr || !bitmap || !width || !height) return false;
    
    uint16_t size = qr->size * scale;
    *width = size;
    *height = size;
    
    // Clear bitmap (white)
    memset(bitmap, 0xFF, (size * size + 7) / 8);
    
    // Draw modules
    for (uint16_t y = 0; y < size; y++) {
        for (uint16_t x = 0; x < size; x++) {
            uint16_t qx = x / scale;
            uint16_t qy = y / scale;
            
            if (qx < qr->size && qy < qr->size && qr->modules[qy][qx]) {
                // Set pixel (black)
                uint32_t idx = y * size + x;
                bitmap[idx / 8] &= ~(1 << (7 - (idx % 8)));
            }
        }
    }
    
    return true;
}

void QRCodeGenerator::render_to_display(const QRCode* qr, int16_t x, int16_t y, 
                                         uint8_t scale) {
    if (!qr) return;
    
    // Draw quiet zone
    int16_t quiet = 4 * scale;  // 4 modules quiet zone
    
    // Draw QR modules
    for (int16_t qy = 0; qy < qr->size; qy++) {
        for (int16_t qx = 0; qx < qr->size; qx++) {
            int16_t px = x + quiet + qx * scale;
            int16_t py = y + quiet + qy * scale;
            
            if (qr->modules[qy][qx]) {
                // Draw black module
                // M5.Display.fillRect(px, py, scale, scale, TFT_BLACK);
            } else {
                // Draw white module
                // M5.Display.fillRect(px, py, scale, scale, TFT_WHITE);
            }
        }
    }
}

// QR Code Scanner
QRCodeScanner::QRCodeScanner() : m_initialized(false), m_scanning(false),
                                  m_has_result(false), m_animated_mode(false),
                                  m_expected_parts(0), m_received_parts(0) {
    memset(m_result, 0, sizeof(m_result));
    memset(m_animation_buffer, 0, sizeof(m_animation_buffer));
}

QRCodeScanner::~QRCodeScanner() {
    deinit();
}

bool QRCodeScanner::init() {
    m_initialized = true;
    return true;
}

void QRCodeScanner::deinit() {
    stop();
    m_initialized = false;
}

bool QRCodeScanner::start() {
    if (!m_initialized) return false;
    m_scanning = true;
    m_has_result = false;
    return true;
}

bool QRCodeScanner::stop() {
    m_scanning = false;
    return true;
}

bool QRCodeScanner::process_frame(const uint8_t* grayscale, uint16_t width, uint16_t height) {
    if (!m_scanning || !grayscale) return false;
    
    // Simple QR detection using corner finding
    // This is a simplified version - production would use quirc or similar
    
    // 1. Find finder patterns (3 squares in corners)
    int16_t corners[8];  // 4 corners x 2 coordinates
    
    if (!find_finder_patterns(grayscale, width, height,
                               &corners[0], &corners[1],
                               &corners[2], &corners[3],
                               &corners[4], &corners[5])) {
        return false;
    }
    
    // 2. Sample QR grid
    QRCode qr;
    if (!sample_qr_grid(grayscale, width, height, corners, &qr)) {
        return false;
    }
    
    // 3. Decode
    if (!decode_qr(&qr, m_result, sizeof(m_result))) {
        return false;
    }
    
    m_has_result = true;
    return true;
}

bool QRCodeScanner::start_animated_scan() {
    m_animated_mode = true;
    m_expected_parts = 0;
    m_received_parts = 0;
    memset(m_animation_buffer, 0, sizeof(m_animation_buffer));
    return start();
}

bool QRCodeScanner::process_animated_frame() {
    if (!m_animated_mode) return false;
    
    // Parse animated QR format (e.g., "p1of3:data...")
    if (!m_has_result) return false;
    
    // Extract part number and total
    int part, total;
    char* data;
    
    if (sscanf(m_result, "p%dof%d:", &part, &total) != 2) {
        // Not an animated QR
        return false;
    }
    
    if (m_expected_parts == 0) {
        m_expected_parts = total;
    }
    
    // Find data after colon
    data = strchr(m_result, ':');
    if (!data) return false;
    data++;
    
    // Store part
    // TODO: Implement proper buffer management
    
    m_received_parts++;
    m_has_result = false;
    
    return true;
}

bool QRCodeScanner::is_animation_complete() const {
    return m_animated_mode && m_received_parts == m_expected_parts && m_expected_parts > 0;
}

const char* QRCodeScanner::get_animated_result() const {
    if (!is_animation_complete()) return nullptr;
    return m_animation_buffer;
}

bool QRCodeScanner::find_finder_patterns(const uint8_t* image, uint16_t w, uint16_t h,
                                          int16_t* x1, int16_t* y1,
                                          int16_t* x2, int16_t* y2,
                                          int16_t* x3, int16_t* y3) {
    // Simplified finder pattern detection
    // Look for 1:1:3:1:1 ratio pattern
    
    // Scan horizontal lines
    for (uint16_t y = 10; y < h - 10; y += 5) {
        for (uint16_t x = 10; x < w - 10; x++) {
            uint8_t pixel = image[y * w + x];
            
            // Look for dark pixel
            if (pixel < 128) {
                // Check for finder pattern ratio
                // This is simplified - real implementation needs proper pattern matching
                *x1 = x;
                *y1 = y;
                *x2 = x + 50;
                *y2 = y;
                *x3 = x;
                *y3 = y + 50;
                return true;
            }
        }
    }
    
    return false;
}

bool QRCodeScanner::sample_qr_grid(const uint8_t* image, uint16_t w, uint16_t h,
                                    const int16_t* corners, QRCode* qr) {
    // Perspective transform and sample
    // This is a placeholder - real implementation needs proper perspective transform
    
    qr->size = 21;  // Version 1
    
    for (int y = 0; y < qr->size; y++) {
        for (int x = 0; x < qr->size; x++) {
            // Simple nearest neighbor sampling
            int px = corners[0] + x * 5;
            int py = corners[1] + y * 5;
            
            if (px >= 0 && px < (int)w && py >= 0 && py < (int)h) {
                qr->modules[y][x] = (image[py * w + px] < 128) ? 1 : 0;
            } else {
                qr->modules[y][x] = 0;
            }
        }
    }
    
    return true;
}

bool QRCodeScanner::decode_qr(const QRCode* qr, char* output, size_t output_len) {
    // Simple mode detection and decoding
    // This is a placeholder - real implementation needs full QR decoder
    
    strcpy(output, "test_data");
    (void)qr;
    (void)output_len;
    return true;
}

} // namespace Utils
} // namespace SeedSigner
