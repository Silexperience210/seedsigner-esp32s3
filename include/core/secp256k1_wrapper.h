#pragma once

#include <stdint.h>
#include <stddef.h>

namespace Secp256k1 {

// Constantes
constexpr size_t PRIVATE_KEY_SIZE = 32;
constexpr size_t PUBLIC_KEY_COMPRESSED_SIZE = 33;
constexpr size_t PUBLIC_KEY_UNCOMPRESSED_SIZE = 65;
constexpr size_t SIGNATURE_SIZE = 64;

// Génère une clé publique à partir d'une clé privée
bool generate_public_key(const uint8_t privkey[PRIVATE_KEY_SIZE], 
                         uint8_t pubkey[PUBLIC_KEY_COMPRESSED_SIZE], 
                         bool compressed = true);

// Génère une clé publique non compressée
bool generate_public_key_uncompressed(const uint8_t privkey[PRIVATE_KEY_SIZE],
                                      uint8_t pubkey[PUBLIC_KEY_UNCOMPRESSED_SIZE]);

// Sign un message (ECDSA)
bool sign(const uint8_t privkey[PRIVATE_KEY_SIZE],
          const uint8_t hash[32],
          uint8_t signature[SIGNATURE_SIZE],
          uint8_t* recid = nullptr);

// Vérifie une signature
bool verify(const uint8_t pubkey[PUBLIC_KEY_COMPRESSED_SIZE],
            const uint8_t hash[32],
            const uint8_t signature[SIGNATURE_SIZE]);

// Tweak add (pour BIP32)
bool seckey_tweak_add(uint8_t seckey[PRIVATE_KEY_SIZE], const uint8_t tweak[32]);
bool pubkey_tweak_add(uint8_t pubkey[PUBLIC_KEY_COMPRESSED_SIZE], const uint8_t tweak[32]);

// Vérifie si une clé privée est valide
bool is_valid_seckey(const uint8_t seckey[PRIVATE_KEY_SIZE]);

// HASH160: RIPEMD160(SHA256(data))
void hash160(const uint8_t* data, size_t len, uint8_t hash160_out[20]);

} // namespace Secp256k1
