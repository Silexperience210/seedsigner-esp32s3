# 🔌 Câblage et spécifications matérielles

## 📋 ESP32-4827S043 - Spécifications

### Microcontrôleur
- **Modèle**: ESP32-S3-WROOM-1
- **CPU**: Dual-core Xtensa LX7 @ 240MHz
- **RAM**: 512KB SRAM + 8MB PSRAM
- **Flash**: 16MB
- **WiFi**: 802.11 b/g/n (2.4GHz)
- **Bluetooth**: BLE 5.0

### Écran LCD
- **Taille**: 4.3 pouces
- **Résolution**: 800x480 pixels
- **Contrôleur**: ST7262 (RGB)
- **Interface**: RGB 16-bit
- **Luminosité**: Réglable via GPIO 2

### Écran tactile
- **Type**: Capacitif 5 points
- **Contrôleur**: GT911
- **Interface**: I2C
- **Résolution tactile**: 800x480

## 🔌 Pinout ESP32-4827S043

### LCD (RGB Interface)
```
LCD_BL     → GPIO 2   (Rétroéclairage / PWM)
LCD_DE     → GPIO 40
LCD_VSYNC  → GPIO 41
LCD_HSYNC  → GPIO 39
LCD_CLK    → GPIO 42
LCD_R0     → GPIO 45
LCD_R1     → GPIO 48
LCD_R2     → GPIO 47
LCD_R3     → GPIO 21
LCD_R4     → GPIO 14
LCD_G0     → GPIO 5
LCD_G1     → GPIO 6
LCD_G2     → GPIO 7
LCD_G3     → GPIO 15
LCD_G4     → GPIO 16
LCD_G5     → GPIO 4
LCD_B0     → GPIO 8
LCD_B1     → GPIO 3
LCD_B2     → GPIO 46
LCD_B3     → GPIO 9
LCD_B4     → GPIO 1
```

### Touch (I2C GT911)
```
TOUCH_SDA  → GPIO 19
TOUCH_SCL  → GPIO 20
TOUCH_INT  → GPIO 4
TOUCH_RST  → GPIO 38
```

### SPI (Alternative - Non utilisé dans ce projet)
```
MOSI       → GPIO 11
MISO       → GPIO 13
SCK        → GPIO 12
CS         → GPIO 10
DC         → GPIO 13
RST        → GPIO 14
```

### Autres GPIO disponibles
```
GPIO 17, 18  → Libres (peuvent être utilisés pour I2C externe)
GPIO 35, 36  → Libres
GPIO 37      → Bouton BOOT (interne)
```

## 🔋 Alimentation

### Spécifications
- **Tension d'entrée**: 5V DC
- **Courant requis**: 
  - Minimum: 1A
  - Recommandé: 2A
  - Maximum: 2.5A
- **Connecteur**: USB-C
- **Consommation**:
  - ESP32 seul: ~200mA
  - Écran LCD + rétroéclairage: ~500-800mA
  - Total typique: ~1A

### ⚠️ Attention
- Utilisez un câble USB-C de qualité
- Évitez les câbles "charge only" sans données
- Préférez une alimentation murale de 5V 2A
- Les ports USB d'ordinateur peuvent être insuffisants

## 🌡️ Capteurs additionnels (optionnels)

### DHT22 - Température/Humidité ambiante
```
DHT22 VCC  → 3.3V
DHT22 DATA → GPIO 35 (libre)
DHT22 GND  → GND
```

### DS18B20 - Température (1-Wire)
```
DS18B20 VCC  → 3.3V
DS18B20 DATA → GPIO 36 (libre) + Pull-up 4.7kΩ vers 3.3V
DS18B20 GND  → GND
```

### Carte microSD (optionnelle)
```
SD_CS   → GPIO 5
SD_MOSI → GPIO 11
SD_MISO → GPIO 13
SD_CLK  → GPIO 12
SD_VCC  → 3.3V
SD_GND  → GND
```

## 🔊 Buzzer (optionnel pour alertes sonores)
```
BUZZER+ → GPIO 17 (libre)
BUZZER- → GND
```

## 🛠️ Câblage réseau

### Configuration réseau recommandée

```
Internet
   |
   |
[Routeur WiFi]
   |
   +--- WiFi --- [ESP32-4827S043 Monitor]
   |
   +--- Ethernet/WiFi --- [Bitaxe #1]
   |
   +--- Ethernet/WiFi --- [Bitaxe #2]
   |
   +--- Ethernet/WiFi --- [Bitaxe #3]
```

### IPs recommandées (statiques)
```
Routeur:    192.168.1.1
Monitor:    192.168.1.150  (statique recommandé)
Bitaxe-1:   192.168.1.100  (statique recommandé)
Bitaxe-2:   192.168.1.101  (statique recommandé)
Bitaxe-3:   192.168.1.102  (statique recommandé)
```

## 📦 Boîtier recommandé

### Dimensions
- **Largeur**: 110mm minimum
- **Hauteur**: 70mm minimum
- **Profondeur**: 20mm minimum

