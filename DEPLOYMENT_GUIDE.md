# 🚀 Deployment Guide - SeedSigner ESP32-S3

## 📦 Créer une Release GitHub

### 1. Préparation

```bash
# S'assurer que tout est commit
git add .
git commit -m "Prepare for v1.0.0 release"

# Créer un tag
git tag -a v1.0.0 -m "Release v1.0.0 - Genesis"

# Pusher le tag
git push origin v1.0.0
```

### 2. Build Manuel (si pas de CI)

```bash
# Build le firmware
pio run

# Créer le répertoire de release
mkdir -p release
cp .pio/build/esp32-s3/firmware.bin release/seedsigner-esp32s3-v1.0.0.bin
cp .pio/build/esp32-s3/bootloader.bin release/bootloader.bin
cp .pio/build/esp32-s3/partitions.bin release/partition-table.bin

# Calculer les checksums
cd release
md5sum seedsigner-esp32s3-v1.0.0.bin > firmware.md5
sha256sum seedsigner-esp32s3-v1.0.0.bin > firmware.sha256
```

### 3. Créer la Release sur GitHub

1. Aller sur GitHub → Releases → Draft a new release
2. Choisir le tag `v1.0.0`
3. Titre: "SeedSigner ESP32-S3 v1.0.0"
4. Description:
```markdown
## 🎉 SeedSigner ESP32-S3 v1.0.0

### ✨ Features
- BIP39 mnemonic generation with hardware entropy
- BIP32 HD wallet with full derivation
- Bitcoin addresses (P2WPKH, P2SH-P2WPKH, P2PKH)
- PSBT signing (BIP174)
- QR code generation
- Touch interface with LVGL
- Camera support (GC0308)
- NFC backup (NTAG424)

### 🔧 Installation

**Web Installer (Recommended):**
Visit: https://silexdev.github.io/seedsigner-esp32s3

**Manual:**
```bash
esptool.py --chip esp32s3 --port COM3 write_flash 0x0 seedsigner-esp32s3-v1.0.0.bin
```

### 📋 Checksums
- MD5: `a1b2c3d4...`
- SHA256: `f6e5d4c3...`
```

5. Upload les fichiers:
   - `seedsigner-esp32s3-v1.0.0.bin`
   - `bootloader.bin`
   - `partition-table.bin`
   - `firmware.md5`
   - `firmware.sha256`
   - `manifest.json`

6. Publier la release!

---

## 🌐 Déployer la Page Web (GitHub Pages)

### Configuration GitHub Pages

1. Aller sur GitHub → Settings → Pages
2. Source: Deploy from a branch
3. Branch: `gh-pages` / folder: `/ (root)`
4. Save

### Méthode 1: GitHub Actions (Automatique)

Le workflow `.github/workflows/build-and-release.yml` déploie automatiquement:
- Build sur chaque push
- Release sur chaque tag `v*`
- Deploy web sur GitHub Pages

### Méthode 2: Manuel

```bash
# Créer une branche orpheine
git checkout --orphan gh-pages

# Copier les fichiers web
cp -r web/* .

# Commit
git add .
git commit -m "Deploy web installer"

# Push
git push origin gh-pages

# Retourner sur main
git checkout main
```

### URL de la Page

Une fois déployé, accessible à:
```
https://silexdev.github.io/seedsigner-esp32s3
```

---

## 📱 Comment les Utilisateurs Flash leur M5

### Méthode 1: Web Installer (1 Click) ⭐ Recommandé

1. **Ouvrir** https://silexdev.github.io/seedsigner-esp32s3 dans Chrome/Edge/Opera
2. **Connecter** le M5Stack CoreS3 en USB
3. **Cliquer** sur "📥 Install SeedSigner v1.0.0"
4. **Sélectionner** le port série (ex: COM3)
5. **Attendre** la fin du flash (~1-2 min)
6. **Redémarrer** le M5
7. ✅ **Fini!** L'interface SeedSigner apparaît

### Méthode 2: PlatformIO (Developers)

```bash
git clone https://github.com/silexdev/seedsigner-esp32s3.git
cd seedsigner-esp32s3
pio run --target upload
```

### Méthode 3: esptool.py (Advanced)

```bash
# Télécharger le firmware depuis GitHub Releases
# Puis:
esptool.py --chip esp32s3 --port COM3 --baud 921600 \
  write_flash 0x0 seedsigner-esp32s3-v1.0.0.bin
```

---

## 🔧 Configuration ESP Web Tools

Le fichier `web/manifest.json` est crucial:

```json
{
  "name": "SeedSigner ESP32-S3",
  "version": "1.0.0",
  "builds": [{
    "chipFamily": "ESP32-S3",
    "parts": [
      {"path": "bootloader.bin", "offset": 4096},
      {"path": "partition-table.bin", "offset": 32768},
      {"path": "seedsigner-esp32s3-v1.0.0.bin", "offset": 65536}
    ]
  }]
}
```

### Mise à jour du manifest

Lors d'une nouvelle version:
1. Mettre à jour `"version"`
2. Renommer le fichier firmware
3. Recalculer les MD5
4. Uploader sur GitHub Releases
5. Mettre à jour la branche `gh-pages`

---

## ✅ Vérification Post-Déploiement

### Test Web Installer

1. Ouvrir https://silexdev.github.io/seedsigner-esp32s3
2. Vérifier que le bouton "Install" apparaît
3. Cliquer et vérifier la détection du port
4. Tester le flash sur un M5Stack

### Test Release

1. Télécharger le firmware depuis Releases
2. Vérifier les checksums:
   ```bash
   sha256sum -c firmware.sha256
   ```
3. Flasher manuellement et tester

### Test GitHub Actions

Vérifier que les workflows sont verts ✅:
- Build
- Release
- Deploy Web

---

## 🚀 Prochaines Versions

Pour release une mise à jour:

```bash
# Modifier le code
git add .
git commit -m "Fix bug XYZ"

# Tag nouvelle version
git tag -a v1.0.1 -m "Release v1.0.1 - Bug fixes"
git push origin v1.0.1

# GitHub Actions fait le reste automatiquement!
```

---

## 📊 Checklist Déploiement

- [ ] Firmware compilé et testé
- [ ] Checksums calculés
- [ ] Release créée sur GitHub
- [ ] Assets uploadés (bin, md5, sha256)
- [ ] Web page fonctionnelle
- [ ] GitHub Pages activé
- [ ] Test flash web réussi
- [ ] Test flash manuel réussi
- [ ] Documentation à jour

---

**🎉 Votre SeedSigner ESP32-S3 est maintenant accessible à tous!**
