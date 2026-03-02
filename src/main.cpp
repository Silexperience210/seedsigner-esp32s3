/**
 * SeedSigner ESP32-S3 Edition
 * Main Entry Point
 * 
 * Air-gapped, stateless Bitcoin hardware wallet
 * Based on SeedSigner firmware, adapted for ESP32-S3
 */

#include <Arduino.h>
#include <lvgl.h>
#include <SPI.h>
#include <Wire.h>

// M5Stack CoreS3 support
#ifdef M5STACK_CORES3
#include <M5Unified.h>
#include <M5GFX.h>
#endif

// Project headers
#include "core/bip39.h"
#include "core/bip32.h"
#include "core/psbt.h"
#include "core/entropy.h"
#include "ui/app.h"
#include "drivers/display.h"
#include "drivers/camera.h"
#include "drivers/nfc.h"
#include "drivers/touch.h"
#include "drivers/sd.h"
#include "utils/memory.h"
#include "utils/qr_code.h"

// Test functions
extern void run_all_tests();
extern void run_camera_tests();
extern void demo_full_workflow();

// ============================================
// Configuration & Constants
// ============================================

#define FIRMWARE_VERSION "0.1.0-alpha"
#define FIRMWARE_NAME "SeedSigner-ESP32S3"

// Security: Disable WiFi and BT at compile time if possible
#if WIFI_ENABLED == 0
#include "esp_wifi.h"
#endif

#if BT_ENABLED == 0
#include "esp_bt.h"
#endif

// ============================================
// Global Objects
// ============================================

static const char* TAG = "SeedSigner";

// UI Application
UI::App* g_app = nullptr;

// Hardware drivers
Drivers::Display* g_display = nullptr;
Drivers::Camera* g_camera = nullptr;
Drivers::NFC* g_nfc = nullptr;
Drivers::Touch* g_touch = nullptr;
Drivers::SDCard* g_sd = nullptr;

// Security: Watchdog timer
hw_timer_t* g_watchdog_timer = nullptr;

// ============================================
// Function Prototypes
// ============================================

void setup_hardware();
void setup_security();
void setup_lvgl();
void setup_drivers();
void run_self_tests();
void watchdog_handler();
void security_wipe_memory();

// LVGL callbacks
void lvgl_flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p);
void lvgl_read_cb(lv_indev_drv_t* drv, lv_indev_data_t* data);
uint32_t lvgl_tick_cb();

// ============================================
// Setup
// ============================================

void setup() {
    // Initialize serial for debugging (disabled in production)
    #ifdef DEBUG_SEEDSIGNER
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n========================================");
    Serial.printf("  %s v%s\n", FIRMWARE_NAME, FIRMWARE_VERSION);
    Serial.println("========================================\n");
    #endif

    // CRITICAL: Security setup first
    setup_security();
    
    // Hardware initialization
    setup_hardware();
    
    // Initialize LVGL
    setup_lvgl();
    
    // Initialize hardware drivers
    setup_drivers();
    
    // Run self-tests
    #ifdef DEBUG_SEEDSIGNER
    run_self_tests();
    #endif
    
    // Create and start UI application
    g_app = new UI::App();
    g_app->init();
    
    Serial.println("Setup complete. Starting main loop...");
}

// ============================================
// Main Loop
// ============================================

void loop() {
    // Feed watchdog
    #if ENABLE_WATCHDOG
    timerWrite(g_watchdog_timer, 0);
    #endif
    
    // Handle LVGL tasks
    lv_timer_handler();
    
    // Handle UI events
    if (g_app) {
        g_app->update();
    }
    
    // Handle NFC polling (non-blocking)
    if (g_nfc) {
        g_nfc->poll();
    }
    
    // Small delay to prevent busy-waiting
    delay(5);
}

// ============================================
// Security Setup
// ============================================

void setup_security() {
    Serial.println("[SECURITY] Initializing security subsystem...");
    
    // 1. Disable WiFi (permanent disable for air-gap)
    #if WIFI_ENABLED == 0
    esp_wifi_stop();
    esp_wifi_deinit();
    WiFi.mode(WIFI_OFF);
    Serial.println("[SECURITY] WiFi disabled");
    #endif
    
    // 2. Disable Bluetooth (permanent disable)
    #if BT_ENABLED == 0
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
    Serial.println("[SECURITY] Bluetooth disabled");
    #endif
    
    // 3. Clear sensitive memory regions
    #if MEMORY_CLEAR_ON_BOOT
    security_wipe_memory();
    #endif
    
    // 4. Initialize secure memory allocator
    Utils::SecureMemory::init();
    
    // 5. Setup watchdog timer
    #if ENABLE_WATCHDOG
    g_watchdog_timer = timerBegin(0, 80, true); // 80MHz / 80 = 1MHz
    timerAttachInterrupt(g_watchdog_timer, watchdog_handler, true);
    timerAlarmWrite(g_watchdog_timer, 10000000, true); // 10 second timeout
    timerAlarmEnable(g_watchdog_timer);
    Serial.println("[SECURITY] Watchdog enabled (10s timeout)");
    #endif
    
    // 6. Disable JTAG/debug interfaces
    // esp_efuse_disable_basic_rom_console();
    
    Serial.println("[SECURITY] Security subsystem initialized");
}

void security_wipe_memory() {
    Serial.println("[SECURITY] Wiping memory regions...");
    
    // Wipe BSS (uninitialized data)
    extern char _bss_start, _bss_end;
    memset(&_bss_start, 0, &_bss_end - &_bss_start);
    
    // Note: Stack and heap will be cleared by normal initialization
    
    Serial.println("[SECURITY] Memory wiped");
}

