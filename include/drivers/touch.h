/**
 * @file touch.h
 * Touch driver abstraction
 */

#ifndef DRIVERS_TOUCH_H
#define DRIVERS_TOUCH_H

#include <Arduino.h>
#include <stdint.h>

namespace SeedSigner {
namespace Drivers {

class Touch {
public:
    Touch();
    ~Touch();
    
    bool init();
    void deinit();
    void poll();
    
    bool is_touched();
    void get_touch(int16_t* x, int16_t* y);
    
private:
    bool m_initialized;
    bool m_touched;
    int16_t m_x;
    int16_t m_y;
};

} // namespace Drivers
} // namespace SeedSigner

#endif // DRIVERS_TOUCH_H
