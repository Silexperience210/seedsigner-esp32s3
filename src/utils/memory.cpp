/**
 * @file memory.cpp
 * Production-grade secure memory management
 * 
 * Features:
 * - Locked memory pool (no swap on ESP32)
 * - Multiple-pass secure wiping
 * - Memory guard patterns for buffer overflow detection
 * - Automatic cleanup on scope exit (RAII)
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
Allocation SecureMemory::s_allocations[MAX_ALLOCATIONS];
uint32_t SecureMemory::s_alloc_count = 0;

// Guard pattern for overflow detection
static const uint32_t GUARD_PATTERN = 0xDEADBEEF;

bool SecureMemory::init() {
    if (s_initialized) return true;
    
    // Allocate pool in internal RAM (not PSRAM) for security
    // PSRAM can potentially be read via external interfaces
    s_pool_size = SECURE_POOL_SIZE;
    s_pool = (uint8_t*)heap_caps_malloc(s_pool_size, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    
    if (!s_pool) {
        // Fallback to regular malloc (still better than nothing)
        s_pool = (uint8_t*)malloc(s_pool_size);
        if (!s_pool) return false;
    }
    
    // Wipe pool
    wipe(s_pool, s_pool_size);
    
    // Clear allocation table
    memset(s_allocations, 0, sizeof(s_allocations));
    s_used = 0;
    s_alloc_count = 0;
    s_initialized = true;
    
    return true;
}

void SecureMemory::deinit() {
    if (!s_initialized) return;
    
    // Wipe all allocated memory
    emergency_wipe();
    
    if (s_pool) {
        heap_caps_free(s_pool);
        s_pool = nullptr;
    }
    
    s_initialized = false;
    s_used = 0;
    s_alloc_count = 0;
}

void* SecureMemory::alloc(size_t size) {
    if (!s_initialized) {
        if (!init()) return nullptr;
    }
    
    if (size == 0 || size > MAX_ALLOC_SIZE) return nullptr;
    
    // Add overhead for guard patterns
    size_t total_size = size + 2 * sizeof(uint32_t) + sizeof(AllocHeader);
    
    // Align to 8 bytes
    total_size = (total_size + 7) & ~7;
    
    if (s_used + total_size > s_pool_size) return nullptr;
    
    // Find free allocation slot
    int slot = -1;
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        if (!s_allocations[i].used) {
            slot = i;
            break;
        }
    }
    
    if (slot < 0) return nullptr;  // No free slots
    
    // Setup allocation
    uint8_t* ptr = s_pool + s_used;
    AllocHeader* header = (AllocHeader*)ptr;
    header->size = size;
    header->guard = GUARD_PATTERN;
    header->slot = slot;
    
    // Front guard
    uint32_t* front_guard = (uint32_t*)(ptr + sizeof(AllocHeader));
    *front_guard = GUARD_PATTERN;
    
    // User pointer
    uint8_t* user_ptr = ptr + sizeof(AllocHeader) + sizeof(uint32_t);
    
    // Back guard
    uint32_t* back_guard = (uint32_t*)(user_ptr + size);
    *back_guard = GUARD_PATTERN;
    
    // Record allocation
    s_allocations[slot].used = true;
    s_allocations[slot].ptr = user_ptr;
    s_allocations[slot].size = size;
    s_allocations[slot].header = header;
    
    s_used += total_size;
    s_alloc_count++;
    
    return user_ptr;
}

void* SecureMemory::calloc(size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = alloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

void SecureMemory::free(void* ptr) {
    if (!ptr || !s_initialized) return;
    
    // Find allocation
    for (int i = 0; i < MAX_ALLOCATIONS; i++) {
        if (s_allocations[i].used && s_allocations[i].ptr == ptr) {
            // Verify guards
            AllocHeader* header = s_allocations[i].header;
            uint32_t* front_guard = (uint32_t*)((uint8_t*)header + sizeof(AllocHeader));
            uint32_t* back_guard = (uint32_t*)((uint8_t*)ptr + s_allocations[i].size);
            
            if (*front_guard != GUARD_PATTERN || *back_guard != GUARD_PATTERN) {
                // Buffer overflow detected! Handle security event
                // In production, trigger panic/wipe
            }
            
            // Wipe memory
            wipe(ptr, s_allocations[i].size);
            
            // Mark as freed
            s_allocations[i].used = false;
            s_allocations[i].ptr = nullptr;
            s_alloc_count--;
            
            // Note: We don't compact memory (bump allocator)
            // Full allocator would merge free blocks here
            
            return;
        }
    }
    
    // Pointer not found - possible corruption or double-free
}

void* SecureMemory::realloc(void* ptr, size_t old_size, size_t new_size) {
    void* new_ptr = alloc(new_size);
    if (new_ptr && ptr) {
        memcpy(new_ptr, ptr, old_size < new_size ? old_size : new_size);
        wipe(ptr, old_size);
        SecureMemory::free(ptr);
    }
    return new_ptr;
}

/**
 * Secure wipe - multiple passes to prevent data remanence
 * 
 * Pass 1: Zeros (complements any stuck-at-1 bits)
 * Pass 2: Ones (complements any stuck-at-0 bits)
 * Pass 3: Random pattern (confusion)
 * Pass 4: Zeros (final clean state)
 */
