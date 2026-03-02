/**
 * @file touch.h
 * Touch screen driver
 */

#ifndef DRIVERS_TOUCH_H
#define DRIVERS_TOUCH_H

#include <Arduino.h>
#include <stdint.h>

namespace SeedSigner {
namespace Drivers {

// Touch event
struct TouchEvent {
    int16_t x;
    int16_t y;
    uint8_t pressure;
    bool pressed;
    uint32_t timestamp;
};

class Touch {
public:
    Touch();
    ~Touch();
    
    bool init();
    void deinit();
    
    // Poll touch state
    bool poll();
    
    // Get current touch state
    bool is_touched() const { return m_current.pressed; }
    void get_touch(int16_t* x, int16_t* y) const;
    void get_event(TouchEvent* event) const;
    
    // Gesture detection
    bool swipe_detected(uint8_t* direction);  // 0=up, 1=down, 2=left, 3=right
    bool tap_detected();
    bool long_press_detected();
    
    // Calibration
    void set_calibration(int16_t xmin, int16_t ymin, int16_t xmax, int16_t ymax);
    void get_calibration(int16_t* xmin, int16_t* ymin, int16_t* xmax, int16_t* ymax);
    
    // Sleep/Wake
    void sleep();
    void wakeup();
    
    // Status
    bool is_initialized() const { return m_initialized; }
    
private:
    bool m_initialized;
    TouchEvent m_current;
    TouchEvent m_last;
    
    // Calibration
    int16_t m_cal_xmin, m_cal_ymin;
    int16_t m_cal_xmax, m_cal_ymax;
    
    // Gesture state
    uint32_t m_touch_start_time;
    int16_t m_touch_start_x, m_touch_start_y;
    
    // Hardware specific
    bool init_ft6336u();
    bool init_gt911();
    bool read_ft6336u(TouchEvent* event);
    bool read_gt911(TouchEvent* event);
};

} // namespace Drivers
} // namespace SeedSigner

#endif // DRIVERS_TOUCH_H
