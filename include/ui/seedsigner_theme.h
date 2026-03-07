/**
 * @file seedsigner_theme.h
 * Official SeedSigner UI Theme - Pixel-perfect replica
 * Adapted for ESP32-S3 320×240 touchscreen
 */

#ifndef UI_SEEDSIGNER_THEME_H
#define UI_SEEDSIGNER_THEME_H

#include <lvgl.h>

namespace SeedSigner {
namespace UI {

// ═══════════════════════════════════════════════════════════════
// SEEDSIGNER OFFICIAL COLORS (from RPi version)
// ═══════════════════════════════════════════════════════════════

// Primary colors
#define SS_COLOR_BLACK          lv_color_hex(0x000000)
#define SS_COLOR_DARK_GRAY      lv_color_hex(0x333333)
#define SS_COLOR_MEDIUM_GRAY    lv_color_hex(0x666666)
#define SS_COLOR_LIGHT_GRAY     lv_color_hex(0x999999)
#define SS_COLOR_WHITE          lv_color_hex(0xFFFFFF)

// Bitcoin/Brand colors
#define SS_COLOR_ORANGE         lv_color_hex(0xF7931A)
#define SS_COLOR_ORANGE_DARK    lv_color_hex(0xD47B0F)
#define SS_COLOR_ORANGE_LIGHT   lv_color_hex(0xFFAD42)

// Status colors
#define SS_COLOR_GREEN          lv_color_hex(0x00AA00)
#define SS_COLOR_RED            lv_color_hex(0xCC0000)
#define SS_COLOR_YELLOW         lv_color_hex(0xFFCC00)

// Background shades
#define SS_COLOR_BG_DARK        lv_color_hex(0x1A1A1A)
#define SS_COLOR_BG_MEDIUM      lv_color_hex(0x2A2A2A)
#define SS_COLOR_BG_LIGHT       lv_color_hex(0x3A3A3A)

// ═══════════════════════════════════════════════════════════════
// DIMENSIONS (Original: 240×240, Adapted: 320×240)
// ═══════════════════════════════════════════════════════════════

// Screen
#define SS_SCREEN_WIDTH         320
#define SS_SCREEN_HEIGHT        240

// Safe area (mimic 240×240 centered)
#define SS_SAFE_X               40
#define SS_SAFE_Y               0
#define SS_SAFE_WIDTH           240
#define SS_SAFE_HEIGHT          240

// Header
#define SS_HEADER_HEIGHT        30
#define SS_HEADER_FONT          &lv_font_montserrat_14

// Footer
#define SS_FOOTER_HEIGHT        25

// Buttons (original RPi style)
#define SS_BUTTON_HEIGHT        35
#define SS_BUTTON_WIDTH         110
#define SS_BUTTON_FONT          &lv_font_montserrat_14

// Text
#define SS_FONT_SMALL           &lv_font_montserrat_14
#define SS_FONT_MEDIUM          &lv_font_montserrat_14
#define SS_FONT_LARGE           &lv_font_montserrat_14
#define SS_FONT_XLARGE          &lv_font_montserrat_14
#define SS_FONT_MNEMONIC        &lv_font_montserrat_14  // Should be monospace

// QR Code
#define SS_QR_SIZE              180
#define SS_QR_QUIET_ZONE        4

// ═══════════════════════════════════════════════════════════════
// SPACING (Original RPi grid)
// ═══════════════════════════════════════════════════════════════

#define SS_PADDING_SMALL        4
#define SS_PADDING_MEDIUM       8
#define SS_PADDING_LARGE        12

#define SS_MARGIN_SMALL         4
#define SS_MARGIN_MEDIUM        8
#define SS_MARGIN_LARGE         16

// ═══════════════════════════════════════════════════════════════
// STYLES (Official SeedSigner look)
// ═══════════════════════════════════════════════════════════════

class SeedSignerTheme {
public:
    static void init();
    static void deinit();
    
    // Main styles
    static lv_style_t* get_style_bg();
    static lv_style_t* get_style_header();
    static lv_style_t* get_style_footer();
    static lv_style_t* get_style_button();
    static lv_style_t* get_style_button_pressed();
    static lv_style_t* get_style_button_disabled();
    static lv_style_t* get_style_text_normal();
    static lv_style_t* get_style_text_highlight();
    static lv_style_t* get_style_text_warning();
    static lv_style_t* get_style_text_error();
    static lv_style_t* get_style_qr_bg();
    static lv_style_t* get_style_panel();
    static lv_style_t* get_style_list_item();
    static lv_style_t* get_style_list_item_selected();
    
    // Screen creation helpers
    static lv_obj_t* create_screen_base();
    static lv_obj_t* create_header(lv_obj_t* parent, const char* title);
    static lv_obj_t* create_footer(lv_obj_t* parent, const char* hint);
    static lv_obj_t* create_button(lv_obj_t* parent, const char* text, 
                                    lv_coord_t x, lv_coord_t y,
                                    lv_event_cb_t cb, void* user_data);
    static lv_obj_t* create_icon_button(lv_obj_t* parent, const char* icon,
                                         lv_coord_t x, lv_coord_t y,
                                         lv_event_cb_t cb, void* user_data);
    static lv_obj_t* create_qr_display(lv_obj_t* parent, const char* data,
                                        lv_coord_t x, lv_coord_t y,
                                        lv_coord_t size);
    
private:
    static bool s_initialized;
    static lv_style_t s_style_bg;
    static lv_style_t s_style_header;
    static lv_style_t s_style_footer;
    static lv_style_t s_style_button;
    static lv_style_t s_style_button_pressed;
    static lv_style_t s_style_button_disabled;
    static lv_style_t s_style_text_normal;
    static lv_style_t s_style_text_highlight;
    static lv_style_t s_style_text_warning;
    static lv_style_t s_style_text_error;
    static lv_style_t s_style_qr_bg;
    static lv_style_t s_style_panel;
    static lv_style_t s_style_list_item;
    static lv_style_t s_style_list_item_selected;
};

// ═══════════════════════════════════════════════════════════════
// COMPONENT HELPERS
// ═══════════════════════════════════════════════════════════════

class SeedSignerUI {
public:
    // Navigation hints (replacing joystick with touch)
    static void show_nav_hint(lv_obj_t* parent, const char* text);
    static void show_back_button(lv_obj_t* parent, lv_event_cb_t cb, void* user_data);
    static void show_confirm_button(lv_obj_t* parent, const char* text,
                                     lv_event_cb_t cb, void* user_data);
    
    // Seed display
    static void display_mnemonic_words(lv_obj_t* parent, const char* mnemonic,
                                        int start_index, int count);
    static void display_seedqr(lv_obj_t* parent, const char* mnemonic);
    
    // Status indicators
    static lv_obj_t* create_status_icon(lv_obj_t* parent, bool success);
    static void show_toast(lv_obj_t* parent, const char* message, uint32_t duration_ms);
    
    // Progress/dialogs
    static lv_obj_t* create_loading_spinner(lv_obj_t* parent, const char* text);
    static lv_obj_t* create_confirmation_dialog(lv_obj_t* parent, 
                                                 const char* title,
                                                 const char* message,
                                                 lv_event_cb_t on_yes,
                                                 lv_event_cb_t on_no,
                                                 void* user_data);
};

} // namespace UI
} // namespace SeedSigner

#endif // UI_SEEDSIGNER_THEME_H
