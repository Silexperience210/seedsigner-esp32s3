# 📋 PLAN D'ACTION - MESSAGERIE BITMESH 100% FONCTIONNELLE

**Date**: 24 Février 2026  
**Objectif**: Envoi/réception de messages LoRa fonctionnel sur APK BitMesh  
**Base**: Analyse protocole MeshCore v1.12.0-1.13.0

---

## 🚨 PROBLÈMES CRITIQUES IDENTIFIÉS

### 1. Format CMD_SEND_TXT_MSG INCORRECT ❌
**Actuel**: Envoie `pubkeyPrefix6` dans le payload  
**Requis**: Envoie `contact_idx` (index dans la table contacts du firmware)

### 2. Pas de synchronisation des contacts ❌
**Actuel**: On stocke les contacts localement sans leur index firmware  
**Requis**: Récupérer la liste contacts via `CMD_GET_CONTACTS` pour obtenir les index

### 3. Parsing PUSH_SEND_CONFIRMED incomplet ❌
**Actuel**: Parse basique du code ACK  
**Requis**: Parser `ack_code(4) + round_trip_ms(4)`

### 4. Pas de gestion CMD_GET_CONTACTS ❌
**Manque**: Impossible de récupérer les contacts existants sur le device

---

## 📌 PHASE 1: CORRECTION FORMAT COMMANDES BLE (CRITIQUE)

### 1.1 Nouvelle structure Contact avec index firmware

```typescript
// types/contact.ts
export interface MeshContact {
  // Index dans la table du firmware (CRITIQUE pour l'envoi)
  firmwareIndex: number;
  
  // Identité
  pubkeyHex: string;
  pubkeyPrefix: string; // 6 premiers bytes hex
  hash: string; // 1er byte = hash MeshCore
  
  // Métadonnées
  name: string;
  type: ContactType;
  flags: number;
  
  // Path connu (pour routage DIRECT)
  outPathLen: number;
  outPath: Uint8Array;
  
  // Timestamps
  lastAdvert: number;
  lastmod: number;
  
  // GPS (optionnel)
  gpsLat?: number;
  gpsLon?: number;
}

export enum ContactType {
  CHAT = 0,
  REPEATER = 1,
  ROOM_SERVER = 2,
  SENSOR = 3,
}
```

### 1.2 Correction sendDirectMessage()

```typescript
// utils/ble-gateway.ts
async sendDirectMessage(
  contactIndex: number,  // ← CHANGÉ: index firmware, pas pubkey!
  text: string, 
  attempt = 0
): Promise<void> {
  if (!this.connectedId) throw new Error('Non connecté');
  
  const ts = Math.floor(Date.now() / 1000);
  const textBytes = new TextEncoder().encode(text);
  
  // Format MeshCore Companion Protocol:
  // [contact_idx(1)] [txtType(1)] [attempt(1)] [timestamp(4)] [text(var)]
  const payload = new Uint8Array(1 + 1 + 1 + 4 + textBytes.length);
  let off = 0;
  
  payload[off++] = contactIndex & 0xFF;     // ← INDEX FIRMWARE (0-99)
  payload[off++] = 0x00;                     // txtType = plain text
  payload[off++] = attempt & 0xFF;           // attempt number
  
  const view = new DataView(payload.buffer, payload.byteOffset);
  view.setUint32(off, ts, true);             // timestamp LE
  off += 4;
  
  payload.set(textBytes, off);
  
  if (__DEV__) {
    console.log(`[BleGateway] CMD_SEND_TXT_MSG idx=${contactIndex}, text="${text.substring(0, 30)}..."`);
  }
  
  await this.sendFrame(CMD_SEND_TXT_MSG, payload);
}
```

### 1.3 Correction sendChannelMessage()

```typescript
async sendChannelMessage(
  channelIndex: number,  // 0 = public, 1-7 = private
  text: string
): Promise<void> {
  if (!this.connectedId) throw new Error('Non connecté');
  
  const textBytes = new TextEncoder().encode(text);
  
  // Format: [channel_idx(1)] [text(var)]
  const payload = new Uint8Array(1 + textBytes.length);
  payload[0] = channelIndex & 0xFF;
  payload.set(textBytes, 1);
  
  if (__DEV__) {
    console.log(`[BleGateway] CMD_SEND_CHANNEL_TXT_MSG ch=${channelIndex}, text="${text.substring(0, 30)}..."`);
  }
  
  await this.sendFrame(CMD_SEND_CHAN_MSG, payload);
}
```

