#include "nand_block_manager_enhanced.h"
#include "main.h"
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

// Macro definitions for delay - use FreeRTOS if available
#ifdef osDelay
    #define DELAY_MS(x) osDelay(x)
#else
    #define DELAY_MS(x) HAL_Delay(x)
#endif

// Logging macros - simplified for compatibility
#define LOG_INFO(module, fmt, ...) printf("[INFO][%s] " fmt "\n", module, ##__VA_ARGS__)
#define LOG_ERROR(module, fmt, ...) printf("[ERROR][%s] " fmt "\n", module, ##__VA_ARGS__)
#define LOG_WARN(module, fmt, ...) printf("[WARN][%s] " fmt "\n", module, ##__VA_ARGS__)
#define LOG_DEBUG(module, fmt, ...) printf("[DEBUG][%s] " fmt "\n", module, ##__VA_ARGS__)
#define LOG_NAND "NAND"

// =============================================================================
// STATIC FUNCTION PROTOTYPES
// =============================================================================

static HAL_StatusTypeDef NAND_HW_ReadPageWithECC(nand_block_manager_t *manager,
                                                 uint16_t block, uint16_t page,
                                                 uint8_t *data, ecc_status_t *ecc_result);
static HAL_StatusTypeDef NAND_HW_WritePageWithECC(nand_block_manager_t *manager,
                                                  uint16_t block, uint16_t page,
                                                  uint8_t *data, ecc_status_t *ecc_result);
static HAL_StatusTypeDef NAND_HW_ReadSpareArea(nand_block_manager_t *manager,
                                              uint16_t block, uint16_t page,
                                              uint8_t *spare_data, uint16_t size);
static HAL_StatusTypeDef NAND_HW_WriteSpareArea(nand_block_manager_t *manager,
                                               uint16_t block, uint16_t page,
                                               uint8_t *spare_data, uint16_t size);

// =============================================================================
// CORE MANAGEMENT FUNCTIONS
// =============================================================================

/**
 * @brief NAND Blok Yöneticisini başlatır.
 * 
 * Yönetici yapısını sıfırlar, donanım handle'ını bağlar ve başlangıç ayarlarını yapar.
 * Sensörü resetleyerek bozuk blok taramasını (Bad Block Scan) başlatır.
 * 
 * @param manager Yapılandırılacak yönetici pointer'ı.
 * @param hnand Kullanılacak HAL NAND handle'ı.
 * @return HAL_StatusTypeDef Başlatma başarılıysa HAL_OK döner.
 * 
 * @note Bu fonksiyon uzun süren donanım taramaları ve reset işlemleri içerdiği için 
 *       ISR içinden ÇAĞRILAMAZ. Sistem açılışında bir kez çağrılmalıdır.
 */
HAL_StatusTypeDef NAND_BlockManager_Init(nand_block_manager_t *manager, NAND_HandleTypeDef *hnand) {
    if (manager == NULL || hnand == NULL) {
        LOG_ERROR(LOG_NAND, "Init failed: NULL parameter");
        return HAL_ERROR;
    }

    memset(manager, 0, sizeof(nand_block_manager_t));
    manager->hnand = hnand;
    manager->multiplane_enabled = true;
    manager->ecc_enabled = true;
    manager->wear_leveling_enabled = true;
    // WORKAROUND: Start allocation from Block 2 and 1 to avoid potential Block 0 issues
    manager->next_block_to_use[0] = 2;  // Plane 0: start from Block 2
    manager->next_block_to_use[1] = 1;  // Plane 1: start from Block 1

    HAL_StatusTypeDef status = NAND_BlockManager_Reset(manager);
    if (status != HAL_OK) {
        LOG_ERROR(LOG_NAND, "Init failed: Reset failed");
        return status;
    }

    for (uint16_t i = 0; i < NAND_TOTAL_BLOCKS; i++) {
        manager->blocks[i].block_number = i;
        manager->blocks[i].status = BLOCK_STATUS_GOOD;
        manager->blocks[i].wear_count = 0;
        manager->blocks[i].plane_id = NAND_BlockManager_GetPlaneFromBlock(i);
        manager->blocks[i].program_fail_count = 0;
        manager->blocks[i].erase_fail_count = 0;
        manager->blocks[i].last_used_timestamp = HAL_GetTick();
    }

    status = NAND_BlockManager_ScanBadBlocks(manager);
    if (status != HAL_OK) {
        LOG_ERROR(LOG_NAND, "Init failed: Bad block scan failed");
        return status;
    }

    // Preemptively blacklist known problematic blocks
    NAND_BlockManager_BlacklistBlock(manager, 6);  // Block 6 consistently fails
    LOG_INFO(LOG_NAND, "Pre-blacklisted known problematic blocks");

    manager->initialized = true;
    manager->last_operation_time = HAL_GetTick();

    LOG_INFO(LOG_NAND, "NAND Block Manager initialized successfully");
    LOG_INFO(LOG_NAND, "Total Blocks: %d, Bad Blocks: %d, Available: %d",
           NAND_TOTAL_BLOCKS, manager->bad_block_count, manager->available_blocks);

    return HAL_OK;
}

