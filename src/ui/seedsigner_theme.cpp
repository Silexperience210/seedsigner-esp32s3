/**
 * @file seedsigner_theme.cpp
 * Official SeedSigner theme implementation
 * Replicates the RPi UI pixel-perfectly
 */

#include "ui/seedsigner_theme.h"
#include <string.h>

namespace SeedSigner {
namespace UI {

// Static members
bool SeedSignerTheme::s_initialized = false;
lv_style_t SeedSignerTheme::s_style_bg;
lv_style_t SeedSignerTheme::s_style_header;
lv_style_t SeedSignerTheme::s_style_footer;
lv_style_t SeedSignerTheme::s_style_button;
lv_style_t SeedSignerTheme::s_style_button_pressed;
lv_style_t SeedSignerTheme::s_style_button_disabled;
lv_style_t SeedSignerTheme::s_style_text_normal;
lv_style_t SeedSignerTheme::s_style_text_highlight;
lv_style_t SeedSignerTheme::s_style_text_warning;
lv_style_t SeedSignerTheme::s_style_text_error;
lv_style_t SeedSignerTheme::s_style_qr_bg;
lv_style_t SeedSignerTheme::s_style_panel;
lv_style_t SeedSignerTheme::s_style_list_item;
lv_style_t SeedSignerTheme::s_style_list_item_selected;

void SeedSignerTheme::init() {
    if (s_initialized) return;
    
    // ═══════════════════════════════════════════════════════════
    // BACKGROUND STYLE (Black, no border)
    // ═══════════════════════════════════════════════════════════
    lv_style_init(&s_style_bg);
    lv_style_set_bg_color(&s_style_bg, SS_COLOR_BG_DARK);
    lv_style_set_bg_opa(&s_style_bg, LV_OPA_COVER);
    lv_style_set_border_width(&s_style_bg, 0);
    lv_style_set_pad_all(&s_style_bg, 0);
    
    // ═══════════════════════════════════════════════════════════
    // HEADER STYLE (Top bar, dark gray)
    // ═══════════════════════════════════════════════════════════
    lv_style_init(&s_style_header);
    lv_style_set_bg_color(&s_style_header, SS_COLOR_BG_MEDIUM);
    lv_style_set_bg_opa(&s_style_header, LV_OPA_COVER);
    lv_style_set_text_color(&s_style_header, SS_COLOR_ORANGE);
    lv_style_set_text_font(&s_style_header, SS_HEADER_FONT);
    lv_style_set_border_width(&s_style_header, 0);
    lv_style_set_pad_hor(&s_style_header, SS_PADDING_MEDIUM);
    lv_style_set_pad_ver(&s_style_header, SS_PADDING_SMALL);
    
    // ═══════════════════════════════════════════════════════════
    // FOOTER STYLE (Bottom hint bar)
    // ═══════════════════════════════════════════════════════════
    lv_style_init(&s_style_footer);
    lv_style_set_bg_color(&s_style_footer, SS_COLOR_BG_MEDIUM);
    lv_style_set_bg_opa(&s_style_footer, LV_OPA_COVER);
    lv_style_set_text_color(&s_style_footer, SS_COLOR_LIGHT_GRAY);
    lv_style_set_text_font(&s_style_footer, SS_FONT_SMALL);
    lv_style_set_border_width(&s_style_footer, 0);
    lv_style_set_pad_hor(&s_style_footer, SS_PADDING_MEDIUM);
    lv_style_set_pad_ver(&s_style_footer, SS_PADDING_SMALL);
    
    // ═══════════════════════════════════════════════════════════
    // BUTTON STYLE (Orange border, dark fill)
    // ═══════════════════════════════════════════════════════════
    lv_style_init(&s_style_button);
    lv_style_set_bg_color(&s_style_button, SS_COLOR_BG_MEDIUM);
    lv_style_set_bg_opa(&s_style_button, LV_OPA_COVER);
    lv_style_set_border_color(&s_style_button, SS_COLOR_ORANGE);
    lv_style_set_border_width(&s_style_button, 2);
    lv_style_set_border_opa(&s_style_button, LV_OPA_COVER);
    lv_style_set_radius(&s_style_button, 4);  // Slight rounding like original
    lv_style_set_text_color(&s_style_button, SS_COLOR_WHITE);
    lv_style_set_text_font(&s_style_button, SS_BUTTON_FONT);
    lv_style_set_pad_hor(&s_style_button, SS_PADDING_MEDIUM);
    lv_style_set_pad_ver(&s_style_button, SS_PADDING_SMALL);
    
    // ═══════════════════════════════════════════════════════════
    // BUTTON PRESSED (Orange fill, black text)
    // ═══════════════════════════════════════════════════════════
    lv_style_init(&s_style_button_pressed);
    lv_style_set_bg_color(&s_style_button_pressed, SS_COLOR_ORANGE);
    lv_style_set_bg_opa(&s_style_button_pressed, LV_OPA_COVER);
    lv_style_set_border_color(&s_style_button_pressed, SS_COLOR_ORANGE);
    lv_style_set_border_width(&s_style_button_pressed, 2);
    lv_style_set_text_color(&s_style_button_pressed, SS_COLOR_BLACK);
    lv_style_set_text_font(&s_style_button_pressed, SS_BUTTON_FONT);
    
    // ═══════════════════════════════════════════════════════════
    // BUTTON DISABLED (Gray border, gray text)
    // ═══════════════════════════════════════════════════════════
    lv_style_init(&s_style_button_disabled);
    lv_style_set_bg_color(&s_style_button_disabled, SS_COLOR_BG_DARK);
    lv_style_set_bg_opa(&s_style_button_disabled, LV_OPA_COVER);
    lv_style_set_border_color(&s_style_button_disabled, SS_COLOR_MEDIUM_GRAY);
    lv_style_set_border_width(&s_style_button_disabled, 2);
    lv_style_set_text_color(&s_style_button_disabled, SS_COLOR_MEDIUM_GRAY);
    lv_style_set_text_font(&s_style_button_disabled, SS_BUTTON_FONT);
    
    // ═══════════════════════════════════════════════════════════
    // TEXT STYLES
    // ═══════════════════════════════════════════════════════════
    lv_style_init(&s_style_text_normal);
    lv_style_set_text_color(&s_style_text_normal, SS_COLOR_WHITE);
    lv_style_set_text_font(&s_style_text_normal, SS_FONT_MEDIUM);
    
    lv_style_init(&s_style_text_highlight);
    lv_style_set_text_color(&s_style_text_highlight, SS_COLOR_ORANGE);
    lv_style_set_text_font(&s_style_text_highlight, SS_FONT_MEDIUM);
    
    lv_style_init(&s_style_text_warning);
    lv_style_set_text_color(&s_style_text_warning, SS_COLOR_YELLOW);
    lv_style_set_text_font(&s_style_text_warning, SS_FONT_MEDIUM);
    
    lv_style_init(&s_style_text_error);
    lv_style_set_text_color(&s_style_text_error, SS_COLOR_RED);
    lv_style_set_text_font(&s_style_text_error, SS_FONT_MEDIUM);
    
    // ═══════════════════════════════════════════════════════════
    // QR BACKGROUND (White with padding)
    // ═══════════════════════════════════════════════════════════
    lv_style_init(&s_style_qr_bg);
    lv_style_set_bg_color(&s_style_qr_bg, SS_COLOR_WHITE);
    lv_style_set_bg_opa(&s_style_qr_bg, LV_OPA_COVER);
    lv_style_set_border_width(&s_style_qr_bg, 0);
    lv_style_set_pad_all(&s_style_qr_bg, SS_QR_QUIET_ZONE);
    
    // ═══════════════════════════════════════════════════════════
    // PANEL (Dark gray background, subtle border)
    // ═══════════════════════════════════════════════════════════
    lv_style_init(&s_style_panel);
    lv_style_set_bg_color(&s_style_panel, SS_COLOR_BG_MEDIUM);
    lv_style_set_bg_opa(&s_style_panel, LV_OPA_COVER);
    lv_style_set_border_color(&s_style_panel, SS_COLOR_MEDIUM_GRAY);
    lv_style_set_border_width(&s_style_panel, 1);
    lv_style_set_radius(&s_style_panel, 0);  // Sharp corners like original
    lv_style_set_pad_all(&s_style_panel, SS_PADDING_MEDIUM);
    
    // ═══════════════════════════════════════════════════════════
    // LIST ITEM (Selectable menu items)
    // ═══════════════════════════════════════════════════════════
    lv_style_init(&s_style_list_item);
    lv_style_set_bg_color(&s_style_list_item, SS_COLOR_BG_DARK);
    lv_style_set_bg_opa(&s_style_list_item, LV_OPA_COVER);
    lv_style_set_border_color(&s_style_list_item, SS_COLOR_MEDIUM_GRAY);
    lv_style_set_border_width(&s_style_list_item, 1);
    lv_style_set_border_side(&s_style_list_item, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_text_color(&s_style_list_item, SS_COLOR_WHITE);
    lv_style_set_text_font(&s_style_list_item, SS_FONT_MEDIUM);
    lv_style_set_pad_all(&s_style_list_item, SS_PADDING_MEDIUM);
    
    lv_style_init(&s_style_list_item_selected);
    lv_style_set_bg_color(&s_style_list_item_selected, SS_COLOR_ORANGE);
    lv_style_set_bg_opa(&s_style_list_item_selected, LV_OPA_COVER);
    lv_style_set_border_width(&s_style_list_item_selected, 0);
    lv_style_set_text_color(&s_style_list_item_selected, SS_COLOR_BLACK);
    lv_style_set_text_font(&s_style_list_item_selected, SS_FONT_MEDIUM);
    lv_style_set_pad_all(&s_style_list_item_selected, SS_PADDING_MEDIUM);
    
    s_initialized = true;
}

void SeedSignerTheme::deinit() {
    if (!s_initialized) return;
    
    lv_style_reset(&s_style_bg);
    lv_style_reset(&s_style_header);
    lv_style_reset(&s_style_footer);
    lv_style_reset(&s_style_button);
    lv_style_reset(&s_style_button_pressed);
    lv_style_reset(&s_style_button_disabled);
    lv_style_reset(&s_style_text_normal);
    lv_style_reset(&s_style_text_highlight);
    lv_style_reset(&s_style_text_warning);
    lv_style_reset(&s_style_text_error);
    lv_style_reset(&s_style_qr_bg);
    lv_style_reset(&s_style_panel);
    lv_style_reset(&s_style_list_item);
    lv_style_reset(&s_style_list_item_selected);
    
    s_initialized = false;
}

// ═══════════════════════════════════════════════════════════════
// STYLE ACCESSORS
// ═══════════════════════════════════════════════════════════════

lv_style_t* SeedSignerTheme::get_style_bg() { return &s_style_bg; }
lv_style_t* SeedSignerTheme::get_style_header() { return &s_style_header; }
lv_style_t* SeedSignerTheme::get_style_footer() { return &s_style_footer; }
lv_style_t* SeedSignerTheme::get_style_button() { return &s_style_button; }
lv_style_t* SeedSignerTheme::get_style_button_pressed() { return &s_style_button_pressed; }
lv_style_t* SeedSignerTheme::get_style_button_disabled() { return &s_style_button_disabled; }
lv_style_t* SeedSignerTheme::get_style_text_normal() { return &s_style_text_normal; }
lv_style_t* SeedSignerTheme::get_style_text_highlight() { return &s_style_text_highlight; }
lv_style_t* SeedSignerTheme::get_style_text_warning() { return &s_style_text_warning; }
lv_style_t* SeedSignerTheme::get_style_text_error() { return &s_style_text_error; }
lv_style_t* SeedSignerTheme::get_style_qr_bg() { return &s_style_qr_bg; }
lv_style_t* SeedSignerTheme::get_style_panel() { return &s_style_panel; }
lv_style_t* SeedSignerTheme::get_style_list_item() { return &s_style_list_item; }
lv_style_t* SeedSignerTheme::get_style_list_item_selected() { return &s_style_list_item_selected; }

// ═══════════════════════════════════════════════════════════════
// SCREEN HELPERS
// ═══════════════════════════════════════════════════════════════

lv_obj_t* SeedSignerTheme::create_screen_base() {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_add_style(screen, &s_style_bg, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    return screen;
}

lv_obj_t* SeedSignerTheme::create_header(lv_obj_t* parent, const char* title) {
    lv_obj_t* header = lv_obj_create(parent);
    lv_obj_set_size(header, SS_SCREEN_WIDTH, SS_HEADER_HEIGHT);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_style(header, &s_style_header, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t* label = lv_label_create(header);
    lv_label_set_text(label, title);
    lv_obj_center(label);
    
    return header;
}

lv_obj_t* SeedSignerTheme::create_footer(lv_obj_t* parent, const char* hint) {
    lv_obj_t* footer = lv_obj_create(parent);
    lv_obj_set_size(footer, SS_SCREEN_WIDTH, SS_FOOTER_HEIGHT);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_style(footer, &s_style_footer, 0);
    lv_obj_clear_flag(footer, LV_OBJ_FLAG_SCROLLABLE);
    
    if (hint) {
        lv_obj_t* label = lv_label_create(footer);
        lv_label_set_text(label, hint);
        lv_obj_center(label);
    }
    
    return footer;
}

lv_obj_t* SeedSignerTheme::create_button(lv_obj_t* parent, const char* text,
                                          lv_coord_t x, lv_coord_t y,
                                          lv_event_cb_t cb, void* user_data) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, SS_BUTTON_WIDTH, SS_BUTTON_HEIGHT);
    lv_obj_set_pos(btn, x, y);
    lv_obj_add_style(btn, SeedSignerTheme::get_style_button(), 0);
    lv_obj_add_style(btn, SeedSignerTheme::get_style_button_pressed(), LV_STATE_PRESSED);
    
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_center(label);
    
    if (cb) {
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, user_data);
    }
    
    return btn;
}

lv_obj_t* SeedSignerTheme::create_icon_button(lv_obj_t* parent, const char* icon,
                                               lv_coord_t x, lv_coord_t y,
                                               lv_event_cb_t cb, void* user_data) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 40, 40);
    lv_obj_set_pos(btn, x, y);
    lv_obj_add_style(btn, SeedSignerTheme::get_style_button(), 0);
    lv_obj_add_style(btn, SeedSignerTheme::get_style_button_pressed(), LV_STATE_PRESSED);
    
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, icon);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_center(label);
    
    if (cb) {
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, user_data);
    }
    
    return btn;
}

