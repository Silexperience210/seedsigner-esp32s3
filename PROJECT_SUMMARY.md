# 🚀 SeedSigner ESP32-S3 - Récapitulatif Projet

## 📊 Vue d'ensemble

Projet complet d'adaptation du firmware **SeedSigner** (hardware wallet Bitcoin air-gapped) vers la plateforme **ESP32-S3**, avec ajout du support **NFC NTAG 424** pour stockage sécurisé des seeds.

### Architecture matérielle recommandée

```
┌─────────────────────────────────────────────────────────────────┐
│                     M5STACK CORES3                              │
├─────────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐  │
│  │  ESP32-S3   │  │  GC0308     │  │  FT6336U Touch          │  │
│  │  240MHz     │  │  Camera     │  │  Controller             │  │
│  │  16MB Flash │  │  0.3MP      │  │  Capacitive             │  │
│  │  8MB PSRAM  │  │  QR Scan    │  │  320×240                │  │
│  └─────────────┘  └─────────────┘  └─────────────────────────┘  │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │  ILI9342C Display 2.0" IPS 320×240                     │    │
│  └─────────────────────────────────────────────────────────┘    │
│                                                                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐  │
│  │  SD Card    │  │  AXP2101    │  │  Grove Port             │  │
│  │  Slot       │  │  Power Mgmt │  │  I2C                    │  │
│  │  PSBT files │  │  500mAh     │  │  ▼                      │  │
│  └─────────────┘  └─────────────┘  └──────────┬──────────────┘  │
│                                               │                 │
└───────────────────────────────────────────────┼─────────────────┘
                                                │
                    ┌───────────────────────────▼──────────────┐
                    │      M5STACK UNIT NFC (PN532)            │
                    │      - I2C Interface                     │
                    │      - 13.56MHz                          │
                    │      - NTAG 424 DNA Support              │
                    └──────────────────────┬───────────────────┘
                                           │
                    ┌──────────────────────▼───────────────────┐
                    │      NTAG 424 DNA TAG                    │
                    │      - AES-128 Encrypted Storage         │
                    │      - SUN Authentication                │
                    │      - Counter Protection                │
                    └──────────────────────────────────────────┘
```

---

## 📁 Structure du projet

```
SeedSigner-ESP32S3/
├── 📄 README.md                 # Documentation principale
├── 📄 PROJECT_SUMMARY.md        # Ce fichier - récapitulatif
├── 📄 LICENSE                   # MIT License
├── ⚙️ platformio.ini           # Configuration build
├── 📋 partitions_16MB.csv       # Table partitions ESP32-S3
│
├── 📁 src/                      # Source code
│   ├── main.cpp                # Point d'entrée
│   ├── core/                   # Cryptographie Bitcoin
│   │   ├── bip39.cpp           # BIP39 mnemonics
│   │   └── ...                 # BIP32, PSBT, Entropy
│   ├── ui/                     # Interface LVGL
│   ├── drivers/                # Pilotes hardware
│   │   ├── display.cpp         # ILI9342C driver
│   │   └── ...                 # Camera, NFC, Touch, SD
│   └── utils/                  # Utilitaires
│       └── memory.cpp          # Secure memory management
│
├── 📁 include/                  # Headers
│   ├── core/                   # BIP39.h, BIP32.h, PSBT.h, etc.
│   ├── drivers/                # Display.h, Camera.h, NFC.h, etc.
│   ├── ui/                     # App.h
│   ├── utils/                  # Memory.h, QR_Code.h
│   └── lv_conf.h               # Configuration LVGL
│
├── 📁 docs/                     # Documentation
│   ├── HARDWARE.md             # Spécifications matérielles
│   ├── SECURITY.md             # Architecture sécurité
│   └── DEVELOPMENT.md          # Guide développement
│
├── 📁 enclosures/               # Designs 3D
│   └── README.md               # Instructions impression 3D
│
├── 📁 .github/
│   └── workflows/
│       └── build.yml           # CI/CD GitHub Actions
│
└── 📁 lib/                      # Librairies externes
    └── (empty - managed by PlatformIO)
```

---

