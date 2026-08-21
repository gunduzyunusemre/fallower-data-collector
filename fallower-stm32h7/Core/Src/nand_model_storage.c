#include "nand_model_storage.h"
#include "nand_block_manager_enhanced.h"
#include "nand_logger.h" // C-compatible logger
#include <string.h>
#include <stdio.h>  // For snprintf
#include "FreeRTOS.h"
#include "task.h"  // For vTaskDelay
#include "cmsis_os.h" // For osDelay
#include "stm32h7xx_hal_conf.h"  // FIX F11: Check if IWDG is enabled

// FIX F11: Conditional IWDG support - only if HAL_IWDG_MODULE_ENABLED is defined
#ifdef HAL_IWDG_MODULE_ENABLED
#include "stm32h7xx_hal_iwdg.h"
extern IWDG_HandleTypeDef hiwdg1;
#define NAND_REFRESH_WATCHDOG() HAL_IWDG_Refresh(&hiwdg1)
#else
#define NAND_REFRESH_WATCHDOG() ((void)0)  // No-op if IWDG not enabled
#endif

// --- External handles from main.cpp ---
extern SDRAM_HandleTypeDef hsdram1; // SDRAM handle for DeInit
extern NAND_HandleTypeDef hnand1;
extern nand_block_manager_t g_nand_manager;

// --- External MSP functions from stm32h7xx_hal_msp.c ---
// These functions manage GPIO pin configuration for FMC peripherals
extern void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef *hsdram);
extern void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram);  // ADDED: For SDRAM restore
extern void HAL_NAND_MspInit(NAND_HandleTypeDef* hnand);

// --- State Tracker ---
static fmc_mode_t g_fmc_mode = FMC_MODE_SDRAM; // System starts in SDRAM mode

// Function to re-initialize FMC for NAND operations
// CRITICAL: This function switches FMC hardware from SDRAM mode to NAND mode
// by reconfiguring GPIO pins and FMC controller registers
/**
 * @brief FMC donanımını NAND Flash moduna göre yeniden yapılandırır.
 * 
 * SDRAM modundaki FMC'yi durdurur, GPIO pinlerini NAND moduna çeker ve 
 * S34ML02G2 NAND sensörü için zamanlama parametrelerini (timing) uygular. 
 * Bu işlem, SDRAM erişimini geçici olarak devre dışı bırakır.
 * 
 * @return HAL_StatusTypeDef İşlem başarılıysa HAL_OK döner.
 * 
 * @note Bu fonksiyon GPIO rekonfigürasyonu ve uzun gecikmeler (50ms stabilization) içerdiği için 
 *       kesinlikle KESME (ISR) içinden ÇAĞRILAMAZ. HardFault riski taşır.
 */
