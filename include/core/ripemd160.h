#pragma once

#include <stdint.h>
#include <stddef.h>

// RIPEMD160 implementation - encapsulated in namespace to avoid conflicts with libraries
namespace SeedCrypto {

void ripemd160(const uint8_t* data, size_t len, uint8_t hash[20]);

} // namespace SeedCrypto
