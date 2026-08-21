#ifndef NAND_BLOCK_MANAGER_ENHANCED_H
#define NAND_BLOCK_MANAGER_ENHANCED_H

#include "stm32h7xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// S34ML02G1 Specifications
#define NAND_TOTAL_BLOCKS           2048
#define NAND_PAGES_PER_BLOCK        64
#define NAND_PAGE_SIZE              2048
#define NAND_SPARE_SIZE             64
#define NAND_BLOCK_SIZE             (NAND_PAGES_PER_BLOCK * NAND_PAGE_SIZE)  // 128KB
#define NAND_TOTAL_PLANES           2
#define NAND_BLOCKS_PER_PLANE       1024
#define NAND_MAX_WEAR_COUNT         100000
#define NAND_INVALID_BLOCK          0xFFFF

// ECC Configuration
#define NAND_ECC_BYTES_PER_512      8    // For 512-byte ECC sectors
#define NAND_ECC_SECTORS_PER_PAGE   4    // 2048/512 = 4 sectors
#define NAND_ECC_TOTAL_BYTES        (NAND_ECC_BYTES_PER_512 * NAND_ECC_SECTORS_PER_PAGE)

// Status Register Bits (S34ML02G1)
#define NAND_SR_FAIL_BIT            (1 << 0)  // I/O0: Program/Erase failure
#define NAND_SR_FAILC_BIT           (1 << 1)  // I/O1: Previous operation failure
#define NAND_SR_READY_BIT           (1 << 6)  // I/O6: Ready/Busy status
#define NAND_SR_WP_BIT              (1 << 7)  // I/O7: Write protect status

// Timeouts (microseconds and milliseconds)
#define NAND_TIMEOUT_READ_US        25      // 25μs max random access
#define NAND_TIMEOUT_PROGRAM_MS     1       // 1ms program timeout (converted from 200μs)
#define NAND_TIMEOUT_ERASE_MS       10000   // 10 seconds erase timeout for problematic blocks (increased)
#define NAND_TIMEOUT_RESET_MS       1       // 1ms reset timeout (converted from 5μs)
#define NAND_TIMEOUT_PROBLEMATIC_ERASE_MS  20000  // 20 seconds for problematic blocks like Block 6

// Block Status Types
typedef enum {
    BLOCK_STATUS_GOOD           = 0x00,
    BLOCK_STATUS_BAD            = 0xFF,
    BLOCK_STATUS_RESERVED       = 0xFE,
    BLOCK_STATUS_WORN           = 0xFD,
    BLOCK_STATUS_PROGRAM_FAIL   = 0xFC,
    BLOCK_STATUS_ERASE_FAIL     = 0xFB,
    BLOCK_STATUS_BLACKLISTED    = 0xFA  // Hardware problematic blocks
} block_status_t;

// ECC Result Types
typedef enum {
    ECC_STATUS_SUCCESS          = 0x00,
    ECC_STATUS_CORRECTED_1BIT   = 0x01,
    ECC_STATUS_CORRECTED_MULTI  = 0x02,
    ECC_STATUS_UNCORRECTABLE    = 0xFF
} ecc_status_t;

// Block Information Structure
typedef struct {
    uint16_t block_number;
    block_status_t status;
    uint32_t wear_count;
    uint32_t last_used_timestamp;
    uint16_t plane_id;              // 0 or 1 for S34ML02G1
    uint8_t program_fail_count;
    uint8_t erase_fail_count;
} block_info_t;

// ECC Statistics
typedef struct {
    uint32_t total_reads;
    uint32_t corrected_1bit_errors;
    uint32_t corrected_multibit_errors;
    uint32_t uncorrectable_errors;
    uint32_t ecc_failures;
} ecc_statistics_t;

// NAND Manager Statistics
typedef struct {
    uint32_t total_page_writes;
    uint32_t total_page_reads;
    uint32_t total_block_erases;
    uint32_t bad_blocks_detected;
    uint32_t wear_level_operations;
    uint32_t multiplane_operations;
    ecc_statistics_t ecc_stats;
} nand_statistics_t;

// Main NAND Block Manager Structure
typedef struct {
    // Block Management
    block_info_t blocks[NAND_TOTAL_BLOCKS];
    uint16_t bad_block_count;
    uint16_t available_blocks;
    uint16_t next_block_to_use[NAND_TOTAL_PLANES];  // Per-plane allocation

    // Statistics
    nand_statistics_t stats;

    // Hardware Handle
    NAND_HandleTypeDef *hnand;

    // Configuration Flags
    bool multiplane_enabled;
    bool ecc_enabled;
    bool wear_leveling_enabled;

    // Runtime State
    bool initialized;
    uint32_t last_operation_time;
    uint8_t current_plane;
} nand_block_manager_t;