HAL_StatusTypeDef FMC_ReInitNAND(void) {
    // Skip if already in NAND mode
    if (g_fmc_mode == FMC_MODE_NAND) return HAL_OK;

    NAND_Log("NAND", "Re-initializing FMC for NAND operations...");

    // CRITICAL FIX: Flush ALL dirty D-Cache lines to physical SDRAM BEFORE switching GPIO.
    // Write-Back cache policy means CPU writes go to cache first (not SDRAM).
    // HAL_SDRAM_MspDeInit() below will reconfigure GPIO pins to NAND mode, making SDRAM
    // physically unreachable. Any subsequent dirty cache eviction → MemManage Fault.
    // SCB_CleanDCache() ensures every dirty line is written to SDRAM while GPIO is still valid.
    // DSB/ISB alone are insufficient — they order instructions but do NOT flush dirty lines.
    SCB_CleanDCache();
    __DSB();
    __ISB();

    // STEP 0 (CRITICAL): Ensure FMC peripheral clock is enabled
    // This MUST be done before any FMC register access
    __HAL_RCC_FMC_CLK_ENABLE();
    NAND_Log("NAND", "FMC clock enabled");

    // STEP 1: De-initialize SDRAM controller (stops SDRAM operations)
    if (HAL_SDRAM_DeInit(&hsdram1) != HAL_OK) {
        NAND_Log("NAND_ERROR", "HAL_SDRAM_DeInit failed");
        // Continue anyway - GPIO reconfiguration is more critical
    }

    // STEP 2 (CRITICAL): Release SDRAM GPIO pins
    // This function sets GPIO pins back to default mode, removing SDRAM configuration
    // Without this, GPIO pins remain in SDRAM alternate function mode
    HAL_SDRAM_MspDeInit(&hsdram1);
    NAND_Log("NAND", "SDRAM GPIO pins released");

    // STEP 3: De-initialize NAND controller (safety measure)
    if (HAL_NAND_DeInit(&hnand1) != HAL_OK) {
        NAND_Log("NAND_ERROR", "HAL_NAND_DeInit failed");
    }

    // STEP 3.5 (CRITICAL): Re-enable FMC clock after DeInit (DeInit may disable it)
    __HAL_RCC_FMC_CLK_ENABLE();
    NAND_Log("NAND", "FMC clock re-enabled after DeInit");

    // STEP 4 (CRITICAL): Configure GPIO pins for NAND mode
    // This function configures GPIO pins to NAND alternate function mode
    // This is essential because SDRAM and NAND share many pins but use different AF modes
    HAL_NAND_MspInit(&hnand1);
    NAND_Log("NAND", "NAND GPIO pins configured");

    // STEP 5: Now safely initialize NAND controller with fresh GPIO configuration
    // CRITICAL: These values MUST EXACTLY MATCH MX_FMC_Init() in main.cpp:615-651
    FMC_NAND_PCC_TimingTypeDef ComSpaceTiming = {0};
    FMC_NAND_PCC_TimingTypeDef AttSpaceTiming = {0};

    // Configure hnand1.Init (MUST match main.cpp:617-628)
    hnand1.Instance = FMC_NAND_DEVICE;
    hnand1.Init.NandBank = FMC_NAND_BANK3;
    hnand1.Init.Waitfeature = FMC_NAND_WAIT_FEATURE_DISABLE;
    hnand1.Init.MemoryDataWidth = FMC_NAND_MEM_BUS_WIDTH_8;
    hnand1.Init.EccComputation = FMC_NAND_ECC_ENABLE;
    hnand1.Init.ECCPageSize = FMC_NAND_ECC_PAGE_SIZE_512BYTE;
    hnand1.Init.TCLRSetupTime = 3;    // CLE-to-RE delay (matches main.cpp:627)
    hnand1.Init.TARSetupTime = 3;     // ALE-to-RE delay (matches main.cpp:628)

    // Configure hnand1.Config (MUST match main.cpp:629-636)
    hnand1.Config.PageSize = 2048;
    hnand1.Config.SpareAreaSize = 64;
    hnand1.Config.BlockSize = 64;
    hnand1.Config.BlockNbr = 2048;
    hnand1.Config.PlaneNbr = 2;
    hnand1.Config.PlaneSize = 1024;
    hnand1.Config.ExtraCommandEnable = ENABLE;

    // ComSpaceTiming - Conservative values for S34ML02G2 reliability (MUST match main.cpp:638-641)
    ComSpaceTiming.SetupTime = 20;        // Was 11, increased by 1.8x for safety
    ComSpaceTiming.WaitSetupTime = 30;    // Was 15, increased by 2x for erase operations
    ComSpaceTiming.HoldSetupTime = 15;    // Was 8, increased by 1.9x for data hold
    ComSpaceTiming.HiZSetupTime = 40;     // Was 23, increased by 1.7x for bus release

    // AttSpaceTiming - Match ComSpaceTiming for consistency (MUST match main.cpp:643-646)
    AttSpaceTiming.SetupTime = 20;        // Was 11, conservative for attribute space
    AttSpaceTiming.WaitSetupTime = 30;    // Was 15, ensures ready/busy signal stability
    AttSpaceTiming.HoldSetupTime = 15;    // Was 8, prevents attribute space glitches
    AttSpaceTiming.HiZSetupTime = 40;     // Was 23, safe bus tristate timing

    if (HAL_NAND_Init(&hnand1, &ComSpaceTiming, &AttSpaceTiming) != HAL_OK) {
        NAND_Log("NAND_ERROR", "FMC NAND re-initialization failed!");
        return HAL_ERROR;
    }
    NAND_Log("NAND", "HAL_NAND_Init completed successfully");

    // ✅ FIX: Use longer fixed delay for FMC stabilization
    // STEP 5.5 (CRITICAL): Wait for FMC hardware to stabilize after init
    // FMC controller needs time to apply new timing parameters to hardware registers
    // HAL_NAND_Read_Status may not work correctly until NAND is fully initialized
    NAND_Log("NAND", "Waiting for FMC hardware to stabilize...");

    // FIX F11: Refresh watchdog before long delay to prevent system reset
    NAND_REFRESH_WATCHDOG();
    osDelay(50);  // ✅ FIX: Increased from 10ms to 50ms for reliable initialization
    NAND_Log("NAND", "FMC hardware stabilized, attempting NAND reset...");

    // STEP 6: NAND reset should now work without HardFault
    // Previous HardFault was caused by GPIO pins still in SDRAM mode
    if (HAL_NAND_Reset(&hnand1) != HAL_OK) {
        NAND_Log("NAND_ERROR", "NAND reset after FMC re-init failed!");
        NAND_Log("NAND_ERROR", "This indicates FMC hardware not responding to commands");
        return HAL_ERROR;
    }
    NAND_Log("NAND", "NAND reset successful!");

    // STEP 7: RTOS-friendly delay (use osDelay for better task management)
    osDelay(5);

    NAND_Log("NAND", "FMC NAND re-initialization completed.");
    g_fmc_mode = FMC_MODE_NAND;

    return HAL_OK;
}

