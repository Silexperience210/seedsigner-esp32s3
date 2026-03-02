# 🔐 Security Architecture - SeedSigner ESP32-S3

> **"Trust but verify"** - Cette documentation décrit les mesures de sécurité implémentées et celles qui restent à l'utilisateur de mettre en œuvre.

---

## 🎯 Threat Model

### Attaquants considérés

| Niveau | Capacités | Exemples |
|--------|-----------|----------|
| **Opportuniste** | Accès physique limité, pas de compétences techniques | Voleur de sac |
| **Physique** | Accès prolongé, outils basiques | Colocataire, employé |
| **Avancé** | Accès étendu, équipement de labo | Agence gouvernementale |
| **Supply Chain** | Accès avant livraison | Fabricant malveillant |

### Actifs à protéger

1. **Clés privées Bitcoin** (valeur maximale)
2. **Seed mnemonic** (récupération)
3. **Passphrase** (dérivation supplémentaire)
4. **Transactions signées** (preuve de possession)

---

## 🛡️ Layers de Sécurité

```
┌─────────────────────────────────────────────────────────────────┐
│ Layer 7: Application (SeedSigner firmware)                     │
│  - Stateless operation, no persistent keys                      │
│  - Memory wiping, secure allocators                            │
├─────────────────────────────────────────────────────────────────┤
│ Layer 6: Operating System (FreeRTOS on ESP32-S3)               │
│  - Task isolation, stack guards                                 │
│  - Watchdog timer, secure boot                                 │
├─────────────────────────────────────────────────────────────────┤
│ Layer 5: Hardware Abstraction (ESP-IDF/Arduino)                │
│  - RNG hardware, crypto accelerators                           │
│  - Flash encryption, efuse protection                          │
├─────────────────────────────────────────────────────────────────┤
│ Layer 4: Physical (M5Stack CoreS3)                             │
│  - No WiFi/BT (air-gap)                                         │
│  - Tamper detection (optionnel)                                │
├─────────────────────────────────────────────────────────────────┤
│ Layer 3: User Interface                                        │
│  - Transaction verification sur écran                          │
│  - PIN pour NFC unlock                                          │
├─────────────────────────────────────────────────────────────────┤
│ Layer 2: Storage (NFC NTAG 424)                                │
│  - AES-128 encryption                                           │
│  - SUN authentication                                           │
├─────────────────────────────────────────────────────────────────┤
│ Layer 1: Backup (SeedQR physique)                              │
│  - Air-gapped, offline                                          │
│  - Tamper-evident storage                                      │
└─────────────────────────────────────────────────────────────────┘
```

---

## 🔒 Stateless Operation (Amnésie)

### Principe

```
┌────────────────────────────────────────────────────────────┐
│                     STATELESS DESIGN                        │
├────────────────────────────────────────────────────────────┤
│                                                             │
│   BOOT ───────► LOAD SEED ───────► USE ───────► WIPE       │
│     │               │                │            │         │
│     ▼               ▼                ▼            ▼         │
│  ┌─────┐      ┌─────────┐      ┌─────────┐   ┌──────┐     │
│  │Clear│      │Manual   │      │Sign TX  │   │Clear │     │
│  │RAM  │      │Entry/NFC│      │Export XPUB   │RAM   │     │
│  └─────┘      └─────────┘      └─────────┘   └──────┘     │
│                                                             │
│   ⚠️  NO PERSISTENT STORAGE OF KEYS ⚠️                     │
│                                                             │
└────────────────────────────────────────────────────────────┘
```

### Implémentation

