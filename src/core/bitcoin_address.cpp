#include "core/bitcoin_address.h"
#include "core/secp256k1_wrapper.h"
#include <string.h>
#include <stdio.h>

// External declarations
extern void sha256(const uint8_t* data, size_t len, uint8_t hash[32]);
extern void ripemd160(const uint8_t* data, size_t len, uint8_t hash[20]);

namespace BitcoinAddress {

// Base58 encoding
static const char BASE58_CHARS[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

static void base58_encode(const uint8_t* data, size_t len, char* out, size_t* out_len) {
    // Count leading zeros
    size_t zeros = 0;
    while (zeros < len && data[zeros] == 0) zeros++;
    
    // Temporary buffer
    size_t size = (len - zeros) * 138 / 100 + 1;
    uint8_t* b58 = new uint8_t[size];
    memset(b58, 0, size);
    
    // Process bytes
    for (size_t i = zeros; i < len; i++) {
        uint32_t carry = data[i];
        for (size_t j = 0; j < size; j++) {
            carry += (uint32_t)b58[j] << 8;
            b58[j] = carry % 58;
            carry /= 58;
        }
    }
    
    // Skip leading zeros
    size_t b58_zeros = 0;
    while (b58_zeros < size && b58[b58_zeros] == 0) b58_zeros++;
    
    // Build result
    size_t idx = 0;
    for (size_t i = 0; i < zeros; i++) {
        out[idx++] = '1';
    }
    for (size_t i = b58_zeros; i < size; i++) {
        out[idx++] = BASE58_CHARS[b58[size - 1 - i]];
    }
    out[idx] = '\0';
    *out_len = idx;
    
    delete[] b58;
}

// Bech32 encoding (BIP173)
static const char BECH32_CHARSET[] = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

static uint32_t bech32_polymod(const uint8_t* values, size_t len) {
    static const uint32_t GENERATORS[] = {
        0x3b6a57b2, 0x26508e6d, 0x1ea119fa, 0x3d4233dd, 0x2a1462b3
    };
    
    uint32_t chk = 1;
    for (size_t i = 0; i < len; i++) {
        uint8_t b = chk >> 25;
        chk = ((chk & 0x1ffffff) << 5) ^ values[i];
        for (int j = 0; j < 5; j++) {
            chk ^= ((b >> j) & 1) ? GENERATORS[j] : 0;
        }
    }
    return chk;
}

static void bech32_hrp_expand(const char* hrp, uint8_t* out) {
    size_t hrp_len = strlen(hrp);
    for (size_t i = 0; i < hrp_len; i++) {
        out[i] = hrp[i] >> 5;
    }
    out[hrp_len] = 0;
    for (size_t i = 0; i < hrp_len; i++) {
        out[hrp_len + 1 + i] = hrp[i] & 0x1f;
    }
}

static bool bech32_convert_bits(const uint8_t* data, size_t data_len,
                                  uint8_t** out, size_t* out_len,
                                  int from_bits, int to_bits, bool pad) {
    int acc = 0;
    int bits = 0;
    size_t max_out_len = (data_len * from_bits + to_bits - 1) / to_bits;
    *out = new uint8_t[max_out_len];
    size_t j = 0;
    
    for (size_t i = 0; i < data_len; i++) {
        acc = (acc << from_bits) | data[i];
        bits += from_bits;
        while (bits >= to_bits) {
            (*out)[j++] = (acc >> (bits - to_bits)) & ((1 << to_bits) - 1);
            bits -= to_bits;
        }
    }
    
    if (pad && bits > 0) {
        (*out)[j++] = (acc << (to_bits - bits)) & ((1 << to_bits) - 1);
    }
    
    // Check padding
    if (!pad && bits >= from_bits) {
        delete[] *out;
        return false;
    }
    
    *out_len = j;
    return true;
}

bool bech32_encode(const char* hrp, const uint8_t* data, size_t data_len, char* out, size_t out_len) {
    // Convert 8-bit to 5-bit
    uint8_t* data5;
    size_t data5_len;
    if (!bech32_convert_bits(data, data_len, &data5, &data5_len, 8, 5, true)) {
        return false;
    }
    
    size_t hrp_len = strlen(hrp);
    size_t total_len = hrp_len + 1 + data5_len + 6; // hrp + '1' + data + checksum
    
    if (out_len < total_len + 1) {
        delete[] data5;
        return false;
    }
    
    // Create checksum input
    size_t chk_len = hrp_len * 2 + 1 + data5_len + 6;
    uint8_t* chk_data = new uint8_t[chk_len];
    bech32_hrp_expand(hrp, chk_data);
    memcpy(chk_data + hrp_len * 2 + 1, data5, data5_len);
    
    // Calculate checksum
    uint32_t polymod = bech32_polymod(chk_data, hrp_len * 2 + 1 + data5_len) ^ 1;
    
    // Build output
    memcpy(out, hrp, hrp_len);
    out[hrp_len] = '1';
    
    for (size_t i = 0; i < data5_len; i++) {
        out[hrp_len + 1 + i] = BECH32_CHARSET[data5[i]];
    }
    
    for (int i = 0; i < 6; i++) {
        out[hrp_len + 1 + data5_len + i] = BECH32_CHARSET[(polymod >> (5 * (5 - i))) & 0x1f];
    }
    
    out[total_len] = '\0';
    
    delete[] data5;
    delete[] chk_data;
    return true;
}

bool bech32_decode(const char* addr, char* hrp_out, uint8_t* data_out, size_t* data_len) {
    // Find separator position
    size_t addr_len = strlen(addr);
    size_t sep_pos = 0;
    for (size_t i = 0; i < addr_len; i++) {
        if (addr[i] == '1') {
            sep_pos = i;
            break;
        }
    }
    
    if (sep_pos == 0 || sep_pos + 7 > addr_len) {
        return false;
    }
    
    // Extract HRP
    memcpy(hrp_out, addr, sep_pos);
    hrp_out[sep_pos] = '\0';
    
    // Decode data characters
    size_t data_str_len = addr_len - sep_pos - 1 - 6; // Exclude checksum
    uint8_t* data5 = new uint8_t[data_str_len];
    
    for (size_t i = 0; i < data_str_len; i++) {
        char c = addr[sep_pos + 1 + i];
        const char* ptr = strchr(BECH32_CHARSET, c);
        if (!ptr) {
            delete[] data5;
            return false;
        }
        data5[i] = ptr - BECH32_CHARSET;
    }
    
    // Verify checksum
    size_t hrp_len = sep_pos;
    size_t chk_len = hrp_len * 2 + 1 + data_str_len + 6;
    uint8_t* chk_data = new uint8_t[chk_len];
    bech32_hrp_expand(hrp_out, chk_data);
    memcpy(chk_data + hrp_len * 2 + 1, data5, data_str_len);
    
    // Add checksum to verification
    for (int i = 0; i < 6; i++) {
        char c = addr[addr_len - 6 + i];
        const char* ptr = strchr(BECH32_CHARSET, c);
        chk_data[hrp_len * 2 + 1 + data_str_len + i] = ptr - BECH32_CHARSET;
    }
    
    if (bech32_polymod(chk_data, chk_len) != 1) {
        delete[] data5;
        delete[] chk_data;
        return false;
    }
    
    // Convert 5-bit to 8-bit
    uint8_t* data8;
    size_t data8_len;
    if (!bech32_convert_bits(data5, data_str_len, &data8, &data8_len, 5, 8, false)) {
        delete[] data5;
        delete[] chk_data;
        return false;
    }
    
    memcpy(data_out, data8, data8_len);
    *data_len = data8_len;
    
    delete[] data5;
    delete[] chk_data;
    delete[] data8;
    return true;
}

bool hash160_to_p2wpkh(const uint8_t hash160[20], char* out, size_t out_len, char network) {
    // P2WPKH: witness version 0 + 20-byte hash160
    uint8_t data[21];
    data[0] = 0x00; // Witness version 0
    memcpy(data + 1, hash160, 20);
    
    const char* hrp = (network == 'm') ? "bc" : "tb";
    return bech32_encode(hrp, data, 21, out, out_len);
}

bool generate_p2wpkh(const uint8_t pubkey[33], char* out, size_t out_len, char network) {
    // HASH160(pubkey) = RIPEMD160(SHA256(pubkey))
    uint8_t hash160[20];
    Secp256k1::hash160(pubkey, 33, hash160);
    
    return hash160_to_p2wpkh(hash160, out, out_len, network);
}

bool generate_p2sh_p2wpkh(const uint8_t pubkey[33], char* out, size_t out_len, char network) {
    // HASH160(pubkey)
    uint8_t hash160[20];
    Secp256k1::hash160(pubkey, 33, hash160);
    
    // RedeemScript: 0x0014 <20 bytes>
    uint8_t redeem[22];
    redeem[0] = 0x00;
    redeem[1] = 0x14;
    memcpy(redeem + 2, hash160, 20);
    
    // HASH160(redeemScript)
    uint8_t script_hash[20];
    uint8_t sha[32];
    sha256(redeem, 22, sha);
    ripemd160(sha, 32, script_hash);
    
    // Base58Check with version 0x05 (mainnet) or 0xc4 (testnet)
    uint8_t data[21];
    data[0] = (network == 'm') ? 0x05 : 0xc4;
    memcpy(data + 1, script_hash, 20);
    
    // Double SHA256 for checksum
    uint8_t hash1[32], hash2[32];
    sha256(data, 21, hash1);
    sha256(hash1, 32, hash2);
    
    // Append checksum and encode
    uint8_t with_checksum[25];
    memcpy(with_checksum, data, 21);
    memcpy(with_checksum + 21, hash2, 4);
    
    size_t b58_len;
    base58_encode(with_checksum, 25, out, &b58_len);
    
    return b58_len > 0;
}

bool generate_p2pkh(const uint8_t pubkey[33], char* out, size_t out_len, char network) {
    // HASH160(pubkey)
    uint8_t hash160[20];
    Secp256k1::hash160(pubkey, 33, hash160);
    
    // Base58Check with version 0x00 (mainnet) or 0x6f (testnet)
    uint8_t data[21];
    data[0] = (network == 'm') ? 0x00 : 0x6f;
    memcpy(data + 1, hash160, 20);
    
    // Double SHA256 for checksum
    uint8_t hash1[32], hash2[32];
    sha256(data, 21, hash1);
    sha256(hash1, 32, hash2);
    
    // Append checksum and encode
    uint8_t with_checksum[25];
    memcpy(with_checksum, data, 21);
    memcpy(with_checksum + 21, hash2, 4);
    
    size_t b58_len;
    base58_encode(with_checksum, 25, out, &b58_len);
    
    return b58_len > 0;
}

bool generate_address(const uint8_t pubkey[33], AddressType type, char* out, size_t out_len, char network) {
    switch (type) {
        case ADDRESS_P2WPKH:
            return generate_p2wpkh(pubkey, out, out_len, network);
        case ADDRESS_P2SH_P2WPKH:
            return generate_p2sh_p2wpkh(pubkey, out, out_len, network);
        case ADDRESS_P2PKH:
            return generate_p2pkh(pubkey, out, out_len, network);
        default:
            return false;
    }
}

} // namespace BitcoinAddress