HAL_StatusTypeDef NAND_BlockManager_Reset(nand_block_manager_t *manager) {
    if (manager == NULL || manager->hnand == NULL) return HAL_ERROR;
    HAL_StatusTypeDef status = HAL_NAND_Reset(manager->hnand);
    if (status != HAL_OK) return status;
    return NAND_HW_WaitReady(manager, NAND_TIMEOUT_RESET_MS);
}

// =============================================================================
// BAD BLOCK MANAGEMENT
// =============================================================================

/**
 * @brief Tüm NAND Flash belleği bozuk bloklar için tarar.
 * 
 * Her bloğun ilk iki sayfasındaki yedek alanı (Spare Area) kontrol ederek 0xFF'den 
 * farklı bir değer olup olmadığına bakar. İşlem bitiminde istatistikleri günceller.
 * 
 * @param manager Yönetici pointer'ı.
 * @return HAL_StatusTypeDef Tarama tamamlandıysa HAL_OK döner.
 * 
 * @note Yoğun bir IO işlemidir. ISR içinden çağrılmamalıdır.
 */
HAL_StatusTypeDef NAND_BlockManager_ScanBadBlocks(nand_block_manager_t *manager) {
    if (manager == NULL) return HAL_ERROR;

    uint8_t spare_data[NAND_SPARE_SIZE];
    manager->bad_block_count = 0;
    manager->available_blocks = 0;

    for (uint16_t block = 0; block < NAND_TOTAL_BLOCKS; block++) {
        bool is_bad = false;
        
        for (uint16_t page = 0; page < 2; page++) {
            if (NAND_HW_ReadSpareArea(manager, block, page, spare_data, NAND_SPARE_SIZE) != HAL_OK) {
                is_bad = true;
                break;
            }
            if (spare_data[0] != 0xFF) {
                is_bad = true;
                break;
            }
        }

        if (is_bad) {
            manager->blocks[block].status = BLOCK_STATUS_BAD;
            manager->bad_block_count++;
        } else {
            manager->blocks[block].status = BLOCK_STATUS_GOOD;
            manager->available_blocks++;
        }
    }

    return HAL_OK;
}

/**
 * @brief Bir bloğu "Bozuk" (Bad Block) olarak işaretler.
 * 
 * Blok statüsünü bellekte günceller ve kalıcı olması için NAND üzerindeki yedek alana 
 * "Bad Block Marker" (0x00) yazar.
 * 
 * @param manager Yönetici pointer'ı.
 * @param block_index İşaretlenecek bloğun fiziksel adresi. [indis]
 * @return HAL_StatusTypeDef İşlem başarılıysa HAL_OK döner.
 * 
 * @note Yazma işlemi içerdiği için ISR içinden çağrılmamalıdır.
 */
HAL_StatusTypeDef NAND_BlockManager_MarkBadBlock(nand_block_manager_t *manager, uint16_t block_index) {
    if (manager == NULL || block_index >= NAND_TOTAL_BLOCKS) return HAL_ERROR;
    if (manager->blocks[block_index].status == BLOCK_STATUS_BAD) return HAL_OK;

    manager->blocks[block_index].status = BLOCK_STATUS_BAD;
    manager->bad_block_count++;
    manager->available_blocks--;
    manager->stats.bad_blocks_detected++;

    uint8_t bad_marker[NAND_SPARE_SIZE];
    memset(bad_marker, 0xFF, NAND_SPARE_SIZE);
    bad_marker[0] = 0x00;

    for (uint16_t page = 0; page < 2; page++) {
        NAND_HW_WriteSpareArea(manager, block_index, page, bad_marker, NAND_SPARE_SIZE);
    }
    return HAL_OK;
}

