# 🔐 SeedSigner ESP32-S3 Edition

> **Hardware Wallet Bitcoin Air-Gapped, Stateless & Open Source**  
> Adaptation officielle du firmware SeedSigner vers ESP32-S3 avec support NFC NTAG 424

![Version](https://img.shields.io/badge/version-0.1.0--alpha-orange)
![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue)
![License](https://img.shields.io/badge/license-MIT-green)

---

## 🎉 PROJET COMPLET - 53 FICHIERS - PRÊT POUR HARDWARE TESTS

Ce projet est une **réplique complète et fidèle** de SeedSigner adaptée pour ESP32-S3 (M5Stack CoreS3), avec:
- 🔐 **Cryptographie complète**: BIP39, BIP32, secp256k1, PSBT
- 🎨 **UI fidèle**: Thème visuel identique au SeedSigner RPi original
- 📱 **Hardware support**: M5Stack CoreS3, caméra, NFC NTAG 424
- 🛡️ **Sécurité maximale**: Stateless, air-gap, encryption AES-128

---

## 📸 Aperçu

```
┌──────────────────────────────────┐
│  │   🟧 SeedSigner    │         │
│  ├────────────────────┤         │
│  │ [Seed]   [Sign]    │         │
│  │ [Tools]  [Settings]│         │
│  │                    │         │
│  │  Recovery Phrase   │         │
│  │  1. abandon        │         │
│  │  2. ability        │         │
│  │  ...               │         │
│  ├────────────────────┤         │
│  │   ● Seed Loaded    │         │
└──────────────────────────────────┘
   320×240 IPS Touch
```

---

## 🎯 Fonctionnalités complètes

### ✅ Cryptographie Bitcoin
- [x] **BIP39**: Génération et validation de mnemonics (12/24 mots)
- [x] **BIP32**: HD wallets avec dérivation complète m/84'/0'/0'
- [x] **secp256k1**: ECDSA sign/verify, key generation (lib intégrée)
- [x] **PSBT**: Parsing et signature de transactions
- [x] **Base58**: Sérialisation xprv/xpub

### ✅ Hardware Support
- [x] **M5Stack CoreS3**: ESP32-S3, 16MB Flash, 8MB PSRAM
- [x] **Display**: ILI9342C 2.0" IPS 320×240
- [x] **Touch**: FT6336U capacitif
- [x] **Camera**: GC0308 avec QR scan
- [x] **NFC**: NTAG 424 DNA avec AES-128 encryption
- [x] **SD Card**: Pour PSBT files

### ✅ UI/UX (10+ Screens)
- [x] **Splash**: Logo + security warning
- [x] **Main Menu**: 4 boutons (Seed/Sign/Tools/Settings)
- [x] **Seed Menu**: Create/Import/NFC options
- [x] **Virtual Keyboard**: Saisie tactile complète
- [x] **Seed Display**: 24 words avec numérotation
- [x] **Sign Scan**: Camera preview + QR frame
- [x] **Sign Review**: Transaction details + fee
- [x] **Settings**: Brightness, auto-lock, wipe
- [x] **Tools**: BIP85, verify, XPUB export

### ✅ Sécurité
- [x] **Stateless**: Seed jamais persisté (RAM uniquement)
- [x] **Air-gap**: WiFi/BT hard-disabled au boot
- [x] **Secure Memory**: Multi-pass wipe avec memory barriers
- [x] **NFC Encryption**: AES-128 + SUN authentication
- [x] **Hardware RNG**: Entropy ESP32-S3 certifié

---

## 🔧 Hardware Recommandé

### Kit Complet (~$70)

| Composant | Référence | Prix | Status |
|-----------|-----------|------|--------|
| **M5Stack CoreS3** | K128 | $43.90 | ✅ Requis |
| **M5Stack Unit NFC** | U001-C | $14.90 | Optionnel |
| **NTAG 424 DNA** | (x5 pack) | $8 | Optionnel |
| **MicroSD 16GB** | Class 10 | $5 | Recommandé |

**Where to buy:**
- [M5Stack Official Store](https://shop.m5stack.com)
- [GoToTags (NTAG 424)](https://store.gototags.com)

---

## 🚀 Installation rapide

### Prérequis
- [PlatformIO Core](https://platformio.org/install) ou VS Code + PlatformIO extension
- Python 3.8+
- Câble USB-C

### Cloner et Build

```bash
# 1. Cloner le repository
git clone https://github.com/yourusername/SeedSigner-ESP32S3.git
cd SeedSigner-ESP32S3

# 2. Installer les dépendances PlatformIO
pio pkg install

# 3. Build pour M5Stack CoreS3
pio run -e m5stack-cores3

# 4. Flasher (connecter le CoreS3 en USB)
pio run -e m5stack-cores3 --target upload

# 5. Monitor (optionnel)
pio device monitor -b 115200
```

### Environnements supportés

| Environnement | Board | Commande |
|---------------|-------|----------|
| `m5stack-cores3` | M5Stack CoreS3 | `pio run -e m5stack-cores3` |
| `m5stack-cores3-debug` | CoreS3 + debug | `pio run -e m5stack-cores3-debug` |
| `lilygo-tdisplay-s3-pro` | LilyGo T-Display S3 Pro | `pio run -e lilygo-tdisplay-s3-pro` |
| `native_test` | Tests unitaires | `pio test -e native_test` |

---

## 📁 Structure du projet (53 fichiers)

```
SeedSigner-ESP32S3/
├── lib/secp256k1/              # ✅ Librairie crypto complète
│   ├── secp256k1.h             # Header officiel
│   └── secp256k1.c             # Implémentation ECDSA
│
├── src/
│   ├── core/                   # ✅ Bitcoin cryptography
│   │   ├── bip39.cpp           # BIP39 implementation
│   │   ├── bip32.cpp           # BIP32 HD wallets
│   │   ├── bip32_secp256k1.cpp # BIP32 + secp256k1
│   │   ├── secp256k1_wrapper.cpp
│   │   ├── psbt.cpp            # PSBT signing
│   │   ├── entropy.cpp         # Entropy sources
│   │   └── wordlist_en.h       # BIP39 wordlist
│   │
│   ├── ui/                     # ✅ Interface utilisateur
│   │   ├── app.cpp             # Application core
│   │   ├── app_screens.cpp     # Screen integration
│   │   ├── seedsigner_theme.cpp# ✅ Thème fidèle SeedSigner
│   │   ├── screens/            # Screens originaux
│   │   │   ├── screen_main.cpp
│   │   │   ├── screen_seed.cpp
│   │   │   ├── screen_sign.cpp
│   │   │   └── screen_settings.cpp
│   │   └── screens_replica/    # ✅ Répliques fidèles RPi
│   │       └── screen_main_replica.cpp
│   │
│   ├── drivers/                # ✅ Hardware drivers
│   │   ├── display.cpp         # ILI9342C driver
│   │   ├── camera.cpp          # GC0308 driver
│   │   ├── nfc.cpp             # NTAG 424 DNA driver
│   │   ├── touch.cpp           # FT6336U driver
│   │   └── sd.cpp              # SD card driver
│   │
│   ├── utils/                  # ✅ Utilities
│   │   ├── memory.cpp          # Secure memory management
│   │   └── qr_code.cpp         # QR generation/scanning
│   │
│   ├── main.cpp                # Entry point (sécurisé)
│   ├── test_crypto.cpp         # Tests unitaires crypto
│   ├── test_camera.cpp         # Tests caméra
│   └── demo_full.cpp           # Demo workflow complet
│
├── include/                    # Headers
├── docs/                       # Documentation complète
├── enclosures/                 # Designs 3D
└── tests/                      # Tests
```

---

## 🔐 Architecture de Sécurité

```
┌─────────────────────────────────────────────────────────────┐
│                    SECURITÉ MAXIMALE                        │
├─────────────────────────────────────────────────────────────┤
│  🛡️ AMNÉSIE (STATELESS)                                     │
│     → Seed uniquement en RAM (PSRAM chiffrée possible)     │
│     → Aucune persistence sur flash                         │
│     → Wipe automatique au shutdown/débranchement           │
│                                                             │
│  🔒 AIR-GAPPED                                              │
│     → WiFi/BT désactivés hard au boot                      │
│     → Communication uniquement via QR codes                │
│     → NFC uniquement avec authentification                 │
│                                                             │
│  🔐 ENTROPIE CONTRÔLÉE                                      │
│     → RNG hardware ESP32-S3 certifié                       │
│     → Entropie caméra (bruit capteur)                      │
│     → Dés physiques (D6/D20)                               │
│                                                             │
│  📱 NFC NTAG 424 SECURISÉ                                   │
│     → Stockage encrypté AES-128                            │
│     → SUN messages (Secure Unique NFC)                     │
│     → Clés dérivées du PIN utilisateur                     │
└─────────────────────────────────────────────────────────────┘
```

---

## 📱 Workflow utilisateur

```
1. BOOT → Splash Screen + Security Warning
2. MAIN MENU → [Seed]
3. SEED MENU → [Create New]
4. GENERATE → 24 Words (Hardware RNG + Camera)
5. DISPLAY → Write down recovery phrase
6. NFC → Save to NTAG 424 (optional)
7. SIGN → [Scan PSBT]
8. SCAN → QR code from wallet
9. REVIEW → Verify amount/fee
10. SIGN → Create signature
11. EXPORT → QR code signed PSBT
```

---

## 🔐 Spécifications NFC NTAG 424

| Feature | Description |
|---------|-------------|
| **Encryption** | AES-128 |
| **Authentication** | SUN messages (unique par lecture) |
| **Counter** | Protection anti-replay |
| **Format** | `SS44` (SeedSigner NTAG 424) |
| **PIN** | 4-8 digits pour déverrouillage |

---

## 🧪 Tests

```bash
# Tests unitaires (crypto, memory)
pio test -e native_test

# Tests hardware (caméra, display)
pio run -e m5stack-cores3-debug
# Puis dans le monitor: appeler test functions

# Demo complet
# Dans le monitor: appeler demo_full_workflow()
```

---

## 🗺️ Roadmap

### ✅ Phase 1 - Core (COMPLÈTE)
- [x] Setup projet PlatformIO
- [x] Libsecp256k1 portée
- [x] BIP39/BIP32 complète
- [x] UI LVGL avec thème fidèle
- [x] Camera QR scan/display

### 📋 Phase 2 - Hardware Tests
- [ ] Tests sur M5Stack CoreS3 réel
- [ ] Calibration caméra QR
- [ ] Tests NFC NTAG 424
- [ ] Optimisation batterie

### 📋 Phase 3 - Production
- [ ] Audit sécurité externe
- [ ] Documentation utilisateur
- [ ] Enclosure design 3D
- [ ] Release v1.0

---

## 🤝 Contribuer

Les contributions sont les bienvenues !

1. Fork le repo
2. Créer une branche (`git checkout -b feature/amazing`)
3. Commit (`git commit -m 'Add amazing feature'`)
4. Push (`git push origin feature/amazing`)
5. Pull Request

### Priorités
- Tests hardware et feedback
- Optimisations performance
- Traductions UI

---

## 📜 License

MIT License - Voir [LICENSE](LICENSE)

SeedSigner original : Copyright (c) 2020-2024 SeedSigner  
Cette adaptation : Copyright (c) 2024 SeedSigner-ESP32S3 Contributors

---

## 🙏 Remerciements

- [SeedSigner](https://seedsigner.com/) - Le projet original
- [M5Stack](https://m5stack.com/) - Hardware excellent
- [NXP](https://www.nxp.com/) - NTAG 424 DNA
- Communauté Bitcoin open source

---

**⚠️ Avertissement**: Ce projet est en développement actif. Tester uniquement avec des petits montants avant usage en production.

**🔒 Security First**: Ce hardware wallet est conçu pour être air-gapped et stateless. Ne jamais activer le WiFi/BT et toujours vérifier les adresses sur l'écran avant signature.

---

*Built with 🔐 and 🧡 for the Bitcoin community*
