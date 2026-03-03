# 🔥 BITAXE MONITOR - Projet complet

## 📦 Contenu du projet

Voici le projet complet pour votre moniteur Bitaxe ultra-futuriste!

### 📂 Structure du projet

```
bitaxe_monitor/
├── 📄 README.md              ← Guide principal (COMMENCEZ ICI!)
├── 📄 QUICKSTART.md          ← Installation en 5 minutes
├── 📄 ADVANCED.md            ← Configurations avancées
├── 📄 EXAMPLES.md            ← Exemples de personnalisation
├── 📄 HARDWARE.md            ← Câblage et spécifications
├── 📄 DESIGN.md              ← Design visuel de l'interface
├── 📄 CHANGELOG.md           ← Historique des versions
├── 📄 LICENSE                ← Licence MIT
├── 📄 .gitignore             ← Fichiers à ignorer pour Git
│
├── 🔧 platformio.ini         ← Configuration du projet
├── 🔧 partitions.csv         ← Schéma de partition mémoire
├── 🔧 User_Setup.h           ← Configuration TFT_eSPI
├── 🔧 build.sh               ← Script de build Linux/Mac
├── 🔧 build.ps1              ← Script de build Windows
│
├── 📁 include/               ← Fichiers header
│   ├── lv_conf.h            ← Configuration LVGL
│   ├── config.h             ← Configuration utilisateur
│   └── GT911.h              ← Driver tactile GT911
│
└── 📁 src/                   ← Code source
    └── main.cpp             ← Fichier principal (2000+ lignes!)
```

## 🚀 Démarrage rapide

### 1️⃣ Ouvrir avec VSCode + PlatformIO

```bash
# Ouvrir le dossier dans VSCode
cd bitaxe_monitor
code .
```

### 2️⃣ Configurer TFT_eSPI

**Windows:**
```powershell
copy User_Setup.h %USERPROFILE%\.platformio\lib\TFT_eSPI\User_Setup.h
```

**Linux/Mac:**
```bash
cp User_Setup.h ~/.platformio/lib/TFT_eSPI/User_Setup.h
```

### 3️⃣ Configurer vos mineurs

Ouvrez `include/config.h` et modifiez les IPs:

```cpp
const MinerConfig MINERS[] = {
    {"192.168.1.100", "Bitaxe-1"},
    {"192.168.1.101", "Bitaxe-2"},
    {"192.168.1.102", "Bitaxe-3"},
};
```

### 4️⃣ Compiler et uploader

**Avec les scripts:**
```bash
# Linux/Mac
./build.sh upload

# Windows
.\build.ps1 upload
```

**Ou avec PlatformIO:**
- Cliquez sur ✓ pour compiler
- Cliquez sur ➡️ pour uploader

## ✨ Fonctionnalités principales

### 🎨 Interface ultra-futuriste
- Design noir/rouge style cyberpunk gothique
- Effets lumineux et animations
- Interface LVGL fluide (60 FPS)

### 📊 Monitoring temps réel
- Hashrate total de tous vos Bitaxe
- Température moyenne
- Nombre de mineurs actifs
- Prix Bitcoin en direct
- Hauteur de bloc actuelle
- Frais moyens du réseau

### 👆 Contrôle tactile
- Swipe gauche/droite pour changer l'affichage
- 3 vues: Heure / Prix BTC / Hashrate
- Interface intuitive

### ⚠️ Alertes intelligentes
- Flash rouge lors de nouveaux blocs
- Clignotement si mineur hors ligne
- Alerte température critique
- Changement de couleur selon l'état

### 🌐 Configuration facile
- Portail WiFi captif au premier démarrage
- Pas besoin de modifier le code pour le WiFi
- Configuration centralisée

## 📚 Documentation

### Guides d'utilisation
- **README.md** - Guide complet et détaillé
- **QUICKSTART.md** - Installation rapide en 5 minutes
- **HARDWARE.md** - Spécifications matérielles et câblage

### Guides avancés
- **ADVANCED.md** - Optimisations, OTA, notifications
- **EXAMPLES.md** - Code pour personnaliser l'interface
- **DESIGN.md** - Visualisation de l'interface

### Autres
- **CHANGELOG.md** - Historique des versions
- **LICENSE** - Licence MIT

## 🎨 Personnalisation

Le projet est conçu pour être facilement personnalisable:

### Changer les couleurs
Modifiez les `#define COLOR_*` dans `src/main.cpp`

### Ajouter des fonctionnalités
Consultez `EXAMPLES.md` pour des exemples prêts à l'emploi:
- Notifications Telegram
- Graphiques historiques
- Capteurs additionnels
- Mode nuit automatique
- Serveur web
- Et plus encore!

