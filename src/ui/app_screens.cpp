/**
 * @file app_screens.cpp
 * Screen integration for App
 */

#include "ui/app.h"
#include <lvgl.h>

namespace SeedSigner {
namespace UI {

// External screen creators
extern void show_splash_screen();
extern void show_security_warning();
extern void create_seed_menu_screen(App* app);
extern void create_seed_display_screen(App* app, const char* mnemonic);
extern void create_sign_scan_screen(App* app);
extern void create_sign_review_screen(App* app, Core::PSBT* psbt);
extern void create_sign_confirm_screen(App* app);
extern void create_settings_screen(App* app);
extern void create_tools_menu_screen(App* app);

void App::create_main_menu() {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1A1A1A), 0);
    
    // Title
    lv_obj_t* title = lv_label_create(screen);
    lv_label_set_text(title, "SeedSigner");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xF7931A), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Version
    lv_obj_t* ver = lv_label_create(screen);
    lv_label_set_text(ver, "ESP32-S3 v0.1");
    lv_obj_set_style_text_color(ver, lv_color_hex(0x888888), 0);
    lv_obj_align(ver, LV_ALIGN_TOP_MID, 0, 50);
    
    // Menu buttons grid
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
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xF7931A), LV_STATE_PRESSED);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xF7931A), 0);
        lv_obj_set_style_border_width(btn, 2, 0);
        
        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, labels[i]);
        lv_obj_center(lbl);
        
        lv_obj_set_user_data(btn, (void*)states[i]);
        
        lv_obj_add_event_cb(btn, [](lv_event_t* e) {
            lv_obj_t* btn = lv_event_get_target(e);
            App* app = (App*)lv_event_get_user_data(e);
            AppState state = (AppState)(uintptr_t)lv_obj_get_user_data(btn);
            app->set_state(state);
        }, LV_EVENT_CLICKED, this);
    }
    
    // Status indicator
    lv_obj_t* status = lv_label_create(screen);
    lv_label_set_text(status, m_seed_loaded ? "\u25CF Seed loaded" : "\u25CB No seed");
    lv_obj_set_style_text_color(status, 
        m_seed_loaded ? lv_color_hex(0x00FF00) : lv_color_hex(0x888888), 0);
    lv_obj_align(status, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    lv_scr_load(screen);
}

void App::create_seed_menu() {
    create_seed_menu_screen(this);
}

void App::create_seed_generate_screen() {
    // Show generation options
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1A1A1A), 0);
    
    lv_obj_t* title = lv_label_create(screen);
    lv_label_set_text(title, "Generate Seed");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xF7931A), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    const char* options[] = {
        "24 Words (Recommended)",
        "12 Words",
        "Use Dice Rolls",
        "Camera Entropy"
    };
    
    for (int i = 0; i < 4; i++) {
        lv_obj_t* btn = lv_btn_create(screen);
        lv_obj_set_size(btn, 280, 45);
        lv_obj_set_pos(btn, 20, 80 + i * 55);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x2A2A2A), 0);
        
        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, options[i]);
        lv_obj_center(lbl);
    }
    
    // Back button
    lv_obj_t* back = lv_btn_create(screen);
    lv_obj_set_size(back, 80, 35);
    lv_obj_align(back, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_set_style_bg_color(back, lv_color_hex(0x444444), 0);
    
    lv_obj_t* back_lbl = lv_label_create(back);
    lv_label_set_text(back_lbl, "Back");
    lv_obj_center(back_lbl);
    
    lv_obj_add_event_cb(back, [](lv_event_t* e) {
        App* app = (App*)lv_event_get_user_data(e);
        app->go_back();
    }, LV_EVENT_CLICKED, this);
    
    lv_scr_load(screen);
}

void App::create_seed_display_screen() {
    // Display current mnemonic
    // For now, show placeholder
    create_seed_display_screen(this, "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about");
}

void App::create_sign_scan_screen() {
    UI::create_sign_scan_screen(this);
}

void App::create_sign_review_screen() {
    UI::create_sign_review_screen(this, nullptr);
}

void App::create_sign_confirm_screen() {
    UI::create_sign_confirm_screen(this);
}

void App::create_settings_screen() {
    UI::create_settings_screen(this);
}

void App::create_tools_menu_screen() {
    UI::create_tools_menu_screen(this);
}

void App::on_enter_state(AppState state) {
    switch (state) {
        case AppState::SPLASH:
            show_splash_screen();
            show_security_warning();
            set_state(AppState::MAIN_MENU);
            break;
            
        case AppState::MAIN_MENU:
            create_main_menu();
            break;
            
        case AppState::SEED_MENU:
            create_seed_menu();
            break;
            
        case AppState::SEED_GENERATE:
            create_seed_generate_screen();
            break;
            
        case AppState::SEED_DISPLAY:
            create_seed_display_screen();
            break;
            
        case AppState::SIGN_SCAN:
            create_sign_scan_screen();
            break;
            
        case AppState::SIGN_REVIEW:
            create_sign_review_screen();
            break;
            
        case AppState::SIGN_CONFIRM:
            create_sign_confirm_screen();
            break;
            
        case AppState::SETTINGS:
            create_settings_screen();
            break;
            
        case AppState::TOOLS_MENU:
            create_tools_menu_screen();
            break;
            
        case AppState::SHUTDOWN:
            shutdown();
            break;
            
        default:
            break;
    }
}

} // namespace UI
} // namespace SeedSigner
