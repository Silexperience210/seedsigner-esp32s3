/**
 * @file camera.cpp
 * Camera driver for GC0308 (M5Stack CoreS3)
 * QR code scanning and entropy collection
 */

#include "drivers/camera.h"
#include "utils/memory.h"
#include <esp_camera.h>
#include <M5Unified.h>

// GC0308 pins for M5Stack CoreS3
#define GC0308_PWDN     -1
#define GC0308_RESET    -1
#define GC0308_XCLK     15
#define GC0308_SIOD     12  // SDA
#define GC0308_SIOC     11  // SCL
#define GC0308_D0       4
#define GC0308_D1       5
#define GC0308_D2       6
#define GC0308_D3       7
#define GC0308_D4       8
#define GC0308_D5       9
#define GC0308_D6       10
#define GC0308_D7       13
#define GC0308_VSYNC    17
#define GC0308_HREF     16
#define GC0308_PCLK     14

namespace SeedSigner {
namespace Drivers {

Camera::Camera() : m_initialized(false), m_scanning(false),
                   m_resolution(CameraResolution::QVGA) {
}

Camera::~Camera() {
    deinit();
}

bool Camera::init() {
    if (m_initialized) return true;

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_pwdn = GC0308_PWDN;
    config.pin_reset = GC0308_RESET;
    config.pin_xclk = GC0308_XCLK;
    config.pin_sccb_sda = GC0308_SIOD;
    config.pin_sccb_scl = GC0308_SIOC;
    config.pin_d7 = GC0308_D7;
    config.pin_d6 = GC0308_D6;
    config.pin_d5 = GC0308_D5;
    config.pin_d4 = GC0308_D4;
    config.pin_d3 = GC0308_D3;
    config.pin_d2 = GC0308_D2;
    config.pin_d1 = GC0308_D1;
    config.pin_d0 = GC0308_D0;
    config.pin_vsync = GC0308_VSYNC;
    config.pin_href = GC0308_HREF;
    config.pin_pclk = GC0308_PCLK;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_GRAYSCALE;  // For QR scanning
    config.frame_size = FRAMESIZE_QVGA;         // 320x240
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        M5_LOG_E("Camera init failed: 0x%x", err);
        return false;
    }

    m_initialized = true;
    m_resolution = CameraResolution::QVGA;
    
    M5_LOG_I("Camera initialized successfully");
    return true;
}

void Camera::deinit() {
    if (m_initialized) {
        esp_camera_deinit();
        m_initialized = false;
    }
}

bool Camera::set_resolution(CameraResolution res) {
    if (!m_initialized) return false;

    framesize_t frame_size;
    switch (res) {
        case CameraResolution::QQVGA:
            frame_size = FRAMESIZE_QQVGA;  // 160x120
            break;
        case CameraResolution::QVGA:
            frame_size = FRAMESIZE_QVGA;   // 320x240
            break;
        case CameraResolution::VGA:
            frame_size = FRAMESIZE_VGA;    // 640x480
            break;
        default:
            frame_size = FRAMESIZE_QVGA;
            break;
    }

    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr) return false;

    s->set_framesize(s, frame_size);
    m_resolution = res;
    
    return true;
}

CameraResolution Camera::get_resolution() const {
    return m_resolution;
}

bool Camera::capture(CameraFrame* frame) {
    if (!m_initialized || !frame) return false;

    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        M5_LOG_E("Camera capture failed");
        return false;
    }

    frame->data = fb->buf;
    frame->len = fb->len;
    frame->width = fb->width;
    frame->height = fb->height;
    frame->resolution = m_resolution;
    
    // Store reference to framebuffer for later release
    frame->_fb = fb;
    
    return true;
}

void Camera::release_framebuffer() {
    esp_camera_fb_return(nullptr);
}

bool Camera::start_qr_scan() {
    if (!m_initialized) return false;
    
    m_scanning = true;
    
    // Set optimal settings for QR scanning
    sensor_t *s = esp_camera_sensor_get();
    if (s) {
        s->set_brightness(s, 0);     // Default brightness
        s->set_contrast(s, 2);       // Higher contrast for QR detection
        s->set_saturation(s, -2);    // Min saturation for grayscale
        s->set_sharpness(s, 2);      // Sharper edges
        s->set_denoise(s, 1);        // Light denoise
    }
    
    return true;
}