```cpp
// RAM uniquement, jamais de flash
class SecureSeedStorage {
private:
    // En PSRAM (pas de flash wear, volatile)
    uint8_t* m_seed_buffer;  
    
public:
    void load_seed(const uint8_t* seed, size_t len) {
        // Wipe any existing seed first
        wipe();
        
        // Allocate in PSRAM (if available) or internal RAM
        m_seed_buffer = (uint8_t*)heap_caps_malloc(len, 
            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        
        if (!m_seed_buffer) {
            m_seed_buffer = (uint8_t*)malloc(len);
        }
        
        memcpy(m_seed_buffer, seed, len);
        m_seed_len = len;
    }
    
    void wipe() {
        if (m_seed_buffer) {
            // Secure wipe (anti-optimizer)
            volatile uint8_t* p = m_seed_buffer;
            for (size_t i = 0; i < m_seed_len; i++) {
                p[i] = 0;
            }
            free(m_seed_buffer);
            m_seed_buffer = nullptr;
        }
    }
    
    ~SecureSeedStorage() {
        wipe();  // Auto-wipe on destruction
    }
};
```

### Auto-Wipe Triggers

| Événement | Action |
|-----------|--------|
| Power loss | RAM clears naturally |
| Inactivité 5min | Deep sleep + wipe |
| Tamper detect | Immediate wipe + reboot |
| User "Lock" | Wipe + return to main menu |
| Signature complète | Optional wipe |

---

## 🔐 Entropy & RNG

### Sources d'entropie

```cpp
class EntropyCollector {
public:
    // 1. Hardware RNG (ESP32-S3 HMAC-SHA based)
    bool get_hardware_random(uint8_t* output, size_t len) {
        // ESP32-S3 RNG is hardware-based, not pseudo-random
        for (size_t i = 0; i < len; i += 4) {
            uint32_t random = esp_random();
            memcpy(output + i, &random, min(4, len - i));
        }
        return true;
    }
    
    // 2. Camera sensor noise
    bool get_camera_entropy(uint8_t* output, size_t len) {
        // Capture frame with lens covered (dark frame)
        // Use LSB of pixel values as entropy
        CameraFrame frame;
        g_camera->capture(&frame);
        
        // Hash pixel noise
        sha256(frame.data, frame.len, output);
        return true;
    }
    
    // 3. User timing (touch events)
    void add_timing_entropy() {
        uint32_t micros = micros();
        // Use low bits of timing
        m_entropy_pool[m_pool_index++] = micros & 0xFF;
    }
};
```

### Entropy Quality

| Source | Bits/s | Trust Level |
|--------|--------|-------------|
| Hardware RNG | ~500K | High (chip hardware) |
| Camera noise | ~100K | High (physical process) |
| Touch timing | ~10 | Medium (user-dependent) |
| Combined | - | Very High |

### Dice Roll Entropy

```cpp
// Convert physical dice to cryptographic entropy
// D6: 2.585 bits/roll
// 50 rolls (D6) = 128 bits
// 99 rolls (D6) = 256 bits

bool generate_from_dice(const uint8_t* rolls, size_t num_rolls, 
                        uint8_t* entropy_out) {
    // Base-6 encoding with rejection sampling
    // to remove bias from imperfect dice
    
    uint64_t accumulator = 0;
    uint8_t bits = 0;
    size_t out_idx = 0;
    
    for (size_t i = 0; i < num_rolls; i++) {
        // Each roll: 0-5
        accumulator = accumulator * 6 + (rolls[i] - 1);
        bits += 3;  // log2(6) ≈ 2.585, we take 3 and compress
        
        // Extract full bytes
        while (bits >= 8 && out_idx < 32) {
            entropy_out[out_idx++] = (accumulator >> (bits - 8)) & 0xFF;
            bits -= 8;
        }
    }
    
    return out_idx >= 16;  // Need at least 128 bits
}
```

---

## 📱 NFC Security (NTAG 424 DNA)

### Pourquoi NTAG 424 DNA ?

