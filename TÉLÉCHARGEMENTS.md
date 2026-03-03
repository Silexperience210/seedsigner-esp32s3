# 📦 Bitaxe Monitor - Téléchargements

## 🎯 Option 1 : Télécharger tout en une fois (RECOMMANDÉ)

**[📥 Télécharger bitaxe_monitor.zip](computer:///mnt/user-data/outputs/bitaxe_monitor.zip)** (35 KB)

Cette archive contient TOUT le projet prêt à utiliser !

---

## 📄 Option 2 : Télécharger les fichiers individuellement

### 🚀 Guides de démarrage

- **[START_HERE.md](computer:///mnt/user-data/outputs/START_HERE.md)** - Commencez ici !
- **[README.md](computer:///mnt/user-data/outputs/README.md)** - Documentation complète
- **[QUICKSTART.md](computer:///mnt/user-data/outputs/QUICKSTART.md)** - Installation en 5 minutes

### 📚 Documentation avancée

- **[ADVANCED.md](computer:///mnt/user-data/outputs/ADVANCED.md)** - Configurations avancées
- **[EXAMPLES.md](computer:///mnt/user-data/outputs/EXAMPLES.md)** - Exemples de code
- **[HARDWARE.md](computer:///mnt/user-data/outputs/HARDWARE.md)** - Spécifications et câblage
- **[DESIGN.md](computer:///mnt/user-data/outputs/DESIGN.md)** - Design visuel
- **[CHANGELOG.md](computer:///mnt/user-data/outputs/CHANGELOG.md)** - Historique des versions

### 🔧 Fichiers de configuration

- **[platformio.ini](computer:///mnt/user-data/outputs/platformio.ini)** - Configuration PlatformIO
- **[User_Setup.h](computer:///mnt/user-data/outputs/User_Setup.h)** - Configuration TFT_eSPI
- **[partitions.csv](computer:///mnt/user-data/outputs/partitions.csv)** - Partitions mémoire

### 💻 Scripts de build

- **[build.sh](computer:///mnt/user-data/outputs/build.sh)** - Script Linux/Mac
- **[build.ps1](computer:///mnt/user-data/outputs/build.ps1)** - Script Windows PowerShell

### 📁 Code source

**Dossier include/** :
- **[include/config.h](computer:///mnt/user-data/outputs/include/config.h)** - Configuration des mineurs
- **[include/lv_conf.h](computer:///mnt/user-data/outputs/include/lv_conf.h)** - Configuration LVGL
- **[include/GT911.h](computer:///mnt/user-data/outputs/include/GT911.h)** - Driver tactile

**Dossier src/** :
- **[src/main.cpp](computer:///mnt/user-data/outputs/src/main.cpp)** - Code principal (2000+ lignes !)

### 📜 Autre

- **[LICENSE](computer:///mnt/user-data/outputs/LICENSE)** - Licence MIT

---

## 🚀 Après téléchargement

### 1️⃣ Extraire l'archive ZIP
```bash
unzip bitaxe_monitor.zip
cd bitaxe_monitor
```

### 2️⃣ Ouvrir avec VSCode
```bash
code .
```

### 3️⃣ Installer PlatformIO
Dans VSCode : Extensions → Rechercher "PlatformIO" → Installer

### 4️⃣ Configurer TFT_eSPI

**Windows:**
```powershell
copy User_Setup.h %USERPROFILE%\.platformio\lib\TFT_eSPI\User_Setup.h
```

**Linux/Mac:**
```bash
cp User_Setup.h ~/.platformio/lib/TFT_eSPI/User_Setup.h
```

### 5️⃣ Modifier vos IPs
Ouvrir `include/config.h` et changer les IPs de vos Bitaxe :

```cpp
const MinerConfig MINERS[] = {
    {"192.168.1.100", "Mon-Bitaxe-1"},  // ← CHANGER ICI
    {"192.168.1.101", "Mon-Bitaxe-2"},
};
```

### 6️⃣ Compiler et uploader
Dans PlatformIO :
- Cliquer sur ✓ (Build)
- Cliquer sur ➡️ (Upload)

---

## 🎨 Aperçu de l'interface

```
╔════════════════════════════════════════════════════════════╗
║              ⛏ BITAXE COMMAND CENTER ⛏                   ║
╠════════════════════════════════════════════════════════════╣
║  ┌──────────┐    ┌────────────────┐    ┌──────────┐      ║
║  │MINERS    │    │  12:34:56      │    │BITCOIN   │      ║
║  │Active: 3 │    │                │    │$42,000   │      ║
║  │1500 GH/s │    │  (swipe ◄►)   │    │Block:    │      ║
║  │Temp: 65°C│    │                │    │870,000   │      ║
║  └──────────┘    └────────────────┘    └──────────┘      ║
╠════════════════════════════════════════════════════════════╣
║                    [Alertes / Status]                      ║
╚════════════════════════════════════════════════════════════╝
```

**Design** : Noir profond avec rouge éclatant, effets lumineux, style gothique

**Animations** :
- ⚡ Flash rouge pour nouveaux blocs
- 🔴 Clignotement si mineur offline
- 🌡️ Changement couleur selon température

**Contrôles** :
- 👆 Swipe gauche/droite pour changer l'affichage central
- 3 vues : Heure / Prix Bitcoin / Hashrate total

---

## ❓ Besoin d'aide ?

Consultez :
1. **START_HERE.md** - Vue d'ensemble
2. **QUICKSTART.md** - Installation rapide
3. **README.md** - Documentation détaillée
4. **HARDWARE.md** - Problèmes de câblage
5. **EXAMPLES.md** - Personnalisation

---

## 🎯 Checklist de démarrage

- [ ] Télécharger bitaxe_monitor.zip
- [ ] Extraire l'archive
- [ ] Installer VSCode + PlatformIO
- [ ] Copier User_Setup.h dans TFT_eSPI
- [ ] Modifier les IPs dans config.h
- [ ] Compiler le projet
- [ ] Brancher l'ESP32-4827S043
- [ ] Uploader le code
- [ ] Configurer le WiFi (portail captif)
- [ ] Profiter ! 🔥

---

⚡ **KEEP MINING!** ⚡

🔴⚫ **Ultra-futuristic Bitaxe monitoring** 🔴⚫
