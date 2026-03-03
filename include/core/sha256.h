#pragma once

#include <stdint.h>
#include <stddef.h>

// SHA256 implementation - encapsulated in namespace to avoid conflicts with libraries
namespace SeedCrypto {

void sha256(const uint8_t* data, size_t len, uint8_t hash[32]);
void sha256_double(const uint8_t* data, size_t len, uint8_t hash[32]);

} // namespace SeedCrypto
