/**
 * @file test_camera.cpp
 * Camera and QR code tests
 */

#include <Arduino.h>
#include "drivers/camera.h"
#include "utils/qr_code.h"

using namespace SeedSigner;

void test_camera_init() {
    Serial.println("\n=== Camera Init Test ===");
    
    Drivers::Camera camera;
    
    Serial.print("Initializing camera... ");
    if (camera.init()) {
        Serial.println("PASS");
        
        Serial.print("Resolution: ");
        switch (camera.get_resolution()) {
            case Drivers::CameraResolution::QVGA:
                Serial.println("QVGA (320x240)");
                break;
            default:
                Serial.println("Other");
                break;
        }
        
        // Test capture
        Serial.print("Capturing frame... ");
        Drivers::CameraFrame frame;
        if (camera.capture(&frame)) {
            Serial.println("PASS");
            Serial.printf("  Size: %dx%d, Length: %d bytes\n", 
                         frame.width, frame.height, frame.len);
            
            // Release framebuffer
            if (frame._fb) {
                esp_camera_fb_return((camera_fb_t*)frame._fb);
                frame._fb = nullptr;
            }
        } else {
            Serial.println("FAIL");
        }
        
        camera.deinit();
    } else {
        Serial.println("FAIL - Check camera connection");
    }
    
    Serial.println("=======================\n");
}

void test_qr_generation() {
    Serial.println("\n=== QR Code Generation Test ===");
    
    Utils::QRCodeGenerator qr_gen;
    Utils::QRCode qr;
    
    const char* test_data = "bitcoin:bc1q...";
    
    Serial.print("Generating QR code... ");
    if (qr_gen.generate(test_data, &qr, Utils::QRErrorLevel::MEDIUM)) {
        Serial.println("PASS");
        Serial.printf("  Version: %d\n", qr.version);
        Serial.printf("  Size: %dx%d modules\n", qr.size, qr.size);
        Serial.printf("  Data: %s\n", qr.data);
        
        // Print ASCII representation
        Serial.println("  QR Preview:");
        for (int y = 0; y < qr.size && y < 25; y++) {
            Serial.print("    ");
            for (int x = 0; x < qr.size && x < 25; x++) {
                Serial.print(qr.modules[y][x] ? "██" : "  ");
            }
            Serial.println();
        }
    } else {
        Serial.println("FAIL");
    }
    
    Serial.println("=======================\n");
}

void test_camera_entropy() {
    Serial.println("\n=== Camera Entropy Test ===");
    
    Drivers::Camera camera;
    
    if (!camera.init()) {
        Serial.println("Camera init failed");
        return;
    }
    
    // Set low resolution for faster capture
    camera.set_resolution(Drivers::CameraResolution::QQVGA);
    
    // Capture frame with lens covered (dark frame noise)
    Serial.println("Cover the camera lens for entropy collection");
    delay(2000);
    
    Drivers::CameraFrame frame;
    if (camera.capture(&frame)) {
        Serial.print("Collecting entropy... ");
        
        uint8_t entropy[32];
        if (camera.collect_entropy_region(&frame, 0, 0, 
                                           frame.width, frame.height,
                                           entropy, sizeof(entropy))) {
            Serial.println("DONE");
            Serial.print("  Entropy: ");
            for (int i = 0; i < 16; i++) {
                Serial.printf("%02x", entropy[i]);
            }
            Serial.println("...");
        } else {
            Serial.println("FAIL");
        }
        
        // Release framebuffer
        if (frame._fb) {
            esp_camera_fb_return((camera_fb_t*)frame._fb);
        }
    }
    
    camera.deinit();
    Serial.println("=======================\n");
}

void test_qr_scan() {
    Serial.println("\n=== QR Scan Test ===");
    
    Drivers::Camera camera;
    Utils::QRCodeScanner scanner;
    
    if (!camera.init()) {
        Serial.println("Camera init failed");
        return;
    }
    
    if (!scanner.init()) {
        Serial.println("Scanner init failed");
        camera.deinit();
        return;
    }
    
    Serial.println("Starting QR scan (5 seconds)...");
    scanner.start();
    
    unsigned long start = millis();
    while (millis() - start < 5000) {
        Drivers::CameraFrame frame;
        if (camera.capture(&frame)) {
            // Process frame for QR
            if (scanner.process_frame(frame.data, frame.width, frame.height)) {
                Serial.println("QR Code detected!");
                Serial.printf("  Data: %s\n", scanner.get_result());
                break;
            }
            
            // Release framebuffer
            if (frame._fb) {
                esp_camera_fb_return((camera_fb_t*)frame._fb);
            }
        }
        delay(100);
    }
    
    scanner.stop();
    camera.deinit();
    
    Serial.println("=======================\n");
}

void run_camera_tests() {
    Serial.println("\n");
    Serial.println("########################################");
    Serial.println("# SeedSigner Camera & QR Tests         #");
    Serial.println("########################################\n");
    
    test_camera_init();
    test_qr_generation();
    test_camera_entropy();
    // test_qr_scan();  // Requires physical QR code
    
    Serial.println("\nCamera tests completed!");
    Serial.println("########################################\n");
}