### Caractéristiques souhaitées
- ✅ Trous pour ventilation
- ✅ Accès au port USB-C
- ✅ Support mural ou sur pied
- ✅ Matériau: ABS ou acrylique
- ✅ Protection contre la poussière

### Fichiers 3D (à créer)
```
/3d_models/
  ├── case_bottom.stl
  ├── case_top.stl
  ├── stand.stl
  └── wall_mount.stl
```

## 🌡️ Gestion thermique

### Températures de fonctionnement
- **ESP32**: -40°C à +85°C
- **Écran LCD**: -20°C à +70°C
- **Optimal**: 15°C à 35°C

### Refroidissement
- Ventilation passive recommandée
- Éviter l'exposition directe au soleil
- Ne pas enfermer hermétiquement
- Distance minimum de 5cm autour de l'appareil

## 🔧 Assemblage étape par étape

### 1. Vérification du matériel
```
☐ ESP32-4827S043
☐ Câble USB-C de qualité
☐ Alimentation 5V 2A
☐ (Optionnel) Boîtier imprimé 3D
☐ (Optionnel) Capteurs additionnels
```

### 2. Test initial
```bash
1. Brancher l'ESP32 à l'ordinateur
2. Ouvrir le moniteur série (115200 bauds)
3. Vérifier les messages de démarrage
4. Vérifier l'allumage de l'écran
5. Tester le tactile
```

### 3. Installation dans le boîtier
```
1. Fixer l'ESP32 avec vis M2.5
2. Faire passer le câble USB par l'ouverture
3. Installer le couvercle
4. Vérifier l'accès au bouton BOOT (si nécessaire)
```

### 4. Montage final
```
1. Choisir l'emplacement (bureau, mur)
2. Installer le support
3. Vérifier la visibilité de l'écran
4. Vérifier l'accès au port USB
5. Vérifier la ventilation
```

## 🔍 Tests de validation

### Liste de vérification
```
☐ Écran s'allume
☐ Tactile fonctionne
☐ WiFi se connecte
☐ GT911 détecté
☐ Données des Bitaxe affichées
☐ Prix Bitcoin mis à jour
☐ Swipe fonctionne
☐ Alertes fonctionnent
☐ Pas de surchauffe après 1h
```

## 📊 Consommation électrique

### Mesures typiques
```
Mode veille:          50mA   @ 5V  = 0.25W
Écran actif (dim):    400mA  @ 5V  = 2W
Écran actif (max):    800mA  @ 5V  = 4W
WiFi en transmission: 200mA  @ 5V  = 1W
```

### Coût annuel (estimation)
```
Consommation moyenne: 2.5W
Heures par jour: 24h
Jours par an: 365
Prix kWh: 0.15€

Coût annuel = 2.5W * 24h * 365 / 1000 * 0.15€
           = 3.29€/an
```

## 🛡️ Protection et sécurité

### Circuit de protection (intégré)
- ✅ Protection contre les surtensions
- ✅ Protection contre les courts-circuits
- ✅ Protection thermique

### Recommandations
- 🔸 Ne pas exposer à l'eau
- 🔸 Ne pas démonter
- 🔸 Utiliser une alimentation certifiée
- 🔸 Ne pas obstruer les aérations

## 📐 Schéma simplifié

```
         ┌─────────────────────────────────┐
         │     ESP32-S3-WROOM-1           │
         │                                 │
         │  ┌──────────────┐              │
         │  │   CPU Core   │              │
USB-C ───┼──│ Dual LX7     │──┐           │
5V 2A    │  │  @ 240MHz    │  │           │
         │  └──────────────┘  │           │
         │          │          │           │
         │     ┌────┴────┐    │           │
         │     │  8MB    │    │           │
         │     │ PSRAM   │    │           │
         │     └─────────┘    │           │
         │                    │           │
         │  ┌─────────────────┴────┐      │
         │  │   RGB Interface      │      │
GPIO ────┼──│   16-bit parallel    │──────┼──── LCD 800x480
         │  └──────────────────────┘      │     ST7262
         │                                 │
         │  ┌──────────────────┐          │
I2C ─────┼──│  I2C Interface   │──────────┼──── Touch GT911
SDA/SCL  │  │  SDA=19 SCL=20   │          │
         │  └──────────────────┘          │
         │                                 │
         │  ┌──────────────────┐          │
         │  │   WiFi Module    │          │
         │  │   2.4GHz b/g/n   │)))       │
         │  └──────────────────┘          │
         └─────────────────────────────────┘
```

## 🔗 Liens utiles

### Documentation officielle
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [GT911 Datasheet](https://www.displayfuture.com/Display/datasheet/controller/GT911.pdf)
- [Bitaxe Documentation](https://github.com/skot/ESP-Miner)

### Communauté
- [Discord Bitaxe](https://discord.gg/bitaxe)
- [Forum ESP32](https://esp32.com/)
- [LVGL Forum](https://forum.lvgl.io/)

---

**Bon montage! ⚡🔧**
