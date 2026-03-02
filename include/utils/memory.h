/**
 * @file memory.h
 * Secure memory management utilities
 */

#ifndef UTILS_MEMORY_H
#define UTILS_MEMORY_H

#include <Arduino.h>
#include <stdint.h>

namespace SeedSigner {
namespace Utils {

// Secure memory allocator
class SecureMemory {
public:
    static bool init();
    static void deinit();
    
    // Allocate secure memory (will be wiped on free)
    static void* alloc(size_t size);
    static void* calloc(size_t num, size_t size);
    static void free(void* ptr);
    
    // Reallocate with wipe of old buffer
    static void* realloc(void* ptr, size_t old_size, size_t new_size);
    
    // Wipe memory (secure memset)
    static void wipe(void* ptr, size_t len);
    
    // Lock memory (prevent swapping - if supported)
    static bool lock(void* ptr, size_t len);
    static bool unlock(void* ptr, size_t len);
    
    // Get secure memory pool stats
    static size_t get_free();
    static size_t get_used();
    static size_t get_total();
    
    // Emergency wipe of all secure memory
    static void emergency_wipe();
    
    // Self-test
    static bool self_test();
    
private:
    static bool s_initialized;
    static uint8_t* s_pool;
    static size_t s_pool_size;
    static size_t s_used;
};

// RAII wrapper for secure buffers
template <size_t N>
class SecureBuffer {
public:
    SecureBuffer() {
        m_data = (uint8_t*)SecureMemory::alloc(N);
        m_size = N;
    }
    
    ~SecureBuffer() {
        if (m_data) {
            SecureMemory::wipe(m_data, m_size);
            SecureMemory::free(m_data);
        }
    }
    
    // Disable copy
    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer& operator=(const SecureBuffer&) = delete;
    
    // Enable move
    SecureBuffer(SecureBuffer&& other) noexcept 
        : m_data(other.m_data), m_size(other.m_size) {
        other.m_data = nullptr;
        other.m_size = 0;
    }
    
    uint8_t* data() { return m_data; }
    const uint8_t* data() const { return m_data; }
    size_t size() const { return m_size; }
    
    uint8_t& operator[](size_t i) { return m_data[i]; }
    const uint8_t& operator[](size_t i) const { return m_data[i]; }
    
    void wipe() {
        if (m_data) {
            SecureMemory::wipe(m_data, m_size);
        }
    }
    
private:
    uint8_t* m_data;
    size_t m_size;
};

// Stack-based secure buffer (auto-wiped on scope exit)
template <size_t N>
class StackBuffer {
public:
    StackBuffer() { m_size = N; }
    ~StackBuffer() { wipe(); }
    
    // Disable copy/move
    StackBuffer(const StackBuffer&) = delete;
    StackBuffer& operator=(const StackBuffer&) = delete;
    StackBuffer(StackBuffer&&) = delete;
    
    uint8_t* data() { return m_data; }
    const uint8_t* data() const { return m_data; }
    size_t size() const { return m_size; }
    
    uint8_t& operator[](size_t i) { return m_data[i]; }
    const uint8_t& operator[](size_t i) const { return m_data[i]; }
    
    void wipe() {
        SecureMemory::wipe(m_data, m_size);
    }
    
private:
    uint8_t m_data[N];
    size_t m_size;
};

} // namespace Utils
} // namespace SeedSigner

#endif // UTILS_MEMORY_H
