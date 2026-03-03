#include "drivers/gc0308.h"
#include <Arduino.h>
#include <esp_camera.h>

// GC0308 camera pins for M5Stack CoreS3
#define GC0308_PWDN     -1
#define GC0308_RESET    -1
#define GC0308_XCLK     15
#define GC0308_SIOD     17  // SDA
#define GC0308_SIOC     18  // SCL
#define GC0308_D7       39
#define GC0308_D6       40
#define GC0308_D5       41
#define GC0308_D4       42
#define GC0308_D3       5
#define GC0308_D2       45
#define GC0308_D1       46
#define GC0308_D0       2
#define GC0308_VSYNC    47
#define GC0308_HREF     48
#define GC0308_PCLK     16

static camera_config_t camera_config;
static bool initialized = false;

namespace GC0308 {

bool init(Resolution res) {
    if (initialized) {
        return true;
    }
    
    camera_config.ledc_channel = LEDC_CHANNEL_0;
    camera_config.ledc_timer = LEDC_TIMER_0;
    camera_config.pin_pwdn = GC0308_PWDN;
    camera_config.pin_reset = GC0308_RESET;
    camera_config.pin_xclk = GC0308_XCLK;
    camera_config.pin_sccb_sda = GC0308_SIOD;
    camera_config.pin_sccb_scl = GC0308_SIOC;
    camera_config.pin_d7 = GC0308_D7;
    camera_config.pin_d6 = GC0308_D6;
    camera_config.pin_d5 = GC0308_D5;
    camera_config.pin_d4 = GC0308_D4;
    camera_config.pin_d3 = GC0308_D3;
    camera_config.pin_d2 = GC0308_D2;
    camera_config.pin_d1 = GC0308_D1;
    camera_config.pin_d0 = GC0308_D0;
    camera_config.pin_vsync = GC0308_VSYNC;
    camera_config.pin_href = GC0308_HREF;
    camera_config.pin_pclk = GC0308_PCLK;
    
    camera_config.xclk_freq_hz = 20000000;
    camera_config.pixel_format = PIXFORMAT_GRAYSCALE;  // For QR scanning
    
    switch (res) {
        case Resolution::VGA:
            camera_config.frame_size = FRAMESIZE_VGA;
            break;
        case Resolution::QVGA:
            camera_config.frame_size = FRAMESIZE_QVGA;
            break;
        case Resolution::QQVGA:
            camera_config.frame_size = FRAMESIZE_QQVGA;
            break;
    }
    
    camera_config.jpeg_quality = 12;
    camera_config.fb_count = 1;
    camera_config.fb_location = CAMERA_FB_IN_PSRAM;
    camera_config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed: 0x%x\n", err);
        return false;
    }
    
    initialized = true;
    Serial.println("GC0308 camera initialized");
    return true;
}

void deinit() {
    if (initialized) {
        esp_camera_deinit();
        initialized = false;
    }
}

bool capture(uint8_t* buffer, size_t buffer_size) {
    if (!initialized) return false;
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        return false;
    }
    
    size_t copy_len = min(buffer_size, fb->len);
    memcpy(buffer, fb->buf, copy_len);
    
    esp_camera_fb_return(fb);
    return true;
}

bool capture_gray(uint8_t* buffer, size_t buffer_size) {
    // Already grayscale mode
    return capture(buffer, buffer_size);
}

const uint8_t* get_frame_buffer() {
    if (!initialized) return nullptr;
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) return nullptr;
    
    // Note: caller must call esp_camera_fb_return() after use
    return fb->buf;
}

uint32_t extract_entropy() {
    if (!initialized) return 0;
    
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) return 0;
    
    // Extract entropy from LSBs of pixel values
    uint32_t entropy = 0;
    for (size_t i = 0; i < min((size_t)256, fb->len); i += 4) {
        entropy ^= fb->buf[i];
        entropy = (entropy << 1) | (entropy >> 31);  // Rotate
    }
    
    esp_camera_fb_return(fb);
    return entropy;
}

void set_exposure(uint16_t exposure) {
    if (!initialized) return;
    // sensor_t* s = esp_camera_sensor_get();
    // s->set_aec_value(s, exposure);
}

void set_gain(uint8_t gain) {
    if (!initialized) return;
    // sensor_t* s = esp_camera_sensor_get();
    // s->set_gainceiling(s, (gain_sense_t)gain);
}

void sleep() {
    if (initialized) {
        esp_camera_deinit();
    }
}

void wake() {
    if (!initialized) {
        init();
    }
}

} // namespace GC0308