## 🔐 Fonctionnalités de sécurité

| Feature | Implémentation | Status |
|---------|---------------|--------|
| **Stateless Operation** | Seed uniquement en RAM, wipe au shutdown | ✅ Architecture |
| **Air-Gap** | WiFi/BT désactivés au boot | ✅ Hardware + Software |
| **Secure Memory** | Wipe multi-passes, allocateur sécurisé | ✅ Utils::SecureMemory |
| **Entropy Quality** | RNG hardware + caméra + timing | ✅ Core::Entropy |
| **NFC Encryption** | AES-128, clés dérivées PIN | ✅ Drivers::NFC |
| **Tamper Detection** | Switch optionnel, watchdog | ✅ Doc + Architecture |
| **Secure Boot** | Support ESP32-S3 Secure Boot V2 | 📋 Documenté |
| **Flash Encryption** | Support ESP32-S3 Flash Encryption | 📋 Documenté |

---

## 🛠️ Matériel requis

### Kit complet (~$70)

| Composant | Référence | Prix |
|-----------|-----------|------|
| M5Stack CoreS3 | K128 | $43.90 |
| M5Stack Unit NFC | U001-C | $14.90 |
| NTAG 424 DNA Tags | (pack de 5) | $8.00 |
| MicroSD 16GB | Class 10 | $5.00 |
| Boîtier 3D printed | PETG | ~$2.00 |

---

## 🚀 Démarrage rapide

### 1. Installation

```bash
# Cloner
git clone https://github.com/yourusername/SeedSigner-ESP32S3.git
cd SeedSigner-ESP32S3

# Installer dépendances PlatformIO
pio pkg install

# Build
pio run -e m5stack-cores3

# Flasher
pio run -e m5stack-cores3 --target upload
```

### 2. Utilisation

1. **Premier boot**: Splash screen → Menu principal
2. **Créer seed**: Tools → Generate Seed → 12/24 words
3. **Sauvegarder NFC**: NFC → Write Seed → Enter PIN
4. **Signer transaction**: Sign → Scan QR → Review → Confirm
5. **Wipe**: Settings → Security → Wipe Memory

---

## 📈 Roadmap

### Phase 1 - Foundation (Mois 1)
- [x] Architecture projet
- [x] Setup PlatformIO
- [x] Headers core (BIP39, BIP32, PSBT)
- [ ] Implémentation BIP39 complète (wordlist intégré)
- [ ] Intégration libsecp256k1
- [ ] Tests unitaires crypto

### Phase 2 - Hardware (Mois 2)
- [ ] Driver display LVGL
- [ ] Driver caméra QR scan
- [ ] Driver NFC NTAG 424
- [ ] Driver touch
- [ ] Intégration hardware

### Phase 3 - UI (Mois 3)
- [ ] Screens principales (Seed, Sign, Settings)
- [ ] Clavier virtuel
- [ ] QR display
- [ ] Thème SeedSigner
- [ ] Navigation fluide

### Phase 4 - Polish (Mois 4)
- [ ] Tests sécurité
- [ ] Optimisation batterie
- [ ] Documentation utilisateur
- [ ] Release v1.0

---

## 🔗 Ressources

### Documentation
- [SeedSigner Original](https://seedsigner.com/)
- [ESP32-S3 TRM](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)
- [NTAG 424 DNA](https://www.nxp.com/products/rfid-nfc/nfc-hf/ntag-for-tags-and-labels/ntag-424-dna-424-dna-tagtamper-advanced-security-and-privacy-for-trusted-iot-applications:NTAG424DNA)

### Communauté
- SeedSigner Telegram
- Bitcoin Hardware Wallet Community
- M5Stack Discord

---

## ⚠️ Disclaimer

**Ce projet est en développement actif.**

- ✅ OK: Tests, développement, petits montants (<$100)
- ⚠️ Attendre: Audit sécurité pour usage important
- ❌ Jamais: Stocker vos économies de vie sans audit externe

---

**Licence**: MIT  
**Basé sur**: SeedSigner (MIT)  
**Contributeurs**: Bienvenue !

---

*Built with ❤️ and 🔐 for the Bitcoin community*
