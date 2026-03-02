# 🔧 Hardware Specification - SeedSigner ESP32-S3

## Recommended Platform: M5Stack CoreS3

### Pourquoi le M5Stack CoreS3 ?

| Critère | M5Stack CoreS3 | LilyGo T-Display S3 Pro |
|---------|----------------|------------------------|
| **Intégration** | ⭐⭐⭐⭐⭐ Tout intégré | Caméra externe requise |
| **Écran** | 2.0" 320×240 tactile capacitif | 2.33" 222×480 (meilleure résolution) |
| **Caméra** | GC0308 intégrée | OV5640 externe (meilleure qualité) |
| **NFC** | Port Grove I2C prêt | Nécessite câblage custom |
| **Batterie** | 500mAh intégrée | Option externe |
| **Carcasse** | Design industriel disponible | DIY uniquement |
| **Prix** | ~$45-55 | ~$25-35 |
| **Communauté** | Grande (M5Stack) | Moyenne |

**Verdict** : M5Stack CoreS3 pour un produit fini professionnel, LilyGo pour un projet DIY économique.

---

## 📋 Bill of Materials (M5Stack CoreS3)

### Core Components

| Item | Part Number | Supplier | Price | Qty |
|------|-------------|----------|-------|-----|
| **M5Stack CoreS3** | K128 | [M5Stack](https://shop.m5stack.com) | $43.90 | 1 |
| **M5Stack Unit NFC** | U001-C | [M5Stack](https://shop.m5stack.com) | $14.90 | 1 |
| **NTAG 424 DNA Tags** | Various | [GoToTags](https://store.gototags.com) | $1-2/tag | 5 |
| **MicroSD Card** | 16GB Class 10 | Any | $5 | 1 |
| **USB-C Cable** | 1m | Any | $3 | 1 |

### Optionnel / Sécurité

| Item | Description | Price |
|------|-------------|-------|
| **Faraday Pouch** | Protection NFC | $5 |
| **Security Seal Stickers** | Tamper-evident | $2 |
| **Hard Case** | Pelican style | $15 |

**Total BOM** : ~$70-85 pour version complète

---

## 🔌 Pinout & Connections

### M5Stack CoreS3 GPIO

```
┌─────────────────────────────────────────────┐
│           M5Stack CoreS3 Pinout             │
├─────────────────────────────────────────────┤
│                                             │
│  Grove Port (I2C) ───────► NFC PN532        │
│  - SDA: GPIO 12                            │
│  - SCL: GPIO 11                            │
│  - 5V / 3.3V                               │
│                                             │
│  SD Card (SPI):                            │
│  - MISO: GPIO 35                           │
│  - MOSI: GPIO 37                           │
│  - CLK:  GPIO 36                           │
│  - CS:   GPIO 4                            │
│                                             │
│  Display (Internal):                       │
│  - Driver: ILI9342C                        │
│  - Resolution: 320×240                     │
│                                             │
│  Touch (Internal):                         │
│  - Driver: FT6336U                         │
│  - I2C: 0x38                               │
│                                             │
│  Camera (Internal):                        │
│  - Sensor: GC0308                          │
│  - I2C: 0x21                               │
│  - DVP Interface                           │
│                                             │
└─────────────────────────────────────────────┘
```

### NFC Module Connection

```
M5Stack CoreS3 Grove Port ───────► M5Stack Unit NFC
┌─────────┐                       ┌───────────┐
│ GND     ├──────────────────────►│ GND       │
│ 5V      ├──────────────────────►│ 5V        │
│ SDA(12) ├──────────────────────►│ SDA       │
│ SCL(11) ├──────────────────────►│ SCL       │
└─────────┘                       └───────────┘
```

---

## 🛡️ Security Hardening

### Hardware Modifications

#### 1. WiFi/BT Physical Disable (Optionnel mais recommandé)

**Option A: Firmware disable (suffisant pour la plupart)**
```cpp
// Dans setup_security()
esp_wifi_stop();
esp_wifi_deinit();
esp_bt_controller_disable();
```

**Option B: Physical removal (avancé)**
- Désouder le module WiFi/BT du PCB
- **Risque**: Peut endommager la carte

#### 2. Tamper Detection

Ajouter un switch de tamper sur le boîtier:
```cpp
#define TAMPER_PIN GPIO_NUM_5

void setup_tamper() {
    pinMode(TAMPER_PIN, INPUT_PULLUP);
    attachInterrupt(TAMPER_PIN, tamper_handler, FALLING);
}

void tamper_handler() {
    // Wipe all secrets immediately
    emergency_wipe_all_memory();
    ESP.restart();
}
```

#### 3. Secure Boot & Flash Encryption

```bash
# Enable secure boot
python -m esptool --chip esp32s3 burn_efuse ABS_DONE_1

# Enable flash encryption
python -m esptool --chip esp32s3 burn_efuse FLASH_CRYPT_CNT

# Configure read protection
python -m esptool --chip esp32s32 burn_efuse RD_DIS
```

⚠️ **Attention**: Ces opérations sont irréversibles !

---

## 🔋 Power Management

### Battery Life Estimation

| Mode | Consumption | Autonomie (500mAh) |
|------|-------------|-------------------|
| Active (max brightness) | ~150mA | ~3h |
| Active (50% brightness) | ~100mA | ~5h |
| Idle (screen on) | ~80mA | ~6h |
| Deep sleep | ~0.5mA | ~40 jours |

### Power Optimization

```cpp
void enter_low_power_mode() {
    // Dim display
    M5.Display.setBrightness(50);
    
    // Disable camera
    g_camera->deinit();
    
    // Reduce CPU frequency
    setCpuFrequencyMhz(80);
    
    // Sleep NFC
    g_nfc->sleep();
}

void enter_deep_sleep() {
    // Wipe secrets before sleep
    clear_seed();
    
    // Configure wake sources
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);  // Power button
    esp_sleep_enable_timer_wakeup(300 * 1000000); // 5 min timeout
    
    // Enter deep sleep
    esp_deep_sleep_start();
}
```

---

## 📐 Enclosure Design

### Option 1: M5Stack DIN Base (Officiel)

- Référence: DinBase (inclus avec CoreS3)
- Montage: Rail DIN ou mural
- Protection: IP20

### Option 2: Custom 3D Printed

Voir `enclosures/` pour fichiers STL:
- `seedsigner_cores3_case.stl` - Boîtier compact
- `seedsigner_cores3_lid.stl` - Couvercle avec tamper switch
- `nfc_tag_holder.stl` - Porte-tag NFC

### Specifications mécaniques

```
Dimensions cibles:
- Longueur: 75mm
- Largeur: 60mm  
- Épaisseur: 25mm
- Fenêtre écran: 45×35mm
- Ouverture caméra: 8×8mm
- Slot NFC: 30×25mm (side access)
```

---

## 📊 Electrical Characteristics

### Power Supply

| Parameter | Min | Typ | Max | Unit |
|-----------|-----|-----|-----|------|
| Input Voltage | 4.5 | 5.0 | 5.5 | V |
| Battery Voltage | 3.0 | 3.7 | 4.2 | V |
| Current (active) | - | 150 | 200 | mA |
| Current (sleep) | - | 0.5 | 1.0 | mA |

### NFC Characteristics

| Parameter | Value |
|-----------|-------|
| Interface | I2C @ 100kHz |
| Voltage | 3.3V |
| Read Range | 0-5 cm |
| Supported Tags | NTAG 2xx, NTAG 424 DNA, MIFARE |

---

## 🔧 Assembly Instructions

### Étape 1: Préparation
1. Flasher le firmware sur le CoreS3
2. Tester tous les composants individuellement

### Étape 2: Assemblage NFC
1. Connecter le module NFC au port Grove
2. Fixer le module NFC à l'intérieur du boîtier
3. Vérifier la portée de lecture (~3cm)

### Étape 3: Fermeture
1. Installer le switch tamper (si utilisé)
2. Refermer le boîtier
3. Appliquer les security seals

### Étape 4: Vérification
1. Test de génération de seed
2. Test scan/affichage QR
3. Test lecture/écriture NFC
4. Test wipe mémoire

---

## 📋 Certification & Compliance

### RF Compliance
- Le CoreS3 a les certifications FCC/CE
- NFC: 13.56MHz ISM band (pas de licence requise)

### Security Certifications
- NTAG 424 DNA: CC EAL4+
- ESP32-S3: Supports Secure Boot, Flash Encryption

### DIY vs Commercial
- **Usage personnel**: Aucune certification requise
- **Vente commerciale**: Nécessite certification FCC/CE complète

---

## 🛠️ Troubleshooting

### Problèmes courants

| Symptôme | Cause probable | Solution |
|----------|---------------|----------|
| NFC ne lit pas | Mauvaise orientation | Tag à 90° de l'antenne |
| QR flou | Focus caméra | Nettoyer lentille |
| Boot loop | Flash corrompu | Reflasher avec `erase_flash` |
| Touch non réactif | Calibration | Recalibrer via settings |
| Battery vide | Deep sleep non configuré | Vérifier `enter_deep_sleep()` |

### Diagnostic UART

```bash
# Monitor debug output
pio device monitor -b 115200 --raw
```

Messages clés:
- `[SECURITY]` - Événements sécurité
- `[NFC]` - Opérations NFC
- `[SEED]` - Génération/chargement seed
- `[SIGN]` - Opérations de signature
