# uBitcoin Integration - secp256k1 Fix

## ✅ Remplacement du Placeholder secp256k1

L'ancienne implémentation `lib/secp256k1/` était un **placeholder** qui copiait simplement les données sans effectuer les opérations cryptographiques réelles.

### Solution: uBitcoin Library

**uBitcoin** est une bibliothèque Bitcoin légère pour microcontrôleurs, maintenue par @micro-bitcoin.

- **Repo**: https://github.com/micro-bitcoin/uBitcoin
- **Features**: secp256k1, BIP32, BIP39, transactions Bitcoin
- **Taille**: ~30KB flash, ~2KB RAM
- **License**: MIT

## 📁 Fichiers Créés

### Wrapper secp256k1
```
include/core/secp256k1_wrapper.h      # Interface C++
src/core/secp256k1_wrapper.cpp        # Wrapper uBitcoin
```

### Crypto Primitives
```
src/core/sha256.cpp                   # SHA256
src/core/hmac_sha512.cpp              # HMAC-SHA512 pour BIP32
src/core/ripemd160.cpp                # RIPEMD160 pour addresses
```

### BIP32 HD Wallet
```
include/core/bip32_secp256k1.h        # Interface BIP32
src/core/bip32_secp256k1.cpp          # Implémentation complète
```

### Tests
```
src/test_crypto.cpp                   # Vecteurs de test BIP32
```

## 🔧 Configuration platformio.ini

```ini
lib_deps = 
    lvgl/lvgl@^9.2.0
    https://github.com/micro-bitcoin/uBitcoin.git  ; uBitcoin
```

## 🎯 API Secp256k1

```cpp
#include "core/secp256k1_wrapper.h"

// Générer clé publique
uint8_t privkey[32] = {...};
uint8_t pubkey[33];
Secp256k1::generate_public_key(privkey, pubkey, true);

// Signer
uint8_t hash[32] = {...};
uint8_t signature[64];
Secp256k1::sign(privkey, hash, signature);

// Vérifier
bool valid = Secp256k1::verify(pubkey, hash, signature);

// Tweak add (BIP32)
Secp256k1::seckey_tweak_add(privkey, tweak);
Secp256k1::pubkey_tweak_add(pubkey, tweak);

// HASH160 (RIPEMD160(SHA256(data)))
uint8_t hash160[20];
Secp256k1::hash160(data, len, hash160);
```

## 🎯 API BIP32

```cpp
#include "core/bip32_secp256k1.h"

// Initialiser depuis seed BIP39
BIP32::ExtendedKey master;
BIP32::init_from_seed(seed, &master);

// Dériver chemin
BIP32::ExtendedKey child;
BIP32::derive_path(&master, "m/44'/0'/0'/0/0", &child);

// Obtenir clé publique
BIP32::ExtendedKey pub;
BIP32::get_public_key(&child, &pub);

// Sérialiser
char xprv[113], xpub[113];
BIP32::serialize(&child, false, xprv, sizeof(xprv));
BIP32::serialize(&pub, true, xpub, sizeof(xpub));

// Nettoyer sécurisé
BIP32::wipe_key(&master);
```

## ✅ Tests

Les tests utilisent les vecteurs officiels BIP32:

```cpp
void run_crypto_tests() {
    test_secp256k1_basic();   // Génération + signature
    test_hash160();           // RIPEMD160(SHA256())
    test_bip32();             // Dérivation HD complète
}
```

### Résultats Attendus

| Test | Description |
|------|-------------|
| secp256k1 | Clé publique: `0279BE667E...` |
| HASH160 | `91b24bf9f5288532960ac687...` |
| BIP32 | xprv/xpub sérialisés correctement |

## 🔒 Sécurité

- **Mémoire**: Toutes les clés privées sont effacées avec `wipe_key()`
- **Hardened**: Support dérivation hardened (`0x80000000`)
- **RNG**: Utilise ESP32-S3 HMAC-DRBG + noise caméra

## 🚀 Prochaines Étapes

1. [ ] Compiler et flasher sur M5Stack CoreS3
2. [ ] Exécuter les tests crypto
3. [ ] Intégrer avec BIP39 (mnemonic)
4. [ ] Implémenter génération d'adresses
5. [ ] PSBT signing
