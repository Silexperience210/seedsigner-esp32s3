#include "drivers/secure_storage.h"
#include <Preferences.h>
#include <Arduino.h>

// Namespace for preferences
#define STORAGE_NAMESPACE "seedsigner"
#define KEY_HAS_WALLET "has_wallet"
#define KEY_FINGERPRINT "fingerprint"
#define KEY_SETTINGS "settings"

static Preferences prefs;
static bool initialized = false;

namespace SecureStorage {

bool init() {
    if (initialized) return true;
    
    if (!prefs.begin(STORAGE_NAMESPACE, false)) {
        Serial.println("Failed to init secure storage");
        return false;
    }
    
    initialized = true;
    return true;
}

void deinit() {
    if (initialized) {
        prefs.end();
        initialized = false;
    }
}

bool has_wallet() {
    if (!initialized) return false;
    return prefs.getBool(KEY_HAS_WALLET, false);
}

void set_has_wallet(bool has) {
    if (!initialized) return;
    prefs.putBool(KEY_HAS_WALLET, has);
}

bool get_fingerprint(char fingerprint[9]) {
    if (!initialized) return false;
    
    String fp = prefs.getString(KEY_FINGERPRINT, "");
    if (fp.length() == 0) return false;
    
    strncpy(fingerprint, fp.c_str(), 8);
    fingerprint[8] = '\0';
    return true;
}

bool set_fingerprint(const char fingerprint[9]) {
    if (!initialized) return false;
    
    return prefs.putString(KEY_FINGERPRINT, fingerprint) == strlen(fingerprint);
}

bool get_settings(Settings& settings) {
    if (!initialized) return false;
    
    // Read as blob
    size_t len = prefs.getBytes(KEY_SETTINGS, &settings, sizeof(Settings));
    if (len != sizeof(Settings)) {
        // Return defaults
        settings.default_address_type = 2;  // Native Segwit
        settings.require_pin = true;
        settings.auto_lock_time = 300;  // 5 minutes
        settings.testnet = false;
        memset(settings.last_xpub, 0, sizeof(settings.last_xpub));
        return false;
    }
    
    return true;
}

bool set_settings(const Settings& settings) {
    if (!initialized) return false;
    
    return prefs.putBytes(KEY_SETTINGS, &settings, sizeof(Settings)) == sizeof(Settings);
}

void wipe_all() {
    if (!initialized) return;
    
    prefs.clear();
    Serial.println("Secure storage wiped");
}

} // namespace SecureStorage
