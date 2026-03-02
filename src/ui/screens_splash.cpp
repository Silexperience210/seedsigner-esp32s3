/**
 * @file screens_splash.cpp
 * Splash screen with security warnings
 */

#include <lvgl.h>
#include <M5Unified.h>

namespace SeedSigner {
namespace UI {

void show_splash_screen() {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1A1A1A), 0);
    
    // Logo/title
    lv_obj_t* title = lv_label_create(screen);
    lv_label_set_text(title, "SeedSigner");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xF7931A), 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -40);
    
    // Subtitle
    lv_obj_t* subtitle = lv_label_create(screen);
    lv_label_set_text(subtitle, "ESP32-S3 Edition");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0x888888), 0);
    lv_obj_align(subtitle, LV_ALIGN_CENTER, 0, -5);
    
    // Version
    lv_obj_t* ver = lv_label_create(screen);
    lv_label_set_text(ver, "v0.1.0-alpha");
    lv_obj_set_style_text_color(ver, lv_color_hex(0x666666), 0);
    lv_obj_align(ver, LV_ALIGN_CENTER, 0, 20);
    
    // Loading indicator
    lv_obj_t* spinner = lv_spinner_create(screen, 1000, 60);
    lv_obj_set_size(spinner, 40, 40);
    lv_obj_align(spinner, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0xF7931A), LV_PART_INDICATOR);
    
    // Security notice
    lv_obj_t* notice = lv_label_create(screen);
    lv_label_set_text(notice, "Air-gapped • Stateless • Open Source");
    lv_obj_set_style_text_color(notice, lv_color_hex(0x00AA00), 0);
    lv_obj_set_style_text_font(notice, &lv_font_montserrat_12, 0);
    lv_obj_align(notice, LV_ALIGN_BOTTOM_MID, 0, -5);
    
    lv_scr_load(screen);
    lv_timer_handler();
    
    // Delay for splash
    delay(2000);
}

void show_security_warning() {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1A1A1A), 0);
    
    // Warning icon
    lv_obj_t* icon = lv_label_create(screen);
    lv_label_set_text(icon, LV_SYMBOL_WARNING);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFFAA00), 0);
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 30);
    
    // Title
    lv_obj_t* title = lv_label_create(screen);
    lv_label_set_text(title, "Security Notice");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFAA00), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 90);
    
    // Warning text
    lv_obj_t* text = lv_label_create(screen);
    lv_label_set_text(text, 
        "This is experimental software.\n"
        "Use only with test funds.\n"
        "Never share your seed phrase.");
    lv_obj_set_style_text_color(text, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(text, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(text, LV_ALIGN_CENTER, 0, 10);
    
    // Continue button
    lv_obj_t* btn = lv_btn_create(screen);
    lv_obj_set_size(btn, 150, 45);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xF7931A), 0);
    
    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "I Understand");
    lv_obj_center(lbl);
    
    lv_scr_load(screen);
    
    // Wait for button press
    bool pressed = false;
    while (!pressed) {
        M5.update();
        if (M5.Touch.getCount() > 0) {
            lv_indev_t* indev = lv_indev_get_next(NULL);
            lv_indev_data_t data;
            lv_indev_read(indev, &data);
            if (data.state == LV_INDEV_STATE_PRESSED) {
                pressed = true;
            }
        }
        lv_timer_handler();
        delay(10);
    }
}

} // namespace UI
} // namespace SeedSigner
