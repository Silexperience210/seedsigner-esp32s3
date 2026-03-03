#pragma once

#include <stdint.h>
#include <stddef.h>

namespace BIP39 {

// Tailles de mnemonic supportées
enum MnemonicSize {
    MNEMONIC_12_WORDS = 12,
    MNEMONIC_24_WORDS = 24
};

// Génère un mnemonic à partir d'entropie
// entropy_len: 16 bytes pour 12 mots, 32 bytes pour 24 mots
// words_out: buffer pour stocker le mnemonic (taille min: 256 bytes pour 12 mots, 512 pour 24)
bool generate_mnemonic(const uint8_t* entropy, size_t entropy_len, char* words_out, size_t out_len);

// Génère un mnemonic aléatoire (utilise le RNG hardware)
bool generate_mnemonic_random(MnemonicSize size, char* words_out, size_t out_len);

// Convertit un mnemonic en seed BIP32 (64 bytes)
// password: optionnel (peut être nullptr), utilise "mnemonic" + password comme salt
bool mnemonic_to_seed(const char* mnemonic, const char* password, uint8_t seed[64]);

// Valide un mnemonic (checksum + mots valides)
bool validate_mnemonic(const char* mnemonic);

// Récupère l'index d'un mot dans la wordlist (pour binary search)
// Retourne -1 si le mot n'existe pas
int find_word_index(const char* word);

// Efface un mnemonic de la mémoire (sécurisé)
void wipe_mnemonic(char* mnemonic);

} // namespace BIP39
