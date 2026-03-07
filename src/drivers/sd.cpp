/**
 * @file sd.cpp
 * SD Card driver implementation
 */

#include "drivers/sd.h"
#include <SD.h>
#include <SPI.h>
#include <M5Unified.h>

namespace SeedSigner {
namespace Drivers {

SDCard::SDCard() : m_initialized(false) {}

SDCard::~SDCard() {
    deinit();
}

bool SDCard::init() {
    if (m_initialized) return true;
    
    #ifdef M5STACK_CORES3
    // M5CoreS3 uses SPI for SD card
    SPI.begin(36, 35, 37, 4);  // SCK, MISO, MOSI, SS
    
    if (!SD.begin(4, SPI)) {
        Serial.println("[SD] Card Mount Failed");
        return false;
    }
    
    m_initialized = true;
    Serial.println("[SD] Card initialized");
    return true;
    #else
    return false;
    #endif
}

void SDCard::deinit() {
    if (m_initialized) {
        SD.end();
        m_initialized = false;
    }
}

bool SDCard::is_inserted() {
    return m_initialized;
}

bool SDCard::write_file(const char* path, const uint8_t* data, size_t len) {
    if (!m_initialized || !path || !data) return false;
    
    File file = SD.open(path, FILE_WRITE);
    if (!file) return false;
    
    size_t written = file.write(data, len);
    file.close();
    
    return written == len;
}

bool SDCard::read_file(const char* path, uint8_t* buffer, size_t* len) {
    if (!m_initialized || !path || !buffer || !len) return false;
    
    File file = SD.open(path, FILE_READ);
    if (!file) return false;
    
    size_t to_read = min(*len, (size_t)file.size());
    size_t read = file.read(buffer, to_read);
    file.close();
    
    *len = read;
    return read > 0;
}

bool SDCard::delete_file(const char* path) {
    if (!m_initialized || !path) return false;
    return SD.remove(path);
}

bool SDCard::file_exists(const char* path) {
    if (!m_initialized || !path) return false;
    return SD.exists(path);
}

} // namespace Drivers
} // namespace SeedSigner
