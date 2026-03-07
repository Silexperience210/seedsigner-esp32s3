/**
 * @file screen_settings.cpp
 * Settings and tools screens
 */

#include "ui/app.h"
#include <lvgl.h>
#include <M5Unified.h>

namespace SeedSigner {
namespace UI {

// Settings screen
void create_settings_screen(App* app) {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1A1A1A), 0);
    
    // Title
    lv_obj_t* title = lv_label_create(screen);
    lv_label_set_text(title, "Settings");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xF7931A), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Create scrollable container
    lv_obj_t* cont = lv_obj_create(screen);
    lv_obj_set_size(cont, 300, 180);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(cont, 10, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x2A2A2A), 0);
    
    // Brightness slider
    lv_obj_t* lbl_bright = lv_label_create(cont);
    lv_label_set_text(lbl_bright, "Brightness");
    lv_obj_set_style_text_color(lbl_bright, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t* slider = lv_slider_create(cont);
    lv_obj_set_width(slider, 250);
    lv_slider_set_range(slider, 10, 255);
    lv_slider_set_value(slider, 128, LV_ANIM_OFF);
    
    lv_obj_add_event_cb(slider, [](lv_event_t* e) {
        int val = lv_slider_get_value(lv_event_get_target(e));
        M5.Display.setBrightness(val);
    }, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Auto-lock switch
    lv_obj_t* cont_lock = lv_obj_create(cont);
    lv_obj_set_size(cont_lock, 280, 40);
    lv_obj_set_flex_flow(cont_lock, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_bg_opa(cont_lock, LV_OPA_TRANSP, 0);
    
    lv_obj_t* lbl_lock = lv_label_create(cont_lock);
    lv_label_set_text(lbl_lock, "Auto-lock");
    lv_obj_set_style_text_color(lbl_lock, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_flex_grow(lbl_lock, 1);
    
    lv_obj_t* sw_lock = lv_switch_create(cont_lock);
    lv_obj_add_state(sw_lock, LV_STATE_CHECKED);
    
    // Test camera button
    lv_obj_t* btn_cam = lv_btn_create(cont);
    lv_obj_set_size(btn_cam, 250, 35);
    lv_obj_set_style_bg_color(btn_cam, lv_color_hex(0x3A3A3A), 0);
    
    lv_obj_t* lbl_cam = lv_label_create(btn_cam);
    lv_label_set_text(lbl_cam, "Test Camera");
    lv_obj_center(lbl_cam);
    
    // Wipe memory button
    lv_obj_t* btn_wipe = lv_btn_create(cont);
    lv_obj_set_size(btn_wipe, 250, 35);
    lv_obj_set_style_bg_color(btn_wipe, lv_color_hex(0xAA2222), 0);
    
    lv_obj_t* lbl_wipe = lv_label_create(btn_wipe);
    lv_label_set_text(lbl_wipe, "Wipe Memory");
    lv_obj_center(lbl_wipe);
    
    lv_obj_add_event_cb(btn_wipe, [](lv_event_t* e) {
        // Show confirmation dialog
        App* app = (App*)lv_event_get_user_data(e);
        
        // Create modal
        lv_obj_t* mbox = lv_msgbox_create(NULL, "Confirm", 
            "This will wipe all seeds! Continue?", NULL, true);
        lv_obj_set_size(mbox, 280, 150);
        
        // Add buttons
        lv_obj_t* btn_yes = lv_btn_create(mbox);
        lv_obj_t* lbl_yes = lv_label_create(btn_yes);
        lv_label_set_text(lbl_yes, "Yes");
        
        lv_obj_t* btn_no = lv_btn_create(mbox);
        lv_obj_t* lbl_no = lv_label_create(btn_no);
        lv_label_set_text(lbl_no, "No");
        
        // Wipe and close
        app->clear_seed();
        lv_msgbox_close(mbox);
        
    }, LV_EVENT_CLICKED, app);
    
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

// Tools menu screen
void create_tools_menu_screen(App* app) {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1A1A1A), 0);
    
    // Title
    lv_obj_t* title = lv_label_create(screen);
    lv_label_set_text(title, "Tools");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xF7931A), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Tools list
    const char* tools[] = {
        "Address Explorer",
        "BIP85 Child Seeds",
        "Message Sign",
        "Verify Address",
        "XPUB Export",
        "Settings QR"
    };
    
    for (int i = 0; i < 6; i++) {
        lv_obj_t* btn = lv_btn_create(screen);
        lv_obj_set_size(btn, 280, 40);
        lv_obj_set_pos(btn, 20, 70 + i * 50);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x2A2A2A), 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xF7931A), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        
        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, tools[i]);
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
    }, LV_EVENT_CLICKED, app);
    
    lv_scr_load(screen);
}

} // namespace UI
} // namespace SeedSigner
