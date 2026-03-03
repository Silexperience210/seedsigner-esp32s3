# Wallet Implementation - BIP39 + BIP32 + Bitcoin Addresses

## ✅ Fonctionnalités Implémentées

### 1. BIP39 - Mnemonic Seed Phrases
- [x] Génération de mnemonics depuis entropie
- [x] Conversion mnemonic → seed (PBKDF2-HMAC-SHA512)
- [x] Validation de checksum
- [x] Support 12 et 24 mots
- [x] Wordlist BIP39 English

```cpp
#include "core/bip39.h"

// Générer depuis entropie
uint8_t entropy[16]; // 16 bytes = 12 mots, 32 = 24 mots
char mnemonic[256];
BIP39::generate_mnemonic(entropy, sizeof(entropy), mnemonic, sizeof(mnemonic));

// Valider
bool valid = BIP39::validate_mnemonic(mnemonic);

// Convertir en seed
uint8_t seed[64];
BIP39::mnemonic_to_seed(mnemonic, "password", seed);
```

### 2. BIP32 - HD Wallet
- [x] Initialisation depuis seed
- [x] Dérivation d'enfants (hardened/normal)
- [x] Dérivation de chemins complets (m/44'/0'/0'/0/0)
- [x] Sérialisation xprv/xpub (Base58Check)
- [x] Fingerprints (HASH160)

```cpp
#include "core/bip32_secp256k1.h"

// Initialiser master
BIP32::ExtendedKey master;
BIP32::init_from_seed(seed, &master);

// Dériver chemin BIP84 (native segwit)
BIP32::ExtendedKey account;
BIP32::derive_path(&master, "m/84'/0'/0'", &account);

// Obtenir xpub
BIP32::ExtendedKey pub;
BIP32::get_public_key(&account, &pub);
char xpub[113];
BIP32::serialize(&pub, true, xpub, sizeof(xpub));
```

### 3. Bitcoin Addresses
- [x] P2WPKH - Native SegWit (bech32) - bc1q...
- [x] P2SH-P2WPKH - Nested SegWit (base58) - 3...
- [x] P2PKH - Legacy (base58) - 1...
- [x] Bech32 encoding/decoding (BIP173)

```cpp
#include "core/bitcoin_address.h"

// Générer adresses depuis clé publique
uint8_t pubkey[33]; // Clé publique compressée
char addr_p2wpkh[64];
char addr_p2sh[64];
char addr_legacy[64];

BitcoinAddress::generate_p2wpkh(pubkey, addr_p2wpkh, sizeof(addr_p2wpkh), 'm');
BitcoinAddress::generate_p2sh_p2wpkh(pubkey, addr_p2sh, sizeof(addr_p2sh), 'm');
BitcoinAddress::generate_p2pkh(pubkey, addr_legacy, sizeof(addr_legacy), 'm');
```

## 📁 Fichiers

```
include/core/
├── bip39.h              # Interface BIP39
└── bitcoin_address.h    # Interface addresses

src/core/
├── bip39.cpp            # Implémentation BIP39
├── bip39_wordlist.h     # Wordlist BIP39 (2048 mots)
└── bitcoin_address.cpp  # Implémentation addresses
```

## 🔧 Architecture

```
┌─────────────────────────────────────────┐
│              APPLICATION                │
├─────────────────────────────────────────┤
│  BIP39 (mnemonic) → BIP32 (HD keys)    │
│                     ↓                   │
│              Bitcoin Address            │
│         (P2WPKH / P2SH / P2PKH)        │
├─────────────────────────────────────────┤
│         secp256k1 (uBitcoin)           │
│         SHA256 / RIPEMD160             │
│         HMAC-SHA512 / PBKDF2           │
└─────────────────────────────────────────┘
```

## 🧪 Tests

```cpp
// Exécuter tous les tests
run_wallet_tests();

// Tests individuels
test_bip39_vectors();           // Vecteurs de test BIP39
test_bip32_bip39_integration(); // Chaîne complète
test_bech32();                  // Bech32 encode/decode
```

### Résultats attendus

| Test | Entrée | Sortie |
|------|--------|--------|
| BIP39 | Entropy 0000... | abandon abandon... about |
| BIP32 | Seed | xprv... / xpub... |
| P2WPKH | Pubkey | bc1q... |
| P2SH | Pubkey | 3... |
| P2PKH | Pubkey | 1... |

## 🚀 Workflow Complet

```cpp
// 1. Générer mnemonic
uint8_t entropy[16];
// ... fill with secure random ...
char mnemonic[256];
BIP39::generate_mnemonic(entropy, sizeof(entropy), mnemonic, sizeof(mnemonic));

// 2. Convertir en seed
uint8_t seed[64];
BIP39::mnemonic_to_seed(mnemonic, "", seed);

// 3. Créer wallet HD
BIP32::ExtendedKey master, account, addr_key;
BIP32::init_from_seed(seed, &master);
BIP32::derive_path(&master, "m/84'/0'/0'/0/0", &addr_key);

// 4. Obtenir clé publique
BIP32::ExtendedKey pub;
BIP32::get_public_key(&addr_key, &pub);

// 5. Générer adresse
char address[64];
BitcoinAddress::generate_p2wpkh(pub.key, address, sizeof(address), 'm');

// 6. Cleanup sécurisé
BIP39::wipe_mnemonic(mnemonic);
BIP32::wipe_key(&master);
BIP32::wipe_key(&addr_key);
```

## ⚠️ Notes

- **Wordlist**: Actuellement 256 mots pour la démo. Production: 2048 mots complets.
- **Sécurité**: Toutes les clés sont effacées avec `wipe_key()` et `wipe_mnemonic()`
- **RNG**: Utiliser l'entropie hardware ESP32-S3 (TRNG + caméra)

## 📚 Références

- [BIP39](https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki) - Mnemonic code
- [BIP32](https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki) - HD Wallets
- [BIP44](https://github.com/bitcoin/bips/blob/master/bip-0044.mediawiki) - Multi-Account Hierarchy
- [BIP84](https://github.com/bitcoin/bips/blob/master/bip-0084.mediawiki) - Native SegWit
- [BIP173](https://github.com/bitcoin/bips/blob/master/bip-0173.mediawiki) - Bech32 Addresses
