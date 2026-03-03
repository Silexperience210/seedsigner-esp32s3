#include "ui/app_controller.h"
#include "core/hardware_rng.h"
#include "core/secure_memory.h"
#include "core/bitcoin_address.h"
#include <M5Unified.h>

namespace UI {

// Global instance
AppController g_app;

AppController::AppController() 
    : state_(AppState::BOOT),
      previous_state_(AppState::BOOT),
      psbt_len_(0) {
    wallet_.clear();
    memset(psbt_buffer_, 0, sizeof(psbt_buffer_));
}

AppController::~AppController() {
    wallet_.clear();
    SecureMemory::zero(psbt_buffer_, sizeof(psbt_buffer_));
}

void AppController::init() {
    Serial.println("AppController::init()");
    
    // Create screens
    screen_welcome_ = new WelcomeScreen();
    screen_seed_gen_ = new SeedGenerateScreen();
    screen_main_menu_ = new MainMenuScreen();
    screen_receive_ = new ReceiveScreen();
    screen_send_confirm_ = new SendConfirmScreen();
    
    // Register screens
    ScreenManager::instance().register_screen(screen_welcome_);
    ScreenManager::instance().register_screen(screen_seed_gen_);
    ScreenManager::instance().register_screen(screen_main_menu_);
    ScreenManager::instance().register_screen(screen_receive_);
    ScreenManager::instance().register_screen(screen_send_confirm_);
    
    // Setup callbacks
    setup_welcome_callbacks();
    setup_main_menu_callbacks();
    setup_receive_callbacks();
    setup_send_callbacks();
    
    // Check if wallet exists in secure storage
    if (SecureStorage::has_wallet()) {
        // Wallet exists, need to unlock
        go_to_welcome();
    } else {
        // No wallet, show welcome
        go_to_welcome();
    }
}

void AppController::setup_welcome_callbacks() {
    screen_welcome_->set_new_wallet_callback([this]() {
        Serial.println("New wallet clicked");
        go_to_create_wallet();
    });
    
    screen_welcome_->set_restore_callback([this]() {
        Serial.println("Restore wallet clicked");
        go_to_restore_wallet();
    });
}

void AppController::setup_main_menu_callbacks() {
    screen_main_menu_->set_receive_callback([this]() {
        go_to_receive();
    });
    
    screen_main_menu_->set_send_callback([this]() {
        go_to_send_scan();
    });
    
    screen_main_menu_->set_settings_callback([this]() {
        go_to_settings();
    });
    
    screen_main_menu_->set_shutdown_callback([this]() {
        go_to_shutdown();
    });
}

void AppController::setup_receive_callbacks() {
    // Generate new address when entering receive screen
    screen_receive_->set_derivation_path("m/84'/0'/0'/0/0");
}

void AppController::setup_send_callbacks() {
    screen_send_confirm_->set_sign_callback([this]() {
        Serial.println("Sign transaction clicked");
        
        if (!wallet_.initialized) {
            Serial.println("ERROR: Wallet not initialized");
            return;
        }
        
        // Sign PSBT
        if (psbt_signer_.sign_all(&wallet_.master_key)) {
            Serial.println("Transaction signed successfully");
            
            // Export signed PSBT
            uint8_t signed_psbt[PSBT::MAX_PSBT_SIZE];
            size_t signed_len;
            if (psbt_signer_.export_signed(signed_psbt, &signed_len)) {
                // Display QR of signed PSBT
                Serial.printf("Signed PSBT size: %d bytes\n", signed_len);
            }
            
            SecureMemory::zero(signed_psbt, sizeof(signed_psbt));
        } else {
            Serial.println("ERROR: Failed to sign transaction");
        }
    });
    
    screen_send_confirm_->set_cancel_callback([this]() {
        Serial.println("Cancel transaction clicked");
        psbt_signer_.clear();
        go_to_main_menu();
    });
}

void AppController::update() {
    // State machine updates
    switch (state_) {
        case AppState::SEND_SCAN:
            // Check for QR code scan
            // In real implementation: process camera frame
            break;
            
        case AppState::RECEIVE:
            // Generate new address if needed
            if (wallet_.initialized && screen_receive_) {
                BIP32::ExtendedKey addr_key;
                if (BIP32::derive_path(&wallet_.master_key, "m/84'/0'/0'/0/0", &addr_key)) {
                    BIP32::ExtendedKey pub_key;
                    BIP32::get_public_key(&addr_key, &pub_key);
                    
                    char address[64];
                    if (BitcoinAddress::generate_p2wpkh(pub_key.key, address, sizeof(address), 'm')) {
                        screen_receive_->set_address(address);
                    }
                    
                    BIP32::wipe_key(&addr_key);
                    BIP32::wipe_key(&pub_key);
                }
            }
            break;
            
        default:
            break;
    }
}

void AppController::go_to_welcome() {
    state_ = AppState::WELCOME;
    switch_screen(ScreenID::WELCOME);
}

void AppController::go_to_create_wallet() {
    state_ = AppState::WALLET_CREATE;
    create_new_wallet();
}

void AppController::go_to_restore_wallet() {
    state_ = AppState::WALLET_RESTORE;
    // In real implementation: show mnemonic input screen
}

void AppController::go_to_main_menu() {
    if (!wallet_.initialized) {
        go_to_welcome();
        return;
    }
    state_ = AppState::MAIN_MENU;
    switch_screen(ScreenID::MAIN_MENU);
}

void AppController::go_to_receive() {
    if (!wallet_.initialized) {
        go_to_welcome();
        return;
    }
    state_ = AppState::RECEIVE;
    switch_screen(ScreenID::RECEIVE);
}

void AppController::go_to_send_scan() {
    if (!wallet_.initialized) {
        go_to_welcome();
        return;
    }
    state_ = AppState::SEND_SCAN;
    // In real implementation: show QR scan screen
}

void AppController::go_to_send_confirm(const PSBT::Transaction& tx) {
    state_ = AppState::SEND_CONFIRM;
    
    // Calculate amounts
    uint64_t total_in = 0, total_out = 0;
    for (int i = 0; i < tx.num_inputs; i++) {
        total_in += tx.inputs[i].amount;
    }
    for (int i = 0; i < tx.num_outputs; i++) {
        total_out += tx.outputs[i].amount;
    }
    uint64_t fee = total_in - total_out;
    
    screen_send_confirm_->set_transaction_info(
        total_in, total_out, fee,
        tx.outputs[0].address  // Assuming first output is recipient
    );
    
    switch_screen(ScreenID::SEND_CONFIRM);
}

void AppController::go_to_settings() {
    state_ = AppState::SETTINGS;
    // In real implementation: show settings screen
}

void AppController::go_to_shutdown() {
    state_ = AppState::SHUTDOWN;
    
    // Secure wipe
    Serial.println("Shutting down...");
    wallet_.clear();
    psbt_signer_.clear();
    
    // Turn off display
    M5.Display.setBrightness(0);
    
    // Enter deep sleep
    esp_deep_sleep_start();
}

void AppController::go_to_error(const char* message) {
    state_ = AppState::ERROR;
    Serial.printf("ERROR: %s\n", message);
    // In real implementation: show error screen
}

bool AppController::create_new_wallet() {
    Serial.println("Creating new wallet...");
    
    // Generate entropy
    uint8_t entropy[16];
    HardwareRNG::fill(entropy, sizeof(entropy));
    
    // Generate mnemonic
    char mnemonic[256];
    if (!BIP39::generate_mnemonic(entropy, sizeof(entropy), mnemonic, sizeof(mnemonic))) {
        SecureMemory::zero(entropy, sizeof(entropy));
        return false;
    }
    
    // Display mnemonic
    screen_seed_gen_->set_mnemonic(mnemonic);
    screen_seed_gen_->set_confirm_callback([this, mnemonic]() {
        // User confirmed they wrote down the seed
        // Convert to seed and initialize wallet
        uint8_t seed[64];
        if (BIP39::mnemonic_to_seed(mnemonic, "", seed)) {
            derive_master_key(seed);
            SecureMemory::zero(seed, sizeof(seed));
            go_to_main_menu();
        }
    });
    
    SecureMemory::zero(entropy, sizeof(entropy));
    
    state_ = AppState::SEED_DISPLAY;
    switch_screen(ScreenID::SEED_GENERATE);
    
    return true;
}

bool AppController::restore_wallet(const char* mnemonic) {
    Serial.println("Restoring wallet...");
    
    // Validate mnemonic
    if (!BIP39::validate_mnemonic(mnemonic)) {
        return false;
    }
    
    // Convert to seed
    uint8_t seed[64];
    if (!BIP39::mnemonic_to_seed(mnemonic, "", seed)) {
        return false;
    }
    
    derive_master_key(seed);
    SecureMemory::zero(seed, sizeof(seed));
    
    return true;
}

bool AppController::add_passphrase(const char* passphrase) {
    if (!wallet_.initialized) return false;
    
    // In real implementation: re-derive with passphrase
    wallet_.has_passphrase = true;
    return true;
}

void AppController::lock_wallet() {
    wallet_.clear();
    state_ = AppState::WELCOME;
}

void AppController::switch_screen(ScreenID id) {
    ScreenManager::instance().switch_to(id);
}

void AppController::derive_master_key(const uint8_t* seed) {
    // Initialize master key from seed
    BIP32::init_from_seed(seed, &wallet_.master_key);
    wallet_.initialized = true;
    
    // Calculate fingerprint
    update_wallet_fingerprint();
    
    Serial.printf("Wallet initialized. Fingerprint: %s\n", wallet_.fingerprint);
}

void AppController::update_wallet_fingerprint() {
    // Get master public key
    BIP32::ExtendedKey pub_key;
    BIP32::get_public_key(&wallet_.master_key, &pub_key);
    
    // Calculate fingerprint (first 4 bytes of HASH160)
    uint32_t fp = BIP32::get_fingerprint(&pub_key);
    snprintf(wallet_.fingerprint, sizeof(wallet_.fingerprint), "%08X", fp);
    
    BIP32::wipe_key(&pub_key);
}

} // namespace UI
