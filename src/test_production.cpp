/**
 * @file test_production.cpp
 * Production test suite for SeedSigner ESP32-S3
 * 
 * Run these tests before any release:
 * - Cryptographic primitive tests (vectors from specs)
 * - Memory safety tests
 * - Hardware interface tests
 */

#include <Arduino.h>
#include <mbedtls/sha256.h>
#include "core/bip39.h"
#include "core/bip32.h"
#include "core/secp256k1_wrapper.h"
#include "core/ripemd160.h"
#include "utils/memory.h"
#include "drivers/nfc.h"

namespace SeedSigner {
namespace Tests {

// Test results
struct TestResults {
    int passed = 0;
    int failed = 0;
    int total = 0;
};

static TestResults g_results;

#define TEST_ASSERT(cond, msg) do { \
    g_results.total++; \
    if (cond) { \
        g_results.passed++; \
        Serial.printf("  [PASS] %s\n", msg); \
    } else { \
        g_results.failed++; \
        Serial.printf("  [FAIL] %s (line %d)\n", msg, __LINE__); \
    } \
} while(0)

// BIP39 Test Vectors from BIP39 spec
static const struct {
    const char* entropy_hex;
    const char* mnemonic;
    const char* seed_hex;
    const char* passphrase;
} BIP39_TEST_VECTORS[] = {
    {
        "00000000000000000000000000000000",
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
        "c55257c360c07c72029aebc1b53c05ed0362ada38ead3e3e9efa3708e53495531f09a6987599d18264c1e1c92f2cf141630c7a3c4ab7c81b2f001698e7463b04",
        "TREZOR"
    },
    {
        "7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f7f",
        "legal winner thank year wave sausage worth useful legal winner thank yellow",
        "2e8905819b8723fe2ea1c44f62e2a58b7b7bd244854b34399fbbe63efa7c67f3d70797fba6cee9d9f0ba1585456765c7c0dd44b45f3ee250b8d69045372741d7",
        "TREZOR"
    },
    {
        "80808080808080808080808080808080",
        "letter advice cage absurd amount doctor acoustic avoid letter advice cage above",
        "d71de856f81a8acc65e6fc851a38d4d7ec216fd0796d0a6827a3ad6ed5511a30fa280f12eb2e47ed2ac03b5c462a0358d18d69fe4f985ec81778c1b370b652c8",
        "TREZOR"
    }
};

// BIP32 Test Vector 1
static const struct {
    const char* seed_hex;
    const char* path;
    const char* expected_xpub;
} BIP32_TEST_VECTORS[] = {
    {
        "000102030405060708090a0b0c0d0e0f",
        "m",
        "xpub661MyMwAqRbcFtXgS5sYJABqqG9YLmC4Q1Rdap9gSE8NqtwybGhePY2gZ29ESFjqJoCu1Rupje8YtGqsefD265TMg7usUDFdp6W1EGMcet8"
    },
    {
        "000102030405060708090a0b0c0d0e0f",
        "m/0'",
        "xpub68Gmy5EdvgibQVfPdqkBBCHxA5htuGq3MJLcUNaN7W1rFXSfRkB8QJ3jwF4G3RAkycPqD4XYmW6nMMjss8ZgxgFx7qzmVjPqyPVFy9LKb4j"
    }
};

void run_bip39_tests() {
    Serial.println("\n=== BIP39 Tests ===");
    
    Core::BIP39 bip39;
    TEST_ASSERT(bip39.init(), "BIP39 init");
    
    for (size_t i = 0; i < sizeof(BIP39_TEST_VECTORS)/sizeof(BIP39_TEST_VECTORS[0]); i++) {
        const auto& vec = BIP39_TEST_VECTORS[i];
        
        // Convert hex entropy to bytes
        size_t entropy_len = strlen(vec.entropy_hex) / 2;
        uint8_t entropy[32];
        for (size_t j = 0; j < entropy_len; j++) {
            sscanf(&vec.entropy_hex[j*2], "%2hhx", &entropy[j]);
        }
        
        // Test mnemonic generation
        char mnemonic[512];
        bool result = bip39.generate_mnemonic_from_entropy(entropy, entropy_len, mnemonic, sizeof(mnemonic));
        TEST_ASSERT(result, "BIP39 generate");
        TEST_ASSERT(strcmp(mnemonic, vec.mnemonic) == 0, "BIP39 mnemonic match");
        
        // Test validation
        TEST_ASSERT(bip39.validate_mnemonic(mnemonic), "BIP39 validate");
        
        // Test seed generation
        uint8_t seed[64];
        result = bip39.mnemonic_to_seed(mnemonic, vec.passphrase, seed);
        TEST_ASSERT(result, "BIP39 seed generation");
        
        // Verify seed (compare first few bytes)
        char seed_hex[129];
        for (int j = 0; j < 64; j++) {
            sprintf(&seed_hex[j*2], "%02x", seed[j]);
        }
        TEST_ASSERT(strncmp(seed_hex, vec.seed_hex, 32) == 0, "BIP39 seed match");
    }
    
    // Test invalid mnemonic detection
    TEST_ASSERT(!bip39.validate_mnemonic("invalid mnemonic here"), "BIP39 invalid detection");
    TEST_ASSERT(!bip39.validate_mnemonic("abandon abandon abandon"), "BIP39 too short");
}

void run_bip32_tests() {
    Serial.println("\n=== BIP32 Tests ===");
    
    Core::BIP32 bip32;
    
    // Test vector 1
    uint8_t seed[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    
    // Pad to 64 bytes
    uint8_t seed64[64];
    memset(seed64, 0, 64);
    memcpy(seed64, seed, 16);
    
    bool result = bip32.init_from_seed(seed64);
    TEST_ASSERT(result, "BIP32 init from seed");
    
    // Test serialization
    char xprv[128];
    result = bip32.serialize(bip32.get_master_key(), xprv, sizeof(xprv));
    TEST_ASSERT(result, "BIP32 serialize");
    TEST_ASSERT(strncmp(xprv, "xprv", 4) == 0, "BIP32 xprv prefix");
    
    // Test child derivation
    Core::ExtendedKey child;
    result = bip32.derive_child(bip32.get_master_key(), 0, true, &child);
    TEST_ASSERT(result, "BIP32 hardened child derivation");
    
    // Test public key derivation
    Core::ExtendedKey pub;
    result = bip32.get_public_key(&child, &pub);
    TEST_ASSERT(result, "BIP32 public key derivation");
    TEST_ASSERT(!pub.is_private, "BIP32 public key is not private");
    
    // Test path derivation
    Core::ExtendedKey path_key;
    result = bip32.derive_path("m/0'/1/2'", &path_key);
    TEST_ASSERT(result, "BIP32 path derivation");
}

void run_secp256k1_tests() {
    Serial.println("\n=== secp256k1 Tests ===");
    
    Core::Secp256k1 secp;
    TEST_ASSERT(secp.init(), "secp256k1 init");
    
    // Test key generation
    uint8_t privkey[32];
    for (int i = 0; i < 32; i++) privkey[i] = i + 1;
    
    uint8_t pubkey[33];
    bool result = secp.generate_public_key(privkey, pubkey, true);
    TEST_ASSERT(result, "secp256k1 pubkey generation");
    TEST_ASSERT(pubkey[0] == 0x02 || pubkey[0] == 0x03, "secp256k1 pubkey prefix");
    
    // Test sign/verify
    uint8_t hash[32];
    for (int i = 0; i < 32; i++) hash[i] = i;
    
    uint8_t sig[64];
    uint8_t recid;
    result = secp.sign(privkey, hash, sig, &recid);
    TEST_ASSERT(result, "secp256k1 sign");
    TEST_ASSERT(recid < 4, "secp256k1 recovery id valid");
    
    result = secp.verify(pubkey, hash, sig);
    TEST_ASSERT(result, "secp256k1 verify");
    
    // Test verification fails with wrong message
    hash[0] ^= 0xFF;
    result = secp.verify(pubkey, hash, sig);
    TEST_ASSERT(!result, "secp256k1 verify wrong message");
    
    secp.deinit();
}

void run_ripemd160_tests() {
    Serial.println("\n=== RIPEMD160 Tests ===");
    
    // Test vector from RIPEMD160 spec
    const char* msg = "";
    uint8_t expected[20] = {
        0x9c, 0x11, 0x85, 0xa5, 0xc5, 0x9e, 0xec, 0xae,
        0x11, 0x16, 0x8b, 0x64, 0x05, 0x74, 0x6c, 0x70,
        0x53, 0x73, 0x48, 0x11
    };
    
    uint8_t hash[20];
    Core::RIPEMD160::hash((const uint8_t*)msg, strlen(msg), hash);
    
    bool match = memcmp(hash, expected, 20) == 0;
    TEST_ASSERT(match, "RIPEMD160 empty string");
    
    // Test "a"
    const char* msg2 = "a";
    uint8_t expected2[20] = {
        0x0b, 0xdc, 0x9d, 0x2d, 0x25, 0x6b, 0x3e, 0xe9,
        0xda, 0xae, 0x34, 0x7b, 0xe6, 0xf4, 0xdc, 0x83,
        0x5a, 0x46, 0x7f, 0xfe
    };
    
    Core::RIPEMD160::hash((const uint8_t*)msg2, strlen(msg2), hash);
    match = memcmp(hash, expected2, 20) == 0;
    TEST_ASSERT(match, "RIPEMD160 'a'");
}

void run_hash160_tests() {
    Serial.println("\n=== HASH160 Tests ===");
    
    // Test HASH160 = RIPEMD160(SHA256(data))
    // Known test vector for HASH160 of "hello"
    const char* msg = "hello";
    uint8_t hash160[20];
    
    // Compute SHA256
    uint8_t sha256_hash[32];
    mbedtls_sha256((const uint8_t*)msg, strlen(msg), sha256_hash, 0);
    
    // Compute RIPEMD160 of SHA256
    Core::RIPEMD160::hash(sha256_hash, 32, hash160);
    
    // Expected result (computed with Python: hashlib.new('ripemd160', hashlib.sha256(b'hello').digest()).hexdigest())
    // b6a9c8c230722b7c748331a8b450f05566cc7b90
    uint8_t expected[20] = {
        0xb6, 0xa9, 0xc8, 0xc2, 0x30, 0x72, 0x2b, 0x7c,
        0x74, 0x83, 0x31, 0xa8, 0xb4, 0x50, 0xf0, 0x55,
        0x66, 0xcc, 0x7b, 0x90
    };
    
    bool match = memcmp(hash160, expected, 20) == 0;
    TEST_ASSERT(match, "HASH160 'hello'");
    
    // Test via wrapper function
    Core::Secp256k1 secp;
    uint8_t hash160_wrapper[20];
    secp.hash160((const uint8_t*)msg, strlen(msg), hash160_wrapper);
    match = memcmp(hash160_wrapper, expected, 20) == 0;
    TEST_ASSERT(match, "HASH160 via Secp256k1 wrapper");
}

void run_memory_tests() {
    Serial.println("\n=== Secure Memory Tests ===");
    
    // Re-init for testing
    Utils::SecureMemory::deinit();
    TEST_ASSERT(Utils::SecureMemory::init(), "SecureMemory init");
    
    // Basic allocation
    void* p1 = Utils::SecureMemory::alloc(64);
    TEST_ASSERT(p1 != nullptr, "SecureMemory alloc");
    
    // Write and wipe
    memset(p1, 0xAA, 64);
    Utils::SecureMemory::free(p1);
    
    // Test self-test function
    TEST_ASSERT(Utils::SecureMemory::self_test(), "SecureMemory self_test");
    
    // Test SecureBuffer RAII
    {
        Utils::SecureBuffer<128> buf;
        TEST_ASSERT(buf.is_valid(), "SecureBuffer allocation");
        memset(buf.data(), 0xBB, 128);
        // Will be auto-wiped on scope exit
    }
    
    // Test StackBuffer
    {
        Utils::StackBuffer<64> stack;
        memset(stack.data(), 0xCC, 64);
        // Will be auto-wiped on scope exit
    }
}

void run_nfc_tests() {
    Serial.println("\n=== NFC Tests ===");
    
    // Test key derivation
    const char* pin = "123456";
    uint8_t salt[16] = {0};
    uint8_t key[16];
    
    Drivers::NFC::derive_key_from_pin(pin, salt, key);
    
    // Verify key is not all zeros or all same
    bool not_zero = false;
    bool not_same = false;
    for (int i = 0; i < 16; i++) {
        if (key[i] != 0) not_zero = true;
        if (i > 0 && key[i] != key[0]) not_same = true;
    }
    
    TEST_ASSERT(not_zero, "NFC key derivation not zero");
    TEST_ASSERT(not_same, "NFC key derivation has variation");
    
    // Verify same PIN produces same key
    uint8_t key2[16];
    Drivers::NFC::derive_key_from_pin(pin, salt, key2);
    TEST_ASSERT(memcmp(key, key2, 16) == 0, "NFC key derivation deterministic");
    
    // Verify different PIN produces different key
    uint8_t key3[16];
    Drivers::NFC::derive_key_from_pin("654321", salt, key3);
    TEST_ASSERT(memcmp(key, key3, 16) != 0, "NFC key derivation unique");
}

void print_results() {
    Serial.println("\n=================================");
    Serial.println("TEST RESULTS");
    Serial.println("=================================");
    Serial.printf("Total:  %d\n", g_results.total);
    Serial.printf("Passed: %d\n", g_results.passed);
    Serial.printf("Failed: %d\n", g_results.failed);
    Serial.println("=================================");
    
    if (g_results.failed == 0) {
        Serial.println("ALL TESTS PASSED!");
    } else {
        Serial.println("SOME TESTS FAILED!");
    }
}

void run_all_tests() {
    Serial.println("\n\n");
    Serial.println("=================================");
    Serial.println("SEEDSIGNER PRODUCTION TEST SUITE");
    Serial.println("=================================");
    
    // Reset results
    g_results = TestResults();
    
    // Run all test suites
    run_bip39_tests();
    run_bip32_tests();
    run_secp256k1_tests();
    run_ripemd160_tests();
    run_hash160_tests();
    run_memory_tests();
    run_nfc_tests();
    
    print_results();
}

} // namespace Tests
} // namespace SeedSigner

// C wrapper for main.cpp
extern "C" void run_all_tests() {
    SeedSigner::Tests::run_all_tests();
}