// Function to restore SDRAM configuration after NAND operations
// CRITICAL: This function switches FMC hardware back from NAND mode to SDRAM mode
/**
 * @brief FMC donanımını SDRAM moduna geri döndürür.
 * 
 * NAND modundaki FMC'yi temizler ve SDRAM için gerekli GPIO ile zamanlama 
 * ayarlarını (CAS=2, 100MHz vb.) tekrar yükler. İşlem sonunda SDRAM'in çalışıp 
 * çalışmadığını 0xDEADBEEF deseniyle doğrular.
 * 
 * @return HAL_StatusTypeDef Restorasyon ve doğrulama başarılıysa HAL_OK döner.
 * 
 * @note SDRAM'in güç açma (power-up) sekansını içerir. ISR içinden çağrılmamalıdır.
 */
HAL_StatusTypeDef FMC_RestoreSDRAM(void) {
    // Skip if already in SDRAM mode
    if (g_fmc_mode == FMC_MODE_SDRAM) return HAL_OK;

    NAND_Log("NAND", "Restoring FMC for SDRAM operations...");

    // FIX F11: Refresh watchdog before long operation
    NAND_REFRESH_WATCHDOG();

    // Stabilize before switch
    osDelay(20); 

    // ✅ FIX: Memory barriers to ensure all pending operations complete
    __DSB();
    __ISB();

    // STEP 1: De-initialize NAND controller
    NAND_Log("NAND", "Calling HAL_NAND_DeInit...");
    if (HAL_NAND_DeInit(&hnand1) != HAL_OK) {
        NAND_Log("NAND_ERROR", "HAL_NAND_DeInit failed during SDRAM restore");
    }
    NAND_Log("NAND", "HAL_NAND_DeInit completed.");

    // Delay to let hardware settle after DeInit
    osDelay(10);

    // STEP 2: Ensure FMC clock is enabled
    __HAL_RCC_FMC_CLK_ENABLE();
    NAND_Log("NAND", "FMC clock enabled for SDRAM restore");

    // STEP 3: Configure GPIO pins for SDRAM mode
    // This reconfigures shared pins from NAND AF to SDRAM AF mode
    NAND_Log("NAND", "Configuring SDRAM GPIO pins (HAL_SDRAM_MspInit)...");
    HAL_SDRAM_MspInit(&hsdram1);
    NAND_Log("NAND", "SDRAM GPIO pins configured");

    // Delay to let GPIOs settle
    osDelay(10);

    // STEP 4: Re-initialize SDRAM controller with original timing parameters
    // These values MUST match MX_FMC_Init() in main.cpp:653-679
    FMC_SDRAM_TimingTypeDef SdramTiming = {0};

    hsdram1.Instance = FMC_SDRAM_DEVICE;
    hsdram1.Init.SDBank = FMC_SDRAM_BANK1;
    hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_9;
    hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_13;
    hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
    hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_2;
    hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
    hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
    hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_2;

    SdramTiming.LoadToActiveDelay = 2;
    SdramTiming.ExitSelfRefreshDelay = 3;     // Optimized: 9 -> 3 (Matches main.cpp)
    SdramTiming.SelfRefreshTime = 2;          // Optimized: 6 -> 2 (Matches main.cpp)
    SdramTiming.RowCycleDelay = 3;            // Optimized: 9 -> 3 (Matches main.cpp)
    SdramTiming.WriteRecoveryTime = 2;        // Optimized: 3 -> 2 (Matches main.cpp)
    SdramTiming.RPDelay = 2;                  // Optimized: 3 -> 2 (Matches main.cpp)
    SdramTiming.RCDDelay = 2;                 // Optimized: 3 -> 2 (Matches main.cpp)

    if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK) {
        NAND_Log("NAND_ERROR", "HAL_SDRAM_Init failed during restore!");
        return HAL_ERROR;
    }
    NAND_Log("NAND", "SDRAM controller re-initialized successfully");

    // STEP 5: Wait for SDRAM hardware to stabilize
    osDelay(10);  // 10ms stabilization delay
    NAND_Log("NAND", "SDRAM hardware stabilized");

    // STEP 6: Perform SDRAM initialization sequence (power-up)
    // This is critical - SDRAM needs initialization commands after GPIO reconfig
    FMC_SDRAM_CommandTypeDef Command;

    // Configure a clock configuration enable command
    Command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber = 1;
    Command.ModeRegisterDefinition = 0;
    if (HAL_SDRAM_SendCommand(&hsdram1, &Command, 1000) != HAL_OK) {
        NAND_Log("NAND_ERROR", "SDRAM clock enable command failed");
        return HAL_ERROR;
    }
    osDelay(1);

    // Configure a PALL (precharge all) command
    Command.CommandMode = FMC_SDRAM_CMD_PALL;
    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber = 1;
    Command.ModeRegisterDefinition = 0;
    if (HAL_SDRAM_SendCommand(&hsdram1, &Command, 1000) != HAL_OK) {
        NAND_Log("NAND_ERROR", "SDRAM precharge command failed");
        return HAL_ERROR;
    }

    // Configure an Auto-Refresh command
    Command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber = 8;  // 8 auto-refresh cycles
    Command.ModeRegisterDefinition = 0;
    if (HAL_SDRAM_SendCommand(&hsdram1, &Command, 1000) != HAL_OK) {
        NAND_Log("NAND_ERROR", "SDRAM auto-refresh command failed");
        return HAL_ERROR;
    }
    osDelay(1);

    // Program the external memory mode register
    Command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber = 1;
    // Mode register: CAS=2, Burst=Full Page
    Command.ModeRegisterDefinition = 0x0220;
    if (HAL_SDRAM_SendCommand(&hsdram1, &Command, 1000) != HAL_OK) {
        NAND_Log("NAND_ERROR", "SDRAM load mode register command failed");
        return HAL_ERROR;
    }

    // Set refresh rate (64ms / 8192 rows = 7.8125us per row)
    // SDRAM clock = 100MHz, Refresh rate = 781 cycles
    if (HAL_SDRAM_ProgramRefreshRate(&hsdram1, 781) != HAL_OK) {
        NAND_Log("NAND_ERROR", "SDRAM refresh rate programming failed");
        return HAL_ERROR;
    }

    NAND_Log("NAND", "FMC SDRAM restoration completed successfully.");

    // === CACHE COHERENCY: Minimal Invalidation After FMC Restore ===
    // STRATEGY: Only invalidate the test address (1 cache line = 32 bytes)
    // NOTE: SDRAM is Write-Back (TEX=0,C=1,B=1) - NOT Write-Through!
    // Full CleanInvalidate for 4MB here is wasteful (called per-block during NAND read).
    // The final CleanInvalidate is done once in ReadModelFromNAND_ToSDRAM after all copies.

    // Invalidate only the test location (1 cache line = 32 bytes)
    SCB_InvalidateDCache_by_Addr((uint32_t*)0xC0000000, 32);
    __DSB();
    __ISB();

    // CRITICAL VERIFICATION: Test SDRAM read/write after FMC restore
    // This ensures SDRAM is actually working after GPIO/FMC reconfiguration
    volatile uint32_t* sdram_test_addr = (volatile uint32_t*)0xC0000000;  // Activation pool start
    uint32_t test_pattern = 0xDEADBEEF;
    uint32_t original_value = *sdram_test_addr;  // Save original value

    // Write test pattern
    *sdram_test_addr = test_pattern;
    __DSB();
    __ISB();

    // Read back and verify
    uint32_t read_value = *sdram_test_addr;
    if (read_value != test_pattern) {
        NAND_Log("NAND_ERROR", "SDRAM verification FAILED after FMC restore!");
        char err_msg[64];
        snprintf(err_msg, sizeof(err_msg), "Expected: 0x%08lX, Got: 0x%08lX", test_pattern, read_value);
        NAND_Log("NAND_ERROR", err_msg);
        return HAL_ERROR;
    }

    // Restore original value
    *sdram_test_addr = original_value;
    __DSB();

    NAND_Log("NAND", "SDRAM verification passed - read/write OK");
    g_fmc_mode = FMC_MODE_SDRAM;

    return HAL_OK;
}

