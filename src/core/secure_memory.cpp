#include "core/secure_memory.h"
#include <string.h>
#include <esp_system.h>

// Stack canary value
static const uint32_t STACK_CANARY_VALUE = 0xDEADBEEF;
static uint32_t* stack_canary_ptr = nullptr;

namespace SecureMemory {

void zero32(volatile uint8_t* ptr) {
    zero(ptr, 32);
}

void zero64(volatile uint8_t* ptr) {
    zero(ptr, 64);
}

void lock_region(void* ptr, size_t len) {
    // On ESP32, we can't really lock memory from swapping
    // (no virtual memory), but we can:
    // 1. Disable flash cache during critical operations
    // 2. Use RTC FAST memory which is always RAM
    
    // This is a placeholder for platforms that support it
    (void)ptr;
    (void)len;
}

void unlock_region(void* ptr, size_t len) {
    (void)ptr;
    (void)len;
}

void init_stack_canary() {
    // Place canary at stack bottom (approximate)
    // In real implementation, use _get_stack_bottom()
    uint32_t canary;
    stack_canary_ptr = &canary - (1024 / 4); // ~1KB from current
    *stack_canary_ptr = STACK_CANARY_VALUE;
}

bool check_stack_canary() {
    if (stack_canary_ptr == nullptr) {
        return false;
    }
    return *stack_canary_ptr == STACK_CANARY_VALUE;
}

} // namespace SecureMemory

// C wrappers for assembly/linker compatibility
extern "C" {

void secure_zero_memory(void* ptr, size_t len) {
    SecureMemory::zero(static_cast<volatile uint8_t*>(ptr), len);
}

} // extern "C"
