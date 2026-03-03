# SeedSigner ESP32-S3

Hardware Bitcoin wallet for ESP32-S3 (M5Stack CoreS3).

**Web Installer:** https://silexperience210.github.io/seedsigner-esp32s3

## Features

- BIP39 mnemonic generation
- BIP32 HD wallet
- Bitcoin address generation (P2WPKH, P2SH, P2PKH)
- PSBT signing (BIP174)
- QR code generation
- NFC backup support
- Air-gapped (no WiFi/BT)

## Hardware

- M5Stack CoreS3 (ESP32-S3, 8MB PSRAM, 16MB Flash)
- 320x240 touchscreen
- GC0308 camera
- PN532 NFC module

## Build

```bash
pio run
```

## Flash via Web

https://silexperience210.github.io/seedsigner-esp32s3

## License

MIT
