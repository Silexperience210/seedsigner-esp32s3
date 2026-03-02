/**
 * @file screen_main_replica.cpp
 * Main menu - Official SeedSigner UI replica
 * Centered 240×240 area on 320×240 screen
 */

#include "ui/app.h"
#include "ui/seedsigner_theme.h"
#include <lvgl.h>

namespace SeedSigner {
namespace UI {

void create_main_menu_replica(App* app) {
    SeedSignerTheme::init();
    
    lv_obj_t* screen = SeedSignerTheme::create_screen_base();
    
    // HEADER (centered 240px)
    lv_obj_t* header = lv_obj_create(screen);
    lv_obj_set_size(header, 240, 28);
    lv_obj_set_pos(header, 40, 0);
    lv_obj_add_style(header, SeedSignerTheme::get_style_header(), 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, "SeedSigner");
    lv_obj_set_style_text_color(title, SS_COLOR_ORANGE, 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
    lv_obj_center(title);
    
    // Subtitle
    lv_obj_t* subtitle = lv_label_create(screen);
    lv_label_set_text(subtitle, "v0.1.0-alpha");
    lv_obj_set_style_text_color(subtitle, SS_COLOR_MEDIUM_GRAY, 0);
    lv_obj_set_style_text_font(subtitle, SS_FONT_SMALL, 0);
    lv_obj_set_pos(subtitle, 40 + 85, 35);
    
    // 4 buttons grid (2×2)
    const char* btn_labels[] = {"Seed", "Sign", "Tools", "Settings"};
    AppState states[] = {
        AppState::SEED_MENU,
        AppState::SIGN_SCAN,
        AppState::TOOLS_MENU,
        AppState::SETTINGS
    };
    
    int btn_width = 105;
    int btn_height = 50;
    int gap_x = 15;
    int gap_y = 15;
    int start_x = 40 + 12;
    int start_y = 65;
    
    for (int i = 0; i < 4; i++) {
        int col = i % 2;
        int row = i / 2;
        int x = start_x + col * (btn_width + gap_x);
        int y = start_y + row * (btn_height + gap_y);
        
        lv_obj_t* btn = lv_btn_create(screen);
        lv_obj_set_size(btn, btn_width, btn_height);
        lv_obj_set_pos(btn, x, y);
        lv_obj_add_style(btn, SeedSignerTheme::get_style_button(), 0);
        lv_obj_add_style(btn, SeedSignerTheme::get_style_button_pressed(), LV_STATE_PRESSED);
        
        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, btn_labels[i]);
        lv_obj_set_style_text_font(lbl, SS_FONT_LARGE, 0);
        lv_obj_center(lbl);
        
        lv_obj_set_user_data(btn, (void*)(intptr_t)states[i]);
        
        lv_obj_add_event_cb(btn, [](lv_event_t* e) {
            App* app = (App*)lv_event_get_user_data(e);
            AppState state = (AppState)(intptr_t)lv_event_get_user_data(e);
            (void)state;
            // app->set_state(state);
        }, LV_EVENT_CLICKED, app);
    }
    
    // FOOTER
    lv_obj_t* footer = lv_obj_create(screen);
    lv_obj_set_size(footer, 240, 22);
    lv_obj_set_pos(footer, 40, 218);
    lv_obj_add_style(footer, SeedSignerTheme::get_style_footer(), 0);
    lv_obj_clear_flag(footer, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t* status = lv_label_create(footer);
    if (app->has_active_seed()) {
        lv_label_set_text(status, "\u25CF Seed Loaded");
        lv_obj_set_style_text_color(status, SS_COLOR_GREEN, 0);
    } else {
        lv_label_set_text(status, "\u25CB No Seed");
        lv_obj_set_style_text_color(status, SS_COLOR_MEDIUM_GRAY, 0);
    }
    lv_obj_set_style_text_font(status, SS_FONT_SMALL, 0);
    lv_obj_center(status);
    
    lv_scr_load(screen);
}

} // namespace UI
} // namespace SeedSigner