### 1.4 Correction du mapping contact → index

```typescript
// providers/BleProvider.tsx
const sendDirectMessage = useCallback(async (pubkeyHex: string, text: string) => {
  // Trouver le contact dans NOTRE liste
  const contact = bleMeshContacts.find(c => c.pubkeyHex === pubkeyHex);
  if (!contact) {
    throw new Error('Contact non trouvé dans la liste locale');
  }
  
  // Vérifier qu'on a un index firmware valide
  if (contact.firmwareIndex === undefined || contact.firmwareIndex < 0) {
    // Le contact n'existe pas encore sur le firmware
    // Il faut d'abord l'ajouter via CMD_ADD_UPDATE_CONTACT
    throw new Error('Contact non synchronisé avec le firmware. Scannez à nouveau.');
  }
  
  await client.sendDirectMessage(contact.firmwareIndex, text);
}, [bleMeshContacts]);
```

---

## 📌 PHASE 2: IMPLÉMENTATION RÉCUPÉRATION CONTACTS

### 2.1 Commande CMD_GET_CONTACTS

```typescript
// utils/ble-gateway.ts
private contactsResolve: ((contacts: MeshContact[]) => void) | null = null;
private contactsTemp: MeshContact[] = [];

async getContacts(sinceTimestamp = 0): Promise<MeshContact[]> {
  if (!this.connectedId) throw new Error('Non connecté');
  
  return new Promise((resolve, reject) => {
    const timeout = setTimeout(() => {
      this.contactsResolve = null;
      this.contactsTemp = [];
      reject(new Error('Timeout récupération contacts'));
    }, 10000);
    
    this.contactsResolve = (contacts) => {
      clearTimeout(timeout);
      resolve(contacts);
    };
    this.contactsTemp = [];
    
    // Format: [since_timestamp(4)] - optionnel, 0 = tous
    const payload = new Uint8Array(4);
    new DataView(payload.buffer).setUint32(0, sinceTimestamp, true);
    
    this.sendFrame(CMD_GET_CONTACTS, payload);
  });
}
```

### 2.2 Parsing réponse RESP_CODE_CONTACT (0x03)

```typescript
// Dans handleFrame():
case RESP_CODE_CONTACT:
  this.parseContactInfo(payload);
  break;

case RESP_CODE_END_OF_CONTACTS:
  if (this.contactsResolve) {
    this.contactsResolve(this.contactsTemp);
    this.contactsResolve = null;
    this.contactsTemp = [];
  }
  break;

private parseContactInfo(data: Uint8Array): void {
  // Structure ContactInfo (148 bytes):
  // Offset  Size  Field
  // 0       1     Response code (0x03)
  // 1       32    Public key
  // 33      1     Type
  // 34      1     Flags
  // 35      1     out_path_len
  // 36      64    out_path
  // 100     32    Name (null-terminated)
  // 132     4     last_advert_timestamp
  // 136     4     gps_lat
  // 140     4     gps_lon
  // 144     4     lastmod
  
  if (data.length < 148) {
    console.warn('[BleGateway] ContactInfo tronqué:', data.length);
    return;
  }
  
  const view = new DataView(data.buffer, data.byteOffset);
  
  const pubkey = data.slice(1, 33);
  const type = data[33];
  const flags = data[34];
  const outPathLen = data[35];
  const outPath = data.slice(36, 36 + outPathLen);
  
  // Name: null-terminated string
  let nameEnd = 100;
  while (nameEnd < 132 && data[nameEnd] !== 0) nameEnd++;
  const name = new TextDecoder().decode(data.slice(100, nameEnd));
  
  const lastAdvert = view.getUint32(132, true);
  const gpsLat = view.getFloat32(136, true);
  const gpsLon = view.getFloat32(140, true);
  const lastmod = view.getUint32(144, true);
  
  // L'index est déterminé par l'ordre de réception
  const firmwareIndex = this.contactsTemp.length;
  
  const contact: MeshContact = {
    firmwareIndex,
    pubkeyHex: Buffer.from(pubkey).toString('hex'),
    pubkeyPrefix: Buffer.from(pubkey.slice(0, 6)).toString('hex'),
    hash: Buffer.from(pubkey.slice(0, 1)).toString('hex'),
    name: name || `MESH-${Buffer.from(pubkey.slice(0, 4)).toString('hex').toUpperCase()}`,
    type,
    flags,
    outPathLen,
    outPath,
    lastAdvert,
    lastmod,
    gpsLat: gpsLat !== 0 ? gpsLat : undefined,
    gpsLon: gpsLon !== 0 ? gpsLon : undefined,
  };
  
  this.contactsTemp.push(contact);
  
  if (__DEV__) {
    console.log(`[BleGateway] Contact reçu #${firmwareIndex}: ${contact.name}`);
  }
}
```

### 2.3 Synchronisation au démarrage

```typescript
// providers/BleProvider.tsx
useEffect(() => {
  if (connected && client) {
    // Synchroniser les contacts au démarrage
    syncContacts();
  }
}, [connected, client]);

