/**
 * @file qr_code.h
 * QR Code generation and scanning utilities
 */

#ifndef UTILS_QR_CODE_H
#define UTILS_QR_CODE_H

#include <Arduino.h>
#include <stdint.h>

namespace SeedSigner {
namespace Utils {

// QR Code capacity
#define QR_MAX_VERSION 20
#define QR_MAX_DATA_LEN 1024
#define QR_MAX_MODULES 97  // Version 20 = 97x97 modules

// QR encoding modes
enum class QREncodeMode {
    NUMERIC = 0,
    ALPHANUMERIC,
    BYTE,
    KANJI
};

// QR Error correction levels
enum class QRErrorLevel {
    QR_LOW = 0,       // ~7%
    QR_MEDIUM,        // ~15%
    QR_QUARTILE,      // ~25%
    QR_HIGH           // ~30%
};

// QR Code structure
struct QRCode {
    uint8_t version;      // 1-20
    QRErrorLevel ec_level;
    uint8_t modules[QR_MAX_MODULES][QR_MAX_MODULES];  // 0=light, 1=dark
    uint8_t size;         // Size in modules (17 + 4*version)
    char data[QR_MAX_DATA_LEN];
    size_t data_len;
};

// QR Code generator
class QRCodeGenerator {
public:
    QRCodeGenerator();
    
    // Generate QR code from string
    bool generate(const char* data, QRCode* qr, 
                  QRErrorLevel ec = QRErrorLevel::QR_MEDIUM);
    
    // Generate with specific version constraint
    bool generate_version(const char* data, uint8_t version,
                          QRErrorLevel ec, QRCode* qr);
    
    // Get minimum version for data
    uint8_t estimate_version(const char* data, QRErrorLevel ec);
    
    // Render QR to bitmap
    bool render_bitmap(const QRCode* qr, uint8_t* bitmap, 
                       uint16_t scale, uint16_t* width, uint16_t* height);
    
    // Render to display
    void render_to_display(const QRCode* qr, int16_t x, int16_t y, 
                           uint8_t scale);
    
private:
    bool encode_data(const char* data, uint8_t* buffer, size_t* len);
    void add_finder_patterns(QRCode* qr);
    void add_alignment_patterns(QRCode* qr);
    void add_timing_patterns(QRCode* qr);
    void add_format_info(QRCode* qr);
    void add_version_info(QRCode* qr);
    void place_data_bits(QRCode* qr, const uint8_t* data, size_t len);
    void apply_mask(QRCode* qr, uint8_t mask_pattern);
    uint8_t select_best_mask(QRCode* qr);
    void calculate_rs_ec(const uint8_t* data, size_t data_len,
                         uint8_t* ec, size_t ec_len);
};

// Maximum animated QR parts
#define MAX_ANIMATED_PARTS 100

// QR Code scanner (using camera)
class QRCodeScanner {
public:
    QRCodeScanner();
    ~QRCodeScanner();
    
    bool init();
    void deinit();
    
    // Start/stop scanning
    bool start();
    bool stop();
    bool is_scanning() const;
    
    // Process frame from camera
    bool process_frame(const uint8_t* grayscale, uint16_t width, uint16_t height);
    
    // Get result
    bool has_result() const;
    const char* get_result() const;
    void clear_result();
    
    // Animated QR (for large data like PSBTs)
    bool start_animated_scan();
    bool process_animated_frame();
    bool is_animation_complete() const;
    const char* get_animated_result() const;
    uint8_t get_progress_percent() const;
    void reset_animation();
    
    // Check if quirc is available
    static bool has_quirc() {
        #if __has_include(<quirc.h>)
            return true;
        #else
            return false;
        #endif
    }
    
private:
    bool m_initialized;
    bool m_scanning;
    bool m_has_result;
    char m_result[QR_MAX_DATA_LEN];
    
    // Animated QR state
    bool m_animated_mode;
    uint8_t m_expected_parts;
    uint8_t m_received_parts;
    char m_animation_buffer[4096];
    uint8_t m_part_received[MAX_ANIMATED_PARTS];
    
    // quirc handle (opaque pointer)
    void* m_quirc;
    
    // QR processing methods
    bool find_finder_patterns(const uint8_t* image, uint16_t w, uint16_t h,
                              int16_t* fx1, int16_t* fy1,
                              int16_t* fx2, int16_t* fy2,
                              int16_t* fx3, int16_t* fy3);
    bool sample_qr_grid(const uint8_t* image, uint16_t w, uint16_t h,
                        const int16_t* corners, QRCode* qr);
    bool decode_qr(const QRCode* qr, char* output, size_t output_len);
    
    // Legacy stub implementations (if quirc not available)
    bool find_finder_patterns_stub(const uint8_t* image, uint16_t w, uint16_t h);
    bool sample_qr_grid_stub(const uint8_t* image, uint16_t w, uint16_t h);
};

} // namespace Utils
} // namespace SeedSigner

#endif // UTILS_QR_CODE_H
