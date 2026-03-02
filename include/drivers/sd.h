/**
 * @file sd.h
 * SD Card driver for PSBT loading
 */

#ifndef DRIVERS_SD_H
#define DRIVERS_SD_H

#include <Arduino.h>
#include <stdint.h>
#include <FS.h>
#include <SD.h>

namespace SeedSigner {
namespace Drivers {

// SD Card state
enum class SDState {
    NOT_INITIALIZED = 0,
    NO_CARD,
    CARD_PRESENT,
    MOUNTED,
    ERROR
};

class SDCard {
public:
    SDCard();
    ~SDCard();
    
    bool init();
    void deinit();
    
    // Card status
    SDState get_state() const { return m_state; }
    bool is_mounted() const { return m_state == SDState::MOUNTED; }
    bool card_present();
    
    // File operations (PSBT only, read-only recommended)
    bool read_file(const char* path, uint8_t* buffer, size_t* len);
    size_t get_file_size(const char* path);
    bool file_exists(const char* path);
    
    // Directory listing
    bool list_psbt_files(char files[][64], size_t max_files, size_t* num_files);
    
    // Write operations (optional, for firmware updates)
    bool write_file(const char* path, const uint8_t* data, size_t len);
    bool delete_file(const char* path);
    
    // Eject
    void eject();
    
private:
    SDState m_state;
    bool m_initialized;
    
    // Chip select pin
    static constexpr uint8_t SD_CS_PIN = 4;
    
    bool mount();
    void unmount();
};

} // namespace Drivers
} // namespace SeedSigner

#endif // DRIVERS_SD_H
