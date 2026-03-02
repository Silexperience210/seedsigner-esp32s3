/**
 * @file camera.h
 * Camera driver for QR code scanning and entropy collection
 */

#ifndef DRIVERS_CAMERA_H
#define DRIVERS_CAMERA_H

#include <Arduino.h>
#include <stdint.h>

namespace SeedSigner {
namespace Drivers {

// Camera resolution
enum class CameraResolution {
    QQVGA = 0,      // 160x120
    QCIF,           // 176x144
    HQVGA,          // 240x176
    QVGA,           // 320x240
    CIF,            // 400x296
    HVGA,           // 480x320
    VGA,            // 640x480
    SVGA,           // 800x600
    XGA,            // 1024x768
    HD,             // 1280x720
    SXGA,           // 1280x1024
    UXGA            // 1600x1200
};

// Framebuffer
struct CameraFrame {
    uint8_t* data;
    size_t len;
    uint16_t width;
    uint16_t height;
    CameraResolution resolution;
    void* _fb;  // Internal framebuffer reference (camera_fb_t*)
};

class Camera {
public:
    Camera();
    ~Camera();
    
    bool init();
    void deinit();
    
    // Configuration
    bool set_resolution(CameraResolution res);
    CameraResolution get_resolution() const;
    
    // Capture
    bool capture(CameraFrame* frame);
    void release_framebuffer();
    
    // QR Code scanning
    bool start_qr_scan();
    bool stop_qr_scan();
    bool get_qr_result(char* output, size_t output_len);
    bool is_scanning() const { return m_scanning; }
    
    // Entropy collection
    bool collect_entropy_region(const CameraFrame* frame, 
                                uint16_t x, uint16_t y, 
                                uint16_t w, uint16_t h,
                                uint8_t* entropy_output, size_t entropy_len);
    
    // Preview
    void enable_preview();
    void disable_preview();
    bool render_preview(int16_t x, int16_t y, int16_t w, int16_t h);
    
    // Status
    bool is_initialized() const { return m_initialized; }
    
private:
    bool m_initialized;
    bool m_scanning;
    CameraResolution m_resolution;
    
    // QR scanner state
    void* m_qr_decoder;  // Opaque QR decoder handle
    
    bool init_gc0308();
    bool init_ov5640();
};

} // namespace Drivers
} // namespace SeedSigner

#endif // DRIVERS_CAMERA_H
