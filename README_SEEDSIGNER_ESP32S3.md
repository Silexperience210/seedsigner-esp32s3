# SeedSigner ESP32-S3 Edition

Air-gapped Bitcoin hardware wallet firmware for ESP32-S3 (M5Stack CoreS3).

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Platform](https://img.shields.io/badge/platform-ESP32--S3-orange)

## 🎯 Features

### Core Wallet
- ✅ **BIP39** - Mnemonic generation with hardware entropy (2048 words)
- ✅ **BIP32** - HD wallet with full derivation paths
- ✅ **BIP44/BIP84** - Multi-account hierarchy and native SegWit
- ✅ **Bitcoin Addresses** - P2WPKH (bc1q...), P2SH-P2WPKH (3...), P2PKH (1...)
- ✅ **PSBT Signing** - BIP174 partial signed Bitcoin transactions
- ✅ **Bech32** - Native SegWit address encoding (BIP173)

### Security
- 🔐 **Hardware RNG** - ESP32-S3 TRNG + camera noise + timing jitter
- 🔐 **Secure Memory** - Multi-pass wiping with memory barriers
- 🔐 **Stack Protection** - Stack canary and overflow detection
- 🔐 **Air-gapped** - WiFi/BT disabled, no network connectivity
- 🔐 **RAM Only** - Private keys never touch flash storage

### Hardware
- 📷 **GC0308 Camera** - QR code scanning and entropy gathering
- 📡 **PN532 NFC** - NTAG424 DNA encrypted backup support
- 📱 **M5Stack CoreS3** - 2.0" IPS 320×240 touchscreen
- 🔋 **Power Management** - Deep sleep and secure shutdown

### UI/UX
- 🎨 **LVGL v9** - Modern embedded GUI
- 🎨 **SeedSigner Theme** - Bitcoin Orange (#F7931A) design
- 🎨 **Touch Interface** - Intuitive screen navigation
- 🎨 **QR Display** - Address and PSBT export

## 📁 Project Structure

```
├── include/
│   ├── core/           # Crypto core
│   │   ├── hardware_rng.h      # Hardware random number generator
│   │   ├── secure_memory.h     # Secure wiping and buffers
│   │   ├── bip39.h             # BIP39 mnemonic
│   │   ├── bip32_secp256k1.h   # HD wallet
│   │   ├── bitcoin_address.h   # Address generation
│   │   ├── psbt_signer.h       # PSBT signing
│   │   ├── qr_generator.h      # QR encoding
│   │   └── bip39_optimized.h   # Fast word lookup
│   ├── ui/             # User interface
│   │   ├── screens.h           # LVGL screens
│   │   └── app_controller.h    # App state machine
│   └── drivers/        # Hardware drivers
│       ├── gc0308.h            # Camera
│       ├── pn532_nfc.h         # NFC
│       └── secure_storage.h    # Encrypted prefs
├── src/
│   ├── core/           # Crypto implementations
│   ├── ui/             # UI implementations
│   ├── drivers/        # Driver implementations
│   └── main.cpp        # Application entry
├── platformio.ini      # Build configuration
└── docs/               # Documentation
```

## 🚀 Quick Start

### Requirements
- M5Stack CoreS3 or ESP32-S3 with:
  - GC0308 camera module
  - PN532 NFC module (optional)
  - 320×240 display
- PlatformIO IDE or CLI

### Build
```bash
# Clone repository
git clone <repo-url>
cd seedsigner-esp32s3

# Build firmware
pio run

# Upload to device
pio run --target upload

# Monitor serial
pio device monitor
```

### First Use
1. Power on device
2. Tap "New Wallet" to generate seed
3. Write down 12/24 word mnemonic
4. Confirm you saved the words
5. Main menu appears - ready to use!

## 🔐 Security Model

### Threat Model
| Threat | Mitigation |
|--------|------------|
| Physical theft | PIN protection, auto-lock |
| Side-channel | Constant-time crypto, noise generation |
| Cold boot | RAM only, no flash storage of keys |
| Supply chain | Reproducible builds, open source |
| Evil maid | Tamper-evident case, secure boot |

### Key Management
- Master seed: RAM only, never persisted
- Mnemonic: User writes down, device forgets after confirmation
- Private keys: Derived on-demand, wiped after use
- Backup: Optional NFC with AES-128 encryption

## 📱 User Interface

### Screens
1. **Welcome** - New wallet or restore
2. **Seed Display** - Show mnemonic (12 or 24 words)
3. **Main Menu** - Receive / Send / Settings
4. **Receive** - Display address + QR code
5. **Send Scan** - Scan PSBT QR from wallet
6. **Send Confirm** - Verify amounts and sign
7. **Settings** - Units, network, security

### Navigation
- Touch buttons for all actions
- Back gesture or button to cancel
- Auto-lock after inactivity

## 🔧 Technical Details

### Hardware RNG
```cpp
// Multi-source entropy
HardwareRNG::fill(buffer, 32);
// - ESP32-S3 TRNG
// - GC0308 camera sensor noise
// - Interrupt timing jitter
// - ADC floating pin noise
```

### BIP39 Wordlist
```cpp
// Binary search O(log n)
BIP39Optimized::find_word_binary(word);
// 2048 words in PROGMEM (16KB flash, 0 RAM)
```

### PSBT Signing
```cpp
PSBT::Signer signer;
signer.parse(psbt_data, len);
signer.validate();
signer.sign_all(&master_key);
signer.export_signed(out, &out_len);
```

### QR Generation
```cpp
QR::Code qr;
QR::encode_address("bc1q...", &qr);
QR::render_bitmap(&qr, bitmap, size);
```

## 📊 Memory Usage

| Component | Flash | RAM |
|-----------|-------|-----|
| uBitcoin | ~30KB | ~2KB |
| Wordlist | ~16KB | 0 |
| LVGL | ~50KB | ~40KB |
| App Code | ~80KB | ~20KB |
| **Total** | **~176KB** | **~62KB** |

Free for heap: ~200KB on ESP32-S3 with 8MB PSRAM

## 🧪 Testing

### Unit Tests
```bash
pio test
```

### Hardware Tests
- Entropy quality: `test/entropy_quality.cpp`
- Secure wipe: `test/secure_memory.cpp`
- Crypto vectors: `test/bip_vectors.cpp`

### Security Audit
- Side-channel resistance
- Power analysis protection
- Glitch attack resistance

## 🤝 Contributing

1. Fork repository
2. Create feature branch
3. Follow coding style
4. Add tests
5. Submit PR

## 📜 License

MIT License - See LICENSE file

## 🙏 Credits

- SeedSigner team for the original design
- uBitcoin library for secp256k1
- LVGL for the UI framework
- Bitcoin BIPs authors

## ⚠️ Disclaimer

This is experimental software. Use at your own risk. Always verify addresses and transaction details before signing. Keep your seed phrase secure and offline.

---

**Made with ❤️ for the Bitcoin community**