const syncContacts = async () => {
  try {
    setState(prev => ({ ...prev, syncingContacts: true }));
    
    // Récupérer tous les contacts du firmware
    const firmwareContacts = await client.getContacts(0);
    
    // Fusionner avec nos contacts locaux
    setState(prev => {
      const merged = [...prev.meshContacts];
      
      firmwareContacts.forEach(fwContact => {
        const existing = merged.find(c => c.pubkeyHex === fwContact.pubkeyHex);
        if (existing) {
          // Mettre à jour l'index firmware
          existing.firmwareIndex = fwContact.firmwareIndex;
          existing.name = fwContact.name || existing.name;
        } else {
          merged.push(fwContact);
        }
      });
      
      return { 
        ...prev, 
        meshContacts: merged,
        syncingContacts: false 
      };
    });
    
  } catch (err) {
    console.error('Sync contacts failed:', err);
    setState(prev => ({ ...prev, syncingContacts: false }));
  }
};
```

---

## 📌 PHASE 3: CORRECTION PARSING RÉPONSES FIRMWARE

### 3.1 Codes réponse à implémenter

```typescript
// Constantes MeshCore Companion Protocol
const RESP_CODE_OK = 0x00;
const RESP_CODE_ERR = 0x01;
const RESP_CODE_CONTACTS_START = 0x02;
const RESP_CODE_CONTACT = 0x03;
const RESP_CODE_END_OF_CONTACTS = 0x04;
const RESP_CODE_CONTACT_MSG_RECV = 0x07;      // v<3
const RESP_CODE_NO_MORE_MESSAGES = 0x10;
const RESP_CODE_CHANNEL_INFO = 0x12;
const RESP_CODE_CONTACT_MSG_RECV_V3 = 0x16;   // v3+
const RESP_CODE_SEND_CONFIRMED = 0x06;        // Confirmation envoi accepté

// Push codes
const PUSH_CODE_ADVERT = 0x80;
const PUSH_CODE_PATH_UPDATED = 0x81;
const PUSH_CODE_SEND_CONFIRMED = 0x82;
const PUSH_CODE_MSG_WAITING = 0x83;
```

### 3.2 Gestion complète des frames

```typescript
private handleFrame(data: Uint8Array): void {
  if (data.length < 1) return;
  
  const code = data[0];
  const payload = data.slice(1);
  
  if (__DEV__) {
    console.log(`[BleGateway] Frame reçu: 0x${code.toString(16).padStart(2, '0')}, len=${data.length}`);
  }
  
  switch (code) {
    case RESP_CODE_OK:
      if (__DEV__) console.log('[BleGateway] RESP_OK');
      break;
      
    case RESP_CODE_ERR:
      this.handleError(payload);
      break;
      
    case RESP_CODE_CONTACTS_START:
      this.contactsTemp = [];
      break;
      
    case RESP_CODE_CONTACT:
      this.parseContactInfo(payload);
      break;
      
    case RESP_CODE_END_OF_CONTACTS:
      if (this.contactsResolve) {
        this.contactsResolve(this.contactsTemp);
        this.contactsResolve = null;
      }
      break;
      
    case RESP_CODE_CONTACT_MSG_RECV:
    case RESP_CODE_CONTACT_MSG_RECV_V3:
      this.parseReceivedMessage(payload, code === RESP_CODE_CONTACT_MSG_RECV_V3);
      break;
      
    case RESP_CODE_NO_MORE_MESSAGES:
      if (__DEV__) console.log('[BleGateway] Plus de messages en attente');
      break;
      
    case RESP_CODE_SEND_CONFIRMED:
      // Message accepté par le firmware, en attente d'ACK LoRa
      if (__DEV__) console.log('[BleGateway] Message accepté par firmware');
      break;
      
    case PUSH_CODE_ADVERT:
      this.parseAdvert(payload);
      break;
      
    case PUSH_CODE_PATH_UPDATED:
      this.parsePathUpdated(payload);
      break;
      
    case PUSH_CODE_SEND_CONFIRMED:
      this.parseSendConfirmed(payload);
      break;
      
    case PUSH_CODE_MSG_WAITING:
      // Des messages sont en attente dans la file offline
      this.syncOfflineMessages();
      break;
      
    default:
      if (__DEV__) console.log(`[BleGateway] Code non géré: 0x${code.toString(16)}`);
  }
}
```

### 3.3 Parsing message reçu

```typescript
private onMessageCallbacks: ((msg: MeshMessage) => void)[] = [];

