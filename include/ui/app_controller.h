#pragma once

#include "ui/screens.h"
#include "core/bip32_secp256k1.h"
#include "core/bip39.h"
#include "core/psbt_signer.h"

// Application state machine
// Manages wallet state, navigation, and user flows

namespace UI {

enum class AppState {
    BOOT,
    WELCOME,
    WALLET_CREATE,
    WALLET_RESTORE,
    SEED_DISPLAY,
    SEED_VERIFY,
    PASSPHRASE_ENTRY,
    MAIN_MENU,
    RECEIVE,
    SEND_SCAN,
    SEND_CONFIRM,
    SEND_SIGNING,
    SETTINGS,
    SHUTDOWN,
    ERROR
};

struct WalletState {
    bool initialized;
    bool has_passphrase;
    BIP32::ExtendedKey master_key;
    char fingerprint[9];  // 8 hex chars + null
    
    void clear() {
        initialized = false;
        has_passphrase = false;
        BIP32::wipe_key(&master_key);
        memset(fingerprint, 0, sizeof(fingerprint));
    }
};

class AppController {
public:
    AppController();
    ~AppController();
    
    // Initialize app
    void init();
    
    // Main update loop
    void update();
    
    // State transitions
    void go_to_welcome();
    void go_to_create_wallet();
    void go_to_restore_wallet();
    void go_to_main_menu();
    void go_to_receive();
    void go_to_send_scan();
    void go_to_send_confirm(const PSBT::Transaction& tx);
    void go_to_settings();
    void go_to_shutdown();
    void go_to_error(const char* message);
    
    // Wallet operations
    bool create_new_wallet();
    bool restore_wallet(const char* mnemonic);
    bool add_passphrase(const char* passphrase);
    void lock_wallet();
    
    // Getters
    AppState get_state() const { return state_; }
    const WalletState& get_wallet() const { return wallet_; }
    bool is_wallet_unlocked() const { return wallet_.initialized; }
    
private:
    AppState state_;
    AppState previous_state_;
    WalletState wallet_;
    
    // Screen instances
    WelcomeScreen* screen_welcome_;
    SeedGenerateScreen* screen_seed_gen_;
    MainMenuScreen* screen_main_menu_;
    ReceiveScreen* screen_receive_;
    SendConfirmScreen* screen_send_confirm_;
    
    // Internal helpers
    void switch_screen(ScreenID id);
    void derive_master_key(const uint8_t* seed);
    void update_wallet_fingerprint();
    
    // Screen callbacks setup
    void setup_welcome_callbacks();
    void setup_main_menu_callbacks();
    void setup_receive_callbacks();
    void setup_send_callbacks();
    
    // PSBT handling (allocated dynamically in PSRAM)
    PSBT::Signer* psbt_signer_;
    uint8_t* psbt_buffer_;
    size_t psbt_len_;
};

// Global app controller instance
extern AppController g_app;

} // namespace UI
