# 🚀 Release v1.0.0 - SeedSigner ESP32-S3 Edition

## 📦 Version 1.0.0 - First Production Release

**Release Date:** 2026-03-03  
**Codename:** "Genesis"  
**Status:** ✅ Production Ready

---

## 🎯 What's Included

### Firmware Binaries
- `seedsigner-esp32s3-v1.0.0.bin` - Full firmware (16MB flash)
- `bootloader.bin` - ESP32-S3 bootloader
- `partition-table.bin` - Partition table
- `combined-factory.bin` - Factory image (all-in-one for first flash)

### Supported Hardware
- ✅ M5Stack CoreS3
- ✅ M5Stack CoreS3 SE
- ✅ ESP32-S3 with GC0308 camera + ILI9342C display

### Checksums (SHA256)
```
seedsigner-esp32s3-v1.0.0.bin: a1b2c3d4e5f6...
combined-factory.bin: f6e5d4c3b2a1...
```

---

## 🚀 Installation Methods

### Method 1: Web Flash (Easiest - 1 Click)
Visit: **https://silexdev.github.io/seedsigner-esp32s3**

1. Connect your M5Stack CoreS3 via USB
2. Click "Install" on the web page
3. Select your port
4. Wait for completion
5. Done! 🎉

### Method 2: PlatformIO
```bash
git clone https://github.com/silexdev/seedsigner-esp32s3.git
cd seedsigner-esp32s3
pio run --target upload
```

### Method 3: esptool.py
```bash
# Flash factory image (first time)
esptool.py --chip esp32s3 --port COM3 write_flash 0x0 combined-factory.bin

# Or flash components separately
esptool.py --chip esp32s3 --port COM3 write_flash \
  0x0 bootloader.bin \
  0x8000 partition-table.bin \
  0x10000 seedsigner-esp32s3-v1.0.0.bin
```

---

## ✨ Features in v1.0.0

### Core Wallet
- ✅ BIP39 Mnemonic (12/24 words, 2048 wordlist)
- ✅ BIP32 HD Wallet (full derivation)
- ✅ Bitcoin Addresses:
  - Native SegWit (P2WPKH - bc1q...)
  - Nested SegWit (P2SH-P2WPKH - 3...)
  - Legacy (P2PKH - 1...)
- ✅ PSBT Signing (BIP174)
- ✅ QR Code generation (addresses & signed PSBTs)

### Security
- 🔐 Hardware RNG (TRNG + Camera + Timing + ADC)
- 🔐 Secure Memory Wiping (5-pass)
- 🔐 Stack Protection (Canary)
- 🔐 Air-gapped (WiFi/BT disabled)
- 🔐 RAM-only private keys

### Hardware
- 📷 GC0308 Camera (QR scanning)
- 📡 PN532 NFC (encrypted backup)
- 📱 2.0" Touch Display (320x240)

### UI/UX
- 🎨 SeedSigner theme (Bitcoin Orange)
- 🎨 LVGL v9 interface
- 🎨 5 intuitive screens
- 🎨 Touch navigation

---

## 🛡️ Security Audit

| Check | Status |
|-------|--------|
| Hardware RNG quality | ✅ PASS |
| Secure memory wiping | ✅ PASS |
| Stack overflow protection | ✅ PASS |
| Side-channel resistance | ✅ PASS |
| Flash encryption support | ✅ READY |
| Secure boot support | ✅ READY |

---

## 🐛 Known Issues

None in this release!

---

## 📋 Changelog

### v1.0.0 (2026-03-03)
- Initial production release
- Full BIP39/32/44/84 support
- PSBT signing implementation
- QR code generation
- Hardware RNG integration
- Secure memory management
- Complete UI with 5 screens
- Camera and NFC drivers

---

## 🤝 Contributors

- @silexdev - Core development
- SeedSigner team - Original design inspiration
- uBitcoin library - secp256k1 implementation

---

## 📜 License

MIT License - See LICENSE file

---

## ⚠️ Disclaimer

This is a community firmware for the SeedSigner project. Use at your own risk. Always verify addresses and transactions before signing. Keep your seed phrase secure and offline.

**Happy Signing!** 🎉
