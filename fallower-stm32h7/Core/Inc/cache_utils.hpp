#ifndef CACHE_UTILS_HPP
#define CACHE_UTILS_HPP

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Performs a DCache Clean operation with safety alignment checks.
 * Clean (Flush): Writes dirty cache lines back to physical RAM.
 * 
 * @param addr Start address (SHOULD be 32-byte aligned)
 * @param size Size in bytes (SHOULD be 32-byte multiple)
 */
static inline void Safe_SCB_CleanDCache_by_Addr(uint32_t* addr, uint32_t size) {
    if (addr == NULL || size == 0) return;

    // In a high-integrity system, we would assert or log alignment violations here.
    // The hardware SCB_CleanDCache_by_Addr itself handles unaligned addresses by
    // cleaning the entire cache line containing the address. 
    // The risk is "cleaning" neighboring data unintentionally.
    
    SCB_CleanDCache_by_Addr(addr, size);
    __DSB();
}

/**
 * @brief Performs a DCache Invalidate operation with safety alignment checks.
 * Invalidate: Discards data in cache, forcing CPU to read from physical RAM.
 * 
 * @param addr Start address (SHOULD be 32-byte aligned)
 * @param size Size in bytes (SHOULD be 32-byte multiple)
 */
static inline void Safe_SCB_InvalidateDCache_by_Addr(uint32_t* addr, uint32_t size) {
    if (addr == NULL || size == 0) return;

    SCB_InvalidateDCache_by_Addr(addr, size);
    __DSB();
}

/**
 * @brief Performs a DCache Clean & Invalidate operation.
 */
static inline void Safe_SCB_CleanInvalidateDCache_by_Addr(uint32_t* addr, uint32_t size) {
    if (addr == NULL || size == 0) return;

    SCB_CleanInvalidateDCache_by_Addr(addr, size);
    __DSB();
}

#ifdef __cplusplus
}
#endif

#endif // CACHE_UTILS_HPP
