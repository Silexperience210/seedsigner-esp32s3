#include "core/secp256k1_wrapper.h"
#include "core/bip32_secp256k1.h"
#include <Arduino.h>
#include <Bitcoin.h>

// Vecteurs de test BIP32
// https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki#test-vectors

void test_secp256k1_basic() {
    Serial.println("\n=== Test secp256k1 (uBitcoin) ===");
    
    // Test 1: Génération de clé publique
    uint8_t privkey[32] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
    };
    
    uint8_t pubkey[33];
    bool result = Secp256k1::generate_public_key(privkey, pubkey, true);
    
    Serial.print("Generate public key: ");
    Serial.println(result ? "PASS" : "FAIL");
    
    if (result) {
        Serial.print("Public key: ");
        for (int i = 0; i < 33; i++) {
            if (pubkey[i] < 0x10) Serial.print("0");
            Serial.print(pubkey[i], HEX);
        }
        Serial.println();
        
        // Expected: 0279BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
        uint8_t expected[33] = {
            0x02, 0x79, 0xBE, 0x66, 0x74, 0xF9, 0xDC, 0xBB,
            0xAC, 0x55, 0xA0, 0x62, 0x95, 0xCE, 0x87, 0x0B,
            0x07, 0x02, 0x9B, 0xFC, 0xDB, 0x2D, 0xCE, 0x28,
            0xD9, 0x59, 0xF2, 0x81, 0x5B, 0x16, 0xF8, 0x17, 0x98
        };
        
        bool match = memcmp(pubkey, expected, 33) == 0;
        Serial.print("Matches expected: ");
        Serial.println(match ? "PASS" : "FAIL");
    }
    
    // Test 2: Signature
    uint8_t hash[32] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    
    uint8_t signature[64];
    result = Secp256k1::sign(privkey, hash, signature, nullptr);
    Serial.print("Sign: ");
    Serial.println(result ? "PASS" : "FAIL");
    
    if (result) {
        // Vérifier la signature
        result = Secp256k1::verify(pubkey, hash, signature);
        Serial.print("Verify: ");
        Serial.println(result ? "PASS" : "FAIL");
    }
}

void test_bip32() {
    Serial.println("\n=== Test BIP32 ===");
    
    // Test vector 1 from BIP32 spec
    // Seed (hex): 000102030405060708090a0b0c0d0e0f
    uint8_t seed[64] = {0};
    for (int i = 0; i < 16; i++) {
        seed[i] = i;
    }
    
    BIP32::ExtendedKey master;
    bool result = BIP32::init_from_seed(seed, &master);
    
    Serial.print("Init from seed: ");
    Serial.println(result ? "PASS" : "FAIL");
    
    if (result) {
        // Check chain code
        Serial.print("Chain code: ");
        for (int i = 0; i < 32; i++) {
            if (master.chain_code[i] < 0x10) Serial.print("0");
            Serial.print(master.chain_code[i], HEX);
        }
        Serial.println();
        
        // Derive m/0'
        BIP32::ExtendedKey child;
        result = BIP32::derive_child(&master, 0x80000000, &child);
        Serial.print("Derive m/0': ");
        Serial.println(result ? "PASS" : "FAIL");
        
        if (result) {
            Serial.print("Child fingerprint: 0x");
            uint32_t fp = BIP32::get_fingerprint(&child);
            Serial.println(fp, HEX);
        }
        
        // Test full path derivation
        BIP32::ExtendedKey path_key;
        result = BIP32::derive_path(&master, "m/44'/0'/0'/0/0", &path_key);
        Serial.print("Derive m/44'/0'/0'/0/0: ");
        Serial.println(result ? "PASS" : "FAIL");
        
        if (result) {
            // Serialize
            char xprv[113];
            result = BIP32::serialize(&path_key, false, xprv, sizeof(xprv));
            Serial.print("Serialize xprv: ");
            Serial.println(result ? "PASS" : "FAIL");
            if (result) {
                Serial.print("xprv: ");
                Serial.println(xprv);
            }
            
            // Get public key
            BIP32::ExtendedKey pub_key;
            result = BIP32::get_public_key(&path_key, &pub_key);
            Serial.print("Get public key: ");
            Serial.println(result ? "PASS" : "FAIL");
            
            if (result) {
                char xpub[113];
                result = BIP32::serialize(&pub_key, true, xpub, sizeof(xpub));
                Serial.print("Serialize xpub: ");
                Serial.println(result ? "PASS" : "FAIL");
                if (result) {
                    Serial.print("xpub: ");
                    Serial.println(xpub);
                }
            }
        }
        
        // Clean up
        BIP32::wipe_key(&master);
        BIP32::wipe_key(&child);
        BIP32::wipe_key(&path_key);
    }
}

void test_hash160() {
    Serial.println("\n=== Test HASH160 ===");
    
    // Test: HASH160 of empty data should produce known result
    uint8_t data[] = {0x02, 0x79, 0xBE, 0x66, 0x74, 0xF9, 0xDC, 0xBB,
                      0xAC, 0x55, 0xA0, 0x62, 0x95, 0xCE, 0x87, 0x0B,
                      0x07, 0x02, 0x9B, 0xFC, 0xDB, 0x2D, 0xCE, 0x28,
                      0xD9, 0x59, 0xF2, 0x81, 0x5B, 0x16, 0xF8, 0x17, 0x98};
    
    uint8_t hash160_result[20];
    Secp256k1::hash160(data, 33, hash160_result);
    
    Serial.print("HASH160: ");
    for (int i = 0; i < 20; i++) {
        if (hash160_result[i] < 0x10) Serial.print("0");
        Serial.print(hash160_result[i], HEX);
    }
    Serial.println();
    
    // Expected: 91b24bf9f5288532960ac687abb035127b1d28a5
    uint8_t expected[20] = {
        0x91, 0xb2, 0x4b, 0xf9, 0xf5, 0x28, 0x85, 0x32,
        0x96, 0x0a, 0xc6, 0x87, 0xab, 0xb0, 0x35, 0x12,
        0x7b, 0x1d, 0x28, 0xa5
    };
    
    bool match = memcmp(hash160_result, expected, 20) == 0;
    Serial.print("Matches expected: ");
    Serial.println(match ? "PASS" : "FAIL");
}

void run_crypto_tests() {
    Serial.println("\n");
    Serial.println("╔══════════════════════════════════════════════════╗");
    Serial.println("║    CRYPTO TESTS - uBitcoin Integration          ║");
    Serial.println("╚══════════════════════════════════════════════════╝");
    
    test_secp256k1_basic();
    test_hash160();
    test_bip32();
    
    Serial.println("\n=== Tests Complete ===");
}
