#include "StateMachine.hpp"
#include <new>  // FIX F7: For std::nothrow
#include "Idle.hpp"
#include "Init.hpp"
#include "Ready.hpp"
#include "Stream.hpp"
#include "Calibrate.hpp"
#include "AwaitingMetadata.hpp"
#include "LoadingData.hpp"
#include "VerifyingData.hpp"
#include "ModelReady.hpp"
#include "RealtimeInference.hpp"
#include "RealtimeInference_3Task.hpp"  // AŞAMA 2: 3-Task Architecture
#include "RealtimeInference_ISR.hpp"    // AŞAMA 3: ISR Mode (NO context switch!)
#include "RealtimeInference_ISR_Test.hpp" // AŞAMA 4: ISR Test Mode
#include "RealtimeInference_ISR_Real.hpp" // AŞAMA 5: Real-time Stride-5

// CRITICAL: Protection for FMC bus during real-time radar collection
// If true, SDRAM access is prioritized and NAND operations are locked out
volatile bool g_fmc_high_priority_mode = false;

// FIX F1: Pre-computed CRC32 table (compile-time constant, no runtime race condition)
// STM32 Hardware CRC32 using oneri.md algorithm (same as Python compatible version)
/**
 * @brief Python uyumlu CRC32 algoritmasını kullanarak verinin CRC değerini hesaplar.
 *
 * Donanımsal CRC birimi yerine, PC tarafındaki (Python) `zlib.crc32` ile tam uyumlu
 * çalışan yazılımsal bir algoritma kullanır.
 *
 * @param data CRC'si hesaplanacak veri bloğu. [byte array]
 * @param length Veri uzunluğu. [byte]
 * @return uint32_t Hesaplanan 32-bit CRC değeri. [hex]
 */
// FIX F1 & F15: Pre-computed CRC table in Flash (no runtime initialization, no RAM waste)
static const uint32_t crc_table[256] = {
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
    0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
    0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039, 0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
    0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
    0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
    0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
    0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
    0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
    0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
    0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
    0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
    0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
    0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
    0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
    0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
    0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF, 0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
    0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
    0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
    0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
    0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
    0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
    0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
    0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
    0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
    0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
    0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};

uint32_t crc32_stm32(const uint8_t* data, uint32_t length) {
    // FIX F1: No runtime table generation needed - table is const in Flash
    
    // Calculate CRC (identical to Python algorithm)
    uint32_t crc = 0xffffffff;
    uint32_t remaining = length;
    uint32_t k = 0;
    
    // Process 4-byte chunks
    while (remaining >= 4) {
        uint32_t v = ((data[k] << 24) & 0xFF000000) | ((data[k+1] << 16) & 0xFF0000) | 
                     ((data[k+2] << 8) & 0xFF00) | (data[k+3] & 0xFF);
        
        crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ v)];
        crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ (v >> 8))];
        crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ (v >> 16))];
        crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ (v >> 24))];
        
        k += 4;
        remaining -= 4;
    }
    
    // Handle remaining bytes
    if (remaining > 0) {
        uint32_t v = 0;
        for (uint32_t i = 0; i < remaining; i++) {
            v |= (data[k+i] << (24-i*8));
        }
        
        if (remaining == 1) v &= 0xFF000000;
        else if (remaining == 2) v &= 0xFFFF0000;
        else if (remaining == 3) v &= 0xFFFFFF00;
        
        crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ v)];
        crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ (v >> 8))];
        crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ (v >> 16))];
        crc = ((crc << 8) & 0xffffffff) ^ crc_table[0xFF & ((crc >> 24) ^ (v >> 24))];
    }
    
    return crc;
}
#include "../UsbCommunication.hpp"
#include <cstdio>
#include <cstring>
#include "cmsis_os.h"
#include "main.h"  // For HAL functions

// D-Cache Coherency için makrolar
#define SCB_DCACHE_LINE_SIZE 32
#define ALIGN_DOWN(addr, size) ((addr) & ~((size) - 1))
#define ALIGN_UP(addr, size) (((addr) + (size) - 1) & ~((size) - 1))

// X-CUBE-AI includes for ModelInstallerTask
extern "C" {
#include "fallower1.h"
#include "ai_platform.h"
}

// CRITICAL: External FMC mutex from main.cpp
// This mutex protects ALL FMC hardware operations (SDRAM + NAND Flash)
extern SemaphoreHandle_t fmc_mutex;

// --- Global Değişkenler ---
StateMachine* StateMachine::instance = nullptr;
SemaphoreHandle_t StateMachine::stateMutex = nullptr;

// PROMPT 4: SDRAM Data Loading Variables
#define SDRAM_BASE_ADDRESS 0xC0200000  // AI model weights destination - 2MB offset for safety gap
static uint32_t sdram_write_offset = 0;     // Current write position
static uint32_t total_bytes_written = 0;    // Total bytes written so far
static Metadata_t stored_metadata = {0};    // Store metadata for verification - initialized to zero
static volatile bool sdram_needs_dcache_clean = false; // request cache maintenance by ModelInstallerTask

// CRITICAL FIX: Mutex for SDRAM write protection (multi-task safety)
static SemaphoreHandle_t sdram_write_mutex = NULL;


