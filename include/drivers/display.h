/**
 * @file display.h
 * Display driver abstraction
 */

#ifndef DRIVERS_DISPLAY_H
#define DRIVERS_DISPLAY_H

#include <Arduino.h>
#include <stdint.h>

namespace SeedSigner {
namespace Drivers {

class Display {
public:
    Display();
    ~Display();
    
    bool init();
    void deinit();
    
    // Basic drawing
    void clear(uint32_t color = 0);
    void draw_pixel(int16_t x, int16_t y, uint32_t color);
    void draw_line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color);
    void draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color);
    void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color);
    void draw_circle(int16_t x, int16_t y, int16_t r, uint32_t color);
    void fill_circle(int16_t x, int16_t y, int16_t r, uint32_t color);
    
    // Text
    void set_text_color(uint32_t fg, uint32_t bg);
    void set_text_size(uint8_t size);
    void set_cursor(int16_t x, int16_t y);
    void print(const char* text);
    void printf(const char* fmt, ...);
    
    // Image/QR
    void draw_bitmap(int16_t x, int16_t y, const uint8_t* bitmap, 
                     int16_t w, int16_t h, uint32_t fg, uint32_t bg);
    void draw_qr_code(int16_t x, int16_t y, const char* data, uint8_t scale);
    
    // Display control
    void set_brightness(uint8_t brightness);  // 0-255
    uint8_t get_brightness() const { return m_brightness; }
    void sleep();
    void wakeup();
    
    // Dimensions
    int16_t width() const;
    int16_t height() const;
    
    // Rotation
    void set_rotation(uint8_t rotation);  // 0-3
    uint8_t get_rotation() const;
    
    // Flush for LVGL
    void start_write();
    void end_write();
    void set_addr_window(int16_t x, int16_t y, int16_t w, int16_t h);
    void write_pixels(const uint16_t* data, size_t len);
    
    // Screenshot to SD
    bool screenshot(const char* filename);
    
private:
    bool m_initialized;
    uint8_t m_brightness;
    uint8_t m_rotation;
    int16_t m_width;
    int16_t m_height;
};

// Color conversion
inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

inline uint16_t color_to_rgb565(uint32_t color) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    return rgb565(r, g, b);
}

} // namespace Drivers
} // namespace SeedSigner

#endif // DRIVERS_DISPLAY_H