onMessage(cb: (msg: MeshMessage) => void): () => void {
  this.onMessageCallbacks.push(cb);
  return () => {
    const idx = this.onMessageCallbacks.indexOf(cb);
    if (idx > -1) this.onMessageCallbacks.splice(idx, 1);
  };
}

private parseReceivedMessage(data: Uint8Array, isV3: boolean): void {
  // Format V3:
  // [contact_idx(1)] [path_len(1)] [path(var)] [timestamp(4)] [text(var)]
  
  if (data.length < 6) {
    console.warn('[BleGateway] Message trop court');
    return;
  }
  
  const view = new DataView(data.buffer, data.byteOffset);
  let off = 0;
  
  const contactIdx = data[off++];
  const pathLen = data[off++];
  const path = data.slice(off, off + pathLen);
  off += pathLen;
  
  const timestamp = view.getUint32(off, true);
  off += 4;
  
  const textBytes = data.slice(off);
  const text = new TextDecoder().decode(textBytes);
  
  const msg: MeshMessage = {
    contactIndex: contactIdx,
    path: Array.from(path),
    timestamp,
    text,
    receivedAt: Date.now(),
  };
  
  if (__DEV__) {
    console.log(`[BleGateway] Message reçu de #${contactIdx}: "${text.substring(0, 30)}..."`);
  }
  
  this.onMessageCallbacks.forEach(cb => {
    try { cb(msg); } catch (e) {}
  });
}
```

---

## 📌 PHASE 4: GESTION PUSH_SEND_CONFIRMED

### 4.1 Parsing confirmation

```typescript
interface SendConfirmation {
  ackCode: number;      // Code ACK (checksum du message)
  roundTripMs: number;  // Temps aller-retour en ms
  contactIndex?: number; // Index du contact (si connu)
}

private onSendConfirmedCallbacks: ((conf: SendConfirmation) => void)[] = [];

onSendConfirmed(cb: (conf: SendConfirmation) => void): () => void {
  this.onSendConfirmedCallbacks.push(cb);
  return () => {
    const idx = this.onSendConfirmedCallbacks.indexOf(cb);
    if (idx > -1) this.onSendConfirmedCallbacks.splice(idx, 1);
  };
}

private parseSendConfirmed(data: Uint8Array): void {
  // Format: [ack_code(4)] [round_trip_ms(4)]
  if (data.length < 8) {
    console.warn('[BleGateway] SEND_CONFIRMED trop court');
    return;
  }
  
  const view = new DataView(data.buffer, data.byteOffset);
  const ackCode = view.getUint32(0, true);
  const roundTripMs = view.getUint32(4, true);
  
  const conf: SendConfirmation = {
    ackCode,
    roundTripMs,
  };
  
  if (__DEV__) {
    console.log(`[BleGateway] PUSH_SEND_CONFIRMED: ACK=${ackCode}, RTT=${roundTripMs}ms`);
  }
  
  this.onSendConfirmedCallbacks.forEach(cb => {
    try { cb(conf); } catch (e) {}
  });
}
```

### 4.2 UI Feedback

```typescript
// providers/BleProvider.tsx
const [pendingMessages, setPendingMessages] = useState<Map<number, PendingMessage>>(new Map());