```
┌────────────────────────────────────────────────────────────┐
│                  NTAG 424 DNA FEATURES                      │
├────────────────────────────────────────────────────────────┤
│                                                             │
│  🔐 AES-128 Encryption                                      │
│     - Keys stored in secure memory                         │
│     - EAL4+ certified silicon                              │
│                                                             │
│  🔄 Secure Unique NFC (SUN)                                 │
│     - Each read generates unique signature                 │
│     - Prevents replay attacks                              │
│                                                             │
│  📊 Counter Protection                                      │
│     - Monotonic read counter                               │
│     - Detects cloning attempts                             │
│                                                             │
│  🔒 File-based ACL                                          │
│     - Different keys for read/write                        │
│     - Configurable access rights                           │
│                                                             │
└────────────────────────────────────────────────────────────┘
```

### Key Derivation from PIN

```cpp
// PIN → AES Key pour NTAG 424
void derive_nfc_key(const char* pin, const uint8_t salt[16], 
                    uint8_t key[16]) {
    // PBKDF2-SHA256: PIN + Salt → 128-bit AES key
    mbedtls_pkcs5_pbkdf2_hmac(
        pin, strlen(pin),
        salt, 16,
        10000,  // 10k iterations
        16, key
    );
}

// Format de stockage sur NTAG 424
struct EncryptedSeedStorage {
    uint8_t magic[4];        // "SS44"
    uint8_t version;         // 0x01
    uint8_t flags;           // Options
    uint8_t salt[16];        // Random salt
    uint8_t iv[16];          // AES IV
    uint8_t encrypted[64];   // AES-128-CBC(seed)
    uint8_t mac[16];         // CMAC for integrity
    uint32_t timestamp;      // Creation time
    char label[32];          // User label
};
```

### SUN Authentication Flow

```
Device                                    NTAG 424
  │                                          │
  │ 1. Authenticate with key                 │
  │ ────────────────────────────────────────►│
  │                                          │
  │ 2. Challenge-response                    │
  │ ◄────────────────────────────────────────│
  │                                          │
  │ 3. Read encrypted seed                   │
  │ ────────────────────────────────────────►│
  │                                          │
  │ 4. Verify SUN signature                  │
  │ ◄─[UID + Counter + CMAC]─────────────────│
  │                                          │
  │ 5. Decrypt seed with derived key         │
  │                                          │
```

---

## 🔐 Air-Gap Enforcement

### Hardware Air-Gap

```cpp
class AirGap {
public:
    static void enforce() {
        #if WIFI_ENABLED == 0
        // Disable WiFi at boot
        esp_wifi_stop();
        esp_wifi_deinit();
        
        // Disable WiFi in NVS
        esp_wifi_set_storage(WIFI_STORAGE_RAM);
        #endif
        
        #if BT_ENABLED == 0
        // Disable Bluetooth
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
        #endif
        
        // Disable USB CDC after boot (optional, extreme)
        // usb_disable();
    }
    
    static bool verify() {
        // Verify WiFi is disabled
        wifi_mode_t mode;
        esp_wifi_get_mode(&mode);
        
        // Verify BT is disabled
        esp_bt_controller_status_t bt_status = esp_bt_controller_get_status();
        
        return (mode == WIFI_MODE_NULL) && 
               (bt_status == ESP_BT_CONTROLLER_STATUS_IDLE);
    }
};
```

### Communication uniquement par QR

```
Software Wallet                           SeedSigner
     │                                        │
     │ 1. Create PSBT                         │
     │ 2. Show as animated QR                 │
     │         ┌─────┐                        │
     │         │ ▄▄▄ │  ◄──── Camera scan     │
     │         │ █▄▀ │                        │
     │         └─────┘                        │
     │                                        │
     │                    3. Sign PSBT        │
     │                    4. Show signed QR   │
     │         ┌─────┐  ◄──── Camera scan     │
     │         │▀▄▀▄ │                        │
     │         │▄▀▄▀ │                        │
     │         └─────┘                        │
     │ 5. Broadcast TX                        │
```

**Avantages:**
- Aucune connexion électronique
- Visuellement vérifiable
- Unidirectionnel (pas de data leakage)

