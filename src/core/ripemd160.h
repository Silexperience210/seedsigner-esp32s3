/**
 * @file ripemd160.h
 * RIPEMD160 hash function - simplified for embedded
 */

#ifndef CORE_RIPEMD160_H
#define CORE_RIPEMD160_H

#include <stdint.h>
#include <stddef.h>

namespace SeedSigner {
namespace Core {

// RIPEMD160 output is 20 bytes
#define RIPEMD160_DIGEST_SIZE 20

// Simple RIPEMD160 implementation
class RIPEMD160 {
public:
    static void hash(const uint8_t* data, size_t len, uint8_t out[20]);
    
private:
    static void process_block(uint32_t* h, const uint8_t* block);
};

} // namespace Core
} // namespace SeedSigner

#endif // CORE_RIPEMD160_H