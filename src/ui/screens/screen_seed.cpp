/**
 * @file screen_seed.cpp
 * Seed management screens
 */

#include "ui/app.h"
#include <lvgl.h>
#include <M5Unified.h>

namespace SeedSigner {
namespace UI {

// Virtual keyboard for seed word entry
class VirtualKeyboard {
public:
    static lv_obj_t* create(lv_obj_t* parent, char* buffer, size_t buffer_size,
                           void (*on_done)(const char*), void (*on_cancel)()) {
        // Create keyboard
        lv_obj_t* kb = lv_keyboard_create(parent);
        lv_obj_set_size(kb, 320, 200);
        lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
        
        // Create text area
        lv_obj_t* ta = lv_textarea_create(parent);
        lv_obj_set_size(ta, 300, 40);
        lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 10);
        lv_textarea_set_max_length(ta, buffer_size - 1);
        lv_keyboard_set_textarea(kb, ta);
        
        // Set mode
        lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
        
        // Store callbacks in user data (simplified)
        (void)on_done;
        (void)on_cancel;
        
        return kb;
    }
};

// Seed display screen
void create_seed_display_screen(App* app, const char* mnemonic) {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1A1A1A), 0);
    
    // Title
    lv_obj_t* title = lv_label_create(screen);
    lv_label_set_text(title, "Recovery Phrase");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xF7931A), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // Warning
    lv_obj_t* warn = lv_label_create(screen);
    lv_label_set_text(warn, "Write these words down!");
    lv_obj_set_style_text_color(warn, lv_color_hex(0xFF0000), 0);
    lv_obj_align(warn, LV_ALIGN_TOP_MID, 0, 40);
    
    // Display words
    char mnemonic_copy[512];
    strncpy(mnemonic_copy, mnemonic, sizeof(mnemonic_copy) - 1);
    mnemonic_copy[sizeof(mnemonic_copy) - 1] = '\0';
    
    char* word = strtok(mnemonic_copy, " ");
    int word_num = 1;
    int y = 70;
    
    while (word && word_num <= 24) {
        lv_obj_t* lbl = lv_label_create(screen);
        lv_label_set_text_fmt(lbl, "%2d. %s", word_num, word);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_pos(lbl, 20 + ((word_num - 1) % 2) * 150, y);
        
        word = strtok(nullptr, " ");
        word_num++;
        if ((word_num - 1) % 2 == 0) {
            y += 20;
        }
    }
    
    // Done button
    lv_obj_t* btn = lv_btn_create(screen);
    lv_obj_set_size(btn, 100, 40);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xF7931A), 0);
    
    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "Done");
    lv_obj_center(lbl);
    
    lv_obj_add_event_cb(btn, [](lv_event_t* e) {
        App* app = (App*)lv_event_get_user_data(e);
        app->set_state(AppState::MAIN_MENU);
    }, LV_EVENT_CLICKED, app);
    
    lv_scr_load(screen);
}

// Seed generation method selection
void create_seed_menu_screen(App* app) {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1A1A1A), 0);
    
    // Title
    lv_obj_t* title = lv_label_create(screen);
    lv_label_set_text(title, "Seed Management");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xF7931A), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Options
    const char* options[] = {
        "Create New Seed",
        "Import Seed",
        "Load from NFC",
        "Export to NFC",
        "Delete Seed"
    };
    
    for (int i = 0; i < 5; i++) {
        lv_obj_t* btn = lv_btn_create(screen);
        lv_obj_set_size(btn, 280, 40);
        lv_obj_set_pos(btn, 20, 70 + i * 50);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x2A2A2A), 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xF7931A), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        
        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, options[i]);
        lv_obj_center(lbl);
        
        lv_obj_add_event_cb(btn, [](lv_event_t* e) {
            int idx = (int)(intptr_t)lv_event_get_user_data(e);
            (void)idx;
            // Handle different options
        }, LV_EVENT_CLICKED, (void*)(intptr_t)i);
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
    }, LV_EVENT_CLICKED, app);
    
    lv_scr_load(screen);
}

} // namespace UI
} // namespace SeedSigner
