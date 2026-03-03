#include "ui/screens.h"
#include "core/secure_memory.h"
#include <string.h>

namespace UI {

// Static style definitions
static lv_style_t style_btn;
static lv_style_t style_btn_pressed;
static lv_style_t style_title;
static lv_style_t style_text;
static bool styles_initialized = false;

void init_styles() {
    if (styles_initialized) return;
    
    // Button style
    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, COLOR_PRIMARY);
    lv_style_set_text_color(&style_btn, lv_color_white());
    lv_style_set_radius(&style_btn, 10);
    lv_style_set_pad_all(&style_btn, 15);
    
    // Button pressed style
    lv_style_init(&style_btn_pressed);
    lv_style_set_bg_color(&style_btn_pressed, lv_color_lighten(COLOR_PRIMARY, 30));
    
    // Title style
    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, &lv_font_montserrat_22);
    lv_style_set_text_color(&style_title, COLOR_PRIMARY);
    
    // Text style
    lv_style_init(&style_text);
    lv_style_set_text_font(&style_text, &lv_font_montserrat_14);
    lv_style_set_text_color(&style_text, COLOR_TEXT);
    
    styles_initialized = true;
}

// ============ WelcomeScreen ============
void WelcomeScreen::create() {
    init_styles();
    
    screen_ = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen_, COLOR_BACKGROUND, 0);
    
    // Title
    lv_obj_t* title = lv_label_create(screen_);
    lv_label_set_text(title, "SeedSigner");
    lv_obj_add_style(title, &style_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 40);
    
    // Subtitle
    lv_obj_t* subtitle = lv_label_create(screen_);
    lv_label_set_text(subtitle, "Air-Gapped Bitcoin Signing Device");
    lv_obj_add_style(subtitle, &style_text, 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 80);
    
    // New Wallet button
    btn_new_ = lv_btn_create(screen_);
    lv_obj_set_size(btn_new_, 200, 60);
    lv_obj_align(btn_new_, LV_ALIGN_CENTER, 0, 20);
    lv_obj_add_style(btn_new_, &style_btn, 0);
    lv_obj_add_style(btn_new_, &style_btn_pressed, LV_STATE_PRESSED);
    
    lv_obj_t* label_new = lv_label_create(btn_new_);
    lv_label_set_text(label_new, "New Wallet");
    lv_obj_center(label_new);
    
    lv_obj_add_event_cb(btn_new_, [](lv_event_t* e) {
        auto* screen = static_cast<WelcomeScreen*>(lv_event_get_user_data(e));
        if (screen->new_wallet_cb_) screen->new_wallet_cb_();
    }, LV_EVENT_CLICKED, this);
    
    // Restore button
    btn_restore_ = lv_btn_create(screen_);
    lv_obj_set_size(btn_restore_, 200, 60);
    lv_obj_align(btn_restore_, LV_ALIGN_CENTER, 0, 100);
    lv_obj_add_style(btn_restore_, &style_btn, 0);
    lv_obj_add_style(btn_restore_, &style_btn_pressed, LV_STATE_PRESSED);
    
    lv_obj_t* label_restore = lv_label_create(btn_restore_);
    lv_label_set_text(label_restore, "Restore Wallet");
    lv_obj_center(label_restore);
    
    lv_obj_add_event_cb(btn_restore_, [](lv_event_t* e) {
        auto* screen = static_cast<WelcomeScreen*>(lv_event_get_user_data(e));
        if (screen->restore_cb_) screen->restore_cb_();
    }, LV_EVENT_CLICKED, this);
}

void WelcomeScreen::destroy() {
    if (screen_) {
        lv_obj_del(screen_);
        screen_ = nullptr;
    }
}

