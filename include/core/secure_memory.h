#pragma once

#include <stddef.h>
#include <stdint.h>

// Secure memory wiping and protection
// Prevents compiler optimization and provides multiple pass wiping

namespace SecureMemory {

// Secure zero - multi-pass with memory barrier
template<typename T>
inline void zero(volatile T* ptr, size_t len) {
    volatile unsigned char* p = reinterpret_cast<volatile unsigned char*>(ptr);
    for (size_t i = 0; i < len; i++) {
        p[i] = 0x00;
        p[i] = 0xFF;  // Second pass
        p[i] = 0xAA;  // Third pass
        p[i] = 0x55;  // Fourth pass
        p[i] = 0x00;  // Final pass
    }
    // Memory barrier to prevent optimization
    __asm__ volatile "" ::: "memory");
}

// Specialized versions for common sizes
void zero32(volatile uint8_t* ptr);   // 32 bytes (seed)
void zero64(volatile uint8_t* ptr);   // 64 bytes (BIP32 seed)

// Lock memory (prevent swapping - noop on ESP32 but good practice)
void lock_region(void* ptr, size_t len);
void unlock_region(void* ptr, size_t len);

// Stack canary for overflow detection
void init_stack_canary();
bool check_stack_canary();

// Secure buffer that auto-wipes on destruction
template<size_t SIZE>
class SecureBuffer {
public:
    SecureBuffer() { data_[0] = 0; }
    ~SecureBuffer() { zero(data_, SIZE); }
    
    uint8_t* data() { return data_; }
    const uint8_t* data() const { return data_; }
    size_t size() const { return SIZE; }
    
    void clear() { zero(data_, SIZE); }
    
private:
    volatile uint8_t data_[SIZE];
    // Prevent copying
    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer& operator=(const SecureBuffer&) = delete;
};

using SecureBuffer32 = SecureBuffer<32>;
using SecureBuffer64 = SecureBuffer<64>;

} // namespace SecureMemory