lv_obj_t* SeedSignerTheme::create_qr_display(lv_obj_t* parent, const char* data,
                                              lv_coord_t x, lv_coord_t y,
                                              lv_coord_t size) {
    // Container with white background
    lv_obj_t* container = lv_obj_create(parent);
    lv_obj_set_size(container, size + SS_QR_QUIET_ZONE * 2, size + SS_QR_QUIET_ZONE * 2);
    lv_obj_set_pos(container, x, y);
    lv_obj_add_style(container, &s_style_qr_bg, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    
    // QR Code widget (requires LVGL QR library)
    // For now, placeholder
    lv_obj_t* qr = lv_qrcode_create(container, size, SS_COLOR_BLACK, SS_COLOR_WHITE);
    lv_qrcode_update(qr, data, strlen(data));
    lv_obj_center(qr);
    
    return container;
}

// ═══════════════════════════════════════════════════════════════
// UI HELPERS IMPLEMENTATION
// ═══════════════════════════════════════════════════════════════

void SeedSignerUI::show_nav_hint(lv_obj_t* parent, const char* text) {
    lv_obj_t* hint = lv_label_create(parent);
    lv_label_set_text(hint, text);
    lv_obj_set_style_text_color(hint, SS_COLOR_LIGHT_GRAY, 0);
    lv_obj_set_style_text_font(hint, SS_FONT_SMALL, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -5);
}

void SeedSignerUI::show_back_button(lv_obj_t* parent, lv_event_cb_t cb, void* user_data) {
    // Bottom-left back button (replacing joystick "back")
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 60, 30);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_add_style(btn, SeedSignerTheme::get_style_button(), 0);
    lv_obj_add_style(btn, SeedSignerTheme::get_style_button_pressed(), LV_STATE_PRESSED);
    
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, "< Back");
    lv_obj_center(label);
    
    if (cb) {
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, user_data);
    }
}