void watchdog_handler() {
    Serial.println("[SECURITY] Watchdog timeout! Restarting...");
    ESP.restart();
}

// ============================================
// Hardware Setup
// ============================================

void setup_hardware() {
    Serial.println("[HARDWARE] Initializing hardware...");
    
    #ifdef M5STACK_CORES3
    auto cfg = M5.config();
    cfg.external_display.module_display = true;
    cfg.external_display.module_gyro = true;
    cfg.external_display.module_rca = false;
    cfg.internal_spk = true;
    cfg.internal_mic = true;
    M5.begin(cfg);
    
    // Set startup brightness
    M5.Display.setBrightness(128);
    
    // Show splash screen
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_ORANGE);
    M5.Display.setTextSize(2);
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.drawString("SeedSigner", M5.Display.width() / 2, M5.Display.height() / 2 - 20);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(1);
    M5.Display.drawString("ESP32-S3 Edition", M5.Display.width() / 2, M5.Display.height() / 2 + 10);
    M5.Display.drawString(FIRMWARE_VERSION, M5.Display.width() / 2, M5.Display.height() / 2 + 30);
    delay(2000);
    #endif
    
    Serial.println("[HARDWARE] Hardware initialized");
}

// ============================================
// LVGL Setup
// ============================================

void setup_lvgl() {
    Serial.println("[LVGL] Initializing LVGL...");
    
    lv_init();
    
    // Allocate draw buffer (in PSRAM if available)
    static lv_color_t* draw_buf = nullptr;
    const size_t draw_buf_size = DISPLAY_WIDTH * DISPLAY_HEIGHT / 10;
    
    #if CONFIG_SPIRAM_USE
    draw_buf = (lv_color_t*)heap_caps_malloc(
        draw_buf_size * sizeof(lv_color_t),
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
    );
    #else
    draw_buf = (lv_color_t*)malloc(draw_buf_size * sizeof(lv_color_t));
    #endif
    
    if (!draw_buf) {
        Serial.println("[LVGL] ERROR: Failed to allocate draw buffer!");
        return;
    }
    
    // Register display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = DISPLAY_WIDTH;
    disp_drv.ver_res = DISPLAY_HEIGHT;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = lv_disp_draw_buf_create(
        DISPLAY_WIDTH * 10,
        draw_buf,
        draw_buf + draw_buf_size / 2,
        draw_buf_size / 2
    );
    lv_disp_drv_register(&disp_drv);
    
    // Register touch driver
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_read_cb;
    lv_indev_drv_register(&indev_drv);
    
    // Set tick source
    lv_tick_set_cb(lvgl_tick_cb);
    
    Serial.println("[LVGL] LVGL initialized");
}

// ============================================
// Driver Setup
// ============================================

void setup_drivers() {
    Serial.println("[DRIVERS] Initializing drivers...");
    
    // Display driver
    g_display = new Drivers::Display();
    if (!g_display->init()) {
        Serial.println("[DRIVERS] ERROR: Display init failed!");
    }
    
    // Touch driver
    g_touch = new Drivers::Touch();
    if (!g_touch->init()) {
        Serial.println("[DRIVERS] ERROR: Touch init failed!");
    }
    
    // Camera driver
    g_camera = new Drivers::Camera();
    if (!g_camera->init()) {
        Serial.println("[DRIVERS] WARNING: Camera init failed");
    }
    
    // NFC driver
    g_nfc = new Drivers::NFC();
    if (!g_nfc->init()) {
        Serial.println("[DRIVERS] WARNING: NFC init failed");
    }
    
    // SD Card driver
    g_sd = new Drivers::SDCard();
    if (!g_sd->init()) {
        Serial.println("[DRIVERS] WARNING: SD Card init failed");
    }
    
    Serial.println("[DRIVERS] Drivers initialized");
}

// ============================================
// LVGL Callbacks
// ============================================

void lvgl_flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
    #ifdef M5STACK_CORES3
    M5.Display.startWrite();
    M5.Display.setAddrWindow(
        area->x1,
        area->y1,
        area->x2 - area->x1 + 1,
        area->y2 - area->y1 + 1
    );
    M5.Display.writePixels(
        (uint16_t*)color_p,
        (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1)
    );
    M5.Display.endWrite();
    #endif
    
    lv_disp_flush_ready(drv);
}

void lvgl_read_cb(lv_indev_drv_t* drv, lv_indev_data_t* data) {
    if (g_touch && g_touch->is_touched()) {
        int16_t x, y;
        g_touch->get_touch(&x, &y);
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

uint32_t lvgl_tick_cb() {
    return millis();
}

// ============================================
// Self Tests
// ============================================

void run_self_tests() {
    Serial.println("[TEST] Running self-tests...");
    
    // Test BIP39
    Serial.println("[TEST] Testing BIP39...");
    Core::BIP39 bip39;
    if (!bip39.self_test()) {
        Serial.println("[TEST] BIP39 self-test FAILED!");
    } else {
        Serial.println("[TEST] BIP39 self-test PASSED");
    }
    
    // Test entropy generation
    Serial.println("[TEST] Testing entropy...");
    Core::Entropy entropy;
    if (!entropy.self_test()) {
        Serial.println("[TEST] Entropy self-test FAILED!");
    } else {
        Serial.println("[TEST] Entropy self-test PASSED");
    }
    
    // Test secure memory
    Serial.println("[TEST] Testing secure memory...");
    if (!Utils::SecureMemory::self_test()) {
        Serial.println("[TEST] Secure memory self-test FAILED!");
    } else {
        Serial.println("[TEST] Secure memory self-test PASSED");
    }
    
    Serial.println("[TEST] Self-tests complete");
}
