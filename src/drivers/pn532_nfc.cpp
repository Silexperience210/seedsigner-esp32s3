#include "drivers/pn532_nfc.h"
#include <Wire.h>
#include <Arduino.h>

// PN532 I2C address
#define PN532_I2C_ADDR 0x24

// PN532 Commands
#define PN532_COMMAND_SAMCONFIGURATION 0x14
#define PN532_COMMAND_INLISTPASSIVETARGET 0x4A
#define PN532_COMMAND_INDATAEXCHANGE 0x40
#define PN532_COMMAND_INSELECT 0x54

// NTAG constants
#define NTAG_PAGE_SIZE 4
#define NTAG424_PAGES 256

static bool initialized = false;
static PN532_NFC::CardType detected_card = PN532_NFC::CardType::UNKNOWN;

// I2C communication helpers
static bool pn532_write_command(const uint8_t* cmd, size_t len) {
    Wire.beginTransmission(PN532_I2C_ADDR);
    
    // Preamble
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.write(0xFF);
    
    // Length
    Wire.write(len + 1);
    Wire.write(~(len + 1) + 1);  // LCS
    
    // TFI + Data
    Wire.write(0xD4);  // Host to PN532
    for (size_t i = 0; i < len; i++) {
        Wire.write(cmd[i]);
    }
    
    // DCS
    uint8_t dcs = 0xD4;
    for (size_t i = 0; i < len; i++) {
        dcs += cmd[i];
    }
    dcs = ~dcs + 1;
    Wire.write(dcs);
    
    // Postamble
    Wire.write(0x00);
    
    return Wire.endTransmission() == 0;
}

static bool pn532_read_response(uint8_t* buffer, size_t max_len, size_t* out_len) {
    Wire.requestFrom(PN532_I2C_ADDR, (uint8_t)min(max_len, (size_t)32));
    
    size_t i = 0;
    while (Wire.available() && i < max_len) {
        buffer[i++] = Wire.read();
    }
    
    if (out_len) *out_len = i;
    return i > 0;
}

namespace PN532_NFC {

bool init() {
    Wire.begin();
    
    // SAM configuration
    uint8_t cmd[] = {PN532_COMMAND_SAMCONFIGURATION, 0x01, 0x14, 0x00};
    if (!pn532_write_command(cmd, sizeof(cmd))) {
        return false;
    }
    
    delay(100);
    
    uint8_t response[16];
    size_t len;
    if (!pn532_read_response(response, sizeof(response), &len)) {
        return false;
    }
    
    initialized = true;
    Serial.println("PN532 NFC initialized");
    return true;
}

void deinit() {
    initialized = false;
}

bool detect_card() {
    if (!initialized) return false;
    
    // InListPassiveTarget for ISO14443A
    uint8_t cmd[] = {PN532_COMMAND_INLISTPASSIVETARGET, 0x01, 0x00};
    if (!pn532_write_command(cmd, sizeof(cmd))) {
        return false;
    }
    
    delay(50);
    
    uint8_t response[32];
    size_t len;
    if (!pn532_read_response(response, sizeof(response), &len)) {
        detected_card = PN532_NFC::CardType::UNKNOWN;
        return false;
    }
    
    // Check response
    if (len > 0 && response[0] == 0x01) {
        // Card detected, try to identify type
        // Simplified: assume NTAG424 for now
        detected_card = PN532_NFC::CardType::NTAG424;
        return true;
    }
    
    detected_card = PN532_NFC::CardType::UNKNOWN;
    return false;
}

CardType get_card_type() {
    return detected_card;
}

bool read_page(uint8_t page, uint8_t data[4]) {
    if (!initialized) return false;
    
    // NTAG read command
    uint8_t cmd[] = {PN532_COMMAND_INDATAEXCHANGE, 0x01, 0x30, page};
    if (!pn532_write_command(cmd, sizeof(cmd))) {
        return false;
    }
    
    delay(20);
    
    uint8_t response[16];
    size_t len;
    if (!pn532_read_response(response, sizeof(response), &len)) {
        return false;
    }
    
    if (len >= 4) {
        memcpy(data, response, 4);
        return true;
    }
    
    return false;
}

bool write_page(uint8_t page, const uint8_t data[4]) {
    if (!initialized) return false;
    
    // NTAG write command (0xA2)
    uint8_t cmd[] = {PN532_COMMAND_INDATAEXCHANGE, 0x01, 0xA2, 
                     page, data[0], data[1], data[2], data[3]};
    
    return pn532_write_command(cmd, sizeof(cmd));
}

bool authenticate_dna(const uint8_t key[16]) {
    // NTAG424 DNA authentication
    // Would use AES-128 authentication here
    (void)key;
    return false;  // Not implemented in this stub
}

bool read_encrypted(uint8_t page, uint8_t data[4]) {
    // Would decrypt after authentication
    (void)page;
    (void)data;
    return false;
}

bool write_encrypted(uint8_t page, const uint8_t data[4]) {
    // Would encrypt before writing
    (void)page;
    (void)data;
    return false;
}

bool write_seed_backup(const SeedBackup& backup) {
    if (!initialized) return false;
    
    // Write backup structure to card
    // Pages 4-19 (16 pages = 64 bytes for encrypted seed)
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&backup);
    
    for (int i = 0; i < sizeof(SeedBackup); i += 4) {
        uint8_t page = 4 + (i / 4);
        if (!write_page(page, ptr + i)) {
            return false;
        }
        delay(10);
    }
    
    return true;
}

bool read_seed_backup(SeedBackup& backup) {
    if (!initialized) return false;
    
    uint8_t* ptr = reinterpret_cast<uint8_t*>(&backup);
    
    for (int i = 0; i < sizeof(SeedBackup); i += 4) {
        uint8_t page = 4 + (i / 4);
        if (!read_page(page, ptr + i)) {
            return false;
        }
    }
    
    // Verify checksum
    uint8_t checksum = 0;
    for (int i = 0; i < sizeof(SeedBackup) - 1; i++) {
        checksum ^= ptr[i];
    }
    
    return checksum == backup.checksum;
}

bool verify_sun(const uint8_t* message, size_t len, const uint8_t* signature) {
    // SUN (Secure Unique NFC) verification
    (void)message;
    (void)len;
    (void)signature;
    return false;  // Not implemented
}

void rf_field_on() {
    // Not implemented in this stub
}

void rf_field_off() {
    // Not implemented in this stub
}

} // namespace PN532_NFC