---

## 🚨 Tamper Detection

### Software Tamper Detection

```cpp
class TamperDetector {
public:
    // Vérifier intégrité du firmware
    bool verify_firmware_hash() {
        uint8_t current_hash[32];
        calculate_firmware_hash(current_hash);
        
        return (memcmp(current_hash, s_expected_hash, 32) == 0);
    }
    
    // Détecter debugging
    bool is_debugger_attached() {
        // Check ESP32 debug registers
        return esp_cpu_dbgr_is_attached();
    }
    
    // Détecter glitching
    void check_execution_timing() {
        uint32_t start = esp_cpu_get_cycle_count();
        
        // Operation connue
        volatile uint32_t x = 0;
        for (int i = 0; i < 1000; i++) x += i;
        
        uint32_t elapsed = esp_cpu_get_cycle_count() - start;
        
        // Si trop rapide = possible glitch
        if (elapsed < EXPECTED_CYCLES_MIN) {
            trigger_security_event();
        }
    }
    
private:
    void trigger_security_event() {
        // 1. Wipe secrets
        g_seed_storage.wipe();
        
        // 2. Log event (if audit log enabled)
        log_security_event(EVENT_TAMPER_DETECTED);
        
        // 3. Alert user
        show_tamper_warning();
        
        // 4. Reboot
        ESP.restart();
    }
};
```

### Hardware Tamper Detection

```cpp
// Tamper switch on enclosure
#define TAMPER_PIN GPIO_NUM_5

void IRAM_ATTR tamper_isr() {
    // Immediate wipe in ISR
    for (volatile int i = 0; i < 64; i++) {
        ((volatile uint8_t*)PSRAM_BASE)[i] = 0;
    }
    
    // Trigger watchdog
    esp_restart_noos();
}

void setup_tamper() {
    pinMode(TAMPER_PIN, INPUT_PULLUP);
    attachInterrupt(TAMPER_PIN, tamper_isr, FALLING);
}
```

---

## 📝 Security Audit Checklist

### Avant usage avec fonds réels

- [ ] **Firmware Build**
  - [ ] Build reproducible vérifié
  - [ ] Hash du firmware documenté
  - [ ] Pas de warnings de compilation

- [ ] **Hardware**
  - [ ] WiFi/BT désactivés (vérifié avec scanner)
  - [ ] Pas de composants suspects sur le PCB
  - [ ] Tamper seals appliqués

- [ ] **Software Tests**
  - [ ] Self-tests crypto passent
  - [ ] Memory wipe vérifié (dump RAM après shutdown)
  - [ ] NFC encryption fonctionne
  - [ ] QR scan/display fonctionnel

- [ ] **Procédures**
  - [ ] Seed backup testé (restauration)
  - [ ] Passphrase testée
  - [ ] Test transaction réussi (small amount)
  - [ ] Procédure de wipe d'urgence connue

### Tests de pénétration recommandés

1. **Cold boot attack**: Vérifier si data persiste en RAM refroidie
2. **Glitching**: Test de voltage/clock glitching sur signature
3. **Side-channel**: Analyse consommation pendant opérations crypto
4. **NFC sniffing**: Vérifier que SUN prévente replay

---

## 📚 Références

- [SeedSigner Security Model](https://seedsigner.com/security)
- [ESP32-S3 Security Features](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/security/security.html)
- [NTAG 424 DNA Datasheet](https://www.nxp.com/docs/en/data-sheet/NTAG424DNA.pdf)
- [BIP39](https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki)
- [BIP32](https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki)
- [PSBT BIP174](https://github.com/bitcoin/bips/blob/master/bip-0174.mediawiki)

---

**⚠️ Disclaimer**: Ce projet est fourni "as-is" sans garantie. L'utilisateur est responsable de la sécurité de ses fonds. Effectuez toujours des tests avec de petits montants avant usage en production.