// ============ SeedGenerateScreen ============
void SeedGenerateScreen::create() {
    screen_ = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen_, COLOR_BACKGROUND, 0);
    
    // Title
    lv_obj_t* title = lv_label_create(screen_);
    lv_label_set_text(title, "Backup These Words");
    lv_obj_add_style(title, &style_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // Word list
    word_list_ = lv_textarea_create(screen_);
    lv_obj_set_size(word_list_, 280, 160);
    lv_obj_align(word_list_, LV_ALIGN_TOP_MID, 0, 50);
    lv_textarea_set_text(word_list_, mnemonic_);
    lv_textarea_set_one_line(word_list_, false);
    lv_obj_set_style_bg_color(word_list_, COLOR_SECONDARY, 0);
    lv_obj_set_style_text_color(word_list_, COLOR_TEXT, 0);
    lv_textarea_set_readonly(word_list_, true);
    
    // Confirm button
    btn_confirm_ = lv_btn_create(screen_);
    lv_obj_set_size(btn_confirm_, 150, 50);
    lv_obj_align(btn_confirm_, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_style(btn_confirm_, &style_btn, 0);
    
    lv_obj_t* label = lv_label_create(btn_confirm_);
    lv_label_set_text(label, "I've Written Them Down");
    lv_obj_center(label);
    
    lv_obj_add_event_cb(btn_confirm_, [](lv_event_t* e) {
        auto* screen = static_cast<SeedGenerateScreen*>(lv_event_get_user_data(e));
        if (screen->confirm_cb_) screen->confirm_cb_();
    }, LV_EVENT_CLICKED, this);
}

void SeedGenerateScreen::destroy() {
    SecureMemory::zero(mnemonic_, sizeof(mnemonic_));
    if (screen_) {
        lv_obj_del(screen_);
        screen_ = nullptr;
    }
}

void SeedGenerateScreen::set_mnemonic(const char* mnemonic) {
    strncpy(mnemonic_, mnemonic, sizeof(mnemonic_) - 1);
    mnemonic_[sizeof(mnemonic_) - 1] = '\0';
    if (word_list_) {
        lv_textarea_set_text(word_list_, mnemonic_);
    }
}

// ============ ReceiveScreen ============
void ReceiveScreen::create() {
    screen_ = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen_, COLOR_BACKGROUND, 0);
    
    // Title
    lv_obj_t* title = lv_label_create(screen_);
    lv_label_set_text(title, "Receive Bitcoin");
    lv_obj_add_style(title, &style_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // QR placeholder (would be actual QR in full implementation)
    qr_code_ = lv_obj_create(screen_);
    lv_obj_set_size(qr_code_, 150, 150);
    lv_obj_align(qr_code_, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_bg_color(qr_code_, lv_color_white(), 0);
    lv_obj_set_style_border_color(qr_code_, COLOR_PRIMARY, 0);
    lv_obj_set_style_border_width(qr_code_, 2, 0);
    
    lv_obj_t* qr_label = lv_label_create(qr_code_);
    lv_label_set_text(qr_label, "QR");
    lv_obj_center(qr_label);
    lv_obj_set_style_text_color(qr_label, lv_color_black(), 0);
    
    // Address
    addr_label_ = lv_label_create(screen_);
    lv_obj_add_style(addr_label_, &style_text, 0);
    lv_label_set_long_mode(addr_label_, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(addr_label_, 280);
    lv_label_set_text(addr_label_, address_);
    lv_obj_align(addr_label_, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_set_style_text_align(addr_label_, LV_TEXT_ALIGN_CENTER, 0);
    
    // Path
    path_label_ = lv_label_create(screen_);
    lv_obj_add_style(path_label_, &style_text, 0);
    lv_obj_set_style_text_color(path_label_, COLOR_TEXT_DIM, 0);
    lv_label_set_text(path_label_, path_);
    lv_obj_align(path_label_, LV_ALIGN_BOTTOM_MID, 0, -10);
}

void ReceiveScreen::destroy() {
    SecureMemory::zero(address_, sizeof(address_));
    if (screen_) {
        lv_obj_del(screen_);
        screen_ = nullptr;
    }
}

void ReceiveScreen::set_address(const char* address) {
    strncpy(address_, address, sizeof(address_) - 1);
    address_[sizeof(address_) - 1] = '\0';
    if (addr_label_) {
        lv_label_set_text(addr_label_, address_);
    }
}

void ReceiveScreen::set_derivation_path(const char* path) {
    strncpy(path_, path, sizeof(path_) - 1);
    path_[sizeof(path_) - 1] = '\0';
    if (path_label_) {
        lv_label_set_text(path_label_, path_);
    }
}

void ReceiveScreen::update_qr() {
    // Would generate actual QR code in full implementation
}

// ============ SendConfirmScreen ============
void SendConfirmScreen::create() {
    screen_ = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen_, COLOR_BACKGROUND, 0);
    
    // Title
    lv_obj_t* title = lv_label_create(screen_);
    lv_label_set_text(title, "Confirm Transaction");
    lv_obj_add_style(title, &style_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // Amount label
    amount_label_ = lv_label_create(screen_);
    lv_obj_add_style(amount_label_, &style_text, 0);
    lv_obj_align(amount_label_, LV_ALIGN_TOP_MID, 0, 60);
    
    // Fee label
    fee_label_ = lv_label_create(screen_);
    lv_obj_add_style(fee_label_, &style_text, 0);
    lv_obj_set_style_text_color(fee_label_, COLOR_WARNING, 0);
    lv_obj_align(fee_label_, LV_ALIGN_TOP_MID, 0, 90);
    
    // Recipient
    recipient_label_ = lv_label_create(screen_);
    lv_obj_add_style(recipient_label_, &style_text, 0);
    lv_obj_set_style_text_color(recipient_label_, COLOR_TEXT_DIM, 0);
    lv_label_set_long_mode(recipient_label_, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(recipient_label_, 280);
    lv_obj_align(recipient_label_, LV_ALIGN_TOP_MID, 0, 120);
    
    // Sign button
    btn_sign_ = lv_btn_create(screen_);
    lv_obj_set_size(btn_sign_, 120, 50);
    lv_obj_align(btn_sign_, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_add_style(btn_sign_, &style_btn, 0);
    
    lv_obj_t* label_sign = lv_label_create(btn_sign_);
    lv_label_set_text(label_sign, "Sign");
    lv_obj_center(label_sign);
    
    lv_obj_add_event_cb(btn_sign_, [](lv_event_t* e) {
        auto* screen = static_cast<SendConfirmScreen*>(lv_event_get_user_data(e));
        if (screen->sign_cb_) screen->sign_cb_();
    }, LV_EVENT_CLICKED, this);
    
    // Cancel button
    btn_cancel_ = lv_btn_create(screen_);
    lv_obj_set_size(btn_cancel_, 120, 50);
    lv_obj_align(btn_cancel_, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_set_style_bg_color(btn_cancel_, COLOR_SECONDARY, 0);
    
    lv_obj_t* label_cancel = lv_label_create(btn_cancel_);
    lv_label_set_text(label_cancel, "Cancel");
    lv_obj_center(label_cancel);
    
    lv_obj_add_event_cb(btn_cancel_, [](lv_event_t* e) {
        auto* screen = static_cast<SendConfirmScreen*>(lv_event_get_user_data(e));
        if (screen->cancel_cb_) screen->cancel_cb_();
    }, LV_EVENT_CLICKED, this);
}

void SendConfirmScreen::destroy() {
    if (screen_) {
        lv_obj_del(screen_);
        screen_ = nullptr;
    }
}

void SendConfirmScreen::set_transaction_info(uint64_t total_in, uint64_t total_out, 
                                              uint64_t fee, const char* recipient_addr) {
    char buf[128];
    
    // Format amounts (simplified - would use proper BTC formatting)
    float btc_out = total_out / 100000000.0f;
    float btc_fee = fee / 100000000.0f;
    
    snprintf(buf, sizeof(buf), "Send: %.8f BTC", btc_out);
    if (amount_label_) lv_label_set_text(amount_label_, buf);
    
    snprintf(buf, sizeof(buf), "Fee: %.8f BTC", btc_fee);
    if (fee_label_) lv_label_set_text(fee_label_, buf);
    
    if (recipient_label_) lv_label_set_text(recipient_label_, recipient_addr);
}

// ============ MainMenuScreen ============
void MainMenuScreen::create() {
    screen_ = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen_, COLOR_BACKGROUND, 0);
    
    // Title
    lv_obj_t* title = lv_label_create(screen_);
    lv_label_set_text(title, "SeedSigner");
    lv_obj_add_style(title, &style_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Receive button
    btn_receive_ = lv_btn_create(screen_);
    lv_obj_set_size(btn_receive_, 260, 60);
    lv_obj_align(btn_receive_, LV_ALIGN_TOP_MID, 0, 80);
    lv_obj_add_style(btn_receive_, &style_btn, 0);
    
    lv_obj_t* label_recv = lv_label_create(btn_receive_);
    lv_label_set_text(label_recv, LV_SYMBOL_DOWNLOAD " Receive");
    lv_obj_center(label_recv);
    
    lv_obj_add_event_cb(btn_receive_, [](lv_event_t* e) {
        auto* screen = static_cast<MainMenuScreen*>(lv_event_get_user_data(e));
        if (screen->receive_cb_) screen->receive_cb_();
    }, LV_EVENT_CLICKED, this);
    
    // Send button
    btn_send_ = lv_btn_create(screen_);
    lv_obj_set_size(btn_send_, 260, 60);
    lv_obj_align(btn_send_, LV_ALIGN_TOP_MID, 0, 150);
    lv_obj_add_style(btn_send_, &style_btn, 0);
    
    lv_obj_t* label_send = lv_label_create(btn_send_);
    lv_label_set_text(label_send, LV_SYMBOL_UPLOAD " Send");
    lv_obj_center(label_send);
    
    lv_obj_add_event_cb(btn_send_, [](lv_event_t* e) {
        auto* screen = static_cast<MainMenuScreen*>(lv_event_get_user_data(e));
        if (screen->send_cb_) screen->send_cb_();
    }, LV_EVENT_CLICKED, this);
    
    // Settings button (small, bottom left)
    btn_settings_ = lv_btn_create(screen_);
    lv_obj_set_size(btn_settings_, 100, 40);
    lv_obj_align(btn_settings_, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_set_style_bg_color(btn_settings_, COLOR_SECONDARY, 0);
    
    lv_obj_t* label_set = lv_label_create(btn_settings_);
    lv_label_set_text(label_set, LV_SYMBOL_SETTINGS);
    lv_obj_center(label_set);
    
    lv_obj_add_event_cb(btn_settings_, [](lv_event_t* e) {
        auto* screen = static_cast<MainMenuScreen*>(lv_event_get_user_data(e));
        if (screen->settings_cb_) screen->settings_cb_();
    }, LV_EVENT_CLICKED, this);
    
    // Shutdown button (small, bottom right)
    btn_shutdown_ = lv_btn_create(screen_);
    lv_obj_set_size(btn_shutdown_, 100, 40);
    lv_obj_align(btn_shutdown_, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_set_style_bg_color(btn_shutdown_, COLOR_ERROR, 0);
    
    lv_obj_t* label_off = lv_label_create(btn_shutdown_);
    lv_label_set_text(label_off, LV_SYMBOL_POWER);
    lv_obj_center(label_off);
    
    lv_obj_add_event_cb(btn_shutdown_, [](lv_event_t* e) {
        auto* screen = static_cast<MainMenuScreen*>(lv_event_get_user_data(e));
        if (screen->shutdown_cb_) screen->shutdown_cb_();
    }, LV_EVENT_CLICKED, this);
}

void MainMenuScreen::destroy() {
    if (screen_) {
        lv_obj_del(screen_);
        screen_ = nullptr;
    }
}

// ============ ScreenManager ============
ScreenManager& ScreenManager::instance() {
    static ScreenManager instance;
    return instance;
}

void ScreenManager::init() {
    init_styles();
}

void ScreenManager::register_screen(Screen* screen) {
    if (screen) {
        int idx = static_cast<int>(screen->get_id());
        if (idx >= 0 && idx < static_cast<int>(ScreenID::COUNT)) {
            screens_[idx] = screen;
        }
    }
}

void ScreenManager::switch_to(ScreenID id) {
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= static_cast<int>(ScreenID::COUNT)) return;
    
    Screen* new_screen = screens_[idx];
    if (!new_screen) return;
    
    if (current_screen_) {
        current_screen_->on_exit();
    }
    
    current_screen_ = new_screen;
    current_screen_->create();
    current_screen_->on_entry();
    
    lv_scr_load(current_screen_->get_lv_obj());
}

void init_theme() {
    init_styles();
}

} // namespace UI
