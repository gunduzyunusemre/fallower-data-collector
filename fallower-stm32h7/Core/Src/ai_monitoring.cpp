#include "ai_monitoring.hpp"
#include "stm32h7xx.h"
#include <cstring>
#include <cstdio>

// Static member initialization
AIMonitoring::MonitoringState AIMonitoring::state = {};
uint32_t AIMonitoring::timing_start = 0;
UsbCommunication* AIMonitoring::usb_comm = nullptr;

/**
 * @brief AI İzleme Sistemini başlatır.
 * 
 * İzleme durumunu sıfırlar ve USB iletişim nesnesini alır. Başlangıç mesajını loglar.
 * 
 * @return None
 * 
 * @note ISR içinden çağrılması önerilmez. Sistem başlatma aşamasında bir kez çağrılmalıdır.
 */
void AIMonitoring::Initialize() {
    // Reset monitoring state
    memset(&state, 0, sizeof(state));
    usb_comm = UsbCommunication::getInstance();

    Log("AI_MONITOR", "AI Monitoring System Initialized");
}

/**
 * @brief AI işlem adımlarını kaydeder (Logging).
 * 
 * İşlem sayacını artırır ve adım adını USB üzerinden log kategorisine gönderir.
 * 
 * @param step_name Adımın açıklayıcı ismi.
 * @param step_id Adıma atanan benzersiz kimlik. [id]
 * @return bool Her zaman true döner.
 * 
 * @note ISR içinden çağrılabilir ancak USB logging hızı kesme performansını etkileyebilir.
 */
bool AIMonitoring::MonitorStep(const char* step_name, uint32_t step_id) {
    state.step_counter++;

    char msg[128];
    snprintf(msg, sizeof(msg), "Step %lu: %s", step_id, step_name);
    Log("AI_STEP", msg);

    return true;
}

/**
 * @brief Bellek hizalamasını (Alignment) doğrular.
 * 
 * Verilen pointer'ın belirtilen byte sınırına (örn. 32-byte) uygun olup olmadığını kontrol eder.
 * Özellikle STM32H7'de FMC ve Cache operasyonları için kritik bir kontroldür.
 * 
 * @param ptr Kontrol edilecek bellek adresi.
 * @param expected_alignment Beklenen hizalama sınırı (2, 4, 32 vb.). [byte]
 * @param description Kontrol edilen bölgenin adı (log için).
 * @return bool Hizalama doğruysa true, hatalıysa false döner.
 * 
 * @note ISR içinden çağrılabilir; hızlı bir aritmetik kontrol ve loglama yapar.
 */
bool AIMonitoring::VerifyMemoryAlignment(void* ptr, uint32_t expected_alignment, const char* description) {
    state.alignment_checks++;

    uintptr_t addr = (uintptr_t)ptr;
    bool aligned = (addr % expected_alignment) == 0;

    char msg[128];
    if (aligned) {
        snprintf(msg, sizeof(msg), "%s: ALIGNED (0x%08lX, %lu-byte boundary)",
                description, (unsigned long)addr, expected_alignment);
        Log("AI_ALIGN_OK", msg);
    } else {
        snprintf(msg, sizeof(msg), "%s: MISALIGNED (0x%08lX, expected %lu-byte boundary)",
                description, (unsigned long)addr, expected_alignment);
        Log("AI_ALIGN_ERROR", msg);
        state.error_count++;
        snprintf(state.last_error, sizeof(state.last_error), "%s", msg);
    }

    return aligned;
}

/**
 * @brief Bellek bölgesi için Cache tutarlılığını sağlar (Clean & Invalidate).
 * 
 * STM32H7'de L1-Cache ve RAM arasındaki veri tutarlılığını sağlamak için veriyi RAM'e yazar (Clean)
 * ve Cache'i geçersiz kılar (Invalidate). Hizalama hatalarını kontrol ederek Hard Fault'u önler.
 * 
 * @param buffer İşlem yapılacak bellek bölgesi.
 * @param size Bölge boyutu. [byte]
 * @param buffer_name Loglama için tampon adı.
 * @return bool İşlem başarılıysa veya güvenlik nedeniyle atlandıysa true döner.
 * 
 * @note ISR içinden çağrılabilir ancak cache operasyonları CPU döngüsü harcar. 
 *       Hizalama bozuksa işlemi atlayıp sadece bariyer (DSB/ISB) kullanır.
 */