### Modifier l'interface
Utilisez LVGL pour créer vos propres éléments graphiques

## 🔧 Configuration matérielle

### Requis
- **ESP32-4827S043** (ESP32-S3 avec écran 4.3" 800x480)
- **1+ Bitaxe** (Supra, Ultra, Max, Hex...)
- **Alimentation** 5V 2A
- **Câble USB-C** de qualité

### Connexions
Tout est intégré sur la carte ESP32-4827S043:
- ✅ Écran LCD (RGB 16-bit)
- ✅ Tactile GT911 (I2C)
- ✅ WiFi intégré
- ✅ Alimentation via USB-C

Voir `HARDWARE.md` pour les détails complets.

## 🐛 Résolution de problèmes

### Écran noir
1. Vérifier l'alimentation (5V 2A)
2. Vérifier le câble USB
3. Regarder le moniteur série

### Tactile ne fonctionne pas
1. Vérifier les connexions I2C
2. Redémarrer l'ESP32
3. Vérifier "GT911 initialisé" dans les logs

### Pas de données des mineurs
1. Vérifier les IPs dans `config.h`
2. Tester l'accès: `http://[IP]/api/system/info`
3. Regarder les logs série

Pour plus d'aide, consultez le README.md!

## 📊 Statistiques du projet

```
Lignes de code:     2000+
Fichiers sources:   3 (main.cpp, GT911.h, config.h)
Documentation:      8 fichiers MD
Bibliothèques:      6 (LVGL, TFT_eSPI, WiFiManager, etc.)
Taille compilée:    ~1.5 MB
RAM utilisée:       ~200 KB
PSRAM utilisée:     ~128 KB
```

## 🤝 Contribution

Ce projet est open-source sous licence MIT!

N'hésitez pas à:
- 🐛 Signaler des bugs
- 💡 Proposer des améliorations
- 🔧 Soumettre des pull requests
- ⭐ Mettre une étoile sur GitHub!

## 🎯 Roadmap

### Version actuelle: 1.0.0
✅ Interface futuriste complète
✅ Monitoring temps réel
✅ Alertes intelligentes
✅ Configuration WiFi

### Version 2.0 (à venir)
- [ ] OTA Updates
- [ ] Mode nuit automatique
- [ ] Graphiques historiques
- [ ] Notifications push
- [ ] Interface web

### Version 3.0 (futur)
- [ ] Support multi-pools
- [ ] App mobile
- [ ] Home Assistant
- [ ] IA prédictive

## 🙏 Remerciements

Merci à:
- **Bitaxe Team** - Pour les excellents mineurs
- **mempool.space** - Pour l'API gratuite
- **LVGL** - Pour la bibliothèque graphique
- **Espressif** - Pour l'ESP32-S3
- **Communauté Bitcoin** - Pour le support!

## 📞 Support et contact

- 💬 **Discord**: [Rejoindre]
- 📧 **Email**: [Contact]
- 🐙 **GitHub**: [Issues]
- 📱 **Twitter**: [@bitaxemonitor]

---

## 🎬 Prochaines étapes

1. **Lisez** `QUICKSTART.md` pour une installation rapide
2. **Consultez** `README.md` pour la documentation complète
3. **Explorez** `EXAMPLES.md` pour personnaliser
4. **Profitez** de votre moniteur ultra-futuriste! 🔥

---

⚡ **KEEP MINING! Happy monitoring!** ⚡

🔴⚫ **Ultra-futuristic design by the Bitcoin community** 🔴⚫

```
     ███╗   ███╗██╗███╗   ██╗██╗███╗   ██╗ ██████╗ 
     ████╗ ████║██║████╗  ██║██║████╗  ██║██╔════╝ 
     ██╔████╔██║██║██╔██╗ ██║██║██╔██╗ ██║██║  ███╗
     ██║╚██╔╝██║██║██║╚██╗██║██║██║╚██╗██║██║   ██║
     ██║ ╚═╝ ██║██║██║ ╚████║██║██║ ╚████║╚██████╔╝
     ╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═══╝ ╚═════╝ 
                                                     
        ██████╗ ██╗████████╗ ██████╗ ██████╗ ██╗███╗   ██╗
        ██╔══██╗██║╚══██╔══╝██╔════╝██╔═══██╗██║████╗  ██║
        ██████╔╝██║   ██║   ██║     ██║   ██║██║██╔██╗ ██║
        ██╔══██╗██║   ██║   ██║     ██║   ██║██║██║╚██╗██║
        ██████╔╝██║   ██║   ╚██████╗╚██████╔╝██║██║ ╚████║
        ╚═════╝ ╚═╝   ╚═╝    ╚═════╝ ╚═════╝ ╚═╝╚═╝  ╚═══╝
```
