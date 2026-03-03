# 🔧 Suggestions d'Amélioration - SeedSigner ESP32-S3

## 📋 Résumé du Projet Actuel

| Composant | Status | Note |
|-----------|--------|------|
| uBitcoin/secp256k1 | ✅ | Remplace le placeholder |
| BIP39 (2048 mots) | ✅ | Wordlist complète en PROGMEM |
| BIP32 HD Wallet | ✅ | Dérivation complète |
| Bitcoin Addresses | ✅ | P2WPKH, P2SH, P2PKH |
| Bech32 | ✅ | BIP173 compliant |

---

## 🎯 Suggestions Prioritaires

### 1. **Sécurité & Hardening** 🔐

#### A. Entropie Hardware (CRITIQUE)
```cpp
// Actuel: entropie software seulement
// Recommandé: ESP32-S3 TRNG + capteurs

class HardwareRNG {
public:
    void gather_entropy() {
        // 1. TRNG hardware ESP32-S3
        uint32_t trng = esp_random();
        
        // 2. Bruit capteur caméra (GC0308)
        uint8_t* frame = capture_noise_frame();
        sha256(frame, size, hash);
        
        // 3. Timing interrupts
        uint64_t jitter = get_interrupt_timing();
        
        // 4. Combinaison via HKDF
        hkdf_extract(salt, ikm, prk);
    }
};
```

#### B. Memory Protection
```cpp
// Actuel: memset simple
// Recommandé: volatile + barrier + multiple passes

template<typename T>
void secure_zero(volatile T* ptr, size_t len) {
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    for (size_t i = 0; i < len; i++) {
        p[i] = 0;
        p[i] = 0xFF;  // Double pass
        p[i] = 0;
    }
    __asm__ volatile "" ::: "memory");  // Barrier
}
```

#### C. Stack Protection
```ini
# platformio.ini
build_flags =
    -fstack-protector-strong
    -Wstack-usage=4096
    -Wl,--wrap=malloc
    -Wl,--wrap=free
```

---

### 2. **Optimisations Performance** ⚡

#### A. Binary Search PROGMEM (Wordlist)
```cpp
// Actuel: Linear search O(n)
// Cible: Binary search O(log n)

// Créer index trié en PROGMEM
const uint16_t sorted_indices[2048] PROGMEM = {
    // Indices triés par ordre alphabétique
};

int find_word_fast(const char* word) {
    int left = 0, right = 2047;
    while (left <= right) {
        int mid = (left + right) / 2;
        uint16_t idx = pgm_read_word(&sorted_indices[mid]);
        int cmp = strcmp_P(word, wordlist_en[idx]);
        if (cmp == 0) return idx;
        if (cmp < 0) right = mid - 1;
        else left = mid + 1;
    }
    return -1;
}
```

#### B. PBKDF2 Optimisé
```cpp
// Actuel: 2048 HMAC séquentiels (lent)
// Optimisation: Blocking ou assembly

void pbkdf2_hmac_sha512_fast(const uint8_t* pw, size_t pw_len,
                              const uint8_t* salt, size_t salt_len,
                              uint32_t iterations, uint8_t* out) {
    // Utiliser ESP32-S3 SHA accelerator
    // ou implémentation assembly Xtensa
}
```

#### C. Caching Dérivations
```cpp
class KeyCache {
    struct CacheEntry {
        uint32_t path_hash;
        uint8_t key[32];
        bool valid;
    };
    
    // Cache LRU pour éviter re-dérivation
    CacheEntry cache[8];
    
public:
    bool get(const char* path, uint8_t* key);
    void put(const char* path, const uint8_t* key);
    void clear();  // On lock/sleep
};
```

---

### 3. **Fonctionnalités Manquantes** ➕

#### A. PSBT Signing (CRITIQUE)
```cpp
class PSBTSigner {
public:
    bool parse(const uint8_t* psbt, size_t len);
    bool validate();  // Vérifier inputs/outputs
    bool sign_input(size_t idx, const PrivateKey& key);
    bool finalize(uint8_t* tx_out, size_t* out_len);
    
    // UI: Afficher montants/destinataires
    struct TxSummary {
        uint64_t total_in;
        uint64_t total_out;
        uint64_t fee;
        uint8_t num_inputs;
        uint8_t num_outputs;
    };
};
```

#### B. QR Code Generation
```cpp
// Pour export xpub/addresses/PSBT signed
class QREncoder {
public:
    void encode_text(const char* text, uint8_t* bitmap, size_t size);
    void encode_binary(const uint8_t* data, size_t len, uint8_t* bitmap);
    
    // Support multi-part pour gros PSBT
    void encode_multipart(const uint8_t* data, size_t len, 
                          uint8_t* frames, int* num_frames);
};
```

#### C. NFC Support (NTAG424)
```cpp
class NFCTag {
public:
    bool detect();
    bool authenticate(const uint8_t* key);
    bool read_encrypted(uint8_t* data, size_t len);
    bool write_encrypted(const uint8_t* data, size_t len);
    
    // Format "SS44" SeedSigner
    bool write_seed_backup(const EncryptedBackup& backup);
};
```

#### D. Multi-Signature Support
```cpp
class MultisigWallet {
    struct Config {
        uint8_t m;  // Required signatures
        uint8_t n;  // Total signers
        uint8_t xpubs[16][113];  // Max 16 signers
    };
    
    bool create_address(const Config& config, uint32_t index, char* address);
    bool create_psbt(const Config& config, const TxRequest& tx, uint8_t* psbt);
};
```

---

### 4. **Interface Utilisateur** 🎨

