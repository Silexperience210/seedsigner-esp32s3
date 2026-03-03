# 🚀 Déploiement Rapide - 3 Commandes

## Prérequis
- Git installé
- PlatformIO installé (`pip install platformio`)
- Un compte GitHub

---

## Option 1 : Script Automatique (Recommandé)

### Étape 1 : Exécuter le setup
```powershell
# Dans PowerShell, dans le dossier du projet
cd C:\Users\Silex
.\setup-and-deploy.ps1 -GitHubUsername "tonpseudo"
```

### Étape 2 : Créer le repo sur GitHub
1. Va sur https://github.com/new
2. Nom : `seedsigner-esp32s3`
3. **DÉCOCHER** "Initialize with README"
4. Create repository

### Étape 3 : Build et deploy
```powershell
.\build-and-release.ps1
```

### Étape 4 : Activer GitHub Pages
1. Va sur `https://github.com/tonpseudo/seedsigner-esp32s3/settings/pages`
2. Source : `Deploy from a branch`
3. Branch : `main` / `/(root)`
4. Save

### Étape 5 : Tester !
Attends 2-3 minutes puis va sur :
```
https://tonpseudo.github.io/seedsigner-esp32s3
```

---

## Option 2 : Commandes Manuelles

Si le script ne marche pas, fais ces commandes une par une :

### 1. Setup Git
```powershell
cd C:\Users\Silex
git init
git config user.email "ton@email.com"
git config user.name "Ton Nom"
git remote add origin https://github.com/tonpseudo/seedsigner-esp32s3.git
```

### 2. Commit
```powershell
git add .
git commit -m "Initial commit"
git branch -M main
git push -u origin main
```

### 3. Tag Release
```powershell
git tag -a v1.0.0 -m "Release v1.0.0"
git push origin v1.0.0
```

### 4. Build
```powershell
pio run
```

### 5. Copier binaires
```powershell
cp .pio/build/esp32-s3/bootloader.bin web/
cp .pio/build/esp32-s3/partitions.bin web/partition-table.bin
cp .pio/build/esp32-s3/firmware.bin web/seedsigner-esp32s3-v1.0.0.bin
```

### 6. Update manifest
```powershell
$md5 = (Get-FileHash web/seedsigner-esp32s3-v1.0.0.bin -Algorithm MD5).Hash.ToLower()
(Get-Content web/manifest.json) -replace "PLACEHOLDER_MD5", $md5 | Set-Content web/manifest.json
```

### 7. Push binaires
```powershell
git add web/
git commit -m "Add firmware binaries"
git push origin main
```

---

## 🔧 Si ça ne marche pas

### Problème : "index.lock exists"
```powershell
taskkill /F /IM git.exe
Remove-Item .git/index.lock -Force
```

### Problème : "remote already exists"
```powershell
git remote remove origin
git remote add origin https://github.com/tonpseudo/seedsigner-esp32s3.git
```

### Problème : "Build failed"
```powershell
# Nettoyer et réessayer
pio run -t clean
pio run
```

---

## ✅ Vérification

Après le déploiement, vérifie :

1. **GitHub** : https://github.com/tonpseudo/seedsigner-esp32s3
   - Tous les fichiers sont là
   - Tag v1.0.0 existe

2. **GitHub Pages** : https://tonpseudo.github.io/seedsigner-esp32s3
   - Page s'affiche
   - Bouton "Install" est visible

3. **Test flash** :
   - Connecte ton M5Stack
   - Clique sur Install
   - Ça flash ! 🎉

---

## 📞 Besoin d'aide ?

Ouvre un issue sur GitHub ou vérifie :
- `GITHUB_SETUP.md` pour les détails
- `DEPLOYMENT_GUIDE.md` pour la doc complète
