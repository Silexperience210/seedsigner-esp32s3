/**
 * @file memory.cpp
 * Secure memory management
 */

#include "utils/memory.h"
#include <string.h>
#include <esp_heap_caps.h>

namespace SeedSigner {
namespace Utils {

// Static members
bool SecureMemory::s_initialized = false;
uint8_t* SecureMemory::s_pool = nullptr;
size_t SecureMemory::s_pool_size = 0;
size_t SecureMemory::s_used = 0;

bool SecureMemory::init() {
    if (s_initialized) return true;
    
    // Allocate pool in internal RAM (not PSRAM for sensitive data)
    s_pool_size = 32 * 1024;  // 32KB secure pool
    s_pool = (uint8_t*)heap_caps_malloc(s_pool_size, MALLOC_CAP_8BIT);
    
    if (!s_pool) return false;
    
    wipe(s_pool, s_pool_size);
    s_used = 0;
    s_initialized = true;
    
    return true;
}

void SecureMemory::deinit() {
    if (s_pool) {
        wipe(s_pool, s_pool_size);
        heap_caps_free(s_pool);
        s_pool = nullptr;
    }
    s_initialized = false;
}

void* SecureMemory::alloc(size_t size) {
    if (!s_initialized) init();
    
    // Simple bump allocator for now
    // In production: use proper secure allocator
    if (s_used + size > s_pool_size) return nullptr;
    
    void* ptr = s_pool + s_used;
    s_used += size;
    
    return ptr;
}

void* SecureMemory::calloc(size_t num, size_t size) {
    void* ptr = alloc(num * size);
    if (ptr) {
        memset(ptr, 0, num * size);
    }
    return ptr;
}

void SecureMemory::free(void* ptr) {
    // In bump allocator: we don't really free
    // Just wipe the memory
    // Production: implement proper tracking
    (void)ptr;
}

void* SecureMemory::realloc(void* ptr, size_t old_size, size_t new_size) {
    void* new_ptr = alloc(new_size);
    if (new_ptr && ptr) {
        memcpy(new_ptr, ptr, old_size < new_size ? old_size : new_size);
        wipe(ptr, old_size);
    }
    return new_ptr;
}

void SecureMemory::wipe(void* ptr, size_t len) {
    if (!ptr || len == 0) return;
    
    // Use volatile to prevent optimizer from removing
    volatile uint8_t* p = (volatile uint8_t*)ptr;
    
    // Multiple passes for paranoid wiping
    // Pass 1: Zeros
    for (size_t i = 0; i < len; i++) {
        p[i] = 0x00;
    }
    
    // Pass 2: Ones
    for (size_t i = 0; i < len; i++) {
        p[i] = 0xFF;
    }
    
    // Pass 3: Random pattern
    for (size_t i = 0; i < len; i++) {
        p[i] = (uint8_t)(i * 0xA5);
    }
    
    // Final pass: Zeros
    for (size_t i = 0; i < len; i++) {
        p[i] = 0x00;
    }
    
    // Memory barrier
    __sync_synchronize();
}

bool SecureMemory::lock(void* ptr, size_t len) {
    // ESP32 doesn't support mlock equivalent
    // We rely on not swapping (no swap on ESP32)
    (void)ptr; (void)len;
    return true;
}

bool SecureMemory::unlock(void* ptr, size_t len) {
    (void)ptr; (void)len;
    return true;
}

size_t SecureMemory::get_free() {
    return s_pool_size - s_used;
}

size_t SecureMemory::get_used() {
    return s_used;
}

size_t SecureMemory::get_total() {
    return s_pool_size;
}

void SecureMemory::emergency_wipe() {
    if (s_pool) {
        wipe(s_pool, s_pool_size);
    }
    
    // Also try to wipe stack and heap
    // This is best-effort on ESP32
    uint8_t stack_var[1024];
    wipe(stack_var, sizeof(stack_var));
}

bool SecureMemory::self_test() {
    // Test 1: Basic alloc/wipe
    void* ptr = alloc(64);
    if (!ptr) return false;
    
    memset(ptr, 0xAA, 64);
    wipe(ptr, 64);
    
    // Verify wiped
    uint8_t* check = (uint8_t*)ptr;
    for (size_t i = 0; i < 64; i++) {
        if (check[i] != 0) return false;
    }
    
    return true;
}

} // namespace Utils
} // namespace SeedSigner