void SeedSignerUI::show_confirm_button(lv_obj_t* parent, const char* text,
                                        lv_event_cb_t cb, void* user_data) {
    // Bottom-right confirm button
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 80, 30);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_add_style(btn, SeedSignerTheme::get_style_button(), 0);
    lv_obj_add_style(btn, SeedSignerTheme::get_style_button_pressed(), LV_STATE_PRESSED);
    
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_center(label);
    
    if (cb) {
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, user_data);
    }
}

void SeedSignerUI::display_mnemonic_words(lv_obj_t* parent, const char* mnemonic,
                                           int start_index, int count) {
    // Parse mnemonic
    char buffer[512];
    strncpy(buffer, mnemonic, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    char* words[24];
    int word_count = 0;
    char* token = strtok(buffer, " ");
    while (token && word_count < 24) {
        words[word_count++] = token;
        token = strtok(NULL, " ");
    }
    
    // Display words in grid (2 columns like original)
    int y_start = 45;
    int row_height = 18;
    
    for (int i = start_index; i < start_index + count && i < word_count; i++) {
        int col = (i - start_index) % 2;
        int row = (i - start_index) / 2;
        
        lv_obj_t* lbl = lv_label_create(parent);
        lv_label_set_text_fmt(lbl, "%2d. %s", i + 1, words[i]);
        lv_obj_set_style_text_font(lbl, SS_FONT_MNEMONIC, 0);
        lv_obj_set_style_text_color(lbl, SS_COLOR_WHITE, 0);
        lv_obj_set_pos(lbl, 15 + col * 145, y_start + row * row_height);
    }
}

lv_obj_t* SeedSignerUI::create_loading_spinner(lv_obj_t* parent, const char* text) {
    // Semi-transparent overlay
    lv_obj_t* overlay = lv_obj_create(parent);
    lv_obj_set_size(overlay, LV_PCT(100), LV_PCT(100));
    lv_obj_center(overlay);
    lv_obj_set_style_bg_color(overlay, SS_COLOR_BLACK, 0);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_70, 0);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);
    
    // Spinner
    lv_obj_t* spinner = lv_spinner_create(overlay, 1000, 60);
    lv_obj_set_size(spinner, 50, 50);
    lv_obj_center(spinner);
    lv_obj_set_style_arc_color(spinner, SS_COLOR_ORANGE, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(spinner, SS_COLOR_BG_MEDIUM, LV_PART_MAIN);
    
    // Text
    if (text) {
        lv_obj_t* lbl = lv_label_create(overlay);
        lv_label_set_text(lbl, text);
        lv_obj_set_style_text_color(lbl, SS_COLOR_WHITE, 0);
        lv_obj_align_to(lbl, spinner, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    }
    
    return overlay;
}

} // namespace UI
} // namespace SeedSigner
