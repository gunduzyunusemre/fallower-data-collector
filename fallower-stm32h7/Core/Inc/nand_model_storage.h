#ifndef NAND_MODEL_STORAGE_H
#define NAND_MODEL_STORAGE_H

#include "stm32h7xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Enum for NAND Task operations
typedef enum {
    NAND_OP_WRITE_MODEL,
    NAND_OP_READ_MODEL
} NAND_Operation_t;

// Enum for FMC Hardware Mode
typedef enum {
    FMC_MODE_UNKNOWN = 0,
    FMC_MODE_SDRAM,
    FMC_MODE_NAND
} fmc_mode_t;

// CONSTANTS
#define NAND_METADATA_MAGIC 0x4D4F444C // "MODL"
#define NAND_METADATA_BLOCK 1
#define NAND_DATA_START_BLOCK 2
#define MAX_MODEL_BLOCKS 256

// Metadata structure with block map
typedef struct {
    uint32_t magic_number;
    uint32_t model_size;
    uint32_t write_timestamp;
    uint16_t block_count; // Number of blocks ACTUALLY used
    char model_name[32];
    uint16_t block_map[MAX_MODEL_BLOCKS]; // Map of logical to physical blocks
} nand_model_metadata_t;

// Function Prototypes
HAL_StatusTypeDef WriteModelToNAND(uint8_t* model_data, uint32_t file_size, const char* model_name);
HAL_StatusTypeDef ReadModelFromNAND_ToSDRAM(void);
HAL_StatusTypeDef ReadModelMetadata(nand_model_metadata_t* metadata);
HAL_StatusTypeDef FMC_ReInitNAND(void);
HAL_StatusTypeDef FMC_RestoreSDRAM(void);  // ADDED: Restore SDRAM after NAND operations

#ifdef __cplusplus
}
#endif

#endif // NAND_MODEL_STORAGE_H