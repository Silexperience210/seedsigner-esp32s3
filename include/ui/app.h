/**
 * @file app.h
 * Main UI Application (LVGL-based)
 */

#ifndef UI_APP_H
#define UI_APP_H

#include <Arduino.h>
#include <lvgl.h>
#include "core/bip39.h"
#include "core/bip32.h"
#include "core/psbt.h"

namespace SeedSigner {
namespace UI {

// Application states
enum class AppState {
    SPLASH = 0,
    MAIN_MENU,
    SEED_MENU,
    SEED_GENERATE,
    SEED_IMPORT,
    SEED_DISPLAY,
    SIGN_SCAN,
    SIGN_REVIEW,
    SIGN_CONFIRM,
    SETTINGS,
    TOOLS_MENU,
    ADDRESS_VERIFY,
    XPUB_EXPORT,
    BIP85_DERIVE,
    NFC_READ,
    NFC_WRITE,
    SHUTDOWN
};

// Settings
struct Settings {
    uint8_t brightness;
    bool sound_enabled;
    bool auto_lock;
    uint32_t auto_lock_timeout;
    char default_network[8];  // "mainnet", "testnet"
    bool disable_wifi_bt;     // Always true for security
};

class App {
public:
    App();
    ~App();
    
    bool init();
    void update();
    
    // State management
    void set_state(AppState state);
    AppState get_state() const { return m_state; }
    void go_back();
    
    // Event handlers
    void on_touch(int16_t x, int16_t y);
    void on_button(uint8_t button);
    void on_nfc_event(bool present);
    
    // Seed management
    bool has_active_seed() const { return m_seed_loaded; }
    void load_seed(const uint8_t seed[64]);
    void clear_seed();
    void reset_activity_timer();
    const Core::ExtendedKey* get_xpub() const { return &m_xpub; }
    
    // Settings
    const Settings* get_settings() const { return &m_settings; }
    void save_settings();
    void load_settings();
    
    // Shutdown
    void shutdown();
    
private:
    AppState m_state;
    AppState m_previous_state;
    Settings m_settings;
    
    // Seed state
    bool m_seed_loaded;
    uint8_t m_seed[64];
    Core::BIP39 m_bip39;
    Core::BIP32 m_bip32;
    Core::ExtendedKey m_xpub;
    
    // PSBT state
    Core::PSBT m_psbt;
    Core::PSBTProcessor m_psbt_processor;
    
    // LVGL screens (opaque handles)
    lv_obj_t* m_screen_splash;
    lv_obj_t* m_screen_main;
    lv_obj_t* m_screen_seed_menu;
    lv_obj_t* m_screen_seed_generate;
    lv_obj_t* m_screen_seed_display;
    lv_obj_t* m_screen_sign_scan;
    lv_obj_t* m_screen_sign_review;
    lv_obj_t* m_screen_settings;
    
    // Screen creation functions
    void create_splash_screen();
    void create_main_menu();
    void create_seed_menu();
    void create_seed_generate_screen();
    void create_seed_display_screen();
    void create_sign_scan_screen();
    void create_sign_review_screen();
    void create_sign_confirm_screen();
    void create_settings_screen();
    void create_tools_menu_screen();
    void create_nfc_screen();
    
    // State transition handlers
    void on_enter_state(AppState state);
    void on_exit_state(AppState state);
    
    // Theme and styling
    void setup_theme();
    void apply_seedsigner_theme();
    
    // Keyboard handling
    void show_keyboard(const char* title, char* buffer, size_t buffer_len);
    void hide_keyboard();
    
    // Alert/Confirm dialogs
    void show_alert(const char* title, const char* message);
    void show_confirm(const char* title, const char* message, 
                      void (*on_yes)(), void (*on_no)());
    
    // Auto-lock
    uint32_t m_last_activity;
    void check_auto_lock();
};

} // namespace UI
} // namespace SeedSigner

#endif // UI_APP_H
