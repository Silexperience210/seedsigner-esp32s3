/**
 * @file app.cpp
 * Main UI Application implementation
 */

#include "ui/app.h"
#include <lvgl.h>
#include <M5Unified.h>

namespace SeedSigner {
namespace UI {

// Color scheme
#define SS_ORANGE lv_color_hex(0xF7931A)
#define SS_BG lv_color_hex(0x1A1A1A)
#define SS_TEXT lv_color_hex(0xFFFFFF)

App::App() : m_state(AppState::SPLASH), m_previous_state(AppState::SPLASH),
             m_seed_loaded(false) {
    memset(&m_settings, 0, sizeof(m_settings));
    memset(m_seed, 0, sizeof(m_seed));
    memset(&m_xpub, 0, sizeof(m_xpub));
}

App::~App() {}

bool App::init() {
    // Load settings
    load_settings();
    
    // Apply theme
    setup_theme();
    
    // Show splash
    set_state(AppState::MAIN_MENU);
    
    return true;
}

void App::update() {
    // Check auto-lock
    check_auto_lock();
}

void App::set_state(AppState state) {
    if (m_state == state) return;
    
    on_exit_state(m_state);
    m_previous_state = m_state;
    m_state = state;
    on_enter_state(m_state);
}

void App::go_back() {
    set_state(m_previous_state);
}

void App::on_enter_state(AppState state) {
    switch (state) {
        case AppState::SPLASH:
            // Show splash screen
            break;
            
        case AppState::MAIN_MENU:
            create_main_menu();
            break;
            
        case AppState::SEED_MENU:
            // create_seed_menu();
            break;
            
        case AppState::SEED_GENERATE:
            // create_seed_generate_screen();
            break;
            
        case AppState::SIGN_SCAN:
            // create_sign_scan_screen();
            break;
            
        case AppState::SETTINGS:
            // create_settings_screen();
            break;
            
        case AppState::SHUTDOWN:
            shutdown();
            break;
            
        default:
            break;
    }
}

void App::on_exit_state(AppState state) {
    // Cleanup current screen
    // LVGL handles this automatically when loading new screen
    (void)state;
}

void App::create_main_menu() {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, SS_BG, 0);
    
    // Title
    lv_obj_t* title = lv_label_create(screen);
    lv_label_set_text(title, "SeedSigner");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(title, SS_ORANGE, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Version
    lv_obj_t* ver = lv_label_create(screen);
    lv_label_set_text(ver, "ESP32-S3 v0.1");
    lv_obj_set_style_text_color(ver, lv_color_hex(0x888888), 0);
    lv_obj_align(ver, LV_ALIGN_TOP_MID, 0, 50);
    
    // Menu buttons
    const char* labels[] = {"Seed", "Sign", "Tools", "Settings"};
    AppState states[] = {
        AppState::SEED_MENU,
        AppState::SIGN_SCAN,
        AppState::TOOLS_MENU,
        AppState::SETTINGS
    };
    
    for (int i = 0; i < 4; i++) {
        lv_obj_t* btn = lv_btn_create(screen);
        lv_obj_set_size(btn, 120, 50);
        lv_obj_set_pos(btn, 40 + (i % 2) * 140, 100 + (i / 2) * 70);
        
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x2A2A2A), 0);
        lv_obj_set_style_bg_color(btn, SS_ORANGE, LV_STATE_PRESSED);
        lv_obj_set_style_border_color(btn, SS_ORANGE, 0);
        lv_obj_set_style_border_width(btn, 2, 0);
        
        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, labels[i]);
        lv_obj_center(lbl);
        
        // Store state in user data
        lv_obj_set_user_data(btn, (void*)states[i]);
        
        lv_obj_add_event_cb(btn, [](lv_event_t* e) {
            lv_obj_t* btn = lv_event_get_target(e);
            App* app = (App*)lv_event_get_user_data(e);
            AppState state = (AppState)(uintptr_t)lv_obj_get_user_data(btn);
            app->set_state(state);
        }, LV_EVENT_CLICKED, this);
    }
    
    // Status
    lv_obj_t* status = lv_label_create(screen);
    lv_label_set_text(status, m_seed_loaded ? "● Seed loaded" : "○ No seed");
    lv_obj_set_style_text_color(status, 
        m_seed_loaded ? lv_color_hex(0x00FF00) : lv_color_hex(0x888888), 0);
    lv_obj_align(status, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    lv_scr_load(screen);
}

void App::setup_theme() {
    // Configure LVGL theme
    lv_disp_t* disp = lv_disp_get_default();
    lv_theme_t* th = lv_theme_default_init(disp, SS_ORANGE, SS_BG, 
                                            true, &lv_font_montserrat_14);
    lv_disp_set_theme(disp, th);
}

void App::load_seed(const uint8_t seed[64]) {
    if (!seed) return;
    
    memcpy(m_seed, seed, 64);
    m_seed_loaded = true;
    
    // Derive xpub
    m_bip32.init_from_seed(seed);
    const Core::ExtendedKey* master = m_bip32.get_master_key();
    m_bip32.get_public_key(master, &m_xpub);
}

void App::clear_seed() {
    // Secure wipe
    for (int i = 0; i < 64; i++) {
        m_seed[i] = 0;
    }
    m_seed_loaded = false;
    m_bip32.clear();
    memset(&m_xpub, 0, sizeof(m_xpub));
}

void App::load_settings() {
    // Load from NVS or use defaults
    m_settings.brightness = 128;
    m_settings.sound_enabled = false;
    m_settings.auto_lock = true;
    m_settings.auto_lock_timeout = 300000;  // 5 minutes
    strcpy(m_settings.default_network, "mainnet");
    m_settings.disable_wifi_bt = true;
}

void App::save_settings() {
    // Save to NVS
}

void App::check_auto_lock() {
    if (!m_settings.auto_lock) return;
    
    // Check timeout
    // TODO: Implement activity tracking
}

void App::reset_activity_timer() {
    // m_last_activity = millis();
}

void App::shutdown() {
    // Wipe secrets
    clear_seed();
    
    // Show shutdown message
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, SS_BG, 0);
    
    lv_obj_t* lbl = lv_label_create(screen);
    lv_label_set_text(lbl, "Shutting down...");
    lv_obj_set_style_text_color(lbl, SS_TEXT, 0);
    lv_obj_center(lbl);
    
    lv_scr_load(screen);
    lv_timer_handler();
    
    delay(1000);
    
    // Power off
    M5.Power.powerOff();
}

} // namespace UI
} // namespace SeedSigner
