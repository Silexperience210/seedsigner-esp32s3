/**
 * @file sd.h
 * SD Card driver abstraction
 */

#ifndef DRIVERS_SD_H
#define DRIVERS_SD_H

#include <Arduino.h>
#include <stdint.h>

namespace SeedSigner {
namespace Drivers {

class SDCard {
public:
    SDCard();
    ~SDCard();
    
    bool init();
    void deinit();
    
    bool is_inserted();
    bool write_file(const char* path, const uint8_t* data, size_t len);
    bool read_file(const char* path, uint8_t* buffer, size_t* len);
    bool delete_file(const char* path);
    bool file_exists(const char* path);
    
private:
    bool m_initialized;
};

} // namespace Drivers
} // namespace SeedSigner

#endif // DRIVERS_SD_H