#### A. LVGL Optimisations
```cpp
// Actuel: LVGL 9.2
// Recommandations:

// 1. Double buffering
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[320*40];  // 40 lines
static lv_disp_drv_t disp_drv;

// 2. DMA SPI pour affichage
void my_flush_cb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
    // Utiliser DMA au lieu de CPU
    spi_device_queue_trans(spi, &trans, portMAX_DELAY);
}

// 3. Thème SeedSigner
static void apply_seedsigner_theme() {
    lv_theme_t* th = lv_theme_default_init(
        disp, 
        lv_color_hex(0xF7931A),  // Orange Bitcoin
        lv_color_hex(0x1A1A1A),  // Fond noir
        true,  // Dark mode
        &lv_font_montserrat_14
    );
}
```

#### B. Écrans Essentiels
```cpp
enum Screen {
    SCREEN_WELCOME,           // Logo + "SeedSigner"
    SCREEN_NEW_WALLET,        // Créer nouveau / restaurer
    SCREEN_SEED_GENERATE,     // Génération entropie
    SCREEN_SEED_VERIFY,       // Vérification mots
    SCREEN_PASSPHRASE,        // Saisie passphrase
    SCREEN_MAIN_MENU,         // Receive / Send / Settings
    SCREEN_RECEIVE,           // QR adresse + texte
    SCREEN_SEND_SCAN,         // Scan PSBT QR
    SCREEN_SEND_CONFIRM,      // Vérification tx
    SCREEN_SEND_SIGN,         // Signature + QR résultat
    SCREEN_SETTINGS,          // Unités, réseau, etc.
    SCREEN_SHUTDOWN           // Effacement sécurisé
};
```

---

### 5. **Tests & Validation** 🧪

#### A. Test Vecteurs BIP (WIP)
```cpp
// BIP39 test vectors
const struct {
    const char* entropy_hex;
    const char* mnemonic;
    const char* seed_hex;
    const char* xprv;
} BIP39_TESTS[] = {
    {
        "00000000000000000000000000000000",
        "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about",
        "c55257c360c07c72029aebc1b53c05ed0362ada38ead3e3e9efa3708e53495531f09a6987599d18264c1e1c92f2cf141630c7a3c4ab7c81b2f001698e7463b04",
        "xprv9s21ZrQH143K3h3fDYiay8mocZ3afhfULfb5GX8xCBZGZX5MAyN4tvEJg8t72p6UhQhLArGjxviv8jXBW2x3BdMtNyNSenCaFajtvRojBC"
    },
    // ... 24 word test vector
};
```

#### B. Fuzzing Targets
```cpp
// LibFuzzer targets
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // Fuzz BIP39 validation
    char input[512];
    memcpy(input, data, min(size, sizeof(input)-1));
    input[min(size, sizeof(input)-1)] = 0;
    BIP39::validate_mnemonic(input);
    return 0;
}
```

#### C. Hardware-in-Loop
```python
# Test script Python
import serial
import hashlib

def test_signature_verification():
    esp = serial.Serial('COM3', 115200)
    
    # Envoyer message à signer
    msg_hash = hashlib.sha256(b"test").digest()
    esp.write(b"SIGN " + msg_hash.hex() + b"\n")
    
    # Vérifier réponse
    sig = esp.readline().strip()
    assert verify_ecdsa_sig(pubkey, msg_hash, sig)
```

---

### 6. **Documentation** 📚

#### A. Security Model
```markdown
## Threat Model

### Assets
- Private keys (RAM only)
- Seed phrase (user responsibility)
- PIN/passphrase (user input)

### Threats
1. **Physical theft** → PIN protection
2. **Side-channel** → Constant-time crypto
3. **Cold boot** → RAM encryption/flush
4. **Supply chain** → Reproducible builds

### Mitigations
- Air-gapped (no WiFi/BT)
- RAM-only keys (no flash storage)
- Secure boot + flash encryption
- Tamper-evident case
```

#### B. Build Instructions
```bash
# Reproducible build
docker run --rm -v $(pwd):/project \
    espressif/idf:latest \
    bash -c "cd /project && pio run"

# Verify firmware hash
sha256sum .pio/build/esp32-s3/firmware.bin
```

---

### 7. **Optimisations Mémoire** 💾

| Composant | Actuel | Cible | Méthode |
|-----------|--------|-------|---------|
| Wordlist | ~16KB | ~16KB | ✅ PROGMEM OK |
| uBitcoin | ~30KB | ~25KB | Strip unused |
| LVGL heap | 64KB | 48KB | Optim params |
| Stack | 8KB | 4KB | Reduce recursion |
| **Total RAM** | **~200KB** | **~150KB** | Libre pour UI |

---

## 📅 Roadmap Recommandée

### Phase 1: Sécurité (1-2 sem)
- [ ] Hardware RNG
- [ ] Secure memory wiping
- [ ] Stack protection

### Phase 2: Fonctionnalités (2-3 sem)
- [ ] PSBT signing
- [ ] QR generation
- [ ] UI screens

### Phase 3: Tests (1-2 sem)
- [ ] Test vectors BIP
- [ ] Fuzzing
- [ ] Hardware tests

### Phase 4: Production (1 sem)
- [ ] Documentation
- [ ] Reproducible builds
- [ ] Security audit

---

## 🔗 Ressources Utiles

- [BIP39 Spec](https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki)
- [BIP32 Spec](https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki)
- [BIP174 (PSBT)](https://github.com/bitcoin/bips/blob/master/bip-0174.mediawiki)
- [SeedSigner GitHub](https://github.com/SeedSigner/seedsigner)
- [ESP32-S3 TRNG](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/system/random.html)
