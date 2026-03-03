#include "core/secp256k1_wrapper.h"
#include <Bitcoin.h>
#include <string.h>

namespace Secp256k1 {

bool generate_public_key(const uint8_t privkey[PRIVATE_KEY_SIZE], 
                         uint8_t pubkey[PUBLIC_KEY_COMPRESSED_SIZE], 
                         bool compressed) {
    PrivateKey priv(privkey, PRIVATE_KEY_SIZE);
    if (!priv.isValid()) {
        return false;
    }
    PublicKey pub = priv.getPublicKey();
    return pub.toBytes(pubkey, compressed) == (compressed ? 33 : 65);
}

bool generate_public_key_uncompressed(const uint8_t privkey[PRIVATE_KEY_SIZE],
                                      uint8_t pubkey[PUBLIC_KEY_UNCOMPRESSED_SIZE]) {
    return generate_public_key(privkey, pubkey, false);
}

bool sign(const uint8_t privkey[PRIVATE_KEY_SIZE],
          const uint8_t hash[32],
          uint8_t signature[SIGNATURE_SIZE],
          uint8_t* recid) {
    PrivateKey priv(privkey, PRIVATE_KEY_SIZE);
    if (!priv.isValid()) {
        return false;
    }
    
    Signature sig = priv.sign(hash, 32);
    if (!sig.isValid()) {
        return false;
    }
    
    // Export signature en format compact (64 bytes: r || s)
    uint8_t der[72];
    size_t der_len = sig.toBytes(der, sizeof(der));
    if (der_len == 0) {
        return false;
    }
    
    // Convertir DER vers format compact
    // DER: 0x30 [len] 0x02 [len_r] [r] 0x02 [len_s] [s]
    if (der[0] != 0x30) {
        return false;
    }
    
    size_t offset = 2;
    if (der[1] & 0x80) {
        offset += der[1] & 0x7F;
    }
    
    // Extraire r
    if (der[offset] != 0x02) {
        return false;
    }
    size_t r_len = der[offset + 1];
    const uint8_t* r_bytes = &der[offset + 2];
    
    // Padding r à 32 bytes
    memset(signature, 0, 32);
    size_t r_offset = 32 > r_len ? 32 - r_len : 0;
    memcpy(signature + r_offset, r_bytes, r_len > 32 ? 32 : r_len);
    
    offset += 2 + r_len;
    
    // Extraire s
    if (der[offset] != 0x02) {
        return false;
    }
    size_t s_len = der[offset + 1];
    const uint8_t* s_bytes = &der[offset + 2];
    
    // Padding s à 32 bytes
    memset(signature + 32, 0, 32);
    size_t s_offset = 32 > s_len ? 32 - s_len : 0;
    memcpy(signature + 32 + s_offset, s_bytes, s_len > 32 ? 32 : s_len);
    
    if (recid) {
        *recid = sig.getRecid();
    }
    
    return true;
}

bool verify(const uint8_t pubkey[PUBLIC_KEY_COMPRESSED_SIZE],
            const uint8_t hash[32],
            const uint8_t signature[SIGNATURE_SIZE]) {
    PublicKey pub(pubkey, PUBLIC_KEY_COMPRESSED_SIZE);
    if (!pub.isValid()) {
        return false;
    }
    
    // Convertir format compact vers DER
    uint8_t der[72];
    der[0] = 0x30;
    
    // r
    size_t r_start = 0;
    while (r_start < 32 && signature[r_start] == 0) r_start++;
    bool r_pad = (signature[r_start] & 0x80) != 0;
    size_t r_len = 32 - r_start + (r_pad ? 1 : 0);
    
    // s
    size_t s_start = 32;
    while (s_start < 64 && signature[s_start] == 0) s_start++;
    bool s_pad = (signature[s_start] & 0x80) != 0;
    size_t s_len = 64 - s_start + (s_pad ? 1 : 0);
    
    size_t total_len = 2 + 2 + r_len + 2 + s_len;
    der[1] = total_len;
    
    size_t idx = 2;
    der[idx++] = 0x02;
    der[idx++] = r_len;
    if (r_pad) {
        der[idx++] = 0;
        r_len--;
    }
    memcpy(&der[idx], &signature[r_start], r_len);
    idx += r_len;
    
    der[idx++] = 0x02;
    der[idx++] = s_len;
    if (s_pad) {
        der[idx++] = 0;
        s_len--;
    }
    memcpy(&der[idx], &signature[s_start], s_len);
    
    Signature sig(der, total_len + 2);
    return pub.verify(hash, 32, sig);
}

bool seckey_tweak_add(uint8_t seckey[PRIVATE_KEY_SIZE], const uint8_t tweak[32]) {
    PrivateKey priv(seckey, PRIVATE_KEY_SIZE);
    if (!priv.isValid()) {
        return false;
    }
    
    // Convertir tweak en PrivateKey
    PrivateKey tweak_priv(tweak, 32);
    
    // Additionner les clés privées (tweak add)
    PrivateKey result = priv + tweak_priv;
    if (!result.isValid()) {
        return false;
    }
    
    result.toBytes(seckey, 32);
    return true;
}

bool pubkey_tweak_add(uint8_t pubkey[PUBLIC_KEY_COMPRESSED_SIZE], const uint8_t tweak[32]) {
    PublicKey pub(pubkey, PUBLIC_KEY_COMPRESSED_SIZE);
    if (!pub.isValid()) {
        return false;
    }
    
    // Créer une clé privée à partir du tweak pour générer un point
    PrivateKey tweak_priv(tweak, 32);
    PublicKey tweak_pub = tweak_priv.getPublicKey();
    
    // Additionner les points
    PublicKey result = pub + tweak_pub;
    if (!result.isValid()) {
        return false;
    }
    
    return result.toBytes(pubkey, true) == 33;
}

bool is_valid_seckey(const uint8_t seckey[PRIVATE_KEY_SIZE]) {
    PrivateKey priv(seckey, PRIVATE_KEY_SIZE);
    return priv.isValid();
}

void hash160(const uint8_t* data, size_t len, uint8_t hash160_out[20]) {
    uint8_t sha[32];
    
    // SHA256
    extern void sha256(const uint8_t* data, size_t len, uint8_t hash[32]);
    sha256(data, len, sha);
    
    // RIPEMD160
    extern void ripemd160(const uint8_t* data, size_t len, uint8_t hash[20]);
    ripemd160(sha, 32, hash160_out);
}

} // namespace Secp256k1