bool Camera::stop_qr_scan() {
    m_scanning = false;
    return true;
}

bool Camera::get_qr_result(char* output, size_t output_len) {
    if (!m_scanning || !output || output_len < 1) return false;

    CameraFrame frame;
    if (!capture(&frame)) {
        return false;
    }

    // Simple QR detection - in production, use quirc library
    // This is a placeholder implementation
    bool found = false;
    
    // Look for finder patterns (simplified)
    // Real implementation would use quirc or similar
    
    // For now, simulate detection based on image variance
    uint32_t sum = 0;
    uint32_t sum_sq = 0;
    size_t sample_size = min((size_t)1000, frame.len);
    
    for (size_t i = 0; i < sample_size; i += 10) {
        uint8_t pixel = frame.data[i];
        sum += pixel;
        sum_sq += pixel * pixel;
    }
    
    float mean = sum / (sample_size / 10.0);
    float variance = (sum_sq / (sample_size / 10.0)) - (mean * mean);
    
    // High variance might indicate QR pattern
    if (variance > 1000) {
        strncpy(output, "test_qr_data", output_len - 1);
        output[output_len - 1] = '\0';
        found = true;
    }

    // Return framebuffer
    if (frame._fb) {
        esp_camera_fb_return((camera_fb_t*)frame._fb);
    }
    
    return found;
}

bool Camera::collect_entropy_region(const CameraFrame* frame, 
                                     uint16_t x, uint16_t y, 
                                     uint16_t w, uint16_t h,
                                     uint8_t* entropy_output, size_t entropy_len) {
    if (!frame || !frame->data || !entropy_output || entropy_len < 32) {
        return false;
    }

    // Clamp region to frame bounds
    uint16_t end_x = min((uint16_t)(x + w), (uint16_t)frame->width);
    uint16_t end_y = min((uint16_t)(y + h), (uint16_t)frame->height);
    
    // Collect LSBs from pixel values as entropy
    // Use pixel noise (especially in dark conditions)
    uint8_t entropy_pool[64] = {0};
    size_t pool_idx = 0;
    
    for (uint16_t py = y; py < end_y && pool_idx < sizeof(entropy_pool); py += 2) {
        for (uint16_t px = x; px < end_x && pool_idx < sizeof(entropy_pool); px += 2) {
            size_t idx = py * frame->width + px;
            if (idx < frame->len) {
                // Use least significant bits (noise)
                entropy_pool[pool_idx++] = frame->data[idx] & 0x03;
            }
        }
    }
    
    // Hash the collected entropy
    // Using simple XOR-based mixing for now
    // In production, use SHA-256
    for (size_t i = 0; i < entropy_len && i < 32; i++) {
        entropy_output[i] = entropy_pool[i] ^ entropy_pool[i + 32];
    }
    
    return true;
}

void Camera::enable_preview() {
    // Preview is always enabled in our implementation
}

void Camera::disable_preview() {
    // Disable continuous capture if implemented
}

bool Camera::render_preview(int16_t x, int16_t y, int16_t w, int16_t h) {
    if (!m_initialized) return false;

    CameraFrame frame;
    if (!capture(&frame)) {
        return false;
    }

    // Scale and render to display
    float scale_x = (float)w / frame.width;
    float scale_y = (float)h / frame.height;
    float scale = min(scale_x, scale_y);
    
    int16_t render_w = frame.width * scale;
    int16_t render_h = frame.height * scale;
    int16_t offset_x = x + (w - render_w) / 2;
    int16_t offset_y = y + (h - render_h) / 2;

    // Draw the frame
    // Note: In production, use optimized DMA transfer
    for (int16_t row = 0; row < render_h; row++) {
        for (int16_t col = 0; col < render_w; col++) {
            uint16_t src_x = col / scale;
            uint16_t src_y = row / scale;
            uint8_t pixel = frame.data[src_y * frame.width + src_x];
            
            // Convert grayscale to RGB565
            uint16_t color = ((pixel >> 3) << 11) | ((pixel >> 2) << 5) | (pixel >> 3);
            M5.Display.drawPixel(offset_x + col, offset_y + row, color);
        }
    }

    // Return framebuffer
    if (frame._fb) {
        esp_camera_fb_return((camera_fb_t*)frame._fb);
    }
    
    return true;
}

} // namespace Drivers
} // namespace SeedSigner
