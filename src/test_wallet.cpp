#include "core/bip39.h"
#include "core/bip32_secp256k1.h"
#include "core/bitcoin_address.h"
#include <Arduino.h>

// Test vectors from BIP39 and BIP32 specs

void test_bip39_vectors() {
    Serial.println("\n=== BIP39 Test Vectors ===");
    
    // Test vector 1 (12 words)
    // Entropy: 00000000000000000000000000000000
    // Mnemonic: abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about
    // Seed: c55257c360c07c72029aebc1b53c05ed0362ada38ead3e3e9efa3708e53495531f09a6987599d18264c1e1c92f2cf141630c7a3c4ab7c81b2f001698e7463b04
    
    uint8_t entropy1[16] = {0};
    char mnemonic1[256];
    
    bool result = BIP39::generate_mnemonic(entropy1, sizeof(entropy1), mnemonic1, sizeof(mnemonic1));
    Serial.print("Generate 12-word mnemonic: ");
    Serial.println(result ? "PASS" : "FAIL");
    
    if (result) {
        Serial.print("Mnemonic: ");
        Serial.println(mnemonic1);
        
        // Validate
        bool valid = BIP39::validate_mnemonic(mnemonic1);
        Serial.print("Validate: ");
        Serial.println(valid ? "PASS" : "FAIL");
        
        // Convert to seed
        uint8_t seed[64];
        result = BIP39::mnemonic_to_seed(mnemonic1, "", seed);
        Serial.print("Mnemonic to seed: ");
        Serial.println(result ? "PASS" : "FAIL");
        
        if (result) {
            Serial.print("Seed: ");
            for (int i = 0; i < 32; i++) { // Print first 32 bytes
                if (seed[i] < 0x10) Serial.print("0");
                Serial.print(seed[i], HEX);
            }
            Serial.println("...");
        }
    }
}

void test_bip32_bip39_integration() {
    Serial.println("\n=== BIP32 + BIP39 Integration ===");
    
    // Generate mnemonic
    char mnemonic[256];
    uint8_t entropy[16];
    
    // Fill with test entropy
    for (int i = 0; i < 16; i++) {
        entropy[i] = i * 17; // Pseudo-random
    }
    
    bool result = BIP39::generate_mnemonic(entropy, sizeof(entropy), mnemonic, sizeof(mnemonic));
    Serial.print("Generate mnemonic: ");
    Serial.println(result ? "PASS" : "FAIL");
    
    if (!result) return;
    
    Serial.print("Mnemonic: ");
    Serial.println(mnemonic);
    
    // Convert to seed
    uint8_t seed[64];
    result = BIP39::mnemonic_to_seed(mnemonic, "", seed);
    Serial.print("To seed: ");
    Serial.println(result ? "PASS" : "FAIL");
    
    if (!result) return;
    
    // Create BIP32 master key
    BIP32::ExtendedKey master;
    result = BIP32::init_from_seed(seed, &master);
    Serial.print("Init BIP32 master: ");
    Serial.println(result ? "PASS" : "FAIL");
    
    if (!result) return;
    
    // Derive first account
    BIP32::ExtendedKey account;
    result = BIP32::derive_path(&master, "m/84'/0'/0'", &account); // BIP84 native segwit
    Serial.print("Derive m/84'/0'/0': ");
    Serial.println(result ? "PASS" : "FAIL");
    
    if (!result) return;
    
    // Get xpub
    BIP32::ExtendedKey account_pub;
    result = BIP32::get_public_key(&account, &account_pub);
    Serial.print("Get account xpub: ");
    Serial.println(result ? "PASS" : "FAIL");
    
    if (result) {
        char xpub[113];
        BIP32::serialize(&account_pub, true, xpub, sizeof(xpub));
        Serial.print("Account xpub: ");
        Serial.println(xpub);
    }
    
    // Derive first receiving address
    BIP32::ExtendedKey addr_key;
    result = BIP32::derive_path(&account, "0/0", &addr_key);
    Serial.print("Derive first address key: ");
    Serial.println(result ? "PASS" : "FAIL");
    
    if (!result) return;
    
    // Get public key
    BIP32::ExtendedKey addr_pub;
    result = BIP32::get_public_key(&addr_key, &addr_pub);
    Serial.print("Get address pubkey: ");
    Serial.println(result ? "PASS" : "FAIL");
    
    if (!result) return;
    
    // Generate P2WPKH address
    char address[64];
    result = BitcoinAddress::generate_p2wpkh(addr_pub.key, address, sizeof(address), 'm');
    Serial.print("Generate P2WPKH address: ");
    Serial.println(result ? "PASS" : "FAIL");
    
    if (result) {
        Serial.print("Address: ");
        Serial.println(address);
    }
    
    // Also generate legacy and nested segwit
    char address_legacy[64];
    char address_nested[64];
    
    BitcoinAddress::generate_p2pkh(addr_pub.key, address_legacy, sizeof(address_legacy), 'm');
    BitcoinAddress::generate_p2sh_p2wpkh(addr_pub.key, address_nested, sizeof(address_nested), 'm');
    
    Serial.print("P2PKH (legacy): ");
    Serial.println(address_legacy);
    Serial.print("P2SH-P2WPKH (nested): ");
    Serial.println(address_nested);
    
    // Clean up
    BIP32::wipe_key(&master);
    BIP32::wipe_key(&account);
    BIP32::wipe_key(&addr_key);
    BIP39::wipe_mnemonic(mnemonic);
}

void test_bech32() {
    Serial.println("\n=== Bech32 Tests ===");
    
    // Test encoding
    uint8_t data[] = {0x00, 0x01, 0x02, 0x03, 0x04};
    char encoded[64];
    
    bool result = BitcoinAddress::bech32_encode("bc", data, sizeof(data), encoded, sizeof(encoded));
    Serial.print("Bech32 encode: ");
    Serial.println(result ? "PASS" : "FAIL");
    
    if (result) {
        Serial.print("Encoded: ");
        Serial.println(encoded);
    }
    
    // Test decoding
    char hrp[16];
    uint8_t decoded[32];
    size_t decoded_len;
    
    result = BitcoinAddress::bech32_decode(encoded, hrp, decoded, &decoded_len);
    Serial.print("Bech32 decode: ");
    Serial.println(result ? "PASS" : "FAIL");
    
    if (result) {
        Serial.print("HRP: ");
        Serial.println(hrp);
        Serial.print("Data: ");
        for (size_t i = 0; i < decoded_len; i++) {
            if (decoded[i] < 0x10) Serial.print("0");
            Serial.print(decoded[i], HEX);
        }
        Serial.println();
    }
}

void run_wallet_tests() {
    Serial.println("\n");
    Serial.println("╔══════════════════════════════════════════════════╗");
    Serial.println("║    WALLET TESTS - BIP39 + BIP32 + Addresses     ║");
    Serial.println("╚══════════════════════════════════════════════════╝");
    
    test_bip39_vectors();
    test_bip32_bip39_integration();
    test_bech32();
    
    Serial.println("\n=== Wallet Tests Complete ===");
}
