/**
 * @file touch.cpp
 * Touch driver for FT6336U (M5Stack CoreS3)
 */

#include "drivers/touch.h"
#include <Wire.h>
#include <M5Unified.h>

namespace SeedSigner {
namespace Drivers {

Touch::Touch() : m_initialized(false), m_touched(false), m_x(0), m_y(0) {}

Touch::~Touch() {
    deinit();
}

bool Touch::init() {
    if (m_initialized) return true;
    
    #ifdef M5STACK_CORES3
    // M5Unified handles touch initialization
    m_initialized = true;
    return true;
    #else
    return false;
    #endif
}

void Touch::deinit() {
    m_initialized = false;
}

void Touch::poll() {
    if (!m_initialized) return;
    
    #ifdef M5STACK_CORES3
    m_touched = M5.Touch.getCount() > 0;
    if (m_touched) {
        auto t = M5.Touch.getDetail();
        m_x = t.x;
        m_y = t.y;
    }
    #endif
}

bool Touch::is_touched() {
    poll();
    return m_touched;
}

void Touch::get_touch(int16_t* x, int16_t* y) {
    if (x) *x = m_x;
    if (y) *y = m_y;
}

} // namespace Drivers
} // namespace SeedSigner
