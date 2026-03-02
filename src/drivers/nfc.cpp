/**
 * @file nfc.cpp
 * NFC driver for NTAG 424 DNA with secure storage
 */

#include "drivers/nfc.h"
#include "utils/memory.h"
#include <Wire.h>
#include <string.h>

// PN532 I2C Address
#define PN532_I2C_ADDRESS 0x24

// PN532 Commands
#define PN532_COMMAND_GETFIRMWAREVERSION 0x02
#define PN532_COMMAND_SAMCONFIGURATION   0x14
#define PN532_COMMAND_INLISTPASSIVETARGET 0x4A
#define PN532_COMMAND_INDATAEXCHANGE      0x40
#define PN532_COMMAND_INRELEASE           0x52

// NTAG 424 Commands
#define NTAG424_CMD_GET_VERSION           0x60
#define NTAG424_CMD_READ_SIG              0x3C
#define NTAG424_CMD_AUTHENTICATE_EV2_FIRST 0x71
#define NTAG424_CMD_AUTHENTICATE_EV2_NONFIRST 0x77
#define NTAG424_CMD_READ_DATA             0xAD
#define NTAG424_CMD_WRITE_DATA            0x8D
#define NTAG424_CMD_GET_FILE_SETTINGS     0xF5
#define NTAG424_CMD_CHANGE_FILE_SETTINGS  0x5F
#define NTAG424_CMD_GET_CARD_UID          0x51

// File IDs
#define NTAG424_FILE_CC_FILE              0x01
#define NTAG424_FILE_NDEF_FILE            0x02
#define NTAG424_FILE_PROPRIETARY_FILE     0x03

