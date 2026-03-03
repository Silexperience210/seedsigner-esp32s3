#pragma once

#include <stdint.h>
#include <stddef.h>

namespace BIP32 {

// Constantes
constexpr size_t SEED_SIZE = 64;
constexpr size_t CHAIN_CODE_SIZE = 32;
constexpr size_t KEY_SIZE = 33;  // Compressed key
constexpr size_t FINGERPRINT_SIZE = 4;

// Clé étendue (BIP32)
struct ExtendedKey {
    uint8_t key[KEY_SIZE];           // Clé privée (préfixe 0x00) ou publique (0x02/0x03)
    uint8_t chain_code[CHAIN_CODE_SIZE];
    uint8_t depth;
    uint8_t fingerprint[FINGERPRINT_SIZE];
    uint32_t child_number;
};

// Initialise une clé master à partir d'une seed BIP39
bool init_from_seed(const uint8_t seed[SEED_SIZE], ExtendedKey* out_key);

// Dérive un enfant (hardened si index >= 0x80000000)
bool derive_child(const ExtendedKey* parent, uint32_t index, ExtendedKey* out_child);

// Dérive un chemin complet (ex: "m/44'/0'/0'/0/0")
bool derive_path(const ExtendedKey* master, const char* path, ExtendedKey* out_key);

// Obtient la clé publique à partir de la clé privée
bool get_public_key(const ExtendedKey* priv_key, ExtendedKey* out_pub_key);

// Calcule le fingerprint (HASH160 des 4 premiers bytes)
uint32_t get_fingerprint(const ExtendedKey* key);

// Sérialisation xprv/xpub
bool serialize(const ExtendedKey* key, bool public_only, char* out, size_t out_len);

// Nettoie la mémoire sécurisée
void wipe_key(ExtendedKey* key);

} // namespace BIP32
