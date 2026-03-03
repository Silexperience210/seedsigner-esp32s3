#pragma once

#include <stdint.h>
#include <stddef.h>

namespace BitcoinAddress {

// Types d'adresses supportés
enum AddressType {
    ADDRESS_P2WPKH,      // Native SegWit (bech32) - bc1q...
    ADDRESS_P2SH_P2WPKH, // Nested SegWit (base58) - 3...
    ADDRESS_P2PKH        // Legacy (base58) - 1...
};

// Génère une adresse Bitcoin à partir d'une clé publique (33 bytes compressed)
// network: 'm' = mainnet, 't' = testnet
bool generate_address(const uint8_t pubkey[33], AddressType type, char* out, size_t out_len, char network = 'm');

// Génère une adresse P2WPKH native SegWit (bech32)
// pubkey: 33 bytes compressed
// network: 'm' = mainnet (bc1q), 't' = testnet (tb1q)
bool generate_p2wpkh(const uint8_t pubkey[33], char* out, size_t out_len, char network = 'm');

// Génère une adresse P2SH-P2WPKH (Nested SegWit)
bool generate_p2sh_p2wpkh(const uint8_t pubkey[33], char* out, size_t out_len, char network = 'm');

// Génère une adresse P2PKH Legacy
bool generate_p2pkh(const uint8_t pubkey[33], char* out, size_t out_len, char network = 'm');

// Encode en Bech32 (BIP173)
// hrp: "bc" pour mainnet, "tb" pour testnet
bool bech32_encode(const char* hrp, const uint8_t* data, size_t data_len, char* out, size_t out_len);

// Decode Bech32
bool bech32_decode(const char* addr, char* hrp_out, uint8_t* data_out, size_t* data_len);

// Convertit hash160 en adresse P2WPKH
bool hash160_to_p2wpkh(const uint8_t hash160[20], char* out, size_t out_len, char network = 'm');

} // namespace BitcoinAddress
