# 🚀 Guide de Développement

## Setup environnement de développement

### Prérequis

```bash
# 1. Installer VS Code
# https://code.visualstudio.com/

# 2. Installer PlatformIO extension dans VS Code

# 3. Cloner le projet
git clone https://github.com/yourusername/SeedSigner-ESP32S3.git
cd SeedSigner-ESP32S3

# 4. Ouvrir dans VS Code
code .
```

### Build

```bash
# Build pour M5Stack CoreS3
pio run -e m5stack-cores3

# Build avec debug symbols
pio run -e m5stack-cores3-debug

# Clean
pio run --target clean
```

### Upload

```bash
# USB Upload (standard)
pio run -e m5stack-cores3 --target upload

# Upload avec monitor
pio run -e m5stack-cores3 --target upload && pio device monitor

# OTA Upload (si configuré)
pio run -e upload_ota --target upload
```

### Tests

```bash
# Run unit tests
pio test -e native_test

# Run hardware tests
pio test -e m5stack-cores3
```

---

## Architecture du code

```
src/
├── main.cpp              # Entry point
├── core/                 # Cryptographie Bitcoin
│   ├── bip39.cpp         # Mnemonics
│   ├── bip32.cpp         # HD wallets
│   ├── psbt.cpp          # Transaction signing
│   └── entropy.cpp       # Random generation
├── ui/                   # Interface LVGL
│   ├── app.cpp           # Main app state machine
│   ├── screens/          # Individual screens
│   └── components/       # Reusable widgets
├── drivers/              # Hardware abstraction
│   ├── display.cpp
│   ├── camera.cpp
│   ├── nfc.cpp
│   ├── touch.cpp
│   └── sd.cpp
└── utils/                # Utilities
    ├── qr_code.cpp
    ├── memory.cpp
    └── crypto_utils.cpp
```

---

## Conventions de code

### Style

- C++17
- 4 spaces indentation
- `snake_case` for functions/variables
- `PascalCase` for classes
- `SCREAMING_SNAKE_CASE` for constants

### Exemple

```cpp
namespace SeedSigner {
namespace Core {

class Bip39Generator {
public:
    static constexpr size_t MAX_WORDS = 24;
    
    bool generate_mnemonic_from_entropy(const uint8_t* entropy, 
                                         size_t entropy_len,
                                         char* output, 
                                         size_t output_len);
    
private:
    uint16_t calculate_checksum(const uint8_t* data, size_t len);
};

} // namespace Core
} // namespace SeedSigner
```

---

## Debug

### Serial Monitor

```bash
pio device monitor -b 115200
```

### Debug avec JTAG (avancé)

```bash
# Nécessite ESP-PROG ou similaire
pio debug -e m5stack-cores3-debug
```

---

## Contribution

1. Fork le repo
2. Créer une branche (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push (`git push origin feature/amazing-feature`)
5. Ouvrir une Pull Request

### Checklist PR

- [ ] Code compile sans warnings
- [ ] Tests passent
- [ ] Documentation mise à jour
- [ ] Commit messages clairs