/**
 * @brief Bir AI modelini NAND Flash'a kalıcı olarak yazar.
 * 
 * Gelen model verisini bloklar halinde NAND'a yazar ve meta-veri (metadata) bloğunu günceller.
 * Bozuk blok (Bad Block) yönetimi yaparak hatalı blokları atlar.
 * 
 * @param model_data Yazılacak modelin ham verisi. [byte array]
 * @param file_size Modelin toplam boyutu. [byte]
 * @param model_name Modele verilecek isim (maks 32 karakter).
 * @return HAL_StatusTypeDef Yazma ve doğrulama başarılıysa HAL_OK döner.
 * 
 * @note Bu fonksiyon FMC modları arasında geçiş yaptığı için SDRAM'deki verileri etkileyebilir. 
 *       RTOS task'ı içinden çağrılmalıdır.
 */
// Shared staging buffer for FMC mode-switching NAND operations.
// CRITICAL: Must be in internal SRAM (AHB SRAM D2), NOT SDRAM.
// SDRAM is inaccessible when FMC is in NAND mode — this buffer is always reachable.
// Both WriteModelToNAND and ReadModelFromNAND_ToSDRAM use this; they never run concurrently.
static uint8_t nand_fmc_staging[NAND_BLOCK_SIZE] __attribute__((section(".ram_d2"))) __attribute__((aligned(32)));

