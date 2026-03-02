/**
 * @file nfc.h
 * NFC driver for NTAG 424 DNA secure storage
 */

#ifndef DRIVERS_NFC_H
#define DRIVERS_NFC_H

#include <Arduino.h>
#include <stdint.h>

namespace SeedSigner {
namespace Drivers {

// NTAG 424 constants
#define NTAG424_MAX_DATA_SIZE 256
#define NTAG424_UID_SIZE 7
#define NTAG424_KEY_SIZE 16

// NTAG 424 file IDs
#define NTAG424_FILE_CC 0x01      // Capability Container
#define NTAG424_FILE_NDEF 0x02    // NDEF message
#define NTAG424_FILE_PROPRIETARY 0x03  // Proprietary data

// Authentication status
enum class NFCAuthStatus {
    NOT_AUTH = 0,
    AUTH_KEY0,    // Read-only key
    AUTH_KEY1,    // Read/Write key
    AUTH_KEY2,    // Custom key
    AUTH_KEY3     // Custom key
};

// NTAG 424 tag info
struct NTAG424Tag {
    uint8_t uid[NTAG424_UID_SIZE];
    uint8_t version[7];
    uint16_t memory_size;
    NFCAuthStatus auth_status;
    bool is_tamper_evident;
    uint32_t read_counter;
};

// Encrypted seed storage format
struct NFCSeedStorage {
    uint8_t magic[4];           // "SS44"
    uint8_t version;            // 0x01
    uint8_t flags;              // Bit flags
    uint8_t encrypted_seed[64]; // AES-128 encrypted BIP39 seed
    uint8_t iv[16];             // AES IV
    uint8_t checksum[4];        // CRC32
    char label[32];             // User label
    uint32_t timestamp;         // Creation timestamp
};

class NFC {
public:
    NFC();
    ~NFC();
    
    bool init();
    void deinit();
    
    // Polling (call in loop)
    void poll();
    
    // Tag detection
    bool is_tag_present();
    bool read_tag_info(NTAG424Tag* info);
    
    // Authentication
    bool authenticate(const uint8_t key[NTAG424_KEY_SIZE], uint8_t key_num);
    bool change_key(uint8_t key_num, const uint8_t old_key[NTAG424_KEY_SIZE],
                    const uint8_t new_key[NTAG424_KEY_SIZE]);
    
    // Read/Write operations
    bool read_data(uint8_t file_id, uint8_t* data, size_t* len);
    bool write_data(uint8_t file_id, const uint8_t* data, size_t len);
    
    // Seed-specific operations
    bool write_seed(const NFCSeedStorage* seed, const uint8_t key[NTAG424_KEY_SIZE]);
    bool read_seed(NFCSeedStorage* seed, const uint8_t key[NTAG424_KEY_SIZE]);
    bool delete_seed(const uint8_t key[NTAG424_KEY_SIZE]);
    bool has_seed();
    
    // SUN message authentication
    bool get_sun_message(uint8_t* sun_msg, size_t* len);
    bool verify_sun_signature(const uint8_t* sun_msg, size_t len,
                              const uint8_t key[NTAG424_KEY_SIZE]);
    
    // Format tag (factory reset)
    bool format_tag(const uint8_t default_key[NTAG424_KEY_SIZE]);
    
    // Key derivation from PIN
    static void derive_key_from_pin(const char* pin, 
                                    const uint8_t salt[16],
                                    uint8_t key[NTAG424_KEY_SIZE]);
    
    // Status
    bool is_initialized() const { return m_initialized; }
    NFCAuthStatus get_auth_status() const { return m_auth_status; }
    
    // Callbacks
    typedef void (*TagCallback)(bool present, const NTAG424Tag* info);
    void set_tag_callback(TagCallback cb) { m_tag_callback = cb; }
    
private:
    bool m_initialized;
    bool m_tag_present;
    NFCAuthStatus m_auth_status;
    NTAG424Tag m_current_tag;
    TagCallback m_tag_callback;
    
    // PN532 interface
    void* m_pn532;  // Opaque PN532 handle
    
    // Low-level NTAG 424 commands
    bool ntag424_read_version(uint8_t* version);
    bool ntag424_authenticate_ev2_first(uint8_t key_num, 
                                        const uint8_t key[NTAG424_KEY_SIZE]);
    bool ntag424_authenticate_ev2_non_first(uint8_t key_num);
    bool ntag424_read_data_file(uint8_t file_id, uint8_t offset,
                                uint8_t* data, size_t len);
    bool ntag424_write_data_file(uint8_t file_id, uint8_t offset,
                                 const uint8_t* data, size_t len);
    bool ntag424_get_sun(uint8_t* sun_data, size_t* len);
    
    // Crypto helpers
    void calculate_session_keys(const uint8_t* rand_chal, 
                                const uint8_t* rand_resp,
                                uint8_t* enc_key, uint8_t* mac_key);
    void calculate_mac(const uint8_t* data, size_t len,
                       const uint8_t* key, uint8_t* mac);
};

} // namespace Drivers
} // namespace SeedSigner

#endif // DRIVERS_NFC_H