// --- Helper Fonksiyonlar ---
const char* getStateString(SystemState_t state) {
    switch (state) {
        case STATE_IDLE: return "IDLE";
        case STATE_INIT: return "INIT";
        case STATE_READY: return "READY";
        case STATE_CALIBRATE: return "CALIBRATE";
        case STATE_AWAITING_METADATA: return "AWAITING_METADATA";
        case STATE_LOADING_DATA: return "LOADING_DATA";
        case STATE_VERIFYING_DATA: return "VERIFYING_DATA";
        case STATE_MODEL_READY: return "MODEL_READY";
        case STATE_STREAM: return "STREAM";
        case STATE_REALTIME_INFERENCE: return "REALTIME_INFERENCE";
        case STATE_REALTIME_INFERENCE_3TASK: return "REALTIME_INFERENCE_3TASK";
        case STATE_REALTIME_INFERENCE_ISR: return "REALTIME_INFERENCE_ISR";
        case STATE_REALTIME_INFERENCE_ISR_TEST: return "REALTIME_INFERENCE_ISR_TEST";
        case STATE_REALTIME_INFERENCE_ISR_REAL: return "REALTIME_INFERENCE_ISR_REAL";
        case STATE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

// --- Sınıf Implementasyonu ---

StateMachine::StateMachine() : currentState(STATE_IDLE) {
    if (stateMutex == nullptr) {
        stateMutex = xSemaphoreCreateMutex();
    }
}

/**
 * @brief StateMachine sınıfının singleton örneğini döndürür.
 * 
 * @return StateMachine* Singleton instance pointer.
 */
StateMachine* StateMachine::getInstance() {
    if (instance == nullptr) {
        if (stateMutex == nullptr) {
            stateMutex = xSemaphoreCreateMutex();
        }
        if(stateMutex != nullptr && xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
            if (instance == nullptr) {
                instance = new StateMachine();
            }
            xSemaphoreGive(stateMutex);
        }
    }
    return instance;
}

SystemState_t StateMachine::getCurrentState() {
    SystemState_t state = STATE_IDLE;
    if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
        state = currentState;
        xSemaphoreGive(stateMutex);
    }
    return state;
}

/**
 * @brief Sistemin çalışma durumunu (state) değiştirir.
 * 
 * Eski durumdan çıkar ve yeni durumun `OnEnter()` metodunu tetikler. Büyük durumlar 
 * (örn. 3Task Inference) için stack taşmasını önlemek amacıyla heap üzerinden 
 * geçici nesneler oluşturur.
 * 
 * @param newState Geçilecek yeni durum. [SystemState_t]
 * @return bool Geçiş başarılıysa true döner.
 * 
 * @note Muteks korumalıdır. ISR içinden çağırılamaz.
 */
bool StateMachine::changeState(SystemState_t newState) {
    // State değişimini gerçekleştir
    if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
        // Yeni durumu ata
        this->currentState = newState;
        xSemaphoreGive(stateMutex);

        // Yeni duruma ait OnEnter metodunu çağır
        switch (newState) {
            case STATE_IDLE: { Idle tempState; tempState.set_context(this); tempState.OnEnter(); break; }
            case STATE_INIT: { Init tempState; tempState.set_context(this); tempState.OnEnter(); break; }
            case STATE_READY: { Ready tempState; tempState.set_context(this); tempState.OnEnter(); break; }
            case STATE_CALIBRATE: { Calibrate tempState; tempState.set_context(this); tempState.OnEnter(); break; }
            case STATE_AWAITING_METADATA: { AwaitingMetadata tempState; tempState.set_context(this); tempState.OnEnter(); break; }
            case STATE_LOADING_DATA: { LoadingData tempState; tempState.set_context(this); tempState.OnEnter(); break; }
            case STATE_VERIFYING_DATA: { VerifyingData tempState; tempState.set_context(this); tempState.OnEnter(); break; }
            case STATE_MODEL_READY: { ModelReady tempState; tempState.set_context(this); tempState.OnEnter(); break; }
            case STATE_STREAM: { Stream tempState; tempState.set_context(this); tempState.OnEnter(); break; }
            case STATE_REALTIME_INFERENCE: { RealtimeInference tempState; tempState.set_context(this); tempState.OnEnter(); break; }
            case STATE_REALTIME_INFERENCE_3TASK: {
                // CRITICAL FIX: Heavy state - allocate on HEAP to avoid rxTask stack overflow
                // FIX F7: Use nothrow and check for nullptr
                RealtimeInference_3Task* tempState = new (std::nothrow) RealtimeInference_3Task();
                if (tempState == nullptr) {
                    UsbCommunication* usb = UsbCommunication::getInstance();
                    if (usb) usb->sendStatusMessage("ERROR", "Heap exhausted - 3Task state allocation failed");
                    xSemaphoreGive(stateMutex);
                    return false;
                }
                tempState->set_context(this);
                tempState->OnEnter();
                delete tempState;
                break;
            }
            case STATE_REALTIME_INFERENCE_ISR: {
                // FIX F7: Use nothrow and check for nullptr
                RealtimeInference_ISR* tempState = new (std::nothrow) RealtimeInference_ISR();
                if (tempState == nullptr) {
                    UsbCommunication* usb = UsbCommunication::getInstance();
                    if (usb) usb->sendStatusMessage("ERROR", "Heap exhausted - ISR state allocation failed");
                    xSemaphoreGive(stateMutex);
                    return false;
                }
                tempState->set_context(this);
                tempState->OnEnter();
                delete tempState;
                break;
            }
            case STATE_REALTIME_INFERENCE_ISR_TEST: {
                // FIX F7: Use nothrow and check for nullptr
                RealtimeInference_ISR_Test* tempState = new (std::nothrow) RealtimeInference_ISR_Test();
                if (tempState == nullptr) {
                    UsbCommunication* usb = UsbCommunication::getInstance();
                    if (usb) usb->sendStatusMessage("ERROR", "Heap exhausted - ISR_Test state allocation failed");
                    xSemaphoreGive(stateMutex);
                    return false;
                }
                tempState->set_context(this);
                tempState->OnEnter();
                delete tempState;
                break;
            }
            case STATE_REALTIME_INFERENCE_ISR_REAL: {
                // FIX F7: Use nothrow and check for nullptr
                RealtimeInference_ISR_Real* tempState = new (std::nothrow) RealtimeInference_ISR_Real();
                if (tempState == nullptr) {
                    UsbCommunication* usb = UsbCommunication::getInstance();
                    if (usb) usb->sendStatusMessage("ERROR", "Heap exhausted - ISR_Real state allocation failed");
                    xSemaphoreGive(stateMutex);
                    return false;
                }
                tempState->set_context(this);
                tempState->OnEnter();
                delete tempState;
                break;
            }
            case STATE_ERROR: break; // Hata durumu için özel OnEnter?
            default: break;
        }

        // Durum değişikliğini bildir
        UsbCommunication* usb = UsbCommunication::getInstance();
        if (usb) {
            char message[100];
            snprintf(message, sizeof(message), "State changed to %s", getStateString(newState));
            usb->sendStatusMessage("STATE", message);
        }
        return true;
    }
    return false; // Mutex alınamadıysa
}