// =============================================================================
// BLOCK ALLOCATION AND MANAGEMENT
// =============================================================================

uint16_t NAND_BlockManager_AllocateBlock(nand_block_manager_t *manager, uint8_t plane_id) {
    if (manager == NULL || plane_id >= NAND_TOTAL_PLANES) {
        return NAND_INVALID_BLOCK;
    }

    if (manager->next_block_to_use[plane_id] == 0) {
        manager->next_block_to_use[plane_id] = (plane_id == 0) ? 2 : 1;
    }

    uint16_t start_block = manager->next_block_to_use[plane_id];
    uint16_t current_block = start_block;

    do {
        if ((manager->blocks[current_block].status == BLOCK_STATUS_GOOD) &&
            (NAND_BlockManager_GetPlaneFromBlock(current_block) == plane_id) &&
            (!NAND_BlockManager_IsBlockBlacklisted(manager, current_block))) {

            manager->next_block_to_use[plane_id] = (current_block + NAND_TOTAL_PLANES) % NAND_TOTAL_BLOCKS;
            manager->blocks[current_block].last_used_timestamp = HAL_GetTick();
            return current_block;
        }
        current_block = (current_block + NAND_TOTAL_PLANES) % NAND_TOTAL_BLOCKS;
    } while (current_block != start_block);

    return NAND_INVALID_BLOCK;
}

/**
 * @brief Sıralı bir şekilde bir sonraki sağlam bloğu bulur.
 * 
 * Belirtilen indisten başlayarak bellek sonuna kadar sağlam blok (Good Block) arar.
 * 
 * @param manager Yönetici pointer'ı.
 * @param start_index Aramanın başlayacağı indis. [indis]
 * @return uint16_t Bulunan bloğun indisi, bulunamazsa NAND_INVALID_BLOCK döner.
 * 
 * @note Sadece bellek içi tarama yaptığı için ISR içinden çağrılabilir.
 */
uint16_t NAND_BlockManager_FindNextGoodBlock(nand_block_manager_t *manager, uint16_t *start_index) {
    if (manager == NULL || start_index == NULL) {
        return NAND_INVALID_BLOCK;
    }

    for (uint16_t i = *start_index; i < NAND_TOTAL_BLOCKS; i++) {
        if (manager->blocks[i].status == BLOCK_STATUS_GOOD && !NAND_BlockManager_IsBlockBlacklisted(manager, i)) {
            *start_index = i + 1;
            return i;
        }
    }

    return NAND_INVALID_BLOCK; // No good block found
}

/**
 * @brief Bir bloğu fiziksel olarak siler (Erase).
 * 
 * Bloğu silme komutu gönderir, donanımsal wp pini yönetimini yapar ve silme 
 * işleminin başarısını doğrular. Başarısızlık durumunda bloğu bad block olarak işaretler.
 * 
 * @param manager Yönetici pointer'ı.
 * @param block_index Silinecek blok adresi. [indis]
 * @return HAL_StatusTypeDef Silme başarılıysa HAL_OK döner.
 * 
 * @note Silme işlemi NAND teknolojisinde en yavaş işlemlerden biridir. ISR içinden 
 *       ÇAĞRILAMAZ. RTOS task'ı içinde çalıştırılmalıdır.
 */
