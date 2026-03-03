#pragma once

#include <stdint.h>
#include <stddef.h>

// PN532 NFC Driver
// Supports NTAG424 DNA for encrypted seed backup

namespace PN532_NFC {

// Card types
enum class CardType {
    UNKNOWN,
    NTAG213,
    NTAG215,
    NTAG216,
    NTAG424,  // DNA - supports AES encryption
    MIFARE_CLASSIC
};

// Initialize PN532 over I2C
bool init();
void deinit();

// Card detection
bool detect_card();
CardType get_card_type();

// Read/Write NTAG
bool read_page(uint8_t page, uint8_t data[4]);
bool write_page(uint8_t page, const uint8_t data[4]);

// NTAG424 DNA specific (encrypted)
bool authenticate_dna(const uint8_t key[16]);
bool read_encrypted(uint8_t page, uint8_t data[4]);
bool write_encrypted(uint8_t page, const uint8_t data[4]);

// SeedSigner backup format (SS44)
struct SeedBackup {
    uint8_t version;      // 0x44 = 'D' for DNA
    uint8_t encrypted_seed[64];
    uint8_t fingerprint[4];
    uint8_t checksum;
};

bool write_seed_backup(const SeedBackup& backup);
bool read_seed_backup(SeedBackup& backup);

// SUN message authentication (NTAG424)
bool verify_sun(const uint8_t* message, size_t len, const uint8_t* signature);

// Field control
void rf_field_on();
void rf_field_off();

} // namespace PN532_NFC
