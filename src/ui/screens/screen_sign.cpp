/**
 * @file screen_sign.cpp
 * PSBT signing screens
 */

#include "ui/app.h"
#include "core/psbt.h"
#include <lvgl.h>
#include <M5Unified.h>

namespace SeedSigner {
namespace UI {

// QR scan screen for PSBT
void create_sign_scan_screen(App* app) {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1A1A1A), 0);
    
    // Title
    lv_obj_t* title = lv_label_create(screen);
    lv_label_set_text(title, "Scan Transaction");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xF7931A), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Instructions
    lv_obj_t* instr = lv_label_create(screen);
    lv_label_set_text(instr, "Scan PSBT QR code from wallet");
    lv_obj_set_style_text_color(instr, lv_color_hex(0x888888), 0);
    lv_obj_align(instr, LV_ALIGN_TOP_MID, 0, 50);
    
    // Camera preview area
    lv_obj_t* preview = lv_obj_create(screen);
    lv_obj_set_size(preview, 280, 210);
    lv_obj_align(preview, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_bg_color(preview, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_color(preview, lv_color_hex(0xF7931A), 0);
    lv_obj_set_style_border_width(preview, 2, 0);
    
    // Scan frame overlay
    lv_obj_t* frame = lv_obj_create(preview);
    lv_obj_set_size(frame, 180, 180);
    lv_obj_center(frame);
    lv_obj_set_style_bg_opa(frame, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(frame, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_border_width(frame, 2, 0);
    
    // Corner markers
    int corner_size = 20;
    for (int i = 0; i < 4; i++) {
        lv_obj_t* corner = lv_obj_create(preview);
        lv_obj_set_size(corner, corner_size, corner_size);
        lv_obj_set_style_bg_color(corner, lv_color_hex(0x00FF00), 0);
        lv_obj_set_style_border_width(corner, 0, 0);
        
        int x = (i % 2) == 0 ? 30 : 230;
        int y = (i / 2) == 0 ? 15 : 195;
        lv_obj_set_pos(corner, x, y);
    }
    
    // Status
    lv_obj_t* status = lv_label_create(screen);
    lv_label_set_text(status, "Scanning...");
    lv_obj_set_style_text_color(status, lv_color_hex(0x00FF00), 0);
    lv_obj_align(status, LV_ALIGN_BOTTOM_MID, 0, -50);
    
    // Cancel button
    lv_obj_t* cancel = lv_btn_create(screen);
    lv_obj_set_size(cancel, 100, 40);
    lv_obj_align(cancel, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(cancel, lv_color_hex(0xFF4444), 0);
    
    lv_obj_t* cancel_lbl = lv_label_create(cancel);
    lv_label_set_text(cancel_lbl, "Cancel");
    lv_obj_center(cancel_lbl);
    
    lv_obj_add_event_cb(cancel, [](lv_event_t* e) {
        App* app = (App*)lv_event_get_user_data(e);
        app->go_back();
    }, LV_EVENT_CLICKED, app);
    
    lv_scr_load(screen);
}

// Transaction review screen
void create_sign_review_screen(App* app, Core::PSBT* psbt) {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1A1A1A), 0);
    
    // Title
    lv_obj_t* title = lv_label_create(screen);
    lv_label_set_text(title, "Review Transaction");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xF7931A), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // Warning
    lv_obj_t* warn = lv_label_create(screen);
    lv_label_set_text(warn, "Verify amounts before signing!");
    lv_obj_set_style_text_color(warn, lv_color_hex(0xFF8800), 0);
    lv_obj_align(warn, LV_ALIGN_TOP_MID, 0, 40);
    
    // Transaction details container
    lv_obj_t* cont = lv_obj_create(screen);
    lv_obj_set_size(cont, 300, 140);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x2A2A2A), 0);
    
    // Fee (example values)
    lv_obj_t* fee_lbl = lv_label_create(cont);
    if (psbt) {
        lv_label_set_text_fmt(fee_lbl, "Fee: %d sats", (int)psbt->fee);
    } else {
        lv_label_set_text(fee_lbl, "Fee: -- sats");
    }
    lv_obj_set_style_text_color(fee_lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(fee_lbl, LV_ALIGN_TOP_LEFT, 10, 10);
    
    // Total outputs
    lv_obj_t* out_lbl = lv_label_create(cont);
    lv_label_set_text(out_lbl, "Outputs: 2");
    lv_obj_set_style_text_color(out_lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(out_lbl, LV_ALIGN_TOP_LEFT, 10, 35);
    
    // Address preview
    lv_obj_t* addr_lbl = lv_label_create(cont);
    lv_label_set_text(addr_lbl, "To: bc1q...");
    lv_obj_set_style_text_color(addr_lbl, lv_color_hex(0x888888), 0);
    lv_obj_align(addr_lbl, LV_ALIGN_TOP_LEFT, 10, 60);
    
    // Amount
    lv_obj_t* amt_lbl = lv_label_create(cont);
    lv_label_set_text(amt_lbl, "Amount: 0.001 BTC");
    lv_obj_set_style_text_color(amt_lbl, lv_color_hex(0x00FF00), 0);
    lv_obj_align(amt_lbl, LV_ALIGN_TOP_LEFT, 10, 85);
    
    // Buttons
    lv_obj_t* btn_sign = lv_btn_create(screen);
    lv_obj_set_size(btn_sign, 120, 45);
    lv_obj_align(btn_sign, LV_ALIGN_BOTTOM_LEFT, 20, -10);
    lv_obj_set_style_bg_color(btn_sign, lv_color_hex(0x00AA00), 0);
    
    lv_obj_t* sign_lbl = lv_label_create(btn_sign);
    lv_label_set_text(sign_lbl, "Sign");
    lv_obj_center(sign_lbl);
    
    lv_obj_add_event_cb(btn_sign, [](lv_event_t* e) {
        App* app = (App*)lv_event_get_user_data(e);
        app->set_state(AppState::SIGN_CONFIRM);
    }, LV_EVENT_CLICKED, app);
    
    lv_obj_t* btn_cancel = lv_btn_create(screen);
    lv_obj_set_size(btn_cancel, 120, 45);
    lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_RIGHT, -20, -10);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0xAA0000), 0);
    
    lv_obj_t* cancel_lbl = lv_label_create(btn_cancel);
    lv_label_set_text(cancel_lbl, "Cancel");
    lv_obj_center(cancel_lbl);
    
    lv_obj_add_event_cb(btn_cancel, [](lv_event_t* e) {
        App* app = (App*)lv_event_get_user_data(e);
        app->go_back();
    }, LV_EVENT_CLICKED, app);
    
    lv_scr_load(screen);
}

// Sign confirmation screen
void create_sign_confirm_screen(App* app) {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1A1A1A), 0);
    
    // Title
    lv_obj_t* title = lv_label_create(screen);
    lv_label_set_text(title, "Signing...");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xF7931A), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 40);
    
    // Spinner
    lv_obj_t* spinner = lv_spinner_create(screen, 1000, 60);
    lv_obj_set_size(spinner, 80, 80);
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0xF7931A), LV_PART_INDICATOR);
    
    // Status
    lv_obj_t* status = lv_label_create(screen);
    lv_label_set_text(status, "Creating signature...");
    lv_obj_set_style_text_color(status, lv_color_hex(0x888888), 0);
    lv_obj_align(status, LV_ALIGN_BOTTOM_MID, 0, -40);
    
    // In real implementation, trigger signing here
    // Then transition to QR display screen
    
    lv_scr_load(screen);
}

} // namespace UI
} // namespace SeedSigner