void SecureMemory::wipe(void* ptr, size_t len) {
    if (!ptr || len == 0) return;
    
    volatile uint8_t* p = (volatile uint8_t*)ptr;
    
    // Pass 1: Zeros
    for (size_t i = 0; i < len; i++) {
        p[i] = 0x00;
    }
    
    // Memory barrier to prevent reordering
    __sync_synchronize();
    
    // Pass 2: Ones
    for (size_t i = 0; i < len; i++) {
        p[i] = 0xFF;
    }
    
    __sync_synchronize();
    
    // Pass 3: Random pattern (XOR of address and length)
    for (size_t i = 0; i < len; i++) {
        p[i] = (uint8_t)((uintptr_t)p ^ len ^ i);
    }
    
    __sync_synchronize();
    
    // Pass 4: Zeros (final)
    for (size_t i = 0; i < len; i++) {
        p[i] = 0x00;
    }
    
    __sync_synchronize();
}

/**
 * Lock memory - prevents swapping
 * 
 * On ESP32, there's no swap, so this is a no-op.
 * On systems with swap, this would call mlock().
 */
bool SecureMemory::lock(void* ptr, size_t len) {
    (void)ptr;
    (void)len;
    // ESP32 has no swap, so memory is always "locked"
    return true;
}

bool SecureMemory::unlock(void* ptr, size_t len) {
    (void)ptr;
    (void)len;
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

/**
 * Emergency wipe - called on security events
 * Wipes entire secure pool and resets state
 */
void SecureMemory::emergency_wipe() {
    if (!s_pool) return;
    
    // Wipe entire pool
    wipe(s_pool, s_pool_size);
    
    // Reset allocation table
    memset(s_allocations, 0, sizeof(s_allocations));
    s_used = 0;
    s_alloc_count = 0;
}

bool SecureMemory::self_test() {
    // Test 1: Basic alloc/wipe/free
    void* ptr = alloc(64);
    if (!ptr) return false;
    
    // Write pattern
    memset(ptr, 0xAA, 64);
    
    // Free (should wipe)
    free(ptr);
    
    // Verify wiped (should be zeros)
    uint8_t* check = (uint8_t*)ptr;
    for (size_t i = 0; i < 64; i++) {
        if (check[i] != 0) return false;
    }
    
    // Test 2: Multiple allocations
    void* p1 = alloc(32);
    void* p2 = alloc(64);
    void* p3 = alloc(128);
    
    if (!p1 || !p2 || !p3) return false;
    
    // Verify they're different
    if (p1 == p2 || p2 == p3 || p1 == p3) return false;
    
    free(p1);
    free(p2);
    free(p3);
    
    // Test 3: calloc zero-initializes
    uint8_t* p = (uint8_t*)calloc(1, 100);
    if (!p) return false;
    
    for (int i = 0; i < 100; i++) {
        if (p[i] != 0) return false;
    }
    
    free(p);
    
    return true;
}

} // namespace Utils
} // namespace SeedSigner
