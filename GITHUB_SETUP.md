# 🚀 Mise en Place GitHub - Instructions Complètes

## Étape 1: Créer le Repo sur GitHub

1. Va sur https://github.com/new
2. **Repository name**: `seedsigner-esp32s3`
3. **Description**: `Air-gapped Bitcoin Hardware Wallet for ESP32-S3 (M5Stack CoreS3)`
4. **Public** (coché)
5. **Add a README**: ❌ DÉCOCHER (on en a déjà un)
6. **Add .gitignore**: ❌ DÉCOCHER (on en a déjà un)
7. **Choose a license**: MIT License
8. Click **Create repository**

## Étape 2: Pousser le Code Local

Dans PowerShell (dans le dossier C:\Users\Silex) :

```powershell
# Se placer dans le dossier du projet
cd C:\Users\Silex

# Initialiser le repo git (si pas déjà fait)
git init

# Ajouter le remote GitHub (remplace silexdev par ton username)
git remote add origin https://github.com/silexdev/seedsigner-esp32s3.git

# Configurer git (si pas déjà fait)
git config user.name "Ton Nom"
git config user.email "ton.email@example.com"

# Ajouter tous les fichiers
git add .

# Commit
git commit -m "🎉 Initial commit: Complete SeedSigner ESP32-S3 implementation

Features:
- BIP39/BIP32 HD wallet with 2048 wordlist
- Bitcoin address generation (P2WPKH/P2SH/P2PKH)
- PSBT signing (BIP174)
- Hardware RNG (TRNG + camera + jitter)
- Secure memory wiping
- QR code generation
- LVGL touch interface
- GC0308 camera driver
- PN532 NFC driver
- Web installer ready"

# Push sur main
git branch -M main
git push -u origin main
```

## Étape 3: Activer GitHub Pages

1. Sur GitHub, va dans **Settings** → **Pages**
2. **Source**: Deploy from a branch
3. **Branch**: `main` / `/(root)`
4. Click **Save**

## Étape 4: Créer une Release

```powershell
# Créer un tag
git tag -a v1.0.0 -m "Release v1.0.0 - Genesis"

# Pusher le tag
git push origin v1.0.0
```

## Étape 5: Compiler et Ajouter les Binaires

```powershell
# Build le firmware
pio run

# Copier les binaires dans web/
cp .pio/build/esp32-s3/bootloader.bin web/
cp .pio/build/esp32-s3/partitions.bin web/partition-table.bin
cp .pio/build/esp32-s3/firmware.bin web/seedsigner-esp32s3-v1.0.0.bin

# Mettre à jour le manifest avec le vrai MD5 (Linux/Mac)
# Sur Windows, remplace manuellement dans web/manifest.json
$md5 = (Get-FileHash web/seedsigner-esp32s3-v1.0.0.bin -Algorithm MD5).Hash.ToLower()
(Get-Content web/manifest.json) -replace "PLACEHOLDER_MD5", $md5 | Set-Content web/manifest.json

# Commit et push
git add web/
git commit -m "Add firmware binaries for v1.0.0"
git push origin main
```

## Étape 6: Vérifier le Web Flasher

Attends 2-3 minutes puis va sur :
```
https://silexdev.github.io/seedsigner-esp32s3
```

Tu devrais voir la page avec le bouton **"📥 Install SeedSigner v1.0.0"**

## Commandes Récapitulatives (Copier-Coller)

```powershell
# Tout en une fois
cd C:\Users\Silex
git init
git remote add origin https://github.com/silexdev/seedsigner-esp32s3.git
git add .
git commit -m "🎉 Initial commit: Complete SeedSigner ESP32-S3"
git branch -M main
git push -u origin main
git tag -a v1.0.0 -m "Release v1.0.0"
git push origin v1.0.0
```

## ⚠️ Si Git est Bloqué (index.lock)

```powershell
# Kill tous les processus git
taskkill /F /IM git.exe

# Supprimer le lock
Remove-Item .git/index.lock -Force

# Réessayer
git add .
git commit -m "🎉 Initial commit"
```

## ✅ Vérification Finale

Après le push, vérifie sur GitHub :
- [ ] Tous les fichiers sont présents (src/, include/, web/, .github/)
- [ ] Le tag v1.0.0 existe
- [ ] GitHub Pages est activé
- [ ] La page web est accessible

**🎉 Une fois fait, les utilisateurs peuvent flasher leur M5Stack en 1 click !**