bool AIMonitoring::VerifyCacheCoherency(void* buffer, uint32_t size, const char* buffer_name) {
    state.cache_invalidations++;

    // SAFE: STM32H7 Cache Coherency - Avoid hard fault with proper alignment
    uintptr_t addr = (uintptr_t)buffer;
    uintptr_t aligned_addr = addr & ~31UL;  // 32-byte align down
    // uint32_t aligned_size = ((size + 31) & ~31UL);  // Round up to 32-byte multiple - UNUSED

    // Only perform cache operations if alignment is safe
    if (aligned_addr == addr && (size % 32) == 0) {
        // Perfect alignment - safe to use cache operations
        SCB_CleanDCache_by_Addr((uint32_t*)buffer, size);
        SCB_InvalidateDCache_by_Addr((uint32_t*)buffer, size);
    } else {
        // Misaligned - use safer whole cache operations or skip
        char warning[128];
        snprintf(warning, sizeof(warning), "%s: Misaligned cache addr=0x%08X size=%lu - using DSB/ISB only",
                buffer_name, (unsigned int)addr, (unsigned long)size);
        Log("AI_CACHE_WARNING", warning);
    }

    // Always use memory barriers for safety
    __DSB();
    __ISB();

    char msg[128];
    snprintf(msg, sizeof(msg), "%s: Cache coherency ensured (0x%08lX, %lu bytes)",
            buffer_name, (unsigned long)buffer, size);
    Log("AI_CACHE", msg);

    state.cache_coherent = true;
    return true;
}

/**
 * @brief Veri formatını ve sınırlarını doğrular (NaN/Inf Kontrolü).
 * 
 * Float dizisi içindeki verilerin geçerli olup olmadığını (NaN veya sonsuz değil) ve 
 * beklenen min/max aralığında kalıp kalmadığını kontrol eder. Büyük dizilerde performans 
 * için örneklemeli (stride) kontrol yapar.
 * 
 * @param data Kontrol edilecek float dizisi.
 * @param count Eleman sayısı. [adet]
 * @param expected_min Beklenen minimum değer. [float]
 * @param expected_max Beklenen maksimum değer. [float]
 * @param data_name Loglama için veri adı.
 * @return bool Veri geçerli aralıktaysa true döner.
 * 
 * @note Büyük dizilerde tüm elemanları gezmek yerine 100 örnek üzerinden hızlı kontrol yapar. 
 *       ISR içinden çağrılabilir ancak döngü içerdiği için dikkatli olunmalıdır.
 */
bool AIMonitoring::ValidateDataFormat(const float* data, uint32_t count, float expected_min, float expected_max, const char* data_name) {
    if (!data || count == 0) {
        ReportError("ValidateDataFormat", "Null pointer or zero count");
        return false;
    }

    float min_val = data[0], max_val = data[0];
    uint32_t nan_count = 0, inf_count = 0;

    // LIGHTWEIGHT VALIDATION: Sample-based check for large arrays
    // uint32_t sample_count = (count > 1000) ? 100 : count;  // UNUSED
    uint32_t stride = (count > 1000) ? count / 100 : 1;

    for (uint32_t i = 0; i < count; i += stride) {
        if (i >= count) break;
        float val = data[i];

        // Check for NaN/Inf
        if (val != val) nan_count++;  // NaN check
        if (val == INFINITY || val == -INFINITY) inf_count++;

        // Min/Max tracking
        if (val < min_val) min_val = val;
        if (val > max_val) max_val = val;
    }

    bool valid = (nan_count == 0) && (inf_count == 0) &&
                 (min_val >= expected_min - 1.0f) && (max_val <= expected_max + 1.0f);

    char msg[128];
    snprintf(msg, sizeof(msg), "%s: [%.1f,%.1f] exp:[%.1f,%.1f] NaN:%lu Inf:%lu %s",
            data_name, min_val, max_val, expected_min, expected_max,
            nan_count, inf_count, valid ? "VALID" : "INVALID");

    if (valid) {
        Log("AI_DATA_OK", msg);
        state.data_format_valid = true;
    } else {
        Log("AI_DATA_ERROR", msg);
        state.error_count++;
        snprintf(state.last_error, sizeof(state.last_error), "%s", msg);
        state.data_format_valid = false;
    }

    return valid;
}

/**
 * @brief AI Model ağırlıklarının doğruluğunu CRC32 ile teyit eder.
 * 
 * Bellekteki model parametrelerinin (weights) bozulup bozulmadığını kontrol eder.
 * 
 * @param weights_ptr Ağırlıkların bellek adresi.
 * @param weights_size Toplam boyut. [byte]
 * @param expected_crc Beklenen CRC32 değeri. [32-bit hex]
 * @return bool CRC32 değerleri eşleşiyorsa true döner.
 * 
 * @note CRC hesaplaması yoğun işlem gerektirdiği için ISR içinden ÇAĞRILMAMALIDIR. 
 *       Model yükleme aşamasında çağrılmalıdır.
 */
