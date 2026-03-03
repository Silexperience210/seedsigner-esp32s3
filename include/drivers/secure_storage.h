#pragma once

#include <stdint.h>
#include <stddef.h>

// Secure Storage for preferences
// Uses ESP32 flash encryption when available
// Stores: wallet fingerprint (not seed!), settings

namespace SecureStorage {

// Initialize encrypted preferences
bool init();
void deinit();

// Wallet existence check
bool has_wallet();
void set_has_wallet(bool has);

// Fingerprint storage (public info only)
bool get_fingerprint(char fingerprint[9]);
bool set_fingerprint(const char fingerprint[9]);

// Settings
struct Settings {
    uint8_t default_address_type;  // 0=Legacy, 1=Segwit, 2=Native Segwit
    bool require_pin;
    uint32_t auto_lock_time;  // seconds
    bool testnet;
    char last_xpub[113];
};

bool get_settings(Settings& settings);
bool set_settings(const Settings& settings);

// Wipe all data
void wipe_all();

} // namespace SecureStorage