useEffect(() => {
  if (!client) return;
  
  const unsub = client.onSendConfirmed((conf) => {
    // Mettre à jour le statut du message
    setPendingMessages(prev => {
      const next = new Map(prev);
      // Trouver le message par ackCode ou timestamp
      next.forEach((msg, key) => {
        if (msg.ackCode === conf.ackCode || 
            (Date.now() - msg.sentAt < 30000)) {
          next.set(key, { ...msg, confirmed: true, rtt: conf.roundTripMs });
        }
      });
      return next;
    });
    
    // Notifier l'utilisateur
    Alert.alert(
      '✅ Message confirmé',
      `Transmis sur LoRa et ACK reçu\nRTT: ${conf.roundTripMs}ms`
    );
  });
  
  return () => unsub();
}, [client]);
```

---

## 📌 PHASE 5: AJOUT CONTACT (CMD_ADD_UPDATE_CONTACT)

### 5.1 Ajout d'un contact scanné

```typescript
async addOrUpdateContact(contact: Omit<MeshContact, 'firmwareIndex'>): Promise<number> {
  if (!this.connectedId) throw new Error('Non connecté');
  
  // Format ContactInfo (148 bytes):
  const payload = new Uint8Array(148);
  const view = new DataView(payload.buffer);
  let off = 0;
  
  // Public key (32 bytes)
  const pubkey = Buffer.from(contact.pubkeyHex, 'hex');
  payload.set(pubkey.slice(0, 32), off);
  off += 32;
  
  // Type (1 byte)
  payload[off++] = contact.type ?? ContactType.CHAT;
  
  // Flags (1 byte)
  payload[off++] = contact.flags ?? 0;
  
  // out_path_len (1 byte)
  const pathLen = contact.outPath?.length ?? 0;
  payload[off++] = Math.min(pathLen, 64);
  
  // out_path (64 bytes)
  if (contact.outPath) {
    payload.set(contact.outPath.slice(0, 64), off);
  }
  off += 64;
  
  // Name (32 bytes, null-terminated)
  const nameBytes = new TextEncoder().encode(contact.name.slice(0, 31));
  payload.set(nameBytes, off);
  off += 32;
  
  // last_advert_timestamp (4 bytes)
  view.setUint32(off, contact.lastAdvert ?? 0, true);
  off += 4;
  
  // gps_lat (4 bytes)
  view.setFloat32(off, contact.gpsLat ?? 0, true);
  off += 4;
  
  // gps_lon (4 bytes)
  view.setFloat32(off, contact.gpsLon ?? 0, true);
  off += 4;
  
  // lastmod (4 bytes)
  view.setUint32(off, contact.lastmod ?? Math.floor(Date.now() / 1000), true);
  off += 4;
  
  await this.sendFrame(CMD_ADD_UPDATE_CONTACT, payload);
  
  // Le firmware retourne RESP_CODE_OK ou RESP_CODE_ERR
  // Puis il faut refaire un GET_CONTACTS pour obtenir l'index attribué
  
  return 0; // L'index sera déterminé après sync
}
```

---

## 📌 PHASE 6: RÉCUPÉRATION MESSAGES OFFLINE

### 6.1 Sync messages en attente

```typescript
async syncNextMessage(): Promise<MeshMessage | null> {
  if (!this.connectedId) throw new Error('Non connecté');
  
  // Envoyer CMD_SYNC_NEXT_MESSAGE
  await this.sendFrame(CMD_SYNC_NEXT_MESSAGE, new Uint8Array(0));
  
  // Attendre la réponse (RESP_CODE_CONTACT_MSG_RECV ou RESP_CODE_NO_MORE_MESSAGES)
  return new Promise((resolve) => {
    const timeout = setTimeout(() => resolve(null), 5000);
    
    const unsub = this.onMessage((msg) => {
      clearTimeout(timeout);
      unsub();
      resolve(msg);
    });
  });
}

async syncAllOfflineMessages(): Promise<MeshMessage[]> {
  const messages: MeshMessage[] = [];
  
  while (true) {
    const msg = await this.syncNextMessage();
    if (!msg) break;
    messages.push(msg);
  }
  
  return messages;
}
```

---

## 📌 PHASE 7: TEST & VALIDATION

### 7.1 Checklist de validation

- [ ] **Handshake BLE**: `CMD_APP_START` → `RESP_OK`
- [ ] **Sync contacts**: `CMD_GET_CONTACTS` → Liste complète
- [ ] **Ajout contact**: `CMD_ADD_UPDATE_CONTACT` → Contact visible après re-sync
- [ ] **Envoi DM**: `CMD_SEND_TXT_MSG` → `RESP_SEND_CONFIRMED` → `PUSH_SEND_CONFIRMED`
- [ ] **Envoi canal**: `CMD_SEND_CHANNEL_TXT_MSG` → Broadcast reçu
- [ ] **Réception**: `PUSH_MSG_WAITING` → `CMD_SYNC_NEXT_MESSAGE` → Message affiché
- [ ] **ACK**: Message envoyé → Confirmation RTT affichée

### 7.2 Scénarios de test

```typescript
// Test 1: Envoi vers contact connu
async function testSendDM() {
  // 1. Scanner/Synchroniser contacts
  const contacts = await bleGateway.getContacts(0);
  console.assert(contacts.length > 0, 'Aucun contact');
  
  // 2. Envoyer message
  const contact = contacts[0];
  await bleGateway.sendDirectMessage(contact.firmwareIndex, 'Test message');
  
  // 3. Attendre confirmation (max 30s)
  const confirmed = await waitForSendConfirmed(30000);
  console.assert(confirmed, 'Pas de confirmation reçue');
}

