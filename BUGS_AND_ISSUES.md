# 🔍 Analyse Critique - Bugs, Incohérences et Manquants

**Date d'analyse**: Mars 2026  
**Fichiers analysés**: 54  
**Niveau**: Critique / Warning / Info  

---

## 🚨 BUGS CRITIQUES (Doivent être corrigés avant usage)

### 1. **secp256k1 - IMPLEMENTATION INCOMPLÈTE** 🔴 CRITIQUE

**Fichier**: `lib/secp256k1/secp256k1.c` (lignes 88-123, 141-150)

**Problème**: La fonction `secp256k1_scalar_multiply()` est un **placeholder** qui ne fait pas la vraie multiplication de courbe elliptique !

```c
// Ligne 94-99 - CECI EST UN PLACEHOLDER!
static int secp256k1_scalar_multiply(...) {
    /* Placeholder - real implementation needs EC arithmetic */
    memcpy(result_x, point_x, 32);  // ❌ Copie juste le point, ne calcule rien!
    memcpy(result_y, point_y, 32);
    (void)scalar;  // ❌ Ignore la clé privée!
    return 1;
}
```

**Impact**: 
- Les clés publiques générées sont **INVALIDES**
- Les signatures ECDSA sont **INVALIDES**
- Impossible de signer des transactions Bitcoin réelles

**Correction nécessaire**: Implémenter la vraie multiplication scalaire sur courbe secp256k1 ou utiliser une librairie complète comme micro-bitcoin ou btcec.

---

### 2. **BIP39 - Wordlist INCOMPLÈTE** 🔴 CRITIQUE

**Fichier**: `src/core/bip39.cpp` (lignes 16-21)

**Problème**: Seuls 14 mots sur 2048 sont définis !

```cpp
const char* BIP39::s_wordlist[2048] = {
    "abandon", "ability", "able", "about", "above", "absent", 
    "absorb", "abstract", "absurd", "abuse", "access", "accident",
    // ... full wordlist would be here  ❌ MANQUANT!
    "zoo", "zoom"
};
```

**Impact**:
- La génération de mnemonics va planter ou générer des données invalides
- `index_to_word()` retourne nullptr pour indices 14-2046
- `find_word()` comparera avec des nullptr = crash

**Correction**: Inclure la wordlist complète BIP39 (fichier ~20KB).

---

### 3. **BIP32 - Fingerprint INVALIDE** 🔴 CRITIQUE

**Fichier**: `src/core/bip32_secp256k1.cpp` (lignes 201-213)

**Problème**: Le fingerprint utilise SHA256 au lieu de HASH160 (SHA256 + RIPEMD160)

```cpp
uint32_t BIP32::get_fingerprint(const ExtendedKey* key) {
    // ❌ INCORRECT - Devrait être HASH160, pas SHA256
    m_secp256k1->hash256(key->key + 33, 33, hash);
    // ...
}
```

**Impact**: Les fingerprints ne correspondent pas au standard BIP32.

**Correction**: 
```cpp
uint8_t hash1[32], hash2[20];
sha256(pubkey, 33, hash1);
ripemd160(hash1, 32, hash2);  // Manque RIPEMD160!
return (hash2[0] << 24) | ...;
```

---

### 4. **LVGL - Buffer Allocation ERREUR** 🔴 CRITIQUE

**Fichier**: `src/main.cpp` (lignes 286-292)

**Problème**: `lv_disp_draw_buf_create` est appelé avec des paramètres incorrects

```cpp
// ❌ ERREUR: Premier argument devrait être taille, pas width*10
disp_drv.draw_buf = lv_disp_draw_buf_create(
    DISPLAY_WIDTH * 10,  // ❌ Taille totale, pas buf1
    draw_buf,            // ❌ buf1
    draw_buf + draw_buf_size / 2,  // ❌ buf2
    draw_buf_size / 2    // ❌ taille
);
```

**Impact**: LVGL va écrire hors des buffers alloués = corruption mémoire

**Correction**:
```cpp
static lv_disp_draw_buf_t draw_buf;
lv_disp_draw_buf_init(&draw_buf, buf1, buf2, buf_size);
disp_drv.draw_buf = &draw_buf;
```

---

