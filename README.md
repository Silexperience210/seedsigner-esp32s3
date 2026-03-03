# 🔥 Bitaxe Monitor Ultra - Interface Futuriste

![Version](https://img.shields.io/badge/version-1.0.0-red.svg)
![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

## 📋 Description

Interface de monitoring ultra-futuriste pour vos mineurs Bitaxe sur écran ESP32-4827S043 (800x480 tactile). Design noir et rouge inspiré du style gothique médiéval et cyberpunk.

### ✨ Fonctionnalités

- 🎨 **Interface ultra-futuriste** avec design noir/rouge et effets lumineux
- 📊 **Monitoring en temps réel** de tous vos mineurs Bitaxe
- 💰 **Prix Bitcoin en direct** via API mempool.space
- ⛓️ **Synchronisation blockchain** - hauteur de bloc et frais moyens
- 🔥 **Animations** lors de la découverte de nouveaux blocs
- ⚠️ **Alertes visuelles** en cas de panne de mineur ou surchauffe
- 👆 **Interface tactile** avec swipe gauche/droite pour changer l'affichage central
- 🕐 **Affichage central rotatif**: Heure / Prix BTC / Hashrate total
- 🌐 **Configuration WiFi** via portail captif au premier démarrage

## 🛠️ Matériel requis

### Écran
- **ESP32-4827S043** (ESP32-S3 avec écran 4.3" 800x480 tactile)
  - Contrôleur d'affichage: ST7262
  - Contrôleur tactile: GT911
  - Interface: I2C pour le tactile
  
### Mineurs
- Un ou plusieurs **Bitaxe** (Supra, Ultra, Max, Hex, etc.)
- Tous les mineurs doivent être accessibles sur le même réseau local

## 📦 Installation

### 1. Prérequis logiciels

- [Visual Studio Code](https://code.visualstudio.com/)
- [PlatformIO Extension](https://platformio.org/install/ide?install=vscode)

### 2. Clone du projet

```bash
git clone https://github.com/votre-repo/bitaxe-monitor.git
cd bitaxe-monitor
```

### 3. Configuration de TFT_eSPI

**IMPORTANT**: Copiez le fichier `User_Setup.h` dans le dossier de la bibliothèque TFT_eSPI:

```bash
# Windows
cp User_Setup.h %USERPROFILE%\.platformio\lib\TFT_eSPI\User_Setup.h

# Linux/Mac
cp User_Setup.h ~/.platformio/lib/TFT_eSPI/User_Setup.h
```

### 4. Configuration des mineurs

Éditez le fichier `include/config.h` et ajoutez vos mineurs:

```cpp
const MinerConfig MINERS[] = {
    {"192.168.1.100", "Bitaxe-Supra-1"},
    {"192.168.1.101", "Bitaxe-Ultra-2"},
    {"192.168.1.102", "Bitaxe-Max-3"},
    // Ajoutez autant de mineurs que nécessaire
};
```

### 5. Paramètres optionnels

Dans `include/config.h`, vous pouvez ajuster:

```cpp
// Seuils de température (°C)
#define TEMP_WARNING_THRESHOLD 75.0
#define TEMP_CRITICAL_THRESHOLD 85.0

// Intervalles de mise à jour (millisecondes)
#define UPDATE_MINERS_INTERVAL 5000    // Mise à jour mineurs
#define UPDATE_MEMPOOL_INTERVAL 30000  // Mise à jour blockchain
#define UPDATE_DISPLAY_INTERVAL 1000   // Rafraîchissement écran

// Configuration fuseau horaire
#define GMT_OFFSET 3600      // GMT+1 (France)
#define DAYLIGHT_OFFSET 3600 // Heure d'été
```

### 6. Compilation et upload

```bash
# Dans PlatformIO
pio run -t upload -t monitor

# Ou utilisez les boutons dans VSCode:
# - Upload (flèche vers la droite)
# - Monitor (prise électrique)
```

## 🚀 Premier démarrage

1. **Allumez l'ESP32** - L'écran devrait s'allumer
2. **Connexion WiFi**:
   - Cherchez le réseau WiFi "BitaxeMonitor-Setup"
   - Connectez-vous (pas de mot de passe)
   - Une page web s'ouvre automatiquement
   - Sélectionnez votre réseau WiFi et entrez le mot de passe
   - L'ESP32 redémarre et se connecte

3. **Interface**:
   - L'interface futuriste s'affiche
   - Après quelques secondes, les données apparaissent
   - Swipez l'écran pour changer l'affichage central

## 🎮 Utilisation

### Navigation

- **Swipe vers la droite** ➡️ Affichage suivant
- **Swipe vers la gauche** ⬅️ Affichage précédent

### Zones d'affichage

```
┌──────────────────────────────────────────┐
│     ⛏ BITAXE COMMAND CENTER ⛏          │ ← Barre supérieure
├──────────────────────────────────────────┤
│ ┌─────────┐  ┌──────────────┐ ┌────────┐│
│ │MINERS   │  │   CENTRAL    │ │BITCOIN ││
│ │STATUS   │  │   DISPLAY    │ │NETWORK ││
│ │         │  │ (swipe ◄►)   │ │        ││
│ │Active:3 │  │  12:34:56    │ │$42,000 ││
│ │Total GH │  │              │ │Block   ││
│ │Temp: 65°│  │              │ │Fees    ││
│ └─────────┘  └──────────────┘ └────────┘│
│                                          │
│ ⚠ Status / Alertes                      │ ← Barre inférieure
└──────────────────────────────────────────┘
```

### Affichages centraux (rotation par swipe)

1. **🕐 HEURE** - Affichage de l'heure en gros
2. **💰 PRIX** - Prix du Bitcoin en dollars
3. **⚡ HASHRATE** - Hashrate total de tous vos mineurs

### Codes couleur

- 🔴 **Rouge vif** - Titres, bordures, alertes critiques
- ⚫ **Noir** - Fond principal
- ⬜ **Blanc** - Texte normal
- 🟠 **Orange** - Température élevée (>75°C)
- 🔴 **Rouge clignotant** - Mineur hors ligne ou température critique (>85°C)

## 🔧 API utilisées

### Bitaxe API

L'interface interroge l'API locale de chaque Bitaxe:
```
http://[IP_MINEUR]/api/system/info
```

Réponse attendue:
```json
{
  "hashRate": 500.5,
  "temp": 65.0,
  "power": 15.2,
  "voltage": 1200,
  "current": 2100,
  "fanSpeed": 3000,
  "uptimeSeconds": 86400
}
```

### Mempool.space API

- **Prix**: `https://mempool.space/api/v1/prices`
- **Hauteur de bloc**: `https://mempool.space/api/blocks/tip/height`
- **Frais**: `https://mempool.space/api/v1/fees/recommended`

## 🐛 Dépannage

### L'écran reste noir

1. Vérifiez l'alimentation (5V 2A minimum)
2. Vérifiez le câble USB
3. Regardez les messages du moniteur série (115200 bauds)

### Le tactile ne fonctionne pas

1. Vérifiez les connexions I2C (SDA=19, SCL=20)
2. Redémarrez l'ESP32
3. Vérifiez dans le moniteur série: "GT911 initialisé avec succès"

### Pas de données des mineurs

1. Vérifiez que les IPs sont correctes dans `config.h`
2. Vérifiez que les mineurs sont allumés et connectés au réseau
3. Testez l'accès manuellement: `http://[IP_MINEUR]/api/system/info`
4. Regardez les logs dans le moniteur série

### Pas de connexion WiFi

1. Reconnectez-vous au portail "BitaxeMonitor-Setup"
2. Effacez la configuration WiFi en maintenant le bouton BOOT au démarrage
3. Vérifiez le mot de passe WiFi

## 📊 Structure du projet

```
bitaxe_monitor/
├── platformio.ini          # Configuration PlatformIO
├── include/
│   ├── lv_conf.h          # Configuration LVGL
│   ├── config.h           # Configuration mineurs et paramètres
│   └── GT911.h            # Driver tactile GT911
├── src/
│   └── main.cpp           # Code principal
├── User_Setup.h           # Configuration TFT_eSPI
└── README.md              # Ce fichier
```

## 🎨 Personnalisation

### Modifier les couleurs

Dans `src/main.cpp`, ajustez les couleurs:

```cpp
#define COLOR_BG lv_color_hex(0x0A0A0A)      // Fond noir
#define COLOR_RED lv_color_hex(0xFF0000)     // Rouge principal
#define COLOR_DARK_RED lv_color_hex(0x8B0000) // Rouge foncé
#define COLOR_CRIMSON lv_color_hex(0xDC143C)  // Crimson
```

### Ajouter des animations

Utilisez la bibliothèque LVGL pour créer vos propres animations:

```cpp
lv_anim_t anim;
lv_anim_init(&anim);
lv_anim_set_var(&anim, mon_objet);
lv_anim_set_time(&anim, 1000);
lv_anim_start(&anim);
```

## 🤝 Contribution

Les contributions sont les bienvenues! N'hésitez pas à:
- 🐛 Signaler des bugs
- 💡 Proposer des fonctionnalités
- 🔧 Soumettre des pull requests

## 📜 Licence

MIT License - Voir le fichier LICENSE pour plus de détails

## 🙏 Remerciements

- **Bitaxe Team** pour les excellents mineurs open-source
- **mempool.space** pour l'API blockchain gratuite
- **LVGL** pour la bibliothèque graphique
- **Espressif** pour l'ESP32-S3

## 📞 Support

- 💬 Discord: [Lien vers votre Discord]
- 📧 Email: [votre@email.com]
- 🐙 GitHub Issues: [Lien vers les issues]

---

⚡ **Fait avec ❤️ pour la communauté Bitcoin et Bitaxe** ⚡

🔴⚫ **Design futuriste ultra-badass** 🔴⚫