namespace SeedSigner {
namespace Drivers {

// SeedSigner NTAG 424 Application ID
static const uint8_t SS_APP_ID[3] = {0x53, 0x53, 0x34}; // "SS4"

NFC::NFC() : m_initialized(false), m_tag_present(false),
             m_auth_status(NFCAuthStatus::NOT_AUTH),
             m_tag_callback(nullptr) {
    memset(&m_current_tag, 0, sizeof(m_current_tag));
}

NFC::~NFC() {
    deinit();
}

bool NFC::init() {
    if (m_initialized) return true;
    
    Wire.begin();
    
    // Check PN532 presence
    uint8_t version[4];
    if (!pn532_get_firmware_version(version)) {
        return false;
    }
    
    // Configure SAM (Security Access Module)
    if (!pn532_sam_configuration()) {
        return false;
    }
    
    m_initialized = true;
    return true;
}

void NFC::deinit() {
    m_initialized = false;
    m_tag_present = false;
}

void NFC::poll() {
    if (!m_initialized) return;
    
    static uint32_t last_poll = 0;
    uint32_t now = millis();
    
    // Poll every 100ms
    if (now - last_poll < 100) return;
    last_poll = now;
    
    bool was_present = m_tag_present;
    
    // Try to detect tag
    uint8_t uid[16];
    uint8_t uid_len = 0;
    
    if (pn532_in_list_passive_target(uid, &uid_len)) {
        if (!m_tag_present) {
            // New tag detected
            m_tag_present = true;
            memcpy(m_current_tag.uid, uid, uid_len);
            
            // Read version info
            ntag424_read_version(m_current_tag.version);
            
            if (m_tag_callback) {
                m_tag_callback(true, &m_current_tag);
            }
        }
    } else {
        if (m_tag_present) {
            // Tag removed
            m_tag_present = false;
            m_auth_status = NFCAuthStatus::NOT_AUTH;
            memset(&m_current_tag, 0, sizeof(m_current_tag));
            
            if (m_tag_callback) {
                m_tag_callback(false, nullptr);
            }
        }
    }
}

bool NFC::is_tag_present() {
    return m_tag_present;
}

bool NFC::read_tag_info(NTAG424Tag* info) {
    if (!m_tag_present || !info) return false;
    
    memcpy(info, &m_current_tag, sizeof(NTAG424Tag));
    return true;
}

bool NFC::authenticate(const uint8_t key[NTAG424_KEY_SIZE], uint8_t key_num) {
    if (!m_tag_present) return false;
    
    // EV2 First Authentication
    // 1. Send Auth command with key number
    // 2. Get challenge from tag
    // 3. Generate response
    // 4. Verify tag response
    
    uint8_t cmd[2] = {NTAG424_CMD_AUTHENTICATE_EV2_FIRST, key_num};
    uint8_t response[32];
    size_t response_len = sizeof(response);
    
    if (!ntag424_send_command(cmd, 2, response, &response_len)) {
        return false;
    }
    
    // Parse challenge and perform authentication
    // TODO: Implement full EV2 authentication protocol
    
    m_auth_status = (NFCAuthStatus)(key_num + 1);
    return true;
}

bool NFC::read_data(uint8_t file_id, uint8_t* data, size_t* len) {
    if (!m_tag_present || !data || !len) return false;
    if (m_auth_status == NFCAuthStatus::NOT_AUTH) return false;
    
    uint16_t offset = 0;
    size_t total_read = 0;
    size_t to_read = *len;
    
    while (to_read > 0) {
        uint8_t chunk_size = (to_read > 250) ? 250 : to_read;
        
        uint8_t cmd[7];
        cmd[0] = NTAG424_CMD_READ_DATA;
        cmd[1] = file_id;
        cmd[2] = (offset >> 8) & 0xFF;
        cmd[3] = offset & 0xFF;
        cmd[4] = 0;
        cmd[5] = 0;
        cmd[6] = chunk_size;
        
        uint8_t response[256];
        size_t response_len = sizeof(response);
        
        if (!ntag424_send_command(cmd, 7, response, &response_len)) {
            return false;
        }
        
        memcpy(data + total_read, response, response_len);
        total_read += response_len;
        offset += chunk_size;
        to_read -= chunk_size;
        
        if (response_len < chunk_size) {
            break;  // End of file
        }
    }
    
    *len = total_read;
    return true;
}

bool NFC::write_data(uint8_t file_id, const uint8_t* data, size_t len) {
    if (!m_tag_present || !data) return false;
    if (m_auth_status != NFCAuthStatus::AUTH_KEY1 &&
        m_auth_status != NFCAuthStatus::AUTH_KEY2 &&
        m_auth_status != NFCAuthStatus::AUTH_KEY3) {
        return false;  // Need write access
    }
    
    uint16_t offset = 0;
    size_t written = 0;
    
    while (written < len) {
        uint8_t chunk_size = ((len - written) > 250) ? 250 : (len - written);
        
        uint8_t cmd[7 + 250];
        cmd[0] = NTAG424_CMD_WRITE_DATA;
        cmd[1] = file_id;
        cmd[2] = (offset >> 8) & 0xFF;
        cmd[3] = offset & 0xFF;
        cmd[4] = 0;
        cmd[5] = 0;
        cmd[6] = chunk_size;
        memcpy(cmd + 7, data + written, chunk_size);
        
        uint8_t response[8];
        size_t response_len = sizeof(response);
        
        if (!ntag424_send_command(cmd, 7 + chunk_size, response, &response_len)) {
            return false;
        }
        
        written += chunk_size;
        offset += chunk_size;
    }
    
    return true;
}

bool NFC::write_seed(const NFCSeedStorage* seed, const uint8_t key[NTAG424_KEY_SIZE]) {
    if (!m_tag_present || !seed || !key) return false;
    
    // Authenticate with key
    if (!authenticate(key, 1)) {  // Key 1 for write access
        return false;
    }
    
    // Write to proprietary file (File ID 0x03)
    size_t len = sizeof(NFCSeedStorage);
    return write_data(NTAG424_FILE_PROPRIETARY_FILE, 
                      reinterpret_cast<const uint8_t*>(seed), len);
}

bool NFC::read_seed(NFCSeedStorage* seed, const uint8_t key[NTAG424_KEY_SIZE]) {
    if (!m_tag_present || !seed || !key) return false;
    
    // Authenticate with key
    if (!authenticate(key, 0)) {  // Key 0 for read access
        return false;
    }
    
    // Read from proprietary file
    size_t len = sizeof(NFCSeedStorage);
    uint8_t buffer[sizeof(NFCSeedStorage)];
    
    if (!read_data(NTAG424_FILE_PROPRIETARY_FILE, buffer, &len)) {
        return false;
    }
    
    // Verify magic
    if (buffer[0] != 'S' || buffer[1] != 'S' || 
        buffer[2] != '4' || buffer[3] != '4') {
        return false;  // Not a SeedSigner seed
    }
    
    memcpy(seed, buffer, sizeof(NFCSeedStorage));
    return true;
}

bool NFC::delete_seed(const uint8_t key[NTAG424_KEY_SIZE]) {
    if (!m_tag_present || !key) return false;
    
    if (!authenticate(key, 1)) {
        return false;
    }
    
    // Overwrite with zeros
    uint8_t zeros[sizeof(NFCSeedStorage)];
    memset(zeros, 0, sizeof(zeros));
    
    return write_data(NTAG424_FILE_PROPRIETARY_FILE, zeros, sizeof(zeros));
}

bool NFC::has_seed() {
    if (!m_tag_present) return false;
    
    // Try to read first few bytes to check magic
    uint8_t buffer[4];
    size_t len = 4;
    
    if (!read_data(NTAG424_FILE_PROPRIETARY_FILE, buffer, &len)) {
        return false;
    }
    
    return (buffer[0] == 'S' && buffer[1] == 'S' && 
            buffer[2] == '4' && buffer[3] == '4');
}

void NFC::derive_key_from_pin(const char* pin, const uint8_t salt[16],
                              uint8_t key[NTAG424_KEY_SIZE]) {
    if (!pin || !salt || !key) return;
    
    // Simple PBKDF2-like derivation
    // In production, use proper PBKDF2 with more iterations
    
    uint8_t buffer[64];
    size_t pin_len = strlen(pin);
    
    // HMAC-SHA256(salt, pin)
    // TODO: Implement proper PBKDF2
    
    // For now, simple hash
    memset(key, 0, NTAG424_KEY_SIZE);
    for (size_t i = 0; i < pin_len && i < NTAG424_KEY_SIZE; i++) {
        key[i] = pin[i] ^ salt[i % 16];
    }
    
    Utils::SecureMemory::wipe(buffer, sizeof(buffer));
}

// PN532 Low-level functions
bool NFC::pn532_get_firmware_version(uint8_t version[4]) {
    uint8_t cmd = PN532_COMMAND_GETFIRMWAREVERSION;
    uint8_t response[12];
    size_t response_len = sizeof(response);
    
    if (!pn532_send_command(&cmd, 1, response, &response_len)) {
        return false;
    }
    
    if (response_len >= 4) {
        memcpy(version, response, 4);
        return true;
    }
    
    return false;
}

bool NFC::pn532_sam_configuration() {
    uint8_t cmd[4] = {
        PN532_COMMAND_SAMCONFIGURATION,
        0x01,  // Normal mode
        0x14,  // Timeout 50ms * 20 = 1s
        0x00   // IRQ not used
    };
    
    uint8_t response[8];
    size_t response_len = sizeof(response);
    
    return pn532_send_command(cmd, 4, response, &response_len);
}

bool NFC::pn532_in_list_passive_target(uint8_t* uid, uint8_t* uid_len) {
    uint8_t cmd[3] = {
        PN532_COMMAND_INLISTPASSIVETARGET,
        0x01,  // Max 1 target
        0x00   // ISO14443A (106 kbps)
    };
    
    uint8_t response[32];
    size_t response_len = sizeof(response);
    
    if (!pn532_send_command(cmd, 3, response, &response_len)) {
        return false;
    }
    
    if (response_len < 5 || response[0] != 0x01) {
        return false;  // No target found
    }
    
    // Parse response
    uint8_t target_id = response[1];
    uint8_t sens_res[2] = {response[2], response[3]};
    uint8_t sel_res = response[4];
    uint8_t nfcid_len = response[5];
    
    if (nfcid_len > 0 && nfcid_len <= 10) {
        memcpy(uid, &response[6], nfcid_len);
        *uid_len = nfcid_len;
        return true;
    }
    
    return false;
}

bool NFC::pn532_send_command(const uint8_t* cmd, size_t cmd_len,
                              uint8_t* response, size_t* response_len) {
    // Build PN532 frame
    uint8_t frame[32];
    uint8_t frame_len = 0;
    
    // Preamble
    frame[frame_len++] = 0x00;
    frame[frame_len++] = 0x00;
    frame[frame_len++] = 0xFF;
    
    // Length
    frame[frame_len++] = cmd_len + 1;
    frame[frame_len++] = 0x00 - (cmd_len + 1) + 1;  // LCS
    
    // TFI + Data
    frame[frame_len++] = 0xD4;  // Host to PN532
    memcpy(&frame[frame_len], cmd, cmd_len);
    frame_len += cmd_len;
    
    // DCS (Data Checksum)
    uint8_t dcs = 0xD4;
    for (size_t i = 0; i < cmd_len; i++) {
        dcs += cmd[i];
    }
    frame[frame_len++] = 0x00 - dcs;
    
    // Postamble
    frame[frame_len++] = 0x00;
    
    // Send frame
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    for (uint8_t i = 0; i < frame_len; i++) {
        Wire.write(frame[i]);
    }
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    delay(10);
    
    // Read response
    uint8_t read_len = Wire.requestFrom(PN532_I2C_ADDRESS, (uint8_t)32);
    if (read_len < 6) {
        return false;
    }
    
    // Parse response (simplified)
    uint8_t idx = 0;
    while (Wire.available() && idx < *response_len) {
        response[idx++] = Wire.read();
    }
    *response_len = idx;
    
    return true;
}

// NTAG 424 specific
bool NFC::ntag424_read_version(uint8_t* version) {
    uint8_t cmd = NTAG424_CMD_GET_VERSION;
    uint8_t response[32];
    size_t response_len = sizeof(response);
    
    if (!ntag424_send_command(&cmd, 1, response, &response_len)) {
        return false;
    }
    
    if (response_len >= 7) {
        memcpy(version, response, 7);
        return true;
    }
    
    return false;
}

bool NFC::ntag424_send_command(const uint8_t* cmd, size_t cmd_len,
                                uint8_t* response, size_t* response_len) {
    // Send through PN532 InDataExchange
    uint8_t indata[32];
    indata[0] = PN532_COMMAND_INDATAEXCHANGE;
    indata[1] = 0x01;  // Target ID
    memcpy(&indata[2], cmd, cmd_len);
    
    uint8_t frame[64];
    size_t frame_len = sizeof(frame);
    
    if (!pn532_send_command(indata, 2 + cmd_len, frame, &frame_len)) {
        return false;
    }
    
    // Parse response
    if (frame_len < 2 || frame[0] != 0x00) {
        return false;  // Error status
    }
    
    // Copy data part
    size_t data_len = frame_len - 1;
    if (data_len > *response_len) {
        data_len = *response_len;
    }
    memcpy(response, &frame[1], data_len);
    *response_len = data_len;
    
    return true;
}

} // namespace Drivers
} // namespace SeedSigner
