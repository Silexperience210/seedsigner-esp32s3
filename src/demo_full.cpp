/**
 * @file demo_full.cpp
 * Full demo: Seed generation → XPUB export → QR display
 * Complete workflow demonstration
 */

#include <Arduino.h>
#include <M5Unified.h>
#include "core/bip39.h"
#include "core/bip32.h"
#include "core/entropy.h"
#include "drivers/camera.h"
#include "utils/qr_code.h"
#include "utils/memory.h"

using namespace SeedSigner;

// Demo state machine
enum DemoState {
    DEMO_INIT,
    DEMO_ENTROPY,
    DEMO_GENERATE,
    DEMO_DISPLAY,
    DEMO_XPUB,
    DEMO_QR,
    DEMO_DONE
};

class SeedSignerDemo {
private:
    DemoState m_state;
    Core::BIP39 m_bip39;
    Core::BIP32 m_bip32;
    Core::Entropy m_entropy;
    Drivers::Camera m_camera;
    Utils::QRCodeGenerator m_qr_gen;
    
    char m_mnemonic[512];
    uint8_t m_seed[64];
    bool m_has_seed;
    
public:
    SeedSignerDemo() : m_state(DEMO_INIT), m_has_seed(false) {
        memset(m_mnemonic, 0, sizeof(m_mnemonic));
        memset(m_seed, 0, sizeof(m_seed));
    }
    
    void run() {
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.setTextSize(2);
        
        switch (m_state) {
            case DEMO_INIT:
                do_init();
                break;
            case DEMO_ENTROPY:
                do_entropy();
                break;
            case DEMO_GENERATE:
                do_generate();
                break;
            case DEMO_DISPLAY:
                do_display();
                break;
            case DEMO_XPUB:
                do_xpub();
                break;
            case DEMO_QR:
                do_qr();
                break;
            case DEMO_DONE:
                do_done();
                break;
        }
    }
    
private:
    void do_init() {
        M5.Display.setCursor(10, 10);
        M5.Display.println("SeedSigner Demo");
        M5.Display.setTextSize(1);
        M5.Display.println("\nInitializing...");
        
        // Init components
        m_bip39.init();
        m_entropy.init();
        
        M5.Display.println("OK!");
        delay(1000);
        
        m_state = DEMO_ENTROPY;
    }
    
    void do_entropy() {
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setCursor(10, 10);
        M5.Display.setTextSize(2);
        M5.Display.println("Step 1: Entropy");
        M5.Display.setTextSize(1);
        
        // Option 1: Hardware RNG
        M5.Display.println("\n1. Hardware RNG...");
        uint8_t hw_entropy[32];
        m_entropy.collect_hardware_rng(hw_entropy, 32);
        M5.Display.println("   OK (32 bytes)");
        
        // Option 2: Camera noise (if available)
        M5.Display.println("\n2. Camera noise...");
        if (m_camera.init()) {
            m_camera.set_resolution(Drivers::CameraResolution::QQVGA);
            
            Drivers::CameraFrame frame;
            if (m_camera.capture(&frame)) {
                uint8_t cam_entropy[32];
                if (m_camera.collect_entropy_region(&frame, 0, 0, 
                                                    frame.width/2, frame.height/2,
                                                    cam_entropy, 32)) {
                    // Mix entropies
                    for (int i = 0; i < 32; i++) {
                        hw_entropy[i] ^= cam_entropy[i];
                    }
                    M5.Display.println("   OK (mixed)");
                }
                
                if (frame._fb) {
                    esp_camera_fb_return((camera_fb_t*)frame._fb);
                }
            }
            m_camera.deinit();
        } else {
            M5.Display.println("   SKIP (no cam)");
        }
        
        // Display entropy hash
        M5.Display.println("\nEntropy fingerprint:");
        for (int i = 0; i < 8; i++) {
            M5.Display.printf("%02x", hw_entropy[i]);
        }
        M5.Display.println("...");
        
        // Generate mnemonic
        m_bip39.generate_mnemonic_from_entropy(hw_entropy, 32, m_mnemonic, sizeof(m_mnemonic));
        
        // Secure wipe entropy
        Utils::SecureMemory::wipe(hw_entropy, sizeof(hw_entropy));
        
        delay(2000);
        m_state = DEMO_GENERATE;
    }
    
    void do_generate() {
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setCursor(10, 10);
        M5.Display.setTextSize(2);
        M5.Display.println("Step 2: Seed");
        M5.Display.setTextSize(1);
        
        // Generate seed from mnemonic
        M5.Display.println("\nDeriving seed...");
        m_bip39.mnemonic_to_seed(m_mnemonic, "", m_seed);
        
        // Init BIP32
        M5.Display.println("Initializing HD wallet...");
        m_bip32.init_from_seed(m_seed);
        
        m_has_seed = true;
        
        M5.Display.println("\n✓ Seed generated!");
        delay(1500);
        
        m_state = DEMO_DISPLAY;
    }
    
    void do_display() {
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setCursor(10, 10);
        M5.Display.setTextSize(2);
        M5.Display.setTextColor(TFT_ORANGE);
        M5.Display.println("RECOVERY PHRASE");
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.setTextSize(1);
        
        // Display mnemonic words
        M5.Display.println("\nWrite down these 24 words:\n");
        
        char mnemonic_copy[512];
        strncpy(mnemonic_copy, m_mnemonic, sizeof(mnemonic_copy) - 1);
        
        char* word = strtok(mnemonic_copy, " ");
        int word_num = 1;
        int y = 60;
        
        while (word && word_num <= 24) {
            M5.Display.setCursor(10 + ((word_num - 1) % 2) * 150, y);
            M5.Display.printf("%2d. %s", word_num, word);
            
            word = strtok(nullptr, " ");
            word_num++;
            if (word_num % 2 == 1) {
                y += 15;
            }
        }
        
        M5.Display.setTextColor(TFT_RED);
        M5.Display.setCursor(10, 220);
        M5.Display.println("Never share these words!");
        
        wait_for_touch();
        m_state = DEMO_XPUB;
    }
    
    void do_xpub() {
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setCursor(10, 10);
        M5.Display.setTextSize(2);
        M5.Display.println("Step 3: XPUB");
        M5.Display.setTextSize(1);
        
        // Derive account xpub (m/84'/0'/0')
        M5.Display.println("\nDeriving xpub...");
        M5.Display.println("Path: m/84'/0'/0'");
        
        Core::ExtendedKey account_key;
        if (m_bip32.derive_path("m/84'/0'/0'", &account_key)) {
            // Get public key
            Core::ExtendedKey xpub;
            m_bip32.get_public_key(&account_key, &xpub);
            
            // Serialize
            char xpub_str[128];
            m_bip32.serialize(&xpub, xpub_str, sizeof(xpub_str));
            
            M5.Display.println("\nAccount XPUB:");
            M5.Display.setTextSize(1);
            M5.Display.println(xpub_str);
            
            // Store for QR
            strncpy(m_mnemonic, xpub_str, sizeof(m_mnemonic) - 1);
        } else {
            M5.Display.println("\nDerivation failed");
            strncpy(m_mnemonic, "xpub6DummyKeyForTesting", sizeof(m_mnemonic) - 1);
        }
        
        delay(2000);
        m_state = DEMO_QR;
    }
    
    void do_qr() {
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setCursor(10, 10);
        M5.Display.setTextSize(2);
        M5.Display.println("Step 4: Export");
        M5.Display.setTextSize(1);
        M5.Display.println("\nScan with wallet app:\n");
        
        // Generate QR
        Utils::QRCode qr;
        if (m_qr_gen.generate(m_mnemonic, &qr, Utils::QRErrorLevel::MEDIUM)) {
            // Render QR on screen (centered, scaled)
            int qr_size = qr.size;
            int scale = 6;  // Scale factor
            int offset_x = (320 - qr_size * scale) / 2;
            int offset_y = 50;
            
            // Draw quiet zone
            M5.Display.fillRect(offset_x - 8, offset_y - 8, 
                               qr_size * scale + 16, qr_size * scale + 16, TFT_WHITE);
            
            // Draw QR modules
            for (int y = 0; y < qr_size; y++) {
                for (int x = 0; x < qr_size; x++) {
                    if (qr.modules[y][x]) {
                        M5.Display.fillRect(offset_x + x * scale, 
                                           offset_y + y * scale, 
                                           scale, scale, TFT_BLACK);
                    }
                }
            }
            
            M5.Display.setCursor(10, 220);
            M5.Display.println("XPUB QR Code (Native Segwit)");
        }
        
        wait_for_touch();
        m_state = DEMO_DONE;
    }
    
    void do_done() {
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setCursor(10, 80);
        M5.Display.setTextSize(2);
        M5.Display.setTextColor(TFT_GREEN);
        M5.Display.println("    Demo Complete!");
        
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.setTextSize(1);
        M5.Display.setCursor(10, 140);
        M5.Display.println("Next steps:");
        M5.Display.println("1. Write down recovery phrase");
        M5.Display.println("2. Scan XPUB in wallet app");
        M5.Display.println("3. Verify receive address");
        M5.Display.println("4. Wipe device memory");
        
        M5.Display.setTextColor(TFT_ORANGE);
        M5.Display.setCursor(10, 220);
        M5.Display.println("Tap to restart");
        
        wait_for_touch();
        
        // Wipe everything
        clear_all();
        m_state = DEMO_INIT;
    }
    
    void wait_for_touch() {
        while (true) {
            M5.update();
            if (M5.Touch.getCount() > 0) {
                break;
            }
            delay(50);
        }
        delay(300);  // Debounce
    }
    
    void clear_all() {
        // Wipe sensitive data
        Utils::SecureMemory::wipe(m_mnemonic, sizeof(m_mnemonic));
        Utils::SecureMemory::wipe(m_seed, sizeof(m_seed));
        m_bip32.clear();
        m_has_seed = false;
    }
};

// Global demo instance
static SeedSignerDemo* g_demo = nullptr;

void demo_init() {
    g_demo = new SeedSignerDemo();
}

void demo_run() {
    if (g_demo) {
        g_demo->run();
    }
}

void demo_full_workflow() {
    Serial.println("\n");
    Serial.println("########################################");
    Serial.println("# SeedSigner Full Demo                 #");
    Serial.println("########################################\n");
    
    demo_init();
    
    // Run demo loop
    for (int i = 0; i < 10; i++) {
        demo_run();
        delay(100);
    }
    
    Serial.println("Demo initialized - check display!");
    Serial.println("########################################\n");
}