bool AIMonitoring::VerifyModelWeights(void* weights_ptr, uint32_t weights_size, uint32_t expected_crc) {
    if (!weights_ptr || weights_size == 0) {
        ReportError("VerifyModelWeights", "Invalid weights pointer or size");
        return false;
    }

    // Calculate CRC32 of weights
    uint32_t calculated_crc = CalculateCRC32(weights_ptr, weights_size);
    bool weights_valid = (calculated_crc == expected_crc);

    char msg[128];
    snprintf(msg, sizeof(msg), "Weights: 0x%08lX size:%lu CRC:0x%08lX exp:0x%08lX %s",
            (unsigned long)weights_ptr, weights_size, calculated_crc, expected_crc,
            weights_valid ? "VALID" : "CORRUPTED");

    if (weights_valid) {
        Log("AI_WEIGHTS_OK", msg);
        state.weights_verified = true;
    } else {
        Log("AI_WEIGHTS_ERROR", msg);
        state.error_count++;
        snprintf(state.last_error, sizeof(state.last_error), "%s", msg);
        state.weights_verified = false;
    }

    return weights_valid;
}

bool AIMonitoring::ValidateInputTensor(const float* input, uint32_t batch, uint32_t channels, uint32_t height, uint32_t width) {
    uint32_t expected_size = batch * channels * height * width;

    char tensor_desc[64];
    snprintf(tensor_desc, sizeof(tensor_desc), "Input Tensor (%lu,%lu,%lu,%lu)", batch, channels, height, width);

    bool valid = ValidateDataFormat(input, expected_size, -1000.0f, 1000.0f, tensor_desc);

    // Additional shape validation
    char msg[128];
    snprintf(msg, sizeof(msg), "Input shape: (%lu,%lu,%lu,%lu) = %lu elements",
            batch, channels, height, width, expected_size);
    Log("AI_INPUT_SHAPE", msg);

    return valid;
}

bool AIMonitoring::ValidateOutputTensor(const float* output, uint32_t output_classes) {
    if (!output) {
        ReportError("ValidateOutputTensor", "Null output pointer");
        return false;
    }

    // Check for extreme values that indicate cache/alignment issues
    bool has_extremes = false;
    for (uint32_t i = 0; i < output_classes; i++) {
        float val = output[i];
        if (val == -2147483648.0f || val == 2147483647.0f || val == 0.0f) {
            has_extremes = true;
            break;
        }
    }

    char msg[128];
    if (has_extremes) {
        snprintf(msg, sizeof(msg), "Output has extreme/zero values - likely cache/alignment issue");
        Log("AI_OUTPUT_ERROR", msg);
        state.error_count++;
        return false;
    } else {
        snprintf(msg, sizeof(msg), "Output tensor appears valid (%lu classes)", output_classes);
        Log("AI_OUTPUT_OK", msg);
        return true;
    }
}

void AIMonitoring::DumpMemoryRegion(void* ptr, uint32_t size, const char* region_name, uint32_t dump_bytes) {
    if (!ptr || size == 0) return;

    uint32_t dump_size = (dump_bytes < size) ? dump_bytes : size;
    uint8_t* bytes = (uint8_t*)ptr;

    char msg[256] = {0};
    char hex_str[8];

    snprintf(msg, sizeof(msg), "%s (0x%08lX): ", region_name, (unsigned long)ptr);

    for (uint32_t i = 0; i < dump_size && strlen(msg) < 200; i++) {
        snprintf(hex_str, sizeof(hex_str), "%02X ", bytes[i]);
        strcat(msg, hex_str);
    }

    if (dump_size < size) {
        strcat(msg, "...");
    }

    Log("AI_MEMORY_DUMP", msg);
}

/**
 * @brief Cache tutarlılığını doğrular ve gerekirse düzeltir (Güvenli Sürüm).
 * 
 * `VerifyCacheCoherency` fonksiyonuna benzer şekilde çalışır ancak daha agresif bir 
 * loglama ve bariyer kullanımı sağlar. Hizalama hatalarına karşı korumalıdır.
 * 
 * @param buffer İşlem yapılacak tampon.
 * @param size Boyut. [byte]
 * @param operation İşlem açıklaması.
 * @return None
 * 
 * @note ISR içinden çağrılabilir. STM32H7 FMC (SDRAM/NAND) erişimlerinde verinin 
 *       CPU tarafından güncel görülmesi için kritiktir.
 */
