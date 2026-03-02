/**
 * @file bip39.h
 * BIP39 Mnemonic phrase generation and conversion
 */

#ifndef CORE_BIP39_H
#define CORE_BIP39_H

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

namespace SeedSigner {
namespace Core {

enum class Language { ENGLISH = 0 };
enum class SeedStrength { STRENGTH_128 = 128, STRENGTH_256 = 256 };

class BIP39 {
public:
    BIP39();
    ~BIP39();
    
    bool init();
    bool generate_mnemonic_from_entropy(const uint8_t* entropy, size_t entropy_len, 
                                        char* output, size_t output_len);
    bool generate_mnemonic_from_dice(const uint8_t* rolls, size_t num_rolls,
                                     char* output, size_t output_len);
    bool mnemonic_to_seed(const char* mnemonic, const char* passphrase, uint8_t seed[64]);
    bool validate_mnemonic(const char* mnemonic);
    int word_to_index(const char* word);
    const char* index_to_word(uint16_t index);
    uint16_t calculate_checksum_word(const uint8_t* entropy, size_t entropy_len);
    static size_t entropy_to_word_count(size_t entropy_len);
    bool self_test();
    
private:
    static const char* s_wordlist[2048];
    int find_word(const char* word);
    bool pbkdf2_hmac_sha512(const uint8_t* password, size_t password_len,
                            const uint8_t* salt, size_t salt_len,
                            uint32_t iterations, uint8_t* output, size_t output_len);
};

} // namespace Core
} // namespace SeedSigner

#endif // CORE_BIP39_H
