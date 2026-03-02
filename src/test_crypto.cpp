/**
 * @file test_crypto.cpp
 * Cryptographic unit tests
 */

#include <Arduino.h>
#include "core/bip39.h"
#include "core/bip32.h"
#include "utils/memory.h"

using namespace SeedSigner;

void test_bip39() {
    Serial.println("\n=== BIP39 Tests ===");
    
    Core::BIP39 bip39;
    
    // Test 1: Generate mnemonic from entropy
    Serial.print("Test 1 - Generate mnemonic... ");
    
    uint8_t entropy[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                           0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    char mnemonic[256];
    
    if (bip39.generate_mnemonic_from_entropy(entropy, 16, mnemonic, sizeof(mnemonic))) {
        Serial.println("PASS");
        Serial.print("  Mnemonic: ");
        Serial.println(mnemonic);
    } else {
        Serial.println("FAIL");
    }
    
    // Test 2: Mnemonic to seed
    Serial.print("Test 2 - Mnemonic to seed... ");
    
    uint8_t seed[64];
    if (bip39.mnemonic_to_seed(mnemonic, "", seed)) {
        Serial.println("PASS");
        Serial.print("  Seed: ");
        for (int i = 0; i < 32; i++) {
            Serial.printf("%02x", seed[i]);
        }
        Serial.println("...");
    } else {
        Serial.println("FAIL");
    }
    
    // Test 3: Validate mnemonic
    Serial.print("Test 3 - Validate mnemonic... ");
    if (bip39.validate_mnemonic(mnemonic)) {
        Serial.println("PASS");
    } else {
        Serial.println("FAIL");
    }
    
    // Test 4: Word lookup
    Serial.print("Test 4 - Word lookup... ");
    int idx = bip39.word_to_index("abandon");
    if (idx == 0) {
        Serial.println("PASS");
    } else {
        Serial.println("FAIL");
    }
    
    Serial.println("===================\n");
}

void test_bip32() {
    Serial.println("\n=== BIP32 Tests ===");
    
    Core::BIP32 bip32;
    
    // Test vector from BIP32
    uint8_t seed[64];
    memset(seed, 0, sizeof(seed));
    const char* seed_hex = "000102030405060708090a0b0c0d0e0f";
    for (int i = 0; i < 16; i++) {
        sscanf(seed_hex + i*2, "%2hhx", &seed[i]);
    }
    
    Serial.print("Test 1 - Init from seed... ");
    if (bip32.init_from_seed(seed)) {
        Serial.println("PASS");
    } else {
        Serial.println("FAIL");
    }
    
    Serial.print("Test 2 - Serialize master key... ");
    char xprv[128];
    const Core::ExtendedKey* master = bip32.get_master_key();
    if (bip32.serialize(master, xprv, sizeof(xprv))) {
        Serial.println("PASS");
        Serial.print("  xprv: ");
        Serial.println(xprv);
    } else {
        Serial.println("FAIL");
    }
    
    Serial.print("Test 3 - Derive path... ");
    Core::ExtendedKey derived;
    if (bip32.derive_path("m/0'", &derived)) {
        Serial.println("PASS");
    } else {
        Serial.println("FAIL (expected for now - needs full implementation)");
    }
    
    Serial.println("===================\n");
}

void test_memory() {
    Serial.println("\n=== Secure Memory Tests ===");
    
    Serial.print("Test 1 - Init... ");
    if (Utils::SecureMemory::init()) {
        Serial.println("PASS");
    } else {
        Serial.println("FAIL");
    }
    
    Serial.print("Test 2 - Alloc/Free... ");
    void* ptr = Utils::SecureMemory::alloc(64);
    if (ptr) {
        memset(ptr, 0xAA, 64);
        Utils::SecureMemory::wipe(ptr, 64);
        Utils::SecureMemory::free(ptr);
        Serial.println("PASS");
    } else {
        Serial.println("FAIL");
    }
    
    Serial.print("Test 3 - Self test... ");
    if (Utils::SecureMemory::self_test()) {
        Serial.println("PASS");
    } else {
        Serial.println("FAIL");
    }
    
    Serial.println("===================\n");
}

void run_all_tests() {
    Serial.println("\n\n");
    Serial.println("########################################");
    Serial.println("# SeedSigner ESP32-S3 Crypto Tests     #");
    Serial.println("########################################\n");
    
    test_memory();
    test_bip39();
    test_bip32();
    
    Serial.println("\nAll tests completed!");
    Serial.println("########################################\n");
}
