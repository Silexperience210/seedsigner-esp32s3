# 🔬 ANALYSE COMPLÈTE DU PROTOCOLE MESHCORE

**Date d'analyse**: 24 Février 2026  
**Version firmware analysée**: v1.12.0 - v1.13.0  
**Source**: https://github.com/meshcore-dev/MeshCore

---

## 📑 TABLE DES MATIÈRES

1. [Vue d'ensemble de l'architecture](#1-vue-densemble-de-larchitecture)
2. [Format binaire des paquets LoRa](#2-format-binaire-des-paquets-lora)
3. [Structures de données détaillées](#3-structures-de-données-détaillées)
4. [Système de chiffrement](#4-système-de-chiffrement)
5. [Mécanismes de routage](#5-mécanismes-de-routage)
6. [Protocole Companion (BLE/USB)](#6-protocole-companion-bleusb)
7. [Flow d'exécution](#7-flow-dexécution)
8. [Différences firmware types](#8-différences-firmware-types)
9. [Limitations et contraintes](#9-limitations-et-contraintes)
10. [Edge cases et bugs potentiels](#10-edge-cases-et-bugs-potentiels)

---

## 1. VUE D'ENSEMBLE DE L'ARCHITECTURE

### 1.1 Structure du repository

```
MeshCore/
├── src/                      # Bibliothèque core
│   ├── MeshCore.h           # Constantes fondamentales
│   ├── MeshCore.cpp
│   ├── Mesh.h/cpp           # Couche mesh (routing logique)
│   ├── Packet.h/cpp         # Structure des paquets
│   ├── Dispatcher.h/cpp     # Gestion radio (envoi/réception)
│   ├── Identity.h/cpp       # Identités Ed25519
│   ├── Utils.h/cpp          # Crypto utils (AES, SHA256)
│   └── helpers/             # Helpers divers
├── examples/
│   ├── companion_radio/     # Firmware client (BLE/USB/WiFi)
│   ├── simple_repeater/     # Firmware répéteur
│   ├── simple_room_server/  # Firmware serveur de salon
│   ├── simple_secure_chat/  # Chat terminal direct
│   └── simple_sensor/       # Nœud capteur
└── docs/                    # Documentation
```

### 1.2 Architecture en couches

```
┌─────────────────────────────────────────────────────────┐
│                    APPLICATION                         │
│   (Companion App / Repeater CLI / Room Server)        │
├─────────────────────────────────────────────────────────┤
│                      MESH LAYER                        │
│   (Routing, Encryption, Contact Management)           │
├─────────────────────────────────────────────────────────┤
│                    DISPATCHER                          │
│   (Packet queue, TX/RX timing, Airtime budget)        │
├─────────────────────────────────────────────────────────┤
│                      RADIO DRIVER                      │
│   (LoRa hardware abstraction)                         │
└─────────────────────────────────────────────────────────┘
```

---

## 2. FORMAT BINAIRE DES PAQUETS LORA

### 2.1 Structure générale d'un paquet

```
┌──────────────────────────────────────────────────────────────────────┐
│  HEADER (1 byte)  │  TC (opt)  │  PATH_LEN  │  PATH  │  PAYLOAD    │
├──────────────────────────────────────────────────────────────────────┤
│  0bVVPPPPRR       │  4 bytes   │  1 byte    │ 0-64B  │  0-184B     │
└──────────────────────────────────────────────────────────────────────┘
```

### 2.2 Décodage du Header (Byte 0)

Format: `0bVVPPPPRR`

| Bits | Champ | Masque | Description |
|------|-------|--------|-------------|
| 0-1  | Route Type | `0x03` | Mode de routage |
| 2-5  | Payload Type | `0x3C` | Type de contenu |
| 6-7  | Version | `0xC0` | Version du payload |

**Extraction des champs:**
```cpp
uint8_t route_type = header & 0x03;          // Bits 0-1
uint8_t payload_type = (header >> 2) & 0x0F; // Bits 2-5
uint8_t version = (header >> 6) & 0x03;      // Bits 6-7
```

### 2.3 Route Types (2 bits)

| Valeur | Constante | Description |
|--------|-----------|-------------|
| `0x00` | `ROUTE_TYPE_TRANSPORT_FLOOD` | Flood + Transport Codes |
| `0x01` | `ROUTE_TYPE_FLOOD` | Flood mode standard |
| `0x02` | `ROUTE_TYPE_DIRECT` | Routage direct (path fourni) |
| `0x03` | `ROUTE_TYPE_TRANSPORT_DIRECT` | Direct + Transport Codes |

### 2.4 Payload Types (4 bits)

| Valeur | Constante | Description | Struct payload |
|--------|-----------|-------------|----------------|
| `0x00` | `PAYLOAD_TYPE_REQ` | Requête chiffrée | `dest_hash(1) + src_hash(1) + MAC(2) + ciphertext` |
| `0x01` | `PAYLOAD_TYPE_RESPONSE` | Réponse chiffrée | Même format que REQ |
| `0x02` | `PAYLOAD_TYPE_TXT_MSG` | Message texte | Même format que REQ |
| `0x03` | `PAYLOAD_TYPE_ACK` | Accusé de réception | `ack_crc(4)` |
| `0x04` | `PAYLOAD_TYPE_ADVERT` | Annonce de nœud | `pubkey(32) + timestamp(4) + signature(64) + appdata` |
| `0x05` | `PAYLOAD_TYPE_GRP_TXT` | Message groupe | `channel_hash(1) + MAC(2) + ciphertext` |
| `0x06` | `PAYLOAD_TYPE_GRP_DATA` | Data groupe | Même format |
| `0x07` | `PAYLOAD_TYPE_ANON_REQ` | Requête anonyme | `dest_hash(1) + sender_pubkey(32) + MAC(2) + ciphertext` |
| `0x08` | `PAYLOAD_TYPE_PATH` | Retour de chemin | `dest_hash(1) + src_hash(1) + MAC(2) + path_encrypted` |
| `0x09` | `PAYLOAD_TYPE_TRACE` | Traceroute | `tag(4) + auth(4) + flags(1) + path_data` |
| `0x0A` | `PAYLOAD_TYPE_MULTIPART` | Paquet fragmenté | `part_info + data` |
| `0x0B` | `PAYLOAD_TYPE_CONTROL` | Paquet de contrôle | `control_data` (non chiffré) |
| `0x0F` | `PAYLOAD_TYPE_RAW_CUSTOM` | Payload personnalisé | Raw bytes |

### 2.5 Exemple de paquet hexadécimal décodé

```
Paquet reçu: 09 03 2A 8F 3B ...

Décodage:
- Header: 0x09 = 0b00001001
  * Route type:  0b01 = FLOOD (1)
  * Payload type: 0b0010 = TXT_MSG (2)
  * Version: 0b00 = V1 (0)
  
- path_len: 0x03 = 3 bytes de path
- path: [0x2A, 0x8F, 0x3B] = 3 sauts
- payload: reste du paquet
```

### 2.6 Transport Codes (optionnel, 4 bytes)

Présents uniquement pour `ROUTE_TYPE_TRANSPORT_*`:
```
Bytes 0-1: transport_code_1 (uint16_t) - Code région/filtre
Bytes 2-3: transport_code_2 (uint16_t) - Réservé
```

---

## 3. STRUCTURES DE DONNÉES DÉTAILLÉES

### 3.1 Constantes fondamentales (MeshCore.h)

```cpp
#define MAX_HASH_SIZE           8       // Taille max d'un hash
#define PUB_KEY_SIZE           32       // Ed25519 public key
#define PRV_KEY_SIZE           64       // Ed25519 private key  
#define SEED_SIZE              32       // Seed RNG
#define SIGNATURE_SIZE         64       // Signature Ed25519
#define MAX_ADVERT_DATA_SIZE   32       // Données app dans advert
#define CIPHER_KEY_SIZE        16       // Clé AES-128
#define CIPHER_BLOCK_SIZE      16       // Block AES

// V1 specifics
#define CIPHER_MAC_SIZE         2       // MAC truncaté (HMAC-SHA256)
#define PATH_HASH_SIZE          1       // Hash nœud = 1er byte pubkey

// Tailles paquets
#define MAX_PACKET_PAYLOAD    184      // Max payload LoRa
#define MAX_PATH_SIZE          64      // Max sauts dans path
#define MAX_TRANS_UNIT        255      // MTU radio
```

### 3.2 Structure Packet (Packet.h)

```cpp
class Packet {
public:
    uint8_t  header;                    // 0: Header byte
    uint16_t payload_len;               // 1-2: Longueur payload
    uint16_t path_len;                  // 3-4: Longueur path
    uint16_t transport_codes[2];        // 5-8: Codes transport (opt)
    uint8_t  path[MAX_PATH_SIZE];       // 9-72: Path (64 bytes max)
    uint8_t  payload[MAX_PACKET_PAYLOAD]; // 73-256: Données
    int8_t   _snr;                      // SNR stocké (x4)
};
```

**Taille mémoire:** ~256 bytes par paquet

### 3.3 Structure ContactInfo (ContactInfo.h)

```cpp
struct ContactInfo {
    mesh::Identity id;                  // 32 bytes: Public key
    char name[32];                      // 32 bytes: Nom affiché
    uint8_t type;                       // 1 byte: Type de nœud
    uint8_t flags;                      // 1 byte: Flags (favori, etc)
    uint8_t out_path_len;               // 1 byte: Longueur path connu
    uint8_t out_path[MAX_PATH_SIZE];    // 64 bytes: Path vers ce contact
    uint32_t last_advert_timestamp;     // 4 bytes: Dernier advert vu
    float gps_lat;                      // 4 bytes: Latitude
    float gps_lon;                      // 4 bytes: Longitude
    uint32_t lastmod;                   // 4 bytes: Modification timestamp
};
```

**Taille totale:** ~148 bytes par contact

**Flags ContactInfo:**
| Bit | Constante | Description |
|-----|-----------|-------------|
| 0 | `CONTACT_IS_FAVOURITE` | Contact favori (non écrasable) |
| 1 | `CONTACT_IS_LOST` | Contact perdu (path expiré) |
| 2-7 | Réservé | - |

### 3.4 Structure Identity

```cpp
class Identity {
public:
    uint8_t pub_key[PUB_KEY_SIZE];      // 32 bytes: Clé publique Ed25519
};

class LocalIdentity : public Identity {
    uint8_t prv_key[PRV_KEY_SIZE];      // 64 bytes: Clé privée
};
```

**Hash d'identité:** Premier byte de `pub_key`
- Interdit: `0x00` et `0xFF` (réservés)

### 3.5 GroupChannel (canaux de groupe)

```cpp
class GroupChannel {
public:
    uint8_t hash[PATH_HASH_SIZE];       // 1 byte: Hash canal
    uint8_t secret[PUB_KEY_SIZE];       // 32 bytes: Secret partagé
};
```

**Canal public par défaut:**
- Secret: `"izOH6cXN6mrJ5e26oRXNcg=="` (Base64)
- Hash: Premier byte du SHA256 du secret

---

## 4. SYSTÈME DE CHIFFREMENT

### 4.1 Vue d'ensemble

```
┌─────────────────────────────────────────────────────────────────┐
│                    CHIFFREMENT END-TO-END                       │
├─────────────────────────────────────────────────────────────────┤
│  1. Key Exchange: ECDH (Ed25519 → X25519)                      │
│  2. Symmetric Encryption: AES-128-CBC                          │
│  3. Authentication: HMAC-SHA256 (truncated to 2 bytes)         │
└─────────────────────────────────────────────────────────────────┘
```

### 4.2 Dérivation du secret partagé

```cpp
// Alice veut envoyer à Bob
uint8_t shared_secret[32];
alice_local_id.calcSharedSecret(shared_secret, bob_identity);

// Implémentation: Ed25519 → X25519 transpose + ECDH
void calcSharedSecret(uint8_t* secret, const uint8_t* other_pub_key) {
    ed25519_key_exchange(secret, other_pub_key, prv_key);
}
```

### 4.3 Schéma Encrypt-then-MAC

```cpp
// Chiffrement
int encryptThenMAC(const uint8_t* shared_secret, 
                   uint8_t* dest, 
                   const uint8_t* src, 
                   int src_len) {
    // 1. Chiffre avec AES-128
    int enc_len = encrypt(shared_secret, dest + 2, src, src_len);
    
    // 2. Calcule HMAC-SHA256 des données chiffrées
    SHA256 sha;
    sha.resetHMAC(shared_secret, 32);
    sha.update(dest + 2, enc_len);
    sha.finalizeHMAC(shared_secret, 32, dest, 2);  // Truncate à 2 bytes
    
    return 2 + enc_len;  // MAC(2) + ciphertext
}
```

### 4.4 Format des données chiffrées

```
┌────────────────────────────────────────────────────────────┐
│  MAC(2 bytes)  │  Ciphertext (multiple de 16 bytes)       │
├────────────────────────────────────────────────────────────┤
│  HMAC-SHA256   │  AES-128-CBC avec padding zero           │
│  (truncated)   │  Block size: 16 bytes                     │
└────────────────────────────────────────────────────────────┘
```

### 4.5 Déchiffrement et vérification

```cpp
int MACThenDecrypt(const uint8_t* shared_secret,
                   uint8_t* dest,
                   const uint8_t* src,      // MAC(2) + ciphertext
                   int src_len) {
    if (src_len <= 2) return 0;  // Trop court
    
    // Recalcule HMAC
    uint8_t hmac[2];
    SHA256 sha;
    sha.resetHMAC(shared_secret, 32);
    sha.update(src + 2, src_len - 2);
    sha.finalizeHMAC(shared_secret, 32, hmac, 2);
    
    // Vérifie MAC
    if (memcmp(hmac, src, 2) != 0) {
        return 0;  // MAC invalide = rejet
    }
    
    // Déchiffre
    return decrypt(shared_secret, dest, src + 2, src_len - 2);
}
```

### 4.6 Sécurité - Points critiques

| Aspect | Implémentation | Force |
|--------|---------------|-------|
| Courbes | Ed25519 / Curve25519 | Forte |
| Symétrique | AES-128-CBC | Standard |
| MAC | HMAC-SHA256-16 | Faible (2 bytes seulement!) |
| Key Derivation | ECDH direct | Standard |

**⚠️ VULNÉRABILITÉ CONNUE:** Le MAC de 2 bytes offre seulement 16 bits de protection contre les forgeries. Probabilité de collision aléatoire: 1/65536.

---

## 5. MÉCANISMES DE ROUTAGE

### 5.1 Mode FLOOD (Inondation)

```
Alice --flood--> [Répéteurs] --flood--> Bob
                      │
                      └--> [Autres nœuds] (relayent si nouveau)
```

**Processus:**
1. Émetteur envoie en broadcast
2. Chaque nœud qui reçoit vérifie `hasSeen(packet)`
3. Si nouveau: délivre à l'application + retransmet (probabiliste)
4. Si déjà vu: silencieusement ignoré

**Anti-rejeu:**
```cpp
// Hash du paquet = SHA256(payload_type + payload)
void calculatePacketHash(uint8_t* hash) const {
    SHA256 sha;
    uint8_t t = getPayloadType();
    sha.update(&t, 1);
    if (t == PAYLOAD_TYPE_TRACE) {
        sha.update(&path_len, sizeof(path_len));
    }
    sha.update(payload, payload_len);
    sha.finalize(hash, MAX_HASH_SIZE);  // 8 premiers bytes
}
```

### 5.2 Mode DIRECT (Routage par chemin)

```
Alice --direct--> R1 --direct--> R2 --direct--> Bob
         [A,R1]          [R1,R2]          [R2,B]
```

**Structure du path:**
```
path[0] = hash prochain saut
path[1] = hash saut suivant
...
path[n-1] = hash destinataire final
```

**Traitement à chaque saut:**
```cpp
// Dans Mesh::onRecvPacket()
if (self_id.isHashMatch(pkt->path)) {  // Sommes-nous le prochain saut?
    removeSelfFromPath(pkt);            // Avance le curseur path
    
    if (pkt->path_len == 0) {
        // Nous sommes le destinataire final
        deliverToApplication(pkt);
    } else {
        // Relay vers le prochain saut
        return ACTION_RETRANSMIT_DELAYED(priority, delay);
    }
}
```

### 5.3 Découverte de chemin (Path Discovery)

```
1. Alice envoie message en FLOOD à Bob
2. Bob reçoit, extrait le path inverse du flood
3. Bob envoie PATH_RETURN en DIRECT avec le chemin appris
4. Alice reçoit et stocke: out_path[Bob] = path_reçu
5. Prochains messages: Alice utilise DIRECT avec ce path
```

### 5.4 Comparaison FLOOD vs DIRECT

| Aspect | FLOOD | DIRECT |
|--------|-------|--------|
| Latence | Plus élevée (rebonds) | Plus faible (chemin optimal) |
| Fiabilité | Haute (redondance) | Dépend du path connu |
| Airtime | Élevé (inondation) | Faible (unicast) |
| Découverte | Auto | Nécessite apprentissage |
| Cas d'usage | Découverte, groupes, advert | Messages directs connus |

### 5.5 Timings de retransmission

**FLOOD mode:**
```cpp
uint32_t getRetransmitDelay(const Packet* packet) {
    uint32_t airtime = radio->getEstAirtimeFor(packet->getRawLength());
    uint32_t t = (airtime * 52 / 50) / 2;  // Base
    return rng->nextInt(0, 5) * t;          // Jitter 0-4x
}
```

**DIRECT mode:**
```cpp
uint32_t getDirectRetransmitDelay(const Packet* packet) {
    return 0;  // Pas de délai par défaut
}
```

---

## 6. PROTOCOLE COMPANION (BLE/USB)

### 6.1 Service BLE

| Caractéristique | UUID | Direction |
|-----------------|------|-----------|
| Service | `6E400001-B5A3-F393-E0A9-E50E24DCCA9E` | - |
| RX (App→Firmware) | `6E400002-B5A3-F393-E0A9-E50E24DCCA9E` | Write |
| TX (Firmware→App) | `6E400003-B5A3-F393-E0A9-E50E24DCCA9E` | Notify |

### 6.2 Commandes (App → Firmware)

| Code | Commande | Description | Paramètres |
|------|----------|-------------|------------|
| `0x01` | `CMD_APP_START` | Initialisation | ver(1) + "mccli"(9) |
| `0x02` | `CMD_SEND_TXT_MSG` | Envoyer message | contact_idx(1) + msg(var) |
| `0x03` | `CMD_SEND_CHANNEL_TXT_MSG` | Msg groupe | channel_idx(1) + msg(var) |
| `0x04` | `CMD_GET_CONTACTS` | Liste contacts | since_timestamp(4, opt) |
| `0x05` | `CMD_GET_DEVICE_TIME` | Heure device | - |
| `0x06` | `CMD_SET_DEVICE_TIME` | Régler heure | timestamp(4) |
| `0x07` | `CMD_SEND_SELF_ADVERT` | Envoyer advert | delay_ms(4) |
| `0x09` | `CMD_ADD_UPDATE_CONTACT` | Ajouter contact | ContactInfo struct |
| `0x0A` | `CMD_SYNC_NEXT_MESSAGE` | Récupérer msg | - |
| `0x13` | `CMD_RESET_PATH` | Reset path | contact_idx(1) |
| `0x1F` | `CMD_GET_CHANNEL` | Info canal | channel_idx(1) |
| `0x20` | `CMD_SET_CHANNEL` | Config canal | idx(1) + name(32) + secret(32) |
| `0x33` | `CMD_SEND_LOGIN` | Login room/repeater | contact_idx(1) + pwd(var) |
| `0x38` | `CMD_SEND_STATUS_REQ` | Statistiques | contact_idx(1) |
| `0x39` | `CMD_SEND_TELEMETRY_REQ` | Télémétrie | contact_idx(1) |

### 6.3 Réponses (Firmware → App)

| Code | Réponse | Description |
|------|---------|-------------|
| `0x00` | `RESP_CODE_OK` | Succès |
| `0x01` | `RESP_CODE_ERR` | Erreur |
| `0x02` | `RESP_CODE_CONTACTS_START` | Début liste contacts |
| `0x03` | `RESP_CODE_CONTACT` | Entrée contact |
| `0x04` | `RESP_CODE_END_OF_CONTACTS` | Fin liste |
| `0x07` | `RESP_CODE_CONTACT_MSG_RECV` | Message reçu (v<3) |
| `0x10` | `RESP_CODE_NO_MORE_MESSAGES` | File vide |
| `0x12` | `RESP_CODE_CHANNEL_INFO` | Info canal |
| `0x16` | `RESP_CODE_CONTACT_MSG_RECV_V3` | Message reçu (v3+) |

### 6.4 Push Codes (asynchrones)

| Code | Événement | Déclencheur |
|------|-----------|-------------|
| `0x80` | `PUSH_CODE_ADVERT` | Advert reçu |
| `0x81` | `PUSH_CODE_PATH_UPDATED` | Path mis à jour |
| `0x82` | `PUSH_CODE_SEND_CONFIRMED` | ACK reçu |
| `0x83` | `PUSH_CODE_MSG_WAITING` | Message en attente |
| `0x84` | `PUSH_CODE_RAW_DATA` | Données raw reçues |
| `0x85` | `PUSH_CODE_LOGIN_SUCCESS` | Login OK |
| `0x86` | `PUSH_CODE_LOGIN_FAIL` | Login échoué |
| `0x87` | `PUSH_CODE_STATUS_RESPONSE` | Réponse status |
| `0x89` | `PUSH_CODE_TRACE_DATA` | Données trace |
| `0x90` | `PUSH_CODE_CONTACTS_FULL` | Contacts pleins |

### 6.5 Format struct ContactInfo sur le fil

```
Offset  Taille  Champ
─────────────────────────────────────────
0       1       Code réponse (0x03)
1       32      Public key
33      1       Type (0=chat, 1=repeater, 2=room, 3=sensor)
34      1       Flags
35      1       out_path_len
36      64      out_path
100     32      Name (null-terminated)
132     4       last_advert_timestamp
136     4       gps_lat
140     4       gps_lon
144     4       lastmod
─────────────────────────────────────────
Total: 148 bytes
```

---

## 7. FLOW D'EXÉCUTION

### 7.1 Envoi d'un message texte

```
App Companion
     │
     │ writeFrame(CMD_SEND_TXT_MSG)
     ▼
MyMesh::handleCmdFrame()
     │
     ├──► ContactInfo& contact = getContact(idx)
     │
     ├──► Packet* pkt = createDatagram(PAYLOAD_TYPE_TXT_MSG,
     │                                 contact.id, secret,
     │                                 encrypted_data, len)
     │
     ├──► Si contact.out_path_len > 0:
     │         sendDirect(pkt, contact.out_path, contact.out_path_len)
     │    Sinon:
     │         sendFlood(pkt)
     │
     └──► Attente ACK (timeout basé sur airtime)

          ┌─────────────────────────────────────────┐
          │         COUCHE MESH (routing)          │
          └─────────────────────────────────────────┘
                         │
                         ▼
               Mesh::sendDirect() / sendFlood()
                         │
                         ├──► PacketManager::queueOutbound()
                         │         (priorité + délai)
                         │
                         └──► Retour immédiat

          ┌─────────────────────────────────────────┐
          │       COUCHE DISPATCHER (async)        │
          └─────────────────────────────────────────┘
                         │
                         ▼
               Dispatcher::loop()
                         │
                         ├──► checkRecv()  ← Vérifie RX radio
                         │
                         └──► checkSend()  ← Si queue non vide
                                   │
                                   ├──► CAD (Listen Before Talk)
                                   │
                                   ├──► Radio::startSendRaw()
                                   │
                                   └──► Attente isSendComplete()
```

### 7.2 Réception d'un paquet

```
Dispatcher::loop()
     │
     ├──► Radio::recvRaw() → Paquet reçu
     │
     ├──► Packet::readFrom(raw_bytes)  // Parse header, path, payload
     │
     ├──► packet->_snr = radio->getLastSNR() * 4
     │
     ├──► Si FLOOD + score élevé:
     │         queueInbound(pkt, delay)  // Retardé
     │    Sinon:
     │         processRecvPacket(pkt)    // Immédiat
     │
     ▼
processRecvPacket(pkt)
     │
     ├──► Action = Mesh::onRecvPacket(pkt)
     │
     └──► Switch sur payload_type:
              │
              ├── PAYLOAD_TYPE_TXT_MSG:
              │      ├── searchPeersByHash(src_hash)
              │      ├── getPeerSharedSecret()
              │      ├── MACThenDecrypt() → Vérifie MAC + déchiffre
              │      └── onPeerDataRecv() → Callback application
              │
              ├── PAYLOAD_TYPE_ACK:
              │      ├── Extrait ack_crc
              │      └── onAckRecv() → Confirme envoi en attente
              │
              └── DEFAULT:
                     └── routeRecvPacket() → Décide forward

Retour d'action:
     ACTION_RELEASE              → Libère le paquet
     ACTION_MANUAL_HOLD          → Garde référence
     ACTION_RETRANSMIT(p)        → Queue pour forward
     ACTION_RETRANSMIT_DELAYED(p,d) → Queue avec délai
```

### 7.3 Mécanisme d'ACK

```
┌─────────────┐                           ┌─────────────┐
│   Alice     │                           │     Bob     │
└──────┬──────┘                           └──────┬──────┘
       │                                         │
       │  1. sendDatagram(TXT_MSG)               │
       │  2. Stocke expected_ack dans table      │
       │     (timestamp, ack_crc, contact)       │
       │                                         │
       │ ═══════ Paquet chiffré ═══════════════► │
       │                                         │
       │                                         │  3. Déchiffre + vérifie
       │                                         │  4. Déliver à app
       │                                         │  5. Génère ACK
       │                                         │     ack_crc = CRC(timestamp+msg)
       │                                         │
       │ ◄════════════════ ACK ================= │
       │                                         │
       │  6. Reçoit ACK                          │
       │  7. Cherche dans expected_ack_table     │
       │  8. Marque message comme confirmé       │
       │  9. PUSH_CODE_SEND_CONFIRMED → App      │
       │                                         │
```

### 7.4 File d'attente offline

```cpp
// Structure offline_queue (Companion firmware)
struct Frame {
    uint8_t len;
    uint8_t buf[MAX_FRAME_SIZE];  // 255 bytes max
};

Frame offline_queue[OFFLINE_QUEUE_SIZE];  // 16 par défaut
int offline_queue_len;
```

**Flux:**
1. Message reçu alors que App déconnectée
2. `addToOfflineQueue(frame)` ajoute à la file
3. App se reconnecte → `CMD_SYNC_NEXT_MESSAGE`
4. `getFromOfflineQueue()` retourne le plus ancien
5. Si file vide → `RESP_CODE_NO_MORE_MESSAGES`

---

## 8. DIFFÉRENCES FIRMWARE TYPES

### 8.1 Companion Radio

**Rôle:** Interface entre App (mobile) et réseau mesh

**Fonctionnalités:**
```cpp
class MyMesh : public BaseChatMesh, public DataStoreHost {
    // Gestion contacts (MAX_CONTACTS: 100)
    ContactInfo contacts[MAX_CONTACTS];
    
    // File messages offline
    Frame offline_queue[OFFLINE_QUEUE_SIZE];  // 16
    
    // Interfaces
    BaseSerialInterface* _serial;  // BLE / USB / WiFi
    
    // Méthodes spécifiques
    void handleCmdFrame(size_t len);      // Protocole Companion
    void addToOfflineQueue();              
    bool advert();                         // Auto-advert
};
```

**Interfaces supportées:**
- BLE (NRF52, ESP32)
- USB Serial (toutes plates-formes)
- WiFi TCP (ESP32)

**Stockage:**
- Contacts persistés (FS interne)
- Channels persistés
- Messages offline en RAM

### 8.2 Simple Repeater

**Rôle:** Relayer les paquets pour étendre la portée

**Fonctionnalités:**
```cpp
class MyMesh : public mesh::Mesh {
    // Pas de gestion contacts utilisateur
    // ACL pour admin (ClientACL)
    
    // Stats détaillées
    RepeaterStats stats;
    
    // Neighbors tracking
    NeighbourInfo neighbours[MAX_NEIGHBOURS];
    
    // Méthodes spécifiques
    void handleCommand();        // CLI série
    void putNeighbour();         // Tracker voisins
    bool allowPacketForward();   // Active le relay
};
```

**CLI Commands disponibles:**
- `reboot`, `advert`, `erase`
- `set radio freq,bw,sf,cr`
- `set tx <dbm>`
- `set repeat on|off`
- `neighbors`
- `stats-core`, `stats-radio`, `stats-packets`
- `password <admin_pwd>`

**Forward activé:**
```cpp
bool allowPacketForward(const mesh::Packet* packet) override {
    return _prefs.repeat_enabled;  // Active par défaut
}
```

### 8.3 Simple Room Server

**Rôle:** Serveur de messages de groupe (BBS)

**Fonctionnalités:**
```cpp
class MyMesh : public mesh::Mesh {
    // Gestion posts
    static RoomMessage room_msgs[MAX_ROOM_MESSAGES];  // 40
    
    // Authentification
    char password[MAX_PASSWORD_LEN];
    
    // Méthodes spécifiques
    void handleCommand();        // CLI
    void handleLogin();          // Auth clients
    void syncMessagesSince();    // Envoi historique
};
```

**Flux de login:**
```
Client --ANON_REQ(login)--> RoomServer
              │
              ├──► Vérifie password
              └──► Si OK: ajoute à ACL temporaire
              
Client <----PATH_RETURN---- RoomServer (confirme login)
```

### 8.4 Tableau comparatif

| Fonctionnalité | Companion | Repeater | Room Server |
|----------------|-----------|----------|-------------|
| Interface App | BLE/USB/WiFi | Serial CLI | Serial CLI |
| Stockage contacts | ✅ Oui | ❌ Non | ❌ Non |
| Stockage messages | Offline queue | ❌ | Room posts |
| Relay paquets | ❌ Non | ✅ Oui | ❌ Non |
| Chiffrement E2E | ✅ Oui | N/A | ✅ Oui |
| Admin CLI | Limitée | Complète | Complète |
| GPS/Télémétrie | ✅ Optionnel | ✅ Optionnel | ❌ |
| Bridge mode | ❌ | ✅ RS232/ESPNow | ❌ |
| Auto-sleep | ❌ | ✅ Power saving | ❌ |

---

## 9. LIMITATIONS ET CONTRAINTES

### 9.1 Tailles maximales

| Ressource | Limite | Configuration |
|-----------|--------|---------------|
| Contacts | 100 | `MAX_CONTACTS` |
| Path (sauts) | 64 | `MAX_PATH_SIZE` |
| Payload LoRa | 184 bytes | `MAX_PACKET_PAYLOAD` |
| MTU radio | 255 bytes | `MAX_TRANS_UNIT` |
| File offline | 16 messages | `OFFLINE_QUEUE_SIZE` |
| Message texte | ~150 caractères | Payload - overhead crypto |
| Nom nœud | 32 bytes | Hardcoded |
| Publicité data | 32 bytes | `MAX_ADVERT_DATA_SIZE` |
| Secret groupe | 32 bytes | `PUB_KEY_SIZE` |

### 9.2 Timings et timeouts

| Événement | Timeout | Formule |
|-----------|---------|---------|
| Attente ACK (FLOOD) | Variable | `airtime * FLOOD_SEND_TIMEOUT_FACTOR` |
| Attente ACK (DIRECT) | Variable | `airtime * path_len * DIRECT_SEND_PERHOP_FACTOR + DIRECT_SEND_PERHOP_EXTRA_MILLIS` |
| Retry CAD | 200-480ms | `random(1,4) * 120ms` |
| Max CAD busy | 4000ms | `getCADFailMaxDuration()` |
| RX delay (FLOOD) | 0-32s | `calcRxDelay(score, airtime)` |
| No RX watchdog | 8000ms | Radio stuck detection |
| Noise calib | 2000ms | `NOISE_FLOOR_CALIB_INTERVAL` |

### 9.3 Airtime calculations

```cpp
// Budget airtime: 33.3% par défaut (1/3 du temps)
float getAirtimeBudgetFactor() const { return 2.0; }

// Silence radio après TX = airtime * factor
next_tx_time = current_time + (airtime * getAirtimeBudgetFactor());
```

**LoRa Airtime (approximatif pour SF11, BW250):**
- Overhead paquet: ~15 bytes (preamble + header LoRa)
- Path: path_len bytes
- Payload: payload_len bytes
- Airtime ≈ (packet_len * 8) / (SF * BW/(2^SF)) * 1.1

### 9.4 Limites mémoire

| Structure | Taille par instance |
|-----------|---------------------|
| Packet | 256 bytes |
| ContactInfo | 148 bytes |
| Identity | 32 bytes (pub) / 96 bytes (local) |
| offline_queue | 16 * 256 = 4KB |
| contacts[100] | 100 * 148 = 14.5KB |

**⚠️ Contrainte critique:** ESP32 avec PSRAM recommandé pour 100 contacts.

---

## 10. EDGE CASES ET BUGS POTENTIELS

### 10.1 Bugs connus / comportements piégeux

#### 1. MAC faible (2 bytes)
```cpp
#define CIPHER_MAC_SIZE 2  // Trop court!
```
- **Problème:** Probabilité de faux positif: 1/65536
- **Impact:** Acceptation de messages forges (faible mais non négligeable)
- **Workaround:** Vérification additionnelle au niveau applicatif

#### 2. Truncation path en v1.12.0
```cpp
// Dispatcher.cpp L144-L152
if (pkt->path_len > MAX_PATH_SIZE) return false;  // Drop silencieux!
if (pkt->payload_len > sizeof(pkt->payload)) return false;
```
- **Problème:** Packets avec path > 64 ou payload > 184 sont droppés sans log
- **Impact:** Perte silencieuse de messages multipart

#### 3. Contact hash collision
```cpp
// Hash = premier byte de pub_key
bool isHashMatch(const uint8_t* hash) {
    return memcmp(hash, pub_key, PATH_HASH_SIZE) == 0;  // PATH_HASH_SIZE = 1
}
```
- **Problème:** Probabilité de collision: 1/254 (exclut 0x00, 0xFF)
- **Impact:** Message envoyé au mauvais contact si collision
- **Mitigation:** Vérification pubkey complète après déchiffrement

#### 4. Race condition ACK table
```cpp
AckTableEntry expected_ack_table[EXPECTED_ACK_ACK_TABLE_SIZE];  // 8 entrées
```
- **Problème:** Table circulaire de seulement 8 entrées
- **Impact:** ACK perdu si plus de 8 messages en attente simultanés
- **Symptôme:** "Message sent" mais pas de confirmation

#### 5. SNR overflow
```cpp
pkt->_snr = (int8_t)(radio->getLastSNR() * 4.0f);
```
- **Problème:** SNR > 31.75 ou < -32 overflow en int8_t
- **Impact:** Valeurs incorrectes dans les traces

### 10.2 Edge cases réseau

#### 1. Path asymétrique
```
Alice ──► R1 ──► R2 ──► Bob   (Path appris: [R1, R2, Bob])
Alice ◄── ? ◄── ? ◄── Bob      (Pas garanti que le chemin inverse fonctionne)
```
- **Problème:** Les répéteurs peuvent ne pas avoir de lien bidirectionnel
- **Mitigation:** Retour à FLOOD si DIRECT échoue

#### 2. Mobile repeater disappearing
```
Alice apprend path via R_mobile
R_mobile bouge hors de portée
Alice continue d'envoyer vers path invalide
```
- **Timeout:** ~32 secondes avant retry en FLOOD

#### 3. Advert flood storm
```
Plusieurs nœuds envoient adverts fréquemment
→ Saturation du canal
```
- **Mitigation:** Rate limiting par défaut, intervalles configurables

### 10.3 Problèmes de stockage

#### 1. Wear leveling limité
- **Problème:** Écritures fréquentes sur FS interne (NOR Flash)
- **Impact:** Usure prématurée sur NRF52
- **Mitigation:** Cache en RAM, lazy writes

#### 2. Corruption contact
```cpp
// Aucune CRC sur les ContactInfo stockés
```
- **Problème:** Bitflip silencieux → contact invalide
- **Symptôme:** "recv matches no peers" inexpliqué

### 10.4 Problèmes de sécurité

| Problème | Sévérité | Description |
|----------|----------|-------------|
| MAC 2 bytes | Moyenne | Facilite les attaques par forge |
| Pas de replay protection | Moyenne | Packets peuvent être rejoués |
| Hash 1 byte | Faible | Facilite les collisions |
| Pas de forward secrecy | Moyenne | Clé compromise = tout déchiffrable |
| Clock sync requis | Faible | Timestamps pourraient être manipulés |

### 10.5 Ce qui est mal documenté

1. **Auto-add contact types:** Bits 1-4 du masque auto-add non clairement définis
2. **Transport codes:** Usage exact des transport codes peu documenté
3. **Multipart protocol:** Spécification incomplète pour gros messages
4. **Bridge mode:** Fonctionnement interne des bridges RS232/ESPNow
5. **Tuning params:** Effet exact des paramètres de tuning radio

---

## 11. ANNEXES

### 11.1 Codes d'erreur Companion

```cpp
#define ERR_CODE_UNSUPPORTED_CMD    1
#define ERR_CODE_NOT_FOUND          2
#define ERR_CODE_TABLE_FULL         3
#define ERR_CODE_BAD_STATE          4
#define ERR_CODE_FILE_IO_ERROR      5
#define ERR_CODE_ILLEGAL_ARG        6
```

### 11.2 Types de requêtes serveur

```cpp
#define REQ_TYPE_GET_STATUS         0x01
#define REQ_TYPE_KEEP_ALIVE         0x02  // Deprecated
#define REQ_TYPE_GET_TELEMETRY_DATA 0x03
#define REQ_TYPE_GET_MIN_MAX_AVG    0x04  // Sensor nodes
#define REQ_TYPE_GET_ACCESS_LIST    0x05
#define REQ_TYPE_GET_NEIGHBORS      0x06
#define REQ_TYPE_GET_OWNER_INFO     0x07
```

### 11.3 Flags Advert Appdata

```cpp
#define ADV_TYPE_CHAT               0x01
#define ADV_TYPE_REPEATER           0x02
#define ADV_TYPE_ROOM_SERVER        0x03
#define ADV_TYPE_SENSOR             0x04
#define ADV_HAS_LOCATION            0x10
#define ADV_HAS_FEATURE_1           0x20
#define ADV_HAS_FEATURE_2           0x40
#define ADV_HAS_NAME                0x80
```

---

**Fin du rapport**

*Document généré le 24 Février 2026*  
*Basé sur MeshCore firmware v1.12.0 - v1.13.0*
