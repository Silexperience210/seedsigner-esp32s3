/**
 * @file display.cpp
 * Display driver implementation (M5Stack CoreS3)
 */

#include "drivers/display.h"
#include <M5Unified.h>

namespace SeedSigner {
namespace Drivers {

Display::Display() : m_initialized(false), m_brightness(128), 
                     m_rotation(0), m_width(320), m_height(240) {}

Display::~Display() {}

bool Display::init() {
    // M5.Display already initialized in main.cpp
    m_initialized = true;
    m_width = M5.Display.width();
    m_height = M5.Display.height();
    return true;
}

void Display::deinit() {
    m_initialized = false;
}

void Display::clear(uint32_t color) {
    M5.Display.fillScreen(color);
}

void Display::draw_pixel(int16_t x, int16_t y, uint32_t color) {
    M5.Display.drawPixel(x, y, color);
}

void Display::draw_line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, 
                        uint32_t color) {
    M5.Display.drawLine(x1, y1, x2, y2, color);
}

void Display::draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, 
                        uint32_t color) {
    M5.Display.drawRect(x, y, w, h, color);
}

void Display::fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, 
                        uint32_t color) {
    M5.Display.fillRect(x, y, w, h, color);
}

void Display::draw_circle(int16_t x, int16_t y, int16_t r, uint32_t color) {
    M5.Display.drawCircle(x, y, r, color);
}

void Display::fill_circle(int16_t x, int16_t y, int16_t r, uint32_t color) {
    M5.Display.fillCircle(x, y, r, color);
}

void Display::set_text_color(uint32_t fg, uint32_t bg) {
    M5.Display.setTextColor(fg, bg);
}

void Display::set_text_size(uint8_t size) {
    M5.Display.setTextSize(size);
}

void Display::set_cursor(int16_t x, int16_t y) {
    M5.Display.setCursor(x, y);
}

void Display::print(const char* text) {
    M5.Display.print(text);
}

void Display::printf(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    M5.Display.print(buf);
}

void Display::draw_bitmap(int16_t x, int16_t y, const uint8_t* bitmap,
                          int16_t w, int16_t h, uint32_t fg, uint32_t bg) {
    M5.Display.drawBitmap(x, y, bitmap, w, h, fg, bg);
}

void Display::draw_qr_code(int16_t x, int16_t y, const char* data, 
                           uint8_t scale) {
    // TODO: Implement QR rendering
    (void)x; (void)y; (void)data; (void)scale;
}

void Display::set_brightness(uint8_t brightness) {
    m_brightness = brightness;
    M5.Display.setBrightness(brightness);
}

void Display::sleep() {
    M5.Display.sleep();
}

void Display::wakeup() {
    M5.Display.wakeup();
}

int16_t Display::width() const {
    return m_width;
}

int16_t Display::height() const {
    return m_height;
}

void Display::set_rotation(uint8_t rotation) {
    m_rotation = rotation;
    M5.Display.setRotation(rotation);
}

uint8_t Display::get_rotation() const {
    return m_rotation;
}

void Display::start_write() {
    M5.Display.startWrite();
}

void Display::end_write() {
    M5.Display.endWrite();
}

void Display::set_addr_window(int16_t x, int16_t y, int16_t w, int16_t h) {
    M5.Display.setAddrWindow(x, y, w, h);
}

void Display::write_pixels(const uint16_t* data, size_t len) {
    M5.Display.writePixels(data, len);
}

bool Display::screenshot(const char* filename) {
    // TODO: Implement screenshot to SD
    (void)filename;
    return false;
}

} // namespace Drivers
} // namespace SeedSigner