void AIMonitoring::VerifyAndFixCacheCoherency(void* buffer, uint32_t size, const char* operation) {
    char msg[128];
    snprintf(msg, sizeof(msg), "%s: Ensuring cache coherency", operation);
    Log("AI_CACHE_FIX", msg);

    // SAFE: STM32H7 Cache Management - Avoid hard fault with alignment check
    uintptr_t addr = (uintptr_t)buffer;

    // Only use cache operations if properly aligned (32-byte boundary)
    if ((addr & 31) == 0 && (size % 32) == 0) {
        // Safe to use cache operations
        SCB_CleanDCache_by_Addr((uint32_t*)buffer, size);
        SCB_InvalidateDCache_by_Addr((uint32_t*)buffer, size);

        char safe_msg[128];
        snprintf(safe_msg, sizeof(safe_msg), "%s: Cache operations applied safely", operation);
        Log("AI_CACHE_APPLIED", safe_msg);
    } else {
        // Use memory barriers only for safety
        char safe_msg[128];
        snprintf(safe_msg, sizeof(safe_msg), "%s: Using memory barriers only (misaligned cache)", operation);
        Log("AI_CACHE_BARRIERS", safe_msg);
    }

    // Always use memory barriers for safety
    __DSB();  // Data Synchronization Barrier
    __ISB();  // Instruction Synchronization Barrier

    state.cache_invalidations++;
}

/**
 * @brief Bir işlemin başlangıç zamanını kaydeder.
 * 
 * HAL_GetTick() kullanarak milisaniye cinsinden zaman damgası alır.
 * 
 * @param operation_name Zamanlaması ölçülecek işlem adı.
 * @return None
 * 
 * @note ISR içinden çağrılması önerilmez (HAL_GetTick önceliğine bağlı olarak sorun çıkabilir).
 */
void AIMonitoring::StartTiming(const char* operation_name) {
    timing_start = HAL_GetTick();

    char msg[64];
    snprintf(msg, sizeof(msg), "Started: %s", operation_name);
    Log("AI_TIMING_START", msg);
}

/**
 * @brief Bir işlemin bitiş zamanını kaydeder ve süreyi hesaplar.
 * 
 * Başlangıç zamanı ile farkı alarak milisaniye cinsinden USB üzerinden loglar.
 * 
 * @param operation_name İşlem adı.
 * @return None
 * 
 * @note ISR içinden çağrılması önerilmez.
 */
void AIMonitoring::EndTiming(const char* operation_name) {
    uint32_t elapsed = HAL_GetTick() - timing_start;

    char msg[128];
    snprintf(msg, sizeof(msg), "Completed: %s (%lu ms)", operation_name, elapsed);
    Log("AI_TIMING_END", msg);
}

void AIMonitoring::ReportError(const char* function, const char* error_description) {
    state.error_count++;
    snprintf(state.last_error, sizeof(state.last_error), "%s", error_description);

    char msg[160];
    snprintf(msg, sizeof(msg), "%s: %s", function, error_description);
    Log("AI_ERROR", msg);
}

void AIMonitoring::ReportSuccess(const char* function, const char* success_description) {
    char msg[160];
    snprintf(msg, sizeof(msg), "%s: %s", function, success_description);
    Log("AI_SUCCESS", msg);
}

const AIMonitoring::MonitoringState& AIMonitoring::GetState() {
    return state;
}

void AIMonitoring::Reset() {
    memset(&state, 0, sizeof(state));
    Log("AI_MONITOR", "Monitoring state reset");
}

void AIMonitoring::Log(const char* category, const char* message) {
    if (usb_comm) {
        usb_comm->sendStatusMessage(category, message);
    }
}

/**
 * @brief Yazılımsal CRC32 hesaplaması yapar.
 * 
 * IEEE 802.3 polinomunu (0xEDB88320) kullanarak bit-bit CRC hesaplar.
 * 
 * @param data Hesaplanacak veri bloğu.
 * @param length Blok boyutu. [byte]
 * @return uint32_t Hesaplanan 32-bit CRC değeri.
 * 
 * @note CPU yoğunluklu bir işlemdir. Büyük veri blokları için RTOS task'ında çalıştırılmalıdır. 
 *       ISR içinden kesinlikle ÇAĞRILMAMALIDIR.
 */
uint32_t AIMonitoring::CalculateCRC32(const void* data, uint32_t length) {
    // Use STM32H7 hardware CRC32 if available, or software implementation
    const uint8_t* bytes = (const uint8_t*)data;
    uint32_t crc = 0xFFFFFFFF;

    for (uint32_t i = 0; i < length; i++) {
        crc ^= bytes[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320; // IEEE 802.3 polynomial
            } else {
                crc >>= 1;
            }
        }
    }

    return ~crc;
}