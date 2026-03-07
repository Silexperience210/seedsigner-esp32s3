/**
 * @file memory.h
 * Secure memory management utilities - Production Grade
 * 
 * Features:
 * - Secure memory pool in internal RAM
 * - Guard patterns for overflow detection
 * - Multi-pass secure wiping
 * - RAII wrappers for automatic cleanup
 */

#ifndef UTILS_MEMORY_H
#define UTILS_MEMORY_H

#include <Arduino.h>
#include <stdint.h>

namespace SeedSigner {
namespace Utils {

// Configuration
#define SECURE_POOL_SIZE (64 * 1024)  // 64KB secure pool
#define MAX_ALLOCATIONS 64
#define MAX_ALLOC_SIZE (16 * 1024)    // Max single allocation

// Allocation header for tracking
struct AllocHeader {
    size_t size;
    uint32_t guard;
    int slot;
};

// Allocation tracking
struct Allocation {
    bool used;
    void* ptr;
    size_t size;
    AllocHeader* header;
};

/**
 * Secure Memory Allocator
 * 
 * Uses a dedicated memory pool with:
 * - Guard patterns for overflow detection
 * - Automatic wiping on free
 * - No memory leaks (fixed pool)
 */
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
    static uint32_t get_alloc_count() { return s_alloc_count; }
    
    // Emergency wipe of all secure memory
    static void emergency_wipe();
    
    // Self-test
    static bool self_test();
    
private:
    static bool s_initialized;
    static uint8_t* s_pool;
    static size_t s_pool_size;
    static size_t s_used;
    static Allocation s_allocations[MAX_ALLOCATIONS];
    static uint32_t s_alloc_count;
};

/**
 * RAII wrapper for secure buffers (heap allocated)
 * Automatically wipes and frees on destruction
 */
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
    
    bool is_valid() const { return m_data != nullptr; }
    
    void wipe() {
        if (m_data) {
            SecureMemory::wipe(m_data, m_size);
        }
    }
    
private:
    uint8_t* m_data;
    size_t m_size;
};

/**
 * Stack-based secure buffer (auto-wiped on scope exit)
 * No heap allocation - suitable for interrupt handlers
 */
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

/**
 * Secure String - auto-wiping string buffer
 */
class SecureString {
public:
    SecureString(size_t capacity = 256) : m_capacity(capacity), m_length(0) {
        m_data = (char*)SecureMemory::alloc(capacity);
        if (m_data) m_data[0] = '\0';
    }
    
    ~SecureString() {
        if (m_data) {
            SecureMemory::wipe(m_data, m_capacity);
            SecureMemory::free(m_data);
        }
    }
    
    // Disable copy
    SecureString(const SecureString&) = delete;
    SecureString& operator=(const SecureString&) = delete;
    
    bool set(const char* str) {
        if (!m_data || !str) return false;
        size_t len = strlen(str);
        if (len >= m_capacity) return false;
        
        // Wipe old content first
        SecureMemory::wipe(m_data, m_length);
        
        memcpy(m_data, str, len + 1);
        m_length = len;
        return true;
    }
    
    void clear() {
        if (m_data) {
            SecureMemory::wipe(m_data, m_length);
            m_data[0] = '\0';
            m_length = 0;
        }
    }
    
    const char* c_str() const { return m_data ? m_data : ""; }
    size_t length() const { return m_length; }
    size_t capacity() const { return m_capacity; }
    bool is_valid() const { return m_data != nullptr; }
    
private:
    char* m_data;
    size_t m_capacity;
    size_t m_length;
};

} // namespace Utils
} // namespace SeedSigner

#endif // UTILS_MEMORY_H