HAL_StatusTypeDef WriteModelToNAND(uint8_t* model_data, uint32_t file_size, const char* model_name) {
    NAND_Log("NAND_WRITE", "NAND Write Operation Started");
    if (file_size == 0) {
        NAND_Log("NAND_ERROR", "File size is 0, aborting write.");
        return HAL_ERROR;
    }

    uint32_t required_blocks = (file_size + NAND_BLOCK_SIZE - 1) / NAND_BLOCK_SIZE;
    char log_buffer[128];
    snprintf(log_buffer, sizeof(log_buffer), "Required blocks: %lu", required_blocks);
    NAND_Log("NAND_WRITE", log_buffer);

    if (required_blocks > MAX_MODEL_BLOCKS) {
        NAND_Log("NAND_ERROR", "Model too large for block map.");
        return HAL_ERROR;
    }

    nand_model_metadata_t metadata = {0};
    metadata.magic_number = NAND_METADATA_MAGIC;
    metadata.model_size = file_size;
    strncpy(metadata.model_name, model_name, sizeof(metadata.model_name) - 1);
    metadata.write_timestamp = HAL_GetTick();
    metadata.block_count = 0;

    uint32_t data_offset = 0;
    uint16_t next_block_to_check = NAND_DATA_START_BLOCK; // Start searching from data start block

    for (uint16_t i = 0; i < required_blocks; i++) {
        uint16_t physical_block = NAND_BlockManager_FindNextGoodBlock(&g_nand_manager, &next_block_to_check);
        if (physical_block == NAND_INVALID_BLOCK) {
            NAND_Log("NAND_ERROR", "Not enough good blocks available.");
            return HAL_ERROR;
        }
        metadata.block_map[i] = physical_block;
        metadata.block_count++;
        next_block_to_check = physical_block + 1;
    }

    // Erase only the blocks that will be used
    NAND_Log("NAND_ERASE", "Erasing necessary blocks for the new model...");
    
    // CRITICAL FIX: Must switch to NAND mode before erase operations
    if (FMC_ReInitNAND() != HAL_OK) return HAL_ERROR;

    for (uint16_t i = 0; i < metadata.block_count; i++) {
        uint16_t block_to_erase = metadata.block_map[i];
        if (NAND_BlockManager_EraseBlock(&g_nand_manager, block_to_erase) != HAL_OK) {
            snprintf(log_buffer, sizeof(log_buffer), "Failed to erase block %u, will try to find a new one.", block_to_erase);
            NAND_Log("NAND_WARNING", log_buffer);
            // Mark this block as bad and find a new one
            g_nand_manager.blocks[block_to_erase].status = BLOCK_STATUS_BAD;
            uint16_t new_block = NAND_BlockManager_FindNextGoodBlock(&g_nand_manager, &next_block_to_check);
            if (new_block == NAND_INVALID_BLOCK) {
                NAND_Log("NAND_ERROR", "Not enough good blocks available after an erase failure.");
                FMC_RestoreSDRAM(); // Restore SDRAM before returning
                return HAL_ERROR;
            }
            metadata.block_map[i] = new_block;
            next_block_to_check = new_block + 1;
            // Now erase the new block
            if (NAND_BlockManager_EraseBlock(&g_nand_manager, new_block) != HAL_OK) {
                 NAND_Log("NAND_ERROR", "Failed to erase the replacement block. Aborting.");
                 FMC_RestoreSDRAM(); // Restore SDRAM before returning
                 return HAL_ERROR;
            }
        }
    }

    // Restore SDRAM mode after erase loop (Write blocks will handle their own switching)
    if (FMC_RestoreSDRAM() != HAL_OK) return HAL_ERROR;

    // CRITICAL FIX: FMC cannot access SDRAM and NAND simultaneously.
    // model_data points to SDRAM (0xC0200000). Reading it while FMC is in NAND mode
    // causes hardware errors. Stage each block in nand_fmc_staging (AHB SRAM D2)
    // BEFORE switching FMC to NAND mode — always accessible regardless of FMC mode.

    // Write data blocks
    for (uint16_t i = 0; i < metadata.block_count; i++) {
        uint16_t physical_block = metadata.block_map[i];
        uint32_t bytes_remaining = file_size - data_offset;
        uint32_t bytes_in_block = (bytes_remaining > NAND_BLOCK_SIZE) ? NAND_BLOCK_SIZE : bytes_remaining;
        uint16_t pages_in_block = (bytes_in_block + NAND_PAGE_SIZE - 1) / NAND_PAGE_SIZE;

        // Stage block data from SDRAM into AHB SRAM D2 while FMC is still in SDRAM mode
        memset(nand_fmc_staging, 0xFF, NAND_BLOCK_SIZE);  // NAND erased state padding
        memcpy(nand_fmc_staging, model_data + data_offset, bytes_in_block);
        SCB_CleanDCache_by_Addr((uint32_t*)nand_fmc_staging, NAND_BLOCK_SIZE);
        data_offset += bytes_in_block;

        if (FMC_ReInitNAND() != HAL_OK) return HAL_ERROR;

        for (uint16_t page = 0; page < pages_in_block; page++) {
            ecc_status_t ecc_res;
            // Write from AHB SRAM D2 buffer — NOT from SDRAM
            if (NAND_BlockManager_WritePage(&g_nand_manager, physical_block, page,
                                            nand_fmc_staging + (page * NAND_PAGE_SIZE),
                                            &ecc_res) != HAL_OK) {
                snprintf(log_buffer, sizeof(log_buffer), "Failed to write page %u in block %u", page, physical_block);
                NAND_Log("NAND_ERROR", log_buffer);
                FMC_RestoreSDRAM();
                return HAL_ERROR;
            }
        }
        if (FMC_RestoreSDRAM() != HAL_OK) return HAL_ERROR;
    }

    // Write metadata
    if (FMC_ReInitNAND() != HAL_OK) return HAL_ERROR;
    if (NAND_BlockManager_EraseBlock(&g_nand_manager, NAND_METADATA_BLOCK) != HAL_OK) {
        NAND_Log("NAND_WARNING", "Failed to erase metadata block");
    }
    ecc_status_t ecc_res;
    if (NAND_BlockManager_WritePage(&g_nand_manager, NAND_METADATA_BLOCK, 0, (uint8_t*)&metadata, &ecc_res) != HAL_OK) {
        NAND_Log("NAND_ERROR", "Failed to write metadata");
        FMC_RestoreSDRAM();
        return HAL_ERROR;
    }
    if (FMC_RestoreSDRAM() != HAL_OK) return HAL_ERROR;

    NAND_Log("NAND_WRITE", "NAND Write Operation Complete");
    return HAL_OK;
}