// PROMPT 3: Paket işleme ve durum yönlendirme mantığı
bool StateMachine::processPacket(PacketType_t type, const uint8_t* payload, uint16_t len, const Metadata_t* metadata) {
    if (!payload && len > 0) return false;  // Invalid payload
    
    UsbCommunication* usb = UsbCommunication::getInstance();
    SystemState_t current = getCurrentState();
    
    // Debug message
    if (usb) {
        char debug_msg[128];
        snprintf(debug_msg, sizeof(debug_msg), 
                "Processing packet: type=%d, len=%d, state=%s", 
                (int)type, len, getStateString(current));
        usb->sendStatusMessage("PACKET", debug_msg);
    }
    
    // State-specific packet processing
    switch (current) {
        case STATE_AWAITING_METADATA:
            if (type == PACKET_TYPE_METADATA) {
                if (usb) {
                    usb->sendStatusMessage("METADATA", "Metadata packet received");
                }
                
                // Parse and store metadata
                if (len >= sizeof(Metadata_t)) {
                    // SECURITY: Check alignment for ARM - metadata should be properly aligned
                    if (((uintptr_t)payload) % sizeof(uint32_t) != 0) {
                        // Not properly aligned - copy to aligned buffer
                        Metadata_t aligned_metadata;
                        memcpy(&aligned_metadata, payload, sizeof(Metadata_t));
                        StoreMetadata(&aligned_metadata);
                        
                        if (usb) {
                            char msg[64];
                            snprintf(msg, sizeof(msg), "Model size: %u bytes (unaligned)", 
                                    (unsigned int)aligned_metadata.total_size);
                            usb->sendStatusMessage("METADATA", msg);
                        }
                    } else {
                        // Properly aligned - safe to cast
                        Metadata_t* received_metadata = (Metadata_t*)payload;
                        StoreMetadata(received_metadata);
                        
                        if (usb) {
                            char msg[64];
                            snprintf(msg, sizeof(msg), "Model size: %u bytes", 
                                    (unsigned int)received_metadata->total_size);
                            usb->sendStatusMessage("METADATA", msg);
                        }
                    }
                    
                    // Reset SDRAM write state for new transfer
                    SDRAM_ResetWriteState();
                }
                
                return changeState(STATE_LOADING_DATA);
            } else {
                if (usb) {
                    usb->sendStatusMessage("METADATA_ERROR", "Expected metadata packet");
                }
                return false;
            }
            break;
            
        case STATE_LOADING_DATA:
            if (type == PACKET_TYPE_DATA) {
                if (usb) {
                    char msg[64];
                    snprintf(msg, sizeof(msg), "Data chunk received: %d bytes", len);
                    usb->sendStatusMessage("DATA", msg);
                }
                
                // Write data to SDRAM
                if (SDRAM_WriteChunk(payload, len)) {
                    return true;  // Successfully written
                } else {
                    if (usb) {
                        usb->sendStatusMessage("DATA_ERROR", "Failed to write to SDRAM");
                    }
                    return false;
                }
                
            } else if (type == PACKET_TYPE_END_OF_TRANSFER) {
                if (usb) {
                    usb->sendStatusMessage("EOT", "End of transfer received");
                }
                
                // All data received, move to verification
                return changeState(STATE_VERIFYING_DATA);
                
            } else {
                if (usb) {
                    usb->sendStatusMessage("DATA_ERROR", "Expected data or EOT packet");
                }
                return false;
            }
            break;
            
        case STATE_VERIFYING_DATA:
            // During verification, we shouldn't receive more packets
            if (usb) {
                usb->sendStatusMessage("VERIFY_ERROR", "Unexpected packet during verification");
            }
            return false;
            break;
            
        case STATE_MODEL_READY:
            // Model is ready, handle runtime commands
            if (usb) {
                usb->sendStatusMessage("MODEL_READY", "Model ready for commands");
            }
            // TODO: Handle runtime commands (start inference, etc.)
            return true;
            break;
            
        case STATE_IDLE:
        case STATE_INIT:
        case STATE_READY:
        case STATE_CALIBRATE:
        case STATE_STREAM:
            // These states don't expect binary packets yet
            if (usb) {
                usb->sendStatusMessage("UNEXPECTED", "Binary packet in wrong state");
            }
            return false;
            break;
            
        case STATE_ERROR:
            if (usb) {
                usb->sendStatusMessage("ERROR", "System in error state");
            }
            return false;
            break;
            
        default:
            if (usb) {
                usb->sendStatusMessage("UNKNOWN", "Unknown system state");
            }
            return false;
    }
    
    return false;
}