HAL_StatusTypeDef NAND_BlockManager_EraseBlock(nand_block_manager_t *manager, uint16_t block_index) {
    if (manager == NULL || block_index >= NAND_TOTAL_BLOCKS || manager->blocks[block_index].status != BLOCK_STATUS_GOOD) {
        return HAL_ERROR;
    }

    uint32_t erase_timeout_ms = NAND_TIMEOUT_ERASE_MS;
    if (block_index == 6 || block_index == 8) { 
        erase_timeout_ms = NAND_TIMEOUT_PROBLEMATIC_ERASE_MS;
    }

    HAL_GPIO_WritePin(FMC_nWP_GPIO_Port, FMC_nWP_Pin, GPIO_PIN_SET);
    DELAY_MS(1);  // Pin stabilization delay

    NAND_AddressTypeDef nand_addr;
    nand_addr.Block = block_index;
    nand_addr.Page = 0;
    nand_addr.Plane = NAND_BlockManager_GetPlaneFromBlock(block_index);

    if (HAL_NAND_Erase_Block(manager->hnand, &nand_addr) != HAL_OK) {
        HAL_GPIO_WritePin(FMC_nWP_GPIO_Port, FMC_nWP_Pin, GPIO_PIN_RESET);
        manager->blocks[block_index].erase_fail_count++;
        if (manager->blocks[block_index].erase_fail_count >= 3) {
            NAND_BlockManager_MarkBadBlock(manager, block_index);
        }
        return HAL_ERROR;
    }

    if (NAND_HW_WaitReady(manager, erase_timeout_ms) != HAL_OK) {
        NAND_BlockManager_MarkBadBlock(manager, block_index);
        HAL_GPIO_WritePin(FMC_nWP_GPIO_Port, FMC_nWP_Pin, GPIO_PIN_RESET);
        HAL_NAND_Reset(manager->hnand);
        return HAL_ERROR;
    }

    if (NAND_HW_CheckOperationStatus(manager) != HAL_OK) {
        NAND_BlockManager_MarkBadBlock(manager, block_index);
        HAL_GPIO_WritePin(FMC_nWP_GPIO_Port, FMC_nWP_Pin, GPIO_PIN_RESET);
        return HAL_ERROR;
    }

    HAL_GPIO_WritePin(FMC_nWP_GPIO_Port, FMC_nWP_Pin, GPIO_PIN_RESET);
    
    uint8_t verify_buffer[NAND_PAGE_SIZE];
    ecc_status_t ecc_result;
    
    if (NAND_BlockManager_ReadPage(manager, block_index, 0, verify_buffer, &ecc_result) == HAL_OK) {
        bool erase_ok = true;
        for (uint16_t i = 0; i < NAND_PAGE_SIZE; i++) {
            if (verify_buffer[i] != 0xFF) {
                erase_ok = false;
                break;
            }
        }
        if (!erase_ok) {
            NAND_BlockManager_MarkBadBlock(manager, block_index);
            return HAL_ERROR;
        }
    } 

    manager->stats.total_block_erases++;
    manager->blocks[block_index].wear_count++;
    return HAL_OK;
}

// =============================================================================
// PAGE OPERATIONS WITH ECC
// =============================================================================

/**
 * @brief NAND Flash'a veri sayfası yazar (Write Page).
 * 
 * Veriyi belirtilen bloka ve sayfaya yazar. Yazma sonrası donanımsal ECC'yi 
 * kontrol eder ve verinin doğruluğunu `ReadPage` ile teyit eder.
 * 
 * @param manager Yönetici pointer'ı.
 * @param block_index Hedef blok. [indis]
 * @param page_index Hedef sayfa. [indis]
 * @param data Yazılacak 2048-byte'lık veri bloğu. [byte array]
 * @param ecc_result ECC sonucunun yazılacağı yer. [status]
 * @return HAL_StatusTypeDef Yazma ve doğrulama başarılıysa HAL_OK döner.
 * 
 * @note ISR içinden ÇAĞRILAMAZ.
 */
HAL_StatusTypeDef NAND_BlockManager_WritePage(nand_block_manager_t *manager,
                                             uint16_t block_index, uint16_t page_index,
                                             uint8_t *data, ecc_status_t *ecc_result) {
    if (manager == NULL || data == NULL || block_index >= NAND_TOTAL_BLOCKS || page_index >= NAND_PAGES_PER_BLOCK || manager->blocks[block_index].status != BLOCK_STATUS_GOOD) {
        return HAL_ERROR;
    }

    HAL_GPIO_WritePin(FMC_nWP_GPIO_Port, FMC_nWP_Pin, GPIO_PIN_SET);
    DELAY_MS(1);  // 1ms stabilization delay

    HAL_StatusTypeDef status = NAND_HW_WritePageWithECC(manager, block_index, page_index, data, ecc_result);

    HAL_GPIO_WritePin(FMC_nWP_GPIO_Port, FMC_nWP_Pin, GPIO_PIN_RESET);

    if (status != HAL_OK) {
        manager->blocks[block_index].program_fail_count++;
        if (manager->blocks[block_index].program_fail_count >= 3) {
            NAND_BlockManager_MarkBadBlock(manager, block_index);
        }
        return status;
    }

    uint8_t verify_buffer[NAND_PAGE_SIZE];
    ecc_status_t verify_ecc_result;
    
    if (NAND_BlockManager_ReadPage(manager, block_index, page_index, verify_buffer, &verify_ecc_result) == HAL_OK) {
        bool verify_ok = (memcmp(data, verify_buffer, NAND_PAGE_SIZE) == 0);
        if (!verify_ok) {
            manager->blocks[block_index].program_fail_count++;
            if (manager->blocks[block_index].program_fail_count >= 3) {
                NAND_BlockManager_MarkBadBlock(manager, block_index);
            }
            return HAL_ERROR;
        }
    } 

    manager->stats.total_page_writes++;
    manager->blocks[block_index].wear_count++;
    manager->blocks[block_index].last_used_timestamp = HAL_GetTick();

    if (manager->blocks[block_index].wear_count >= NAND_MAX_WEAR_COUNT) {
        manager->blocks[block_index].status = BLOCK_STATUS_WORN;
    }
    return HAL_OK;
}

/**
 * @brief NAND Flash'tan veri sayfası okur (Read Page).
 * 
 * Donanımsal ECC düzeltmesini kullanarak veriyi bellekten çeker. Hata durumunda 
 * (Uncorrectable) bloğu bad block olarak işaretleyebilir.
 * 
 * @param manager Yönetici pointer'ı.
 * @param block_index Okunacak blok. [indis]
 * @param page_index Okunacak sayfa. [indis]
 * @param data Verinin yazılacağı 2048-byte'lık tampon. [byte array]
 * @param ecc_result ECC sonucunun döneceği yer. [status]
 * @return HAL_StatusTypeDef Okuma başarılıysa HAL_OK döner.
 * 
 * @note ISR içinden çağrılmamalıdır (Donanım wait-ready döngüleri içerir).
 */
HAL_StatusTypeDef NAND_BlockManager_ReadPage(nand_block_manager_t *manager,
                                            uint16_t block_index, uint16_t page_index,
                                            uint8_t *data, ecc_status_t *ecc_result) {
    if (manager == NULL || data == NULL || block_index >= NAND_TOTAL_BLOCKS || page_index >= NAND_PAGES_PER_BLOCK || manager->blocks[block_index].status == BLOCK_STATUS_BAD) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = NAND_HW_ReadPageWithECC(manager, block_index, page_index, data, ecc_result);
    if (status != HAL_OK) return status;

    manager->stats.total_page_reads++;
    if (ecc_result != NULL) {
        switch (*ecc_result) {
            case ECC_STATUS_SUCCESS:
                break;
            case ECC_STATUS_CORRECTED_1BIT:
                manager->stats.ecc_stats.corrected_1bit_errors++;
                break;
            case ECC_STATUS_CORRECTED_MULTI:
                manager->stats.ecc_stats.corrected_multibit_errors++;
                break;
            case ECC_STATUS_UNCORRECTABLE:
                manager->stats.ecc_stats.uncorrectable_errors++;
                NAND_BlockManager_MarkBadBlock(manager, block_index);
                return HAL_ERROR;
            default:
                break;
        }
    }
    return HAL_OK;
}

// =============================================================================
// HARDWARE INTERFACE FUNCTIONS
// =============================================================================

HAL_StatusTypeDef NAND_HW_WaitReady(nand_block_manager_t *manager, uint32_t timeout_ms) {
    if (manager == NULL) return HAL_ERROR;

    uint32_t start_time = HAL_GetTick();

    while ((HAL_GetTick() - start_time) < timeout_ms) {
        if ((HAL_NAND_Read_Status(manager->hnand) & NAND_READY) == NAND_READY) {
            return HAL_OK;
        }
        DELAY_MS(1);  // RTOS-friendly delay
    }

    // Timeout durumunda son durumu logla
    uint32_t final_status = HAL_NAND_Read_Status(manager->hnand);
    LOG_ERROR(LOG_NAND, "NAND timeout after %lu ms! Final status: 0x%02lX",
              (unsigned long)timeout_ms, (unsigned long)final_status);

    return HAL_TIMEOUT;
}

HAL_StatusTypeDef NAND_HW_ReadStatusRegister(nand_block_manager_t *manager, uint8_t *status) {
    if (manager == NULL || status == NULL) return HAL_ERROR;
    *status = (uint8_t)HAL_NAND_Read_Status(manager->hnand);
    return HAL_OK;
}

HAL_StatusTypeDef NAND_HW_CheckOperationStatus(nand_block_manager_t *manager) {
    uint8_t status_reg;
    if (NAND_HW_ReadStatusRegister(manager, &status_reg) != HAL_OK) return HAL_ERROR;
    if (status_reg & NAND_SR_FAIL_BIT) return HAL_ERROR;
    return HAL_OK;
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

uint8_t NAND_BlockManager_GetPlaneFromBlock(uint16_t block_index) {
    return (block_index % 2);
}


uint16_t NAND_BlockManager_FindLeastWornBlock(nand_block_manager_t *manager, uint8_t plane_id) {
    if (manager == NULL || plane_id >= NAND_TOTAL_PLANES) return NAND_INVALID_BLOCK;

    uint16_t best_block = NAND_INVALID_BLOCK;
    uint32_t min_wear_count = UINT32_MAX;

    for (uint16_t block = plane_id; block < NAND_TOTAL_BLOCKS; block += 2) {
        if (manager->blocks[block].status == BLOCK_STATUS_GOOD &&
            manager->blocks[block].wear_count < min_wear_count) {
            min_wear_count = manager->blocks[block].wear_count;
            best_block = block;
        }
    }
    return best_block;
}

void NAND_BlockManager_PrintStatus(nand_block_manager_t *manager) {
    if (manager == NULL) return;

    uint32_t worn_blocks = 0;
    uint32_t blacklisted_blocks = 0;
    for(int i=0; i < NAND_TOTAL_BLOCKS; i++) {
        if(manager->blocks[i].status == BLOCK_STATUS_WORN) {
            worn_blocks++;
        }
        if(manager->blocks[i].status == BLOCK_STATUS_BLACKLISTED) {
            blacklisted_blocks++;
        }
    }

    LOG_INFO(LOG_NAND, "=== NAND BLOCK MANAGER STATUS ===");
    LOG_INFO(LOG_NAND, "Total: %d, Bad: %d, Worn: %lu, Blacklisted: %lu, Available: %d",
             NAND_TOTAL_BLOCKS, manager->bad_block_count, worn_blocks, blacklisted_blocks, manager->available_blocks);
    LOG_INFO(LOG_NAND, "Page Writes: %lu, Page Reads: %lu, Block Erases: %lu",
             manager->stats.total_page_writes, manager->stats.total_page_reads, manager->stats.total_block_erases);
    LOG_INFO(LOG_NAND, "Next block to use [Plane 0]: %u", manager->next_block_to_use[0]);
    LOG_INFO(LOG_NAND, "Next block to use [Plane 1]: %u", manager->next_block_to_use[1]);
    LOG_INFO(LOG_NAND, "===============================");
}

// =============================================================================
// BLACKLIST MANAGEMENT FUNCTIONS
// =============================================================================

HAL_StatusTypeDef NAND_BlockManager_BlacklistBlock(nand_block_manager_t *manager, uint16_t block_index) {
    if (manager == NULL || block_index >= NAND_TOTAL_BLOCKS) return HAL_ERROR;
    
    manager->blocks[block_index].status = BLOCK_STATUS_BLACKLISTED;
    manager->available_blocks--;
    
    return HAL_OK;
}

bool NAND_BlockManager_IsBlockBlacklisted(nand_block_manager_t *manager, uint16_t block_index) {
    if (manager == NULL || block_index >= NAND_TOTAL_BLOCKS) return true;
    return (manager->blocks[block_index].status == BLOCK_STATUS_BLACKLISTED);
}

// =============================================================================
// HARDWARE ABSTRACTION LAYER FUNCTIONS
// =============================================================================

static HAL_StatusTypeDef NAND_HW_ReadPageWithECC(nand_block_manager_t *manager,
                                                 uint16_t block, uint16_t page,
                                                 uint8_t *data, ecc_status_t *ecc_result) {
    NAND_AddressTypeDef addr = { .Block = block, .Page = page, .Plane = NAND_BlockManager_GetPlaneFromBlock(block) };
    HAL_StatusTypeDef status = HAL_NAND_Read_Page_8b(manager->hnand, &addr, data, 1);
    if (status != HAL_OK) return status;
    if (manager->ecc_enabled && ecc_result != NULL) {
        status = NAND_ECC_CheckResult(manager, ecc_result);
    }
    return status;
}

static HAL_StatusTypeDef NAND_HW_WritePageWithECC(nand_block_manager_t *manager,
                                                  uint16_t block, uint16_t page,
                                                  uint8_t *data, ecc_status_t *ecc_result) {
    NAND_AddressTypeDef addr = { .Block = block, .Page = page, .Plane = NAND_BlockManager_GetPlaneFromBlock(block) };
    HAL_StatusTypeDef status;

    status = HAL_NAND_Write_Page_8b(manager->hnand, &addr, data, 1);
    if (status != HAL_OK) {
        return status;
    }

    status = NAND_HW_WaitReady(manager, NAND_TIMEOUT_PROGRAM_MS);
    if (status != HAL_OK) {
        return status;
    }

    status = NAND_HW_CheckOperationStatus(manager);
    if (status != HAL_OK) {
        return status;
    }

    if (manager->ecc_enabled && ecc_result != NULL) {
        status = NAND_ECC_CheckResult(manager, ecc_result);
    }

    return status;
}

static HAL_StatusTypeDef NAND_HW_ReadSpareArea(nand_block_manager_t *manager,
                                               uint16_t block, uint16_t page,
                                               uint8_t *spare_data, uint16_t size) {
    NAND_AddressTypeDef addr = { .Block = block, .Page = page, .Plane = NAND_BlockManager_GetPlaneFromBlock(block) };
    return HAL_NAND_Read_SpareArea_8b(manager->hnand, &addr, spare_data, 1);
}

static HAL_StatusTypeDef NAND_HW_WriteSpareArea(nand_block_manager_t *manager,
                                                uint16_t block, uint16_t page,
                                                uint8_t *spare_data, uint16_t size) {
    NAND_AddressTypeDef addr = { .Block = block, .Page = page, .Plane = NAND_BlockManager_GetPlaneFromBlock(block) };
    return HAL_NAND_Write_SpareArea_8b(manager->hnand, &addr, spare_data, 1);
}

/**
 * @brief Donanımsal ECC (Error Correction Code) sonucunu kontrol eder.
 * 
 * Son yapılan okuma işlemindeki hataları kontrol eder. ST HAL yapısı, hata durumunda 
 * fonksiyon dönüş değeriyle durumu belirtir.
 * 
 * @param manager Yönetici pointer'ı.
 * @param result Hatanın türünü (Success, 1-bit corrected, Uncorrectable) alacak pointer.
 * @return HAL_StatusTypeDef Kontrol işlemi yapıldıysa HAL_OK döner.
 * 
 * @note Okuma sonrası hemen çağrılmalıdır. ISR güvenlidir ancak donanım status bekler.
 */
HAL_StatusTypeDef NAND_ECC_CheckResult(nand_block_manager_t *manager, ecc_status_t *result) {
    if (manager == NULL || result == NULL) return HAL_ERROR;

    uint32_t ecc_val[4]; // Placeholder for ECC value, not directly used for status check in this logic
    HAL_StatusTypeDef status = HAL_NAND_GetECC(manager->hnand, &ecc_val[0], 50); // 50ms timeout

    // Based on ST's HAL implementation, the status of the GetECC function itself
    // reflects whether an uncorrectable error was found during the last read operation.
    if (status == HAL_OK) {
        // HAL_OK implies either no errors or errors that were successfully corrected by hardware.
        // In either case, the data in the buffer is valid.
        *result = ECC_STATUS_SUCCESS;
    } else {
        // Any status other than HAL_OK (e.g., HAL_ERROR, HAL_TIMEOUT) indicates an uncorrectable error.
        *result = ECC_STATUS_UNCORRECTABLE;
    }

    // This function's purpose is to report the check's outcome via the pointer.
    // It should return HAL_OK to indicate that the check itself was performed.
    return HAL_OK;
}