/**
 * @brief NAND Flash'tan model meta-verilerini okur.
 * 
 * NAND'ın rezerve edilmiş meta-veri bloğundaki sihirli sayı (magic number) ve model 
 * boyut bilgilerini kontrol eder.
 * 
 * @param metadata Okunan bilgilerin yazılacağı yapı pointer'ı.
 * @return HAL_StatusTypeDef Meta-veri geçerliyse HAL_OK döner.
 * 
 * @note FMC geçişlerini otomatik yapar.
 */
HAL_StatusTypeDef ReadModelMetadata(nand_model_metadata_t* metadata) {
    if (!metadata) return HAL_ERROR;

    char log_buffer[128];
    snprintf(log_buffer, sizeof(log_buffer), "Reading metadata from Block %d, Page 0", NAND_METADATA_BLOCK);
    NAND_Log("NAND_READ", log_buffer);
    uint8_t page_buffer[NAND_PAGE_SIZE];

    if (g_nand_manager.blocks[NAND_METADATA_BLOCK].status == BLOCK_STATUS_BAD) {
        NAND_Log("NAND_ERROR", "Metadata block is bad.");
        return HAL_ERROR;
    }

    if (FMC_ReInitNAND() != HAL_OK) return HAL_ERROR;

    ecc_status_t ecc_res;
    if (NAND_BlockManager_ReadPage(&g_nand_manager, NAND_METADATA_BLOCK, 0, page_buffer, &ecc_res) != HAL_OK) {
        NAND_Log("NAND_ERROR", "Failed to read metadata page.");
        FMC_RestoreSDRAM();
        return HAL_ERROR;
    }

    if (FMC_RestoreSDRAM() != HAL_OK) return HAL_ERROR;

    nand_model_metadata_t local_metadata;
    memcpy(&local_metadata, page_buffer, sizeof(nand_model_metadata_t));
    
    if (local_metadata.magic_number != NAND_METADATA_MAGIC) {
        snprintf(log_buffer, sizeof(log_buffer), "Invalid metadata magic number: got 0x%08lX", local_metadata.magic_number);
        NAND_Log("NAND_ERROR", log_buffer);
        return HAL_ERROR;
    }

    memcpy(metadata, &local_metadata, sizeof(nand_model_metadata_t));
    snprintf(log_buffer, sizeof(log_buffer), "Metadata read: size=%lu, blocks=%u", metadata->model_size, metadata->block_count);
    NAND_Log("NAND_READ", log_buffer);

    return HAL_OK;
}

