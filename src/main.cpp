/**
 * SeedSigner ESP32-S3 Edition
 * Air-Gapped Bitcoin Hardware Wallet
 * 
 * Features:
 * - BIP39 mnemonic generation with hardware entropy
 * - BIP32 HD wallet with full derivation
 * - Bitcoin address generation (P2WPKH, P2SH, P2PKH)
 * - PSBT signing (BIP174)
 * - QR code generation and scanning
 * - LVGL-based UI matching original SeedSigner design
 * - NFC backup support (NTAG424)
 * - State: seed in RAM only, air-gapped (no WiFi/BT)
 */

#include <Arduino.h>
#include <M5Unified.h>
#include <lvgl.h>

// Core crypto
#include "core/hardware_rng.h"
#include "core/secure_memory.h"
#include "core/bip39.h"
#include "core/bip32_secp256k1.h"
#include "core/bitcoin_address.h"
#include "core/psbt_signer.h"
#include "core/qr_generator.h"

// Optimized BIP39
#include "core/bip39_optimized.h"

// UI
#include "ui/screens.h"
#include "ui/app_controller.h"

// Drivers
#include "drivers/gc0308.h"
#include "drivers/pn532_nfc.h"
#include "drivers/secure_storage.h"

// Globals
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1 = nullptr;
static lv_color_t *buf2 = nullptr;

// App controller
AppController g_app;

// Display flush callback for LVGL
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    
    M5.Display.startWrite();
    M5.Display.setAddrWindow(area->x1, area->y1, w, h);
    M5.Display.pushColors((uint16_t *)&color_p->full, w * h, true);
    M5.Display.endWrite();
    
    lv_disp_flush_ready(disp);
}

// Touch read callback
void my_touch_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    auto t = M5.Touch.getDetail();
    if (t.isPressed()) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = t.x;
        data->point.y = t.y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

// LVGL setup
void setup_lvgl() {
    lv_init();
    
    // Allocate display buffers in PSRAM
    const size_t buf_size = 320 * 40;  // 40 lines
    buf1 = (lv_color_t *)heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    buf2 = (lv_color_t *)heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    
    if (!buf1 || !buf2) {
        Serial.println("Failed to allocate LVGL buffers!");
        return;
    }
    
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, buf_size);
    
    // Display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    
    // Touch driver
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touch_read;
    lv_indev_drv_register(&indev_drv);
    
    // Initialize theme
    UI::init_theme();
}

// Security setup
void setup_security() {
    Serial.println("=== Security Setup ===");
    
    // Disable WiFi and BT (air-gapped)
    WiFi.mode(WIFI_OFF);
    btStop();
    
    // Initialize hardware RNG
    HardwareRNG::init();
    
    // Health check
    if (!HardwareRNG::health_check()) {
        Serial.println("WARNING: RNG health check failed!");
    } else {
        Serial.println("RNG health: OK");
    }
    
    // Initialize secure memory
    SecureMemory::init_stack_canary();
    
    // Test secure wipe
    uint8_t test_buf[32];
    memset(test_buf, 0xAA, 32);
    SecureMemory::zero(test_buf, 32);
    
    Serial.println("Security setup complete");
}

// Hardware setup
void setup_hardware() {
    Serial.println("=== Hardware Setup ===");
    
    auto cfg = M5.config();
    cfg.external_display.module_display = 1;
    cfg.external_display.module_gpio = 1;
    M5.begin(cfg);
    
    // Initialize display
    M5.Display.setBrightness(128);
    M5.Display.fillScreen(TFT_BLACK);
    
    // Initialize camera (GC0308)
    if (GC0308::init()) {
        Serial.println("Camera: OK");
    } else {
        Serial.println("Camera: Failed");
    }
    
    // Initialize NFC (PN532)
    if (PN532_NFC::init()) {
        Serial.println("NFC: OK");
    } else {
        Serial.println("NFC: Failed or not connected");
    }
    
    // Secure storage (encrypted preferences)
    SecureStorage::init();
    
    Serial.println("Hardware setup complete");
}

// LVGL tick handler
static void lv_tick_task(void *arg) {
    lv_tick_inc(5);  // 5ms tick
}

// Setup
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n");
    Serial.println("╔══════════════════════════════════════════════════╗");
    Serial.println("║     SeedSigner ESP32-S3 Edition v1.0.0          ║");
    Serial.println("║     Air-Gapped Bitcoin Hardware Wallet          ║");
    Serial.println("╚══════════════════════════════════════════════════╝");
    Serial.println();
    
    // Security first
    setup_security();
    
    // Hardware
    setup_hardware();
    
    // LVGL
    setup_lvgl();
    
    // Create timer for LVGL tick
    esp_timer_create_args_t timer_args = {
        .callback = &lv_tick_task,
        .arg = nullptr,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "lv_tick"
    };
    esp_timer_handle_t timer;
    esp_timer_create(&timer_args, &timer);
    esp_timer_start_periodic(timer, 5000);  // 5ms
    
    // Initialize app controller
    g_app.init();
    
    Serial.println("Setup complete, starting main loop");
}

// Main loop
void loop() {
    // Handle LVGL
    lv_timer_handler();
    
    // Update app controller
    g_app.update();
    
    // Check stack canary periodically
    static unsigned long last_check = 0;
    if (millis() - last_check > 10000) {  // Every 10s
        last_check = millis();
        if (!SecureMemory::check_stack_canary()) {
            Serial.println("ERROR: Stack overflow detected!");
            // In production: wipe keys and reset
        }
    }
    
    // Small delay to yield
    delay(5);
}