### 5. **BIP39 - Extraction bits ERRONÉE** 🔴 CRITIQUE

**Fichier**: `src/core/bip39.cpp` (lignes 56-81)

**Problème**: L'extraction des 11 bits pour l'index du mot est incorrecte

```cpp
// Ligne 63-65 - LOGIQUE FAUSSE
index = ((bits[byte_pos] << bit_offset) | 
         (bits[byte_pos + 1] >> (8 - bit_offset))) >> (8 - bit_offset);
```

Cette formule ne donne pas les bons indices.

**Correction**:
```cpp
uint16_t index = (bits[byte_pos] << 3) | (bits[byte_pos + 1] >> 5);
// Adapté selon bit_offset exact
```

---

## ⚠️ WARNINGS (Problèmes sérieux)

### 6. **Memory - Bump Allocator sans free** 🟡 WARNING

**Fichier**: `src/utils/memory.cpp` (lignes 44-70)

**Problème**: L'allocateur ne supporte pas la libération de mémoire !

```cpp
void SecureMemory::free(void* ptr) {
    // ❌ Ne fait rien! Memory leak garanti
    (void)ptr;
}
```

**Impact**: Fuite mémoire progressive jusqu'à épuisement des 32KB.

**Solution**: Implémenter un vrai allocateur avec tracking ou utiliser `heap_caps_malloc` directement.

---

### 7. **NFC - Pas de vérification erreur I2C** 🟡 WARNING

**Fichier**: `src/drivers/nfc.cpp`

**Problème**: Les fonctions I2C ne vérifient pas les codes retour de `Wire.endTransmission()`

```cpp
// Ligne 56
Wire.beginTransmission(PN532_I2C_ADDRESS);
for (uint8_t i = 0; i < frame_len; i++) {
    Wire.write(frame[i]);
}
if (Wire.endTransmission() != 0) {  // ❌ MANQUANT
    return false;
}
```

**Impact**: Si le module NFC est déconnecté, le code continue comme si tout allait bien.

---

### 8. **Camera - QR Scanner non implémenté** 🟡 WARNING

**Fichier**: `src/utils/qr_code.cpp` (lignes 140-180)

**Problème**: `QRCodeScanner::process_frame()` est un placeholder

```cpp
bool QRCodeScanner::process_frame(...) {
    // ❌ Simplified - real implementation needs proper perspective transform
    return false;  // Toujours false!
}
```

**Impact**: Le scan QR ne fonctionne pas du tout.

**Solution**: Intégrer libquirc ou zxing-cpp.

---

### 9. **BIP32 - Pas de vérification dérivation** 🟡 WARNING

**Fichier**: `src/core/bip32_secp256k1.cpp` (lignes 94-181)

**Problème**: Pas de vérification que la clé dérivée est valide (entre 1 et n-1)

```cpp
// Après tweak_add, il faudrait vérifier:
if (secp256k1_is_zero(child_seckey)) return false;
// Et vérifier < n
```

**Impact**: Si dérivation invalide, génération d'adresses inexistantes.

---

### 10. **PSBT - Structure définie mais pas implémentée** 🟡 WARNING

**Fichier**: `src/core/psbt.h`

**Problème**: Seules les structures sont définies, pas de code de parsing/signing.

**Impact**: Le signing PSBT ne fonctionne pas.

---

## 📋 INCOHÉRENCES ARCHITECTURALES

### 11. **Double définition BIP32** 📋 INFO

**Fichiers**: 
- `src/core/bip32.cpp` (ancienne version)
- `src/core/bip32_secp256k1.cpp` (nouvelle version)

**Problème**: Deux implémentations BIP32 coexistent!

**Solution**: Supprimer `bip32.cpp`, garder uniquement `bip32_secp256k1.cpp`.

---

### 12. **Incohérence include guards** 📋 INFO

**Fichier**: Plusieurs headers

Certains headers utilisent:
```cpp
#ifndef CORE_BIP39_H      // ✅ Correct
```

D'autres:
```cpp
#ifndef UI_SEEDSIGNER_THEME_H  // ✅ Correct mais pattern différent
```

Standardiser sur: `SEEDSIGNER_<MODULE>_<FILE>_H`

---