// Core Management Functions
HAL_StatusTypeDef NAND_BlockManager_Init(nand_block_manager_t *manager, NAND_HandleTypeDef *hnand);
HAL_StatusTypeDef NAND_BlockManager_DeInit(nand_block_manager_t *manager);
HAL_StatusTypeDef NAND_BlockManager_Reset(nand_block_manager_t *manager);

// Block Operations
HAL_StatusTypeDef NAND_BlockManager_ScanBadBlocks(nand_block_manager_t *manager);
uint16_t NAND_BlockManager_AllocateBlock(nand_block_manager_t *manager, uint8_t plane_id);
HAL_StatusTypeDef NAND_BlockManager_MarkBadBlock(nand_block_manager_t *manager, uint16_t block_index);
HAL_StatusTypeDef NAND_BlockManager_EraseBlock(nand_block_manager_t *manager, uint16_t block_index);

// Enhanced Page Operations with ECC
HAL_StatusTypeDef NAND_BlockManager_WritePage(nand_block_manager_t *manager,
                                             uint16_t block_index,
                                             uint16_t page_index,
                                             uint8_t *data,
                                             ecc_status_t *ecc_result);

HAL_StatusTypeDef NAND_BlockManager_ReadPage(nand_block_manager_t *manager,
                                            uint16_t block_index,
                                            uint16_t page_index,
                                            uint8_t *data,
                                            ecc_status_t *ecc_result);

// Multiplane Operations (S34ML02G1 specific)
HAL_StatusTypeDef NAND_BlockManager_WritePageMultiplane(nand_block_manager_t *manager,
                                                       uint16_t block0, uint16_t page0,
                                                       uint16_t block1, uint16_t page1,
                                                       uint8_t *data0, uint8_t *data1);

HAL_StatusTypeDef NAND_BlockManager_EraseBlockMultiplane(nand_block_manager_t *manager,
                                                        uint16_t block0, uint16_t block1);

// Hardware Interface Functions
HAL_StatusTypeDef NAND_HW_WaitReady(nand_block_manager_t *manager, uint32_t timeout_ms);
HAL_StatusTypeDef NAND_HW_ReadStatusRegister(nand_block_manager_t *manager, uint8_t *status);
HAL_StatusTypeDef NAND_HW_CheckOperationStatus(nand_block_manager_t *manager);

// ECC Functions
HAL_StatusTypeDef NAND_ECC_CheckResult(nand_block_manager_t *manager, ecc_status_t *result);
HAL_StatusTypeDef NAND_ECC_CorrectErrors(uint8_t *data, uint8_t *ecc_data, ecc_status_t *result);

// Utility Functions
HAL_StatusTypeDef NAND_BlockManager_GetStatistics(nand_block_manager_t *manager, nand_statistics_t *stats);
uint16_t NAND_BlockManager_GetBadBlockCount(nand_block_manager_t *manager);
uint16_t NAND_BlockManager_GetAvailableBlocks(nand_block_manager_t *manager);
uint8_t NAND_BlockManager_GetPlaneFromBlock(uint16_t block_index);
bool NAND_BlockManager_IsBlockInSamePlane(uint16_t block1, uint16_t block2);

// Wear Leveling
HAL_StatusTypeDef NAND_BlockManager_WearLevel(nand_block_manager_t *manager);
uint16_t NAND_BlockManager_FindLeastWornBlock(nand_block_manager_t *manager, uint8_t plane_id);

// Debug and Maintenance
void NAND_BlockManager_PrintStatus(nand_block_manager_t *manager);
HAL_StatusTypeDef NAND_BlockManager_RunDiagnostics(nand_block_manager_t *manager);

uint16_t NAND_BlockManager_FindNextGoodBlock(nand_block_manager_t *manager, uint16_t *start_index);

// Blacklist Management
HAL_StatusTypeDef NAND_BlockManager_BlacklistBlock(nand_block_manager_t *manager, uint16_t block_index);
bool NAND_BlockManager_IsBlockBlacklisted(nand_block_manager_t *manager, uint16_t block_index);

#endif // NAND_BLOCK_MANAGER_ENHANCED_H

#ifdef __cplusplus
}
#endif
