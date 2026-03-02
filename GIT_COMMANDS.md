# 🚀 Git Commands - Commit & Push

## Initial Commit (à exécuter dans le dossier SeedSigner-ESP32S3)

```bash
# 1. Initialiser Git (si pas déjà fait)
git init

# 2. Configurer (première fois)
git config user.name "Votre Nom"
git config user.email "votre@email.com"

# 3. Ajouter tous les fichiers
git add -A

# 4. Créer le commit
git commit -m "🎉 Initial release: SeedSigner ESP32-S3 Edition v0.1.0-alpha

✅ Complete implementation with:
- BIP39/BIP32/secp256k1 cryptography
- M5Stack CoreS3 hardware support (Display, Camera, NFC)
- 10+ UI screens with official SeedSigner theme
- NTAG 424 DNA secure storage with AES-128
- Stateless/air-gapped security architecture
- 53 files, ~15000 lines of code

Features:
- 24-word mnemonic generation
- PSBT transaction signing
- QR code scan/generate
- NFC encrypted seed backup
- Hardware RNG entropy

Hardware: M5Stack CoreS3 + Unit NFC
License: MIT"

# 5. Connecter au repo GitHub (créer d'abord sur GitHub)
git remote add origin https://github.com/VOTRE_USERNAME/SeedSigner-ESP32S3.git

# 6. Push
git push -u origin main
```

## Ou avec script PowerShell

```powershell
cd SeedSigner-ESP32S3

git init
git add -A
git commit -m "Initial release: SeedSigner ESP32-S3 Edition v0.1.0-alpha"
git remote add origin https://github.com/VOTRE_USERNAME/SeedSigner-ESP32S3.git
git push -u origin main
```

## Vérifier le status

```bash
git status
git log --oneline -5
git branch -a
```

## Si problème de credentials

```bash
# Stocker credentials (optionnel)
git config --global credential.helper store

# Ou utiliser SSH
git remote set-url origin git@github.com:VOTRE_USERNAME/SeedSigner-ESP32S3.git
```

## Fichiers ignorés (.gitignore)

Créer `.gitignore`:
```
.pio/
.vscode/
*.tmp
*.log
.DS_Store
build/
dist/
```

---

**⚠️ Important**: Remplacer `VOTRE_USERNAME` par votre vrai username GitHub !