### 13. **Callbacks LVGL avec variables globales** 📋 INFO

**Fichier**: `src/main.cpp` (lignes 351-384)

**Problème**: `lvgl_read_cb` utilise `g_touch` global sans vérification nullptr

```cpp
void lvgl_read_cb(...) {
    if (g_touch && g_touch->is_touched()) {  // ✅ OK mais pattern risqué
```

**Risque**: Race condition si `g_touch` est détruit pendant le callback.

---

## ❌ FONCTIONNALITÉS MANQUANTES

### 14. **RIPEMD160 manquant** ❌

**Impact**: Impossible de générer des adresses Bitcoin (besoin de HASH160 = SHA256 + RIPEMD160)

**Fichier**: `src/core/secp256k1_wrapper.cpp` (ligne 156-159)

```cpp
void Secp256k1::ripemd160(...) {
    // Placeholder - needs actual RIPEMD160 implementation
    memset(hash, 0, 20);  // ❌ Retourne hash vide!
}
```

---

### 15. **Wordlist BIP39 complète manquante** ❌

**Fichier**: `src/core/wordlist_en.h`

Seulement 256 mots définis sur 2048.

---

### 16. **Bech32/Segwit address encoding manquant** ❌

**Impact**: Impossible de générer des adresses bc1...

---

### 17. **Camera QR decoder manquant** ❌

**Impact**: Impossible de scanner des QR codes

---

### 18. **Tests unitaires incomplets** ❌

**Fichier**: `src/test_crypto.cpp`

Les tests sont des placeholders sans assertions réelles.

---

## 🔧 PROBLÈMES DE COMPILATION POTENTIELS

### 19. **mbedtls pas inclus dans platformio.ini** ⚠️

**Fichier**: `platformio.ini`

Le code utilise `mbedtls/sha256.h` mais ce n'est pas listé dans les dépendances lib_deps.

**Correction**:
```ini
lib_deps = 
    ${env.lib_deps}
    mbedtls  # Ajouter
```

---

### 20. **esp_camera.h include manquant** ⚠️

**Fichier**: `src/drivers/camera.cpp`

Utilise `esp_camera_fb_get()` mais vérifiez que `<esp_camera.h>` est bien inclus.

---

## 📊 RÉSUMÉ DES PRIORITÉS

| Priorité | Problème | Fichier | Effort |
|----------|----------|---------|--------|
| 🔴 P0 | secp256k1 placeholder | `lib/secp256k1/secp256k1.c` | 2-3 jours |
| 🔴 P0 | BIP39 wordlist | `src/core/wordlist_en.h` | 2 heures |
| 🔴 P0 | LVGL buffer | `src/main.cpp` | 30 min |
| 🔴 P0 | BIP39 bit extraction | `src/core/bip39.cpp` | 2 heures |
| 🟡 P1 | RIPEMD160 | `src/core/secp256k1_wrapper.cpp` | 4 heures |
| 🟡 P1 | QR decoder | `src/utils/qr_code.cpp` | 1 jour |
| 🟡 P1 | Memory free | `src/utils/memory.cpp` | 2 heures |
| 📋 P2 | PSBT implementation | `src/core/psbt.cpp` | 2-3 jours |
| 📋 P2 | Bech32 | New file | 1 jour |

---

## ✅ RECOMMANDATIONS

### Immédiat (avant tout test hardware):
1. ✅ Corriger le LVGL buffer allocation
2. ✅ Ajouter wordlist BIP39 complète
3. ✅ Corriger extraction bits BIP39

### Court terme (avant beta):
4. 🔧 Remplacer secp256k1 par librairie complète (micro-bitcoin)
5. 🔧 Implémenter RIPEMD160
6. 🔧 Intégrer quirc pour QR

### Moyen terme:
7. 📋 Compléter PSBT signing
8. 📋 Ajouter Bech32
9. 📭 Tests unitaires complets

---

**⚠️ AVERTISSEMENT**: Avec les bugs actuels (P0), le firmware NE FONCTIONNERA PAS correctement pour:
- Génération de clés valides
- Signature de transactions
- Scan QR

**NE PAS UTILISER AVEC DES VRAIS FONDS** jusqu'à correction des bugs P0!
