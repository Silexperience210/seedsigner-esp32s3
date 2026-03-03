#pragma once

#include <lvgl.h>
#include <functional>

// SeedSigner UI Screens
// LVGL-based interface matching original SeedSigner design

namespace UI {

// Screen identifiers
enum class ScreenID {
    WELCOME,
    NEW_WALLET,
    RESTORE_WALLET,
    SEED_GENERATE,
    SEED_VERIFY,
    PASSPHRASE_ENTRY,
    MAIN_MENU,
    RECEIVE,
    SEND_SCAN,
    SEND_CONFIRM,
    SEND_SIGNING,
    SETTINGS,
    SHUTDOWN,
    ERROR_SCREEN,
    COUNT
};

// Theme colors (SeedSigner orange)
constexpr lv_color_t COLOR_BACKGROUND = LV_COLOR_MAKE(0x1A, 0x1A, 0x1A);
constexpr lv_color_t COLOR_PRIMARY = LV_COLOR_MAKE(0xF7, 0x93, 0x1A);  // Bitcoin Orange
constexpr lv_color_t COLOR_SECONDARY = LV_COLOR_MAKE(0x4A, 0x4A, 0x4A);
constexpr lv_color_t COLOR_TEXT = LV_COLOR_MAKE(0xFF, 0xFF, 0xFF);
constexpr lv_color_t COLOR_TEXT_DIM = LV_COLOR_MAKE(0x80, 0x80, 0x80);
constexpr lv_color_t COLOR_SUCCESS = LV_COLOR_MAKE(0x00, 0xFF, 0x00);
constexpr lv_color_t COLOR_WARNING = LV_COLOR_MAKE(0xFF, 0xFF, 0x00);
constexpr lv_color_t COLOR_ERROR = LV_COLOR_MAKE(0xFF, 0x00, 0x00);

// Screen base class
class Screen {
public:
    Screen(ScreenID id) : id_(id), screen_(nullptr) {}
    virtual ~Screen() = default;
    
    virtual void create() = 0;
    virtual void destroy() = 0;
    virtual void on_entry() {}  // Called when screen is shown
    virtual void on_exit() {}   // Called when screen is hidden
    
    ScreenID get_id() const { return id_; }
    lv_obj_t* get_lv_obj() const { return screen_; }
    
protected:
    ScreenID id_;
    lv_obj_t* screen_;
};

// Welcome Screen - Logo and start
class WelcomeScreen : public Screen {
public:
    WelcomeScreen() : Screen(ScreenID::WELCOME) {}
    
    void create() override;
    void destroy() override;
    
    void set_new_wallet_callback(std::function<void()> cb) { new_wallet_cb_ = cb; }
    void set_restore_callback(std::function<void()> cb) { restore_cb_ = cb; }
    
private:
    lv_obj_t* logo_img_ = nullptr;
    lv_obj_t* btn_new_ = nullptr;
    lv_obj_t* btn_restore_ = nullptr;
    std::function<void()> new_wallet_cb_;
    std::function<void()> restore_cb_;
};

// Seed Generation Screen - Display 12/24 words
class SeedGenerateScreen : public Screen {
public:
    SeedGenerateScreen() : Screen(ScreenID::SEED_GENERATE) {}
    
    void create() override;
    void destroy() override;
    
    void set_mnemonic(const char* mnemonic);
    void set_confirm_callback(std::function<void()> cb) { confirm_cb_ = cb; }
    
private:
    lv_obj_t* word_list_ = nullptr;
    lv_obj_t* btn_confirm_ = nullptr;
    char mnemonic_[512];
    std::function<void()> confirm_cb_;
};

// Receive Screen - Show address and QR
class ReceiveScreen : public Screen {
public:
    ReceiveScreen() : Screen(ScreenID::RECEIVE) {}
    
    void create() override;
    void destroy() override;
    
    void set_address(const char* address);
    void set_derivation_path(const char* path);
    void update_qr();
    
private:
    lv_obj_t* qr_code_ = nullptr;
    lv_obj_t* addr_label_ = nullptr;
    lv_obj_t* path_label_ = nullptr;
    char address_[65];
    char path_[32];
};

// Send Confirm Screen - Show tx details before signing
class SendConfirmScreen : public Screen {
public:
    SendConfirmScreen() : Screen(ScreenID::SEND_CONFIRM) {}
    
    void create() override;
    void destroy() override;
    
    void set_transaction_info(uint64_t total_in, uint64_t total_out, uint64_t fee,
                               const char* recipient_addr);
    void set_sign_callback(std::function<void()> cb) { sign_cb_ = cb; }
    void set_cancel_callback(std::function<void()> cb) { cancel_cb_ = cb; }
    
private:
    lv_obj_t* amount_label_ = nullptr;
    lv_obj_t* fee_label_ = nullptr;
    lv_obj_t* recipient_label_ = nullptr;
    lv_obj_t* btn_sign_ = nullptr;
    lv_obj_t* btn_cancel_ = nullptr;
    std::function<void()> sign_cb_;
    std::function<void()> cancel_cb_;
};

// Main Menu Screen - Navigation hub
class MainMenuScreen : public Screen {
public:
    MainMenuScreen() : Screen(ScreenID::MAIN_MENU) {}
    
    void create() override;
    void destroy() override;
    
    void set_receive_callback(std::function<void()> cb) { receive_cb_ = cb; }
    void set_send_callback(std::function<void()> cb) { send_cb_ = cb; }
    void set_settings_callback(std::function<void()> cb) { settings_cb_ = cb; }
    void set_shutdown_callback(std::function<void()> cb) { shutdown_cb_ = cb; }
    
private:
    lv_obj_t* btn_receive_ = nullptr;
    lv_obj_t* btn_send_ = nullptr;
    lv_obj_t* btn_settings_ = nullptr;
    lv_obj_t* btn_shutdown_ = nullptr;
    std::function<void()> receive_cb_;
    std::function<void()> send_cb_;
    std::function<void()> settings_cb_;
    std::function<void()> shutdown_cb_;
};

// Screen Manager
class ScreenManager {
public:
    static ScreenManager& instance();
    
    void init();
    void switch_to(ScreenID id);
    Screen* get_current() const { return current_screen_; }
    
    void register_screen(Screen* screen);
    
private:
    ScreenManager() = default;
    Screen* screens_[static_cast<int>(ScreenID::COUNT)] = {};
    Screen* current_screen_ = nullptr;
};

// Initialize UI theme
void init_theme();

} // namespace UI
