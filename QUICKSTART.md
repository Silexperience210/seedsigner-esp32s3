# 🚀 Guide de démarrage rapide - Bitaxe Monitor

## ⚡ Installation en 5 minutes

### 1️⃣ Installer VSCode + PlatformIO
```bash
# Télécharger VSCode: https://code.visualstudio.com/
# Installer l'extension PlatformIO depuis VSCode
```

### 2️⃣ Cloner et ouvrir le projet
```bash
git clone https://github.com/votre-repo/bitaxe-monitor.git
cd bitaxe-monitor
code .
```

### 3️⃣ Configurer TFT_eSPI

**Sur Windows:**
```powershell
copy User_Setup.h %USERPROFILE%\.platformio\lib\TFT_eSPI\User_Setup.h
```

**Sur Linux/Mac:**
```bash
cp User_Setup.h ~/.platformio/lib/TFT_eSPI/User_Setup.h
```

### 4️⃣ Configurer vos mineurs

Ouvrez `include/config.h` et modifiez:

```cpp
const MinerConfig MINERS[] = {
    {"192.168.1.100", "Mon-Bitaxe-1"},  // ← Changez l'IP
    {"192.168.1.101", "Mon-Bitaxe-2"},  // ← Ajoutez vos mineurs
};
```

💡 **Astuce**: Pour trouver l'IP de vos Bitaxe:
- Connectez-vous à l'interface web de votre routeur
- Cherchez les appareils nommés "bitaxe"
- Ou utilisez: `arp -a` dans le terminal

### 5️⃣ Compiler et uploader

Dans VSCode avec PlatformIO:
1. Cliquez sur l'icône ✓ (Build)
2. Attendez la fin de la compilation
3. Cliquez sur ➡️ (Upload)
4. Branchez votre ESP32-4827S043 en USB
5. Attendez l'upload

## 📱 Premier lancement

1. **L'écran s'allume** ✅
2. **WiFi**: Connectez-vous à "BitaxeMonitor-Setup"
3. **Page web**: Sélectionnez votre WiFi et entrez le mot de passe
4. **Redémarrage**: L'ESP32 redémarre automatiquement
5. **Interface**: L'interface futuriste apparaît! 🎉

## 🎮 Utilisation rapide

### Gestes tactiles
- **Swipe → (droite)**: Heure → Prix → Hashrate
- **Swipe ← (gauche)**: Hashrate → Prix → Heure

### Indicateurs
- 🟢 **Vert/Blanc**: Tout va bien
- 🟠 **Orange**: Température élevée (>75°C)
- 🔴 **Rouge clignotant**: Alerte! (mineur hors ligne ou >85°C)
- ⚡ **Flash rouge**: Nouveau bloc trouvé!

## 🐛 Problèmes courants

### ❌ Écran noir
```
Solution: Vérifier alimentation 5V 2A + câble USB
```

### ❌ "GT911 non détecté"
```
Solution: Redémarrer l'ESP32, vérifier les connexions tactiles
```

### ❌ Pas de données mineurs
```
Solution 1: Vérifier les IPs dans config.h
Solution 2: Tester dans un navigateur: http://[IP]/api/system/info
Solution 3: Regarder le moniteur série (Ctrl+Alt+M)
```

### ❌ Erreur de compilation TFT_eSPI
```
Solution: Copier User_Setup.h dans le dossier TFT_eSPI
Chemin: ~/.platformio/lib/TFT_eSPI/User_Setup.h
```

## 📊 Moniteur série

Pour voir les logs en temps réel:
```bash
# Dans PlatformIO
pio device monitor -b 115200

# Ou dans VSCode: Icône "prise électrique"
```

Vous devriez voir:
```
Démarrage Bitaxe Monitor...
WiFi connecté!
IP: 192.168.1.150
GT911 initialisé avec succès
Interface créée avec succès!
Mineur Bitaxe-1: 500.50 GH/s, 65.2°C
```

## 🎨 Personnalisation rapide

### Changer les couleurs

Dans `src/main.cpp` ligne ~20:
```cpp
#define COLOR_RED lv_color_hex(0xFF0000)      // Rouge
#define COLOR_DARK_RED lv_color_hex(0x8B0000) // Rouge foncé
```

Remplacez par vos couleurs préférées:
- Bleu: `0x0000FF`
- Vert: `0x00FF00`
- Violet: `0x9400D3`
- Cyan: `0x00FFFF`

### Ajuster les intervalles de mise à jour

Dans `include/config.h`:
```cpp
#define UPDATE_MINERS_INTERVAL 5000    // 5 sec → changez selon vos besoins
#define UPDATE_MEMPOOL_INTERVAL 30000  // 30 sec
```

### Modifier les seuils d'alerte

Dans `include/config.h`:
```cpp
#define TEMP_WARNING_THRESHOLD 75.0   // Alerte orange
#define TEMP_CRITICAL_THRESHOLD 85.0  // Alerte rouge
```

## 🔥 Fonctionnalités avancées

### OTA (Over-The-Air) Update
```cpp
// À venir dans la prochaine version!
```

### Logs sur carte SD
```cpp
// À venir dans la prochaine version!
```

### Mode nuit automatique
```cpp
// À venir dans la prochaine version!
```

## 💡 Astuces pro

1. **Fixez les IPs de vos Bitaxe** dans votre routeur (DHCP statique)
2. **Utilisez un bon câble USB** pour l'ESP32 (qualité = stabilité)
3. **Alimentation 5V 2A minimum** pour l'écran
4. **Vérifiez la ventilation** de l'écran en fonctionnement prolongé

## 📞 Besoin d'aide?

- 📖 [README complet](README.md)
- 🐛 [Signaler un bug](https://github.com/votre-repo/issues)
- 💬 Discord: [Lien]
- 📧 Email: [contact]

---

**Profitez de votre monitoring ultra-futuriste!** 🔥⚡🔴