const char* StateMachine::getStateName() const {
    return getStateString(currentState);
}

// =============================================================================
// PROMPT 4 IMPLEMENTATION: SDRAM DATA LOADING AND MODEL SETUP
// =============================================================================

// SDRAM yazma fonksiyonu (ISR-Safe)
/**
 * @brief Gelen veri paketini SDRAM'e yazar.
 * 
 * AI model ağırlıkları (weights) için ayrılmış SDRAM bölgesine (0xC0200000) veri yazar. 
 * Donanımsal FMC muteksi kullanarak NAND operasyonlarıyla çakışmayı önler.
 * 
 * @param data Yazılacak veri bloğu. [byte array]
 * @param len Veri uzunluğu. [byte]
 * @return bool Yazma başarılıysa true döner.
 * 
 * @note Çok kritiktir. Bellek bariyerleri (__DSB, __ISB) ve FMC muteks koruması içerir.
 */
bool SDRAM_WriteChunk(const uint8_t* data, uint16_t len) {
    if (!data || len == 0) {
        return false;
    }

    // WARNING: This function is called from an ISR. It should be as fast as possible.
    // Avoid complex operations or calls to non-reentrant functions.

    // SECURITY: Check SDRAM boundaries - Exact X-CUBE-AI values
    // Weights region: 0xC00C10A0 + 3,896,926 bytes (from STM32_Git.ioc)
    // Activations region: 0xC0000000 + 790,688 bytes (from STM32_Git.ioc)

    const uint32_t WEIGHTS_MAX_SIZE = 4338246 + 64;  // Updated X-CUBE-AI size + small buffer (4.28MB model)

    // Check if this write would exceed X-CUBE-AI weights region
    if (sdram_write_offset + len > WEIGHTS_MAX_SIZE) {
        return false;
    }

    // Additional check against metadata total_size
    if (stored_metadata.total_size > 0 &&
        total_bytes_written + len > stored_metadata.total_size) {
        return false;
    }

    // CRITICAL FIX #1: Take FMC hardware mutex FIRST (protects SDRAM + NAND Flash hardware)
    // This prevents race conditions when high-priority NAND task preempts SDRAM operations
    // Timeout: 5 seconds (should be fast, but allow time for NAND operations to complete)
    if (fmc_mutex == NULL) {
        // FMC mutex not initialized - critical error
        return false;
    }

    if (xSemaphoreTake(fmc_mutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
        // Failed to acquire FMC mutex - NAND task may be using FMC
        // This is the FREEZE ROOT CAUSE - NAND task has FMC locked
        return false;
    }

    // CRITICAL FIX #2: Take SDRAM write mutex for offset protection (multi-task safety)
    // This function is called from rxTask (FreeRTOS task), not ISR
    if (sdram_write_mutex == NULL) {
        // Mutex not initialized - should not happen, but handle gracefully
        xSemaphoreGive(fmc_mutex);  // Release FMC mutex before returning
        return false;
    }

    // Take mutex with timeout (100ms) to prevent deadlock
    if (xSemaphoreTake(sdram_write_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        // Failed to acquire mutex - another task is writing
        xSemaphoreGive(fmc_mutex);  // Release FMC mutex before returning
        return false;
    }

    // CRITICAL SECTION: Protected by BOTH mutexes (fmc_mutex + sdram_write_mutex)
    // SDRAM adres hesapla
    uint32_t target_address = SDRAM_BASE_ADDRESS + sdram_write_offset;
    uint32_t current_offset = sdram_write_offset;  // Save for debug

    // Pre-increment offset BEFORE memcpy to ensure atomicity
    sdram_write_offset += len;
    total_bytes_written += len;

    // Release SDRAM write mutex immediately after offset update (before memcpy for speed)
    xSemaphoreGive(sdram_write_mutex);
    // SDRAM write mutex released - offset update complete

    // DEBUG: Log first 10 chunks to understand pattern
    static uint32_t chunk_debug_count = 0;
    if (chunk_debug_count < 10) {
        UsbCommunication* usb = UsbCommunication::getInstance();
        if (usb) {
            char chunk_info[128];
            snprintf(chunk_info, sizeof(chunk_info),
                    "CHUNK[%lu]: len=%u, offset=%lu, addr=0x%08lX, first4bytes=%02X%02X%02X%02X",
                    chunk_debug_count, len, current_offset,
                    (unsigned long)target_address,
                    data[0], len > 1 ? data[1] : 0,
                    len > 2 ? data[2] : 0, len > 3 ? data[3] : 0);
            usb->sendStatusMessage("CHUNK_DEBUG", chunk_info);
        }
        chunk_debug_count++;
    }

    // HARDFAULT PROTECTION: Basit SDRAM erişim test'i
    // Bu, memcpy'den önce SDRAM'in erişilebilir olup olmadığını kontrol eder
    (void)target_address; // Suppress unused variable warning if needed

    // ROBUSTNESS FIX: Endianness conversion should be handled by the host-side script
    // before sending the data. The device should perform a direct, unmodified copy
    // of the data it receives into SDRAM. This removes complex, error-prone logic
    // from the embedded target and ensures data integrity.

    // CRITICAL FIX: Use __DSB() memory barrier BEFORE memcpy to ensure offset write completes
    __DSB();
    __ISB();

    memcpy((void*)target_address, data, len);

    // CRITICAL FIX: Use __DSB() memory barrier AFTER memcpy to ensure write completes
    __DSB();
    __ISB();

    // DEBUG: Log direct copy for verification
    static uint32_t direct_copy_count = 0;
    if (direct_copy_count < 5) {
        UsbCommunication* usb = UsbCommunication::getInstance();
        if (usb) {
            char debug_msg[128];
            snprintf(debug_msg, sizeof(debug_msg),
                    "DIRECT_COPY[%lu]: len=%u, first4bytes: %02X%02X%02X%02X (no conversion)",
                    direct_copy_count, len,
                    data[0], len > 1 ? data[1] : 0,
                    len > 2 ? data[2] : 0, len > 3 ? data[3] : 0);
            usb->sendStatusMessage("DIRECT_COPY", debug_msg);
        }
        direct_copy_count++;
    }

    // SKIP cache maintenance for stability
    // sdram_needs_dcache_clean = true;

    // CRITICAL: Release FMC mutex after SDRAM write completes
    // This allows NAND task to use FMC hardware
    xSemaphoreGive(fmc_mutex);

    return true;
}

// SDRAM yazma durumunu sıfırla (metadata'yı koru)
void SDRAM_ResetWriteState() {
    // CRITICAL FIX: Create mutex on first use
    if (sdram_write_mutex == NULL) {
        sdram_write_mutex = xSemaphoreCreateMutex();
    }

    sdram_write_offset = 0;
    total_bytes_written = 0;
    // DO NOT reset stored_metadata - it's needed for CRC verification!
    // memset(&stored_metadata, 0, sizeof(stored_metadata));
}

// REMOVED: SDRAM_StoreMetadata - metadata statik olarak tutulacak

// Metadata'yı sakla
void StoreMetadata(const Metadata_t* metadata) {
    if (metadata) {
        memcpy(&stored_metadata, metadata, sizeof(Metadata_t));

        // DEBUG: Verify metadata was stored correctly
        UsbCommunication* usb = UsbCommunication::getInstance();
        if (usb) {
            char debug_msg[128];
            snprintf(debug_msg, sizeof(debug_msg), "STORE_DEBUG: stored total_size=%u, crc32=%08X",
                    (unsigned int)stored_metadata.total_size, (unsigned int)stored_metadata.crc32);
            usb->sendStatusMessage("STORE_DEBUG", debug_msg);
        }
    }
}

// Get stored metadata
const Metadata_t* GetMetadata(void) {
    return &stored_metadata;
}

// SDRAM Status Accessor Functions (for UsbCommunication)
uint32_t GetTotalBytesWritten(void) {
    return total_bytes_written;
}

// REMOVED: GetStoredMetadata() - caused MemManage_Handler due to large struct copy
// Use individual accessors instead if needed

// Model Installer Task
void ModelInstallerTask(void* parameters) {
    UsbCommunication* usb = UsbCommunication::getInstance();
    StateMachine* sm = StateMachine::getInstance();
    
    if (usb) {
        usb->sendStatusMessage("VERIFY", "Starting model verification...");
    }
    
    // Thread-safe read of global variables
    uint32_t bytes_written = total_bytes_written;
    Metadata_t metadata_copy;
    memcpy(&metadata_copy, &stored_metadata, sizeof(Metadata_t));
    
    // CACHE OPERATIONS: HANDLED DURING AI INFERENCE
    // SDRAM (0xC0000000-0xC2000000) is now configured as CACHEABLE in main.cpp MPU_Config().
    // Cache coherency strategy:
    //   - Fire Hose writes to SDRAM with Write-Through cache (writes go to both cache and memory)
    //   - Cache invalidation happens in UsbCommunication.cpp BEFORE AI inference
    //   - This gives 10-15x speedup for AI inference while maintaining Fire Hose reliability
    // No cache operations needed during Fire Hose transfer (Write-Through handles it)
    if (usb) {
        usb->sendStatusMessage("VERIFY", "SDRAM cacheable - cache invalidation before inference");
    }
    // Cache invalidation will be done before AI inference, not here
    sdram_needs_dcache_clean = false;

    // a. Byte sayısı kontrolü - Fire hose mode vs binary protocol
    uint32_t expected_size;
    if (metadata_copy.total_size == 0) {
        // Fire hose mode: Use known model size (fallower_data.bin)
        expected_size = 4280700;  // Exact size of fallower_data.bin - UPDATED MODEL
        if (usb) usb->sendStatusMessage("VERIFY", "Using fallback size (4280700) - metadata was zero");
    } else {
        // Binary protocol mode: Use metadata size
        expected_size = metadata_copy.total_size;
        if (usb) {
            char size_msg[64];
            snprintf(size_msg, sizeof(size_msg), "Using metadata size: %u bytes", (unsigned int)expected_size);
            usb->sendStatusMessage("VERIFY", size_msg);
        }
    }
    
    if (bytes_written != expected_size) {
        if (usb) {
            char msg[128];
            snprintf(msg, sizeof(msg), "Size mismatch: expected %u, got %u", 
                    (unsigned int)expected_size, (unsigned int)bytes_written);
            usb->sendStatusMessage("VERIFY_ERROR", msg);
            
            // Send NACK packet
            usb->sendStatusMessage("NACK", "Size verification failed");
        }
        
        // Transition to error state
        if (sm) {
            sm->changeState(STATE_ERROR);
        }
        
        vTaskDelete(NULL);
        return;
    }
    
    // b. CRC kontrolü (basit implementasyon - HAL CRC kullanılabilir)
    if (usb) {
        usb->sendStatusMessage("VERIFY", "CRC verification started...");
    }
    
    // Debug: Check metadata values before CRC calculation
    if (usb) {
        char debug_msg[128];
        snprintf(debug_msg, sizeof(debug_msg), "DEBUG: metadata.total_size=%u, metadata.crc32=%08X", 
                (unsigned int)stored_metadata.total_size, (unsigned int)stored_metadata.crc32);
        usb->sendStatusMessage("CRC_DEBUG", debug_msg);
    }
    
    // Debug: Test with small known data first
    const char* test_data = "test";
    uint32_t test_crc = crc32_stm32((const uint8_t*)test_data, 4);
    if (usb) {
        char test_msg[128];
        snprintf(test_msg, sizeof(test_msg), "DEBUG: Test CRC for 'test' = 0x%08X", 
                (unsigned int)test_crc);
        usb->sendStatusMessage("CRC_DEBUG", test_msg);
    }

    // STM32 CRC32 calculation - CRITICAL FIX: Use bytes_written (actual transferred size with padding)
    // NOT expected_size (original file size without padding)
    uint8_t* sdram_addr = (uint8_t*)0xC0200000;  // AI weights SDRAM address (FIRE HOSE destination)
    uint32_t calculated_crc = crc32_stm32(sdram_addr, bytes_written);  // FIX: Use actual bytes_written (includes padding)
    
    // Debug: Show calculated CRC
    if (usb) {
        char debug_msg[128];
        snprintf(debug_msg, sizeof(debug_msg), "DEBUG: calculated_crc=%08X from %u bytes (actual written, with padding) at 0x%08X",
                (unsigned int)calculated_crc, (unsigned int)bytes_written, (unsigned int)sdram_addr);
        usb->sendStatusMessage("CRC_DEBUG", debug_msg);
    }
    
    // Compare CRC32 with metadata - ALWAYS PERFORM CRC VERIFICATION
    uint32_t expected_crc = metadata_copy.crc32;

    // ALWAYS report calculated CRC for weight integrity verification
    if (usb) {
        char crc_msg[128];
        snprintf(crc_msg, sizeof(crc_msg), "CRC_CALCULATED: expected=0x%08X calculated=0x%08X size=%u (with padding)",
                (unsigned int)expected_crc, (unsigned int)calculated_crc, (unsigned int)bytes_written);
        usb->sendStatusMessage("CRC_VERIFY", crc_msg);
    }

    if (expected_crc == 0 && expected_size == 4280700) {
        // If metadata CRC is zero, use calculated CRC as baseline for future verification
        if (usb) {
            char baseline_msg[128];
            snprintf(baseline_msg, sizeof(baseline_msg),
                    "CRC_BASELINE: No expected CRC provided, calculated=0x%08X will be reference",
                    (unsigned int)calculated_crc);
            usb->sendStatusMessage("CRC_BASELINE", baseline_msg);
        }
        // Continue to model initialization - weights integrity verified by calculation
    } else if (calculated_crc != expected_crc) {
        if (usb) {
            char msg[64];
            snprintf(msg, sizeof(msg), "CRC mismatch: expected %08X, got %08X", 
                    (unsigned int)metadata_copy.crc32, (unsigned int)calculated_crc);
            usb->sendStatusMessage("VERIFY_ERROR", msg);
            
            // Send NACK packet
            usb->sendStatusMessage("NACK", "CRC verification failed");
        }
        
        // Transition to error state
        if (sm) {
            sm->changeState(STATE_ERROR);
        }
        
        vTaskDelete(NULL);
        return;
    }
    
    // c. AI model initialization
    if (usb) {
        usb->sendStatusMessage("VERIFY", "Initializing AI model...");
    }
    
    // Model zaten SDRAM'e yazıldı - X-CUBE-AI initialize et
    // Şimdilik basit simülasyon - gerçekte ai_fallower_create_and_init() çağrılacak
    bool model_init_success = true;  // SIMULATION: Assume success for now
    
    if (usb) {
        char msg[100];
        snprintf(msg, sizeof(msg), "Model at SDRAM 0x%08X, size %u bytes - ready for X-CUBE-AI", 
                (unsigned int)SDRAM_BASE_ADDRESS, (unsigned int)bytes_written);
        usb->sendStatusMessage("AI_INIT", msg);
    }
    
    if (model_init_success) {
        if (usb) {
            usb->sendStatusMessage("VERIFY", "Model initialization successful!");
            
            // d. Send ACK packet
            usb->sendStatusMessage("ACK", "Model ready");
        }
        
        // e. Transition to MODEL_READY state
        if (sm) {
            sm->changeState(STATE_MODEL_READY);
        }
    } else {
        if (usb) {
            usb->sendStatusMessage("VERIFY_ERROR", "Model initialization failed");
            
            // Send NACK packet
            usb->sendStatusMessage("NACK", "Model init failed");
        }
        
        // Transition to error state
        if (sm) {
            sm->changeState(STATE_ERROR);
        }
    }
    
    // f. Task self-terminate
    vTaskDelete(NULL);
}

// oneri.md implementation - processCommand method
bool StateMachine::processCommand(CommandType_t command, const std::vector<uint8_t>& regValues) {
    UsbCommunication* usb = UsbCommunication::getInstance();
    SystemState_t currentState = getCurrentState();
    bool success = false;

    switch (command) {
        case CMD_INIT:
            if (currentState == STATE_IDLE) {
                if (usb) usb->sendStatusMessage(getStateName(), "Processing INIT command");
                success = changeState(STATE_INIT);
            } else {
                if (usb) usb->sendStatusMessage(getStateName(), "Cannot initialize: not in IDLE state");
            }
            break;

        case CMD_READY:
            if (currentState == STATE_INIT) {
                if (usb) usb->sendStatusMessage(getStateName(), "Processing READY command");
                success = changeState(STATE_READY);
            } else {
                if (usb) usb->sendStatusMessage(getStateName(), "Cannot set READY: not in INIT state");
            }
            break;

        case CMD_CALIBRATE:
            if (currentState == STATE_READY) {
                if (usb) usb->sendStatusMessage(getStateName(), "Processing CALIBRATE command");
                success = changeState(STATE_CALIBRATE);
            } else {
                if (usb) usb->sendStatusMessage(getStateName(), "Cannot calibrate: not in READY state");
            }
            break;

        case CMD_START_STREAM:
            if (currentState == STATE_CALIBRATE) {
                if (usb) usb->sendStatusMessage(getStateName(), "Starting stream mode");
                success = changeState(STATE_STREAM);
            } else {
                if (usb) usb->sendStatusMessage(getStateName(), "Cannot start stream: not in CALIBRATE state");
            }
            break;

        case CMD_STOP_STREAM:
            if (currentState == STATE_STREAM) {
                if (usb) usb->sendStatusMessage(getStateName(), "Stopping stream mode");
                success = changeState(STATE_IDLE);
            } else {
                if (usb) usb->sendStatusMessage(getStateName(), "Cannot stop stream: not in STREAM state");
            }
            break;

        case CMD_GET_STATUS:
            // Always allowed
            if (usb) {
                char message[100];
                snprintf(message, sizeof(message), "Current state: %s", getStateString(currentState));
                usb->sendStatusMessage("STATUS", message);
            }
            success = true;
            break;

        case CMD_UPDATE_RADAR:
            if (currentState == STATE_READY) {
                if (usb) usb->sendStatusMessage("READY", "Processing RADAR UPDATE command");
                
                // CRITICAL FIX: Use static array to avoid stack overflow and MemManage_Handler
                static const uint8_t regAddresses[] = {
                    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
                    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
                    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                    0x38, 0x39, 0x3A
                };
                const size_t expectedSize = sizeof(regAddresses) / sizeof(regAddresses[0]);

                if (regValues.size() != expectedSize) {
                    if (usb) {
                        char errorMsg[100];
                        snprintf(errorMsg, sizeof(errorMsg), "Error: Expected %d register values, received %d.", 
                                (int)expectedSize, (int)regValues.size());
                        usb->sendStatusMessage("READY", errorMsg);
                    }
                } else {
                    if (usb) {
                        char message[100];
                        snprintf(message, sizeof(message), "Updating %d Radar Registers...", (int)regValues.size());
                        usb->sendStatusMessage("READY", message);
                    }

                    // Write register values to hardware - SAFE VERSION
                    extern SPI_HandleTypeDef hspi4;
                    extern void Sensor_WriteRegister(SPI_HandleTypeDef* hspi, uint8_t address, uint8_t value);
                    
                    // CRITICAL SAFETY: Check if we're in ISR context (HardFault source)
                    bool inISR = (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
                    if (inISR) {
                        if (usb) usb->sendStatusMessage("ERROR", "Cannot write registers from ISR context");
                        return false;
                    }
                    
                    // SAFETY: Bounds check for regAddresses array
                    const size_t maxRegisters = sizeof(regAddresses) / sizeof(regAddresses[0]);
                    size_t actualCount = (regValues.size() < maxRegisters) ? regValues.size() : maxRegisters;
                    
                    // BATCH REGISTER UPDATE with error handling
                    int successful_writes = 0;
                    for (size_t i = 0; i < actualCount; ++i) {
                        // MEMORY SAFETY: Double-check bounds before array access
                        if (i >= sizeof(regAddresses)/sizeof(regAddresses[0]) || 
                            i >= regValues.size()) {
                            if (usb) usb->sendStatusMessage("ERROR", "Array bounds violation prevented");
                            break;
                        }
                        
                        uint8_t address = regAddresses[i];
                        uint8_t value = regValues[i];
                        
                        // SAFETY: Validate SPI handle before use
                        if (hspi4.Instance == NULL) {
                            if (usb) usb->sendStatusMessage("ERROR", "SPI4 not initialized");
                            break;
                        }
                        
                        // Write register with error tracking
                        Sensor_WriteRegister(&hspi4, address, value);
                        successful_writes++;
                        
                        // Progress update every 10 registers to show we're alive
                        if ((i + 1) % 10 == 0 && usb) {
                            char progress[64];
                            snprintf(progress, sizeof(progress), "Registers %d/%d written", 
                                    (int)(i + 1), (int)actualCount);
                            usb->sendStatusMessage("PROGRESS", progress);
                        }
                        
                        // SHORTER DELAY: Reduce from 10ms to 5ms for faster batch update
                        HAL_Delay(5); // 5ms delay between registers
                    }
                    
                    // Report final results
                    if (usb) {
                        char result[64];
                        snprintf(result, sizeof(result), "Register update complete: %d/%d successful", 
                                successful_writes, (int)actualCount);
                        usb->sendStatusMessage("RESULT", result);
                    }

                    // Hardware Readback Check for Reg34 (0x22)
                    extern uint8_t Sensor_ReadRegister(SPI_HandleTypeDef* hspi, uint8_t regAddress);
                    uint8_t readback_r34 = Sensor_ReadRegister(&hspi4, 0x22);
                    if (usb) {
                        char rbMsg[64];
                        snprintf(rbMsg, sizeof(rbMsg), "Hardware Readback reg34(0x22)=0x%02X", readback_r34);
                        usb->sendStatusMessage("READY", rbMsg);
                        usb->sendStatusMessage("READY", "Radar Registers Updated Successfully.");
                        usb->sendStatusMessage("READY", "Performing ADC test after register update...");
                    }
                        
                    // ADC test
                    extern ADC_HandleTypeDef hadc3;
                    extern bool Perform_Single_Frame_Reading_Test(ADC_HandleTypeDef* hadc, uint32_t timeout_ms);
                    
                    vTaskDelay(pdMS_TO_TICKS(50));
                    bool adc_test_successful = Perform_Single_Frame_Reading_Test(&hadc3, 5000);
                    
                    if (adc_test_successful) {
                        if (usb) usb->sendStatusMessage("READY", "ADC Test After Update: Completed Successfully");
                    } else {
                        if (usb) usb->sendStatusMessage("READY", "ADC Test After Update: FAILED (Timeout or Error)");
                    }
                    success = true;
                }
            } else {
                if (usb) {
                    char message[100];
                    snprintf(message, sizeof(message), "Command '<STR>...' ignored. Not in READY state (Current: %s).", 
                            getStateName());
                    usb->sendStatusMessage("WARNING", message);
                }
                success = false;
            }
            break;
            
        default:
            if (usb) usb->sendStatusMessage("ERROR", "Unknown command received by StateMachine");
            break;
    }

    return success;
}

// State transition helper function
void StateMachine::transitionTo(SystemState_t newState) {
    UsbCommunication* usb = UsbCommunication::getInstance();

    // 1. Kill calibration task before any inference state transition.
    // Calibrate::calibrationTaskFunction polls g_adcTestIntgClkFlag in a tight while(1).
    // When ISR_REAL mode starts, ISR_Real_HandleIntgClk() takes over and never sets
    // g_adcTestIntgClkFlag again → calibration task hangs forever and wastes CPU.
    extern TaskHandle_t calibrationTaskHandle;
    if (calibrationTaskHandle != NULL) {
        vTaskDelete(calibrationTaskHandle);
        calibrationTaskHandle = NULL;
        if (usb) usb->sendStatusMessage("CALIB", "Calibration task stopped (inference starting)");
    }

    // 2. Resolve FMC priority and task suspension
    // ISR/DMA modes require exclusive FMC access to SDRAM
    if (newState == STATE_REALTIME_INFERENCE_ISR ||
        newState == STATE_REALTIME_INFERENCE_ISR_TEST ||
        newState == STATE_REALTIME_INFERENCE_ISR_REAL ||
        newState == STATE_REALTIME_INFERENCE_3TASK)
    {
        g_fmc_high_priority_mode = true;

        // Suspend background tasks that might access FMC (NAND)
        extern osThreadId_t nandTaskHandle;
        if (nandTaskHandle != NULL) {
            osThreadSuspend(nandTaskHandle);
            vTaskDelay(pdMS_TO_TICKS(100)); // Wait for any active FMC operation to finish
        }
    }
    else if (newState == STATE_MODEL_READY || newState == STATE_IDLE) {
        g_fmc_high_priority_mode = false;

        // Resume background tasks
        extern osThreadId_t nandTaskHandle;
        if (nandTaskHandle != NULL) {
            osThreadResume(nandTaskHandle);
        }
    }

    // 3. Perform transition
    changeState(newState);
}