// Test 2: Envoi canal public
async function testSendChannel() {
  await bleGateway.sendChannelMessage(0, 'Hello public channel!');
  // Attendre confirmation...
}

// Test 3: Réception message
async function testReceive() {
  // Simuler réception ou attendre message réel
  const msg = await waitForMessage(60000);
  console.assert(msg.text.length > 0, 'Message vide');
  console.assert(msg.timestamp > 0, 'Timestamp invalide');
}
```

---

## 📌 PHASE 8: OPTIMISATIONS UI/UX

### 8.1 Indicateurs de statut

```typescript
// Afficher dans l'UI:
- 🔵 Envoyé (accepté par firmware)
- ⏳ En attente ACK (transmission LoRa en cours)
- ✅ Livré (ACK reçu, RTT: XXms)
- ❌ Échec (timeout ou erreur)
```

### 8.2 Gestion erreurs utilisateur

```typescript
const ERROR_MESSAGES = {
  'Contact non synchronisé': 'Scannez à nouveau ce contact pour l\'ajouter au device',
  'Timeout récupération contacts': 'Vérifiez la connexion BLE',
  'Message trop long': 'Maximum ~150 caractères par message',
  'Contact table full': 'Limite de 100 contacts atteinte',
};
```

---

## 🎯 RÉSUMÉ DES MODIFICATIONS REQUISES

### Fichiers à modifier:

1. **`utils/ble-gateway.ts`**
   - [ ] Corriger `sendDirectMessage()` (utiliser contactIndex)
   - [ ] Corriger `sendChannelMessage()` (ajouter channelIndex)
   - [ ] Ajouter `getContacts()`
   - [ ] Ajouter `addOrUpdateContact()`
   - [ ] Ajouter `syncNextMessage()`
   - [ ] Compléter `handleFrame()` avec tous les codes
   - [ ] Ajouter parsing `parseContactInfo()`
   - [ ] Ajouter parsing `parseReceivedMessage()`
   - [ ] Ajouter parsing `parseSendConfirmed()`

2. **`providers/BleProvider.tsx`**
   - [ ] Ajouter state `syncingContacts`
   - [ ] Ajouter méthode `syncContacts()`
   - [ ] Modifier `sendDirectMessage()` (trouver index par pubkey)
   - [ ] Gérer `onSendConfirmed` pour UI feedback
   - [ ] Gérer `onMessage` pour réception

3. **`types/contact.ts`** (nouveau)
   - [ ] Définir `MeshContact` avec `firmwareIndex`
   - [ ] Définir `MeshMessage`
   - [ ] Définir enums `ContactType`, etc.

4. **`app/(tabs)/mesh/index.tsx`**
   - [ ] Bouton sync contacts
   - [ ] Indicateur statut connexion firmware
   - [ ] Test message avec sélection contact

---

## ⚠️ POINTS D'ATTENTION CRITIQUES

1. **Index Firmware**: TOUJOURS utiliser l'index récupéré via `CMD_GET_CONTACTS`, jamais inventé
2. **Limites**: Respecter 100 contacts max, ~150 caractères/message
3. **Timeouts**: Attendre jusqu'à 30s pour ACK sur réseau LoRa saturé
4. **Sync**: Refaire `CMD_GET_CONTACTS` après chaque `CMD_ADD_UPDATE_CONTACT`
5. **Threading**: Toutes les opérations BLE doivent être sérialisées

---

**Prochaine étape**: Commencer par la Phase 1 (correction format commandes) puis Phase 2 (récupération contacts).