/**
 * @brief AI modelini NAND Flash'tan okuyarak SDRAM'e yükler.
 * 
 * Verimlilik için tek seferlik FMC geçişleri planlar. Blokları ham RAM'e (RAM_D2) okuyup 
 * ardından SDRAM'e taşır. İşlem sonunda AI çıkarımı için SDRAM Cache'ini geçersiz kılar.
 * 
 * @return HAL_StatusTypeDef Yükleme başarılıysa HAL_OK döner.
 * 
 * @note Projenin en kritik ve karmaşık hafıza yönetim fonksiyonudur. 
 *       Sadece sistem başlatma veya model güncelleme sırasında RTOS içinden çağrılmalıdır.
 */
HAL_StatusTypeDef ReadModelFromNAND_ToSDRAM(void) {
    NAND_Log("NAND_READ", "NAND to SDRAM Model Read Started");

    // STEP 1: Read metadata first (requires NAND mode)
    // ReadModelMetadata() handles its own FMC switching
    nand_model_metadata_t metadata;
    if (ReadModelMetadata(&metadata) != HAL_OK) {
        NAND_Log("NAND_ERROR", "Failed to read valid metadata.");
        return HAL_ERROR;
    }

    // STEP 2: CRITICAL FIX - Single FMC switch to NAND mode for ALL blocks
    // Previous bug: FMC switched 33 times per read (66 total operations)
    // New approach: Switch once → Read all blocks → Switch back once
    NAND_Log("NAND_READ", "Switching FMC to NAND mode for bulk read...");
    if (FMC_ReInitNAND() != HAL_OK) {
        NAND_Log("NAND_ERROR", "Failed to initialize FMC for NAND bulk read");
        return HAL_ERROR;
    }

    // STEP 3: Prepare temporary buffer in RAM_D2 for safe block reads
    // CRITICAL: Cannot write directly to SDRAM while FMC is in NAND mode!
    // Strategy: Read NAND block → temp buffer (RAM_D2) → accumulate all blocks → SDRAM write
    // MOVED TO SDRAM to avoid RAM_D2 overflow (RAM_D2 total usage was 616KB > 288KB limit)
    uint8_t* sdram_final_ptr = (uint8_t*)0xC0200000;  // Final destination
    // Use shared nand_fmc_staging buffer (file-scope, AHB SRAM D2) — no extra RAM needed
    uint32_t bytes_copied = 0;
    uint32_t total_bytes_read = 0;
    char log_buffer[128];

    // STEP 4: Read ALL blocks from NAND (FMC stays in NAND mode)
    for (uint16_t i = 0; i < metadata.block_count; i++) {
        uint16_t physical_block = metadata.block_map[i];
        if (g_nand_manager.blocks[physical_block].status == BLOCK_STATUS_BAD) {
            snprintf(log_buffer, sizeof(log_buffer), "Skipping bad block %u listed in map.", physical_block);
            NAND_Log("NAND_WARNING", log_buffer);
            continue;
        }

        uint32_t bytes_to_read_in_block = (total_bytes_read + NAND_BLOCK_SIZE > metadata.model_size)
                                           ? (metadata.model_size - total_bytes_read)
                                           : NAND_BLOCK_SIZE;
        uint16_t pages_in_block = (bytes_to_read_in_block + NAND_PAGE_SIZE - 1) / NAND_PAGE_SIZE;

        // Read entire block into temporary buffer (page by page)
        bytes_copied = 0;
        for (uint16_t page = 0; page < pages_in_block; page++) {
            ecc_status_t ecc_res;
            // Read page into temp buffer (RAM_D2 - safe to access during NAND mode, no SDRAM conflict)
            if (NAND_BlockManager_ReadPage(&g_nand_manager, physical_block, page,
                                           nand_fmc_staging + bytes_copied, &ecc_res) != HAL_OK) {
                snprintf(log_buffer, sizeof(log_buffer), "Failed to read page %u from block %u", page, physical_block);
                NAND_Log("NAND_ERROR", log_buffer);
                FMC_RestoreSDRAM();  // Restore before returning
                return HAL_ERROR;
            }
            bytes_copied += NAND_PAGE_SIZE;
        }

        // Log success for the block read from NAND (before SDRAM switch)
        snprintf(log_buffer, sizeof(log_buffer), "Block %u read from NAND into internal RAM buffer", physical_block);
        NAND_Log("NAND_READ", log_buffer);

        // CRITICAL: Switch to SDRAM mode before writing to SDRAM
        if (FMC_RestoreSDRAM() != HAL_OK) {
            NAND_Log("NAND_ERROR", "Failed to restore SDRAM after block read");
            return HAL_ERROR;
        }

        // Now safe to write to SDRAM (FMC is in SDRAM mode)
        memcpy(sdram_final_ptr + total_bytes_read, nand_fmc_staging, bytes_to_read_in_block);
        __DSB();  // Memory barrier to ensure write completes
        __ISB();

        total_bytes_read += bytes_to_read_in_block;

        // Progress log every 10 blocks
        if ((i % 10) == 0 || i == metadata.block_count - 1) {
            snprintf(log_buffer, sizeof(log_buffer), "NAND read progress: %u/%u blocks (%lu bytes)",
                     i + 1, metadata.block_count, total_bytes_read);
            NAND_Log("NAND_READ", log_buffer);
        }

        // FIX F11: Refresh watchdog every 5 blocks to prevent timeout during long NAND reads
        if ((i % 5) == 0) {
            NAND_REFRESH_WATCHDOG();
        }

        // Switch back to NAND mode for next block (if not last block)
        if (i < metadata.block_count - 1) {
            if (FMC_ReInitNAND() != HAL_OK) {
                NAND_Log("NAND_ERROR", "Failed to re-init NAND for next block");
                return HAL_ERROR;
            }
        }
    }

    // STEP 5: Verify byte count
    if (total_bytes_read < metadata.model_size) {
        snprintf(log_buffer, sizeof(log_buffer), "Size mismatch: copied %lu, expected %lu",
                 total_bytes_read, metadata.model_size);
        NAND_Log("NAND_ERROR", log_buffer);
        return HAL_ERROR;
    }

    // STEP 6: Final SDRAM mode restoration (should already be in SDRAM mode from last b lock)
    // This is a safety check
    if (FMC_RestoreSDRAM() != HAL_OK) {
        NAND_Log("NAND_WARNING", "Final SDRAM restore returned error (may already be in SDRAM mode)");
        // Don't return error - SDRAM write already succeeded
    }

    // === CACHE COHERENCY: Clean+Invalidate after NAND read ===
    // CRITICAL: memcpy() to Write-Back SDRAM only fills D-Cache (dirty lines).
    // Physical SDRAM still has zeros (cleared by load_model_flash verification step).
    // SCB_InvalidateDCache (used previously) DISCARDS dirty lines → data lost!
    // SCB_CleanInvalidateDCache: first WRITES dirty lines to SDRAM, then invalidates.
    // After this, physical SDRAM at 0xC0200000 has real model weights.
    uint32_t aligned_size = (metadata.model_size + 31u) & ~31u;  // Round up to cache line
    NAND_Log("NAND_READ", "CleanInvalidate SDRAM cache after NAND read (Write-Back flush)...");
    SCB_CleanInvalidateDCache_by_Addr((uint32_t*)0xC0200000, aligned_size);
    __DSB();
    __ISB();

    NAND_Log("NAND_READ", "NAND to SDRAM copy complete.");
    return HAL_OK;
}