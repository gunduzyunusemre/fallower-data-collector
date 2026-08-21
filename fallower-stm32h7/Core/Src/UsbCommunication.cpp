#include "UsbCommunication.hpp"
#include "usb_comm_c_interface.h"
#include "usbd_cdc_if.h"
#include "state/StateMachine.hpp"
#include <cstring>
#include <cstdlib>
#include <string>
#include <cfloat>
#include <cmath>

// For cache management functions
#include "stm32h7xx.h"

// CMSIS RTOS v2 for NAND task control
#include "cmsis_os.h"

// Test data for 'run' command - include after generating with ai_test.py
#include "ai_monitoring.hpp"

// X-CUBE-AI includes - moved to top for type definitions
#include "fallower1.h"
#include "fallower1_data.h"
#include "radar_adc_test.hpp"  // For FrameBuffer struct

// REMOVED: Frame-wise normalization function - NOT NEEDED
// Test data from .h is already normalized to [-1000, 1000] range
// Model input format: f32(1x30x688x1) = 20640 floats directly usable
// No additional normalization required in embedded software
#include "radar_flags.h"        // For radar_flags struct
#include "radar_driver.hpp"     // For Sensor_WriteRegister function
#include "main.h"               // For GPIO definitions
#include "data/sit_2751_test_data.h"

#include "nand_model_storage.h"

#include "nand_logger.h"

// Real-time inference state
#include "state/RealtimeInference.hpp"

extern QueueHandle_t nandQueue;

// C-linkage wrapper for logging from C files
extern "C" void NAND_Log(const char* state, const char* message) {
    if (UsbCommunication::getInstance()) {
        UsbCommunication::getInstance()->sendStatusMessage(state, message);
    }
}


// C-linkage accessors for hardfault log (implemented in hardfault_report.cpp)
extern "C" {
    const volatile void* GetHardFaultLog(void);
    void ClearHardFaultLog(void);
}

// Static member initialization
UsbCommunication* UsbCommunication::instance = nullptr;
SemaphoreHandle_t UsbCommunication::usbMutex = nullptr;
SemaphoreHandle_t UsbCommunication::aiMutex = nullptr; // Initialize AI mutex

// Fire hose mode için global flag
static volatile bool fire_hose_mode = false;
static volatile uint32_t fire_hose_bytes_received = 0;
static volatile uint32_t fire_hose_chunk_count = 0;
static volatile uint32_t fire_hose_expected_size = 0;  // metadata'dan gelen beklenen boyut

// Global AI network handle - accessible from static member functions
ai_handle network = AI_HANDLE_NULL;

// SDRAM functions from StateMachine.cpp are already declared in StateMachine.hpp
extern void SDRAM_ResetWriteState(); // StateMachine.cpp'den

// C ARAYÜZ FONKSİYONUNUN IMPLEMENTASYONU - Circular Buffer ile
// Bu fonksiyon ISR'dan çağrılır, bu yüzden çok hızlı olmalı
void UsbRxQueueAddData(uint8_t* data, uint32_t length) {
    UsbCommunication* usb = UsbCommunication::getInstance();
    if (usb) {
        // Her zaman circular buffer'a yaz - fire hose mode'da da
        // RxTask fire hose mode'u kontrol edip SDRAM'e yazar
        usb->writeToCircularBuffer(data, length);
    }
}

// C callback wrapper
extern "C" void UsbDataReceivedCallback(uint8_t* data, uint32_t length) {
    UsbCommunication* usb = UsbCommunication::getInstance();
    if (usb) {
        usb->addToRxQueue(data, length);
    }
}

UsbCommunication::UsbCommunication() : txQueue(nullptr), rxQueue(nullptr),
                                    txTaskHandle(nullptr), rxTaskHandle(nullptr),
                                    initialized(false),
                                    aiWorkerTaskHandle(nullptr), aiCommandQueue(nullptr),
                                    is_usb_rx_paused(false),
                                    gAccumulatedLength(0), gCommandInProgress(false) {
    if (usbMutex == nullptr) {
        usbMutex = xSemaphoreCreateMutex();
    }
    if (aiMutex == nullptr) {
        aiMutex = xSemaphoreCreateMutex(); // Create AI mutex
    }
    
    // Initialize circular buffer with instance buffer
    CB_Init(&circular_buffer, usb_buffer, sizeof(usb_buffer));
    
    // Initialize register command accumulation buffer
    memset(gAccumulatedBuffer, 0, sizeof(gAccumulatedBuffer));
}

UsbCommunication::~UsbCommunication() {
    // Cleanup code
}

/**
 * @brief UsbCommunication sınıfının singleton örneğini (instance) döndürür.
 * 
 * Thread-safe (muteks korumalı) bir şekilde sınıfın tek bir kopyasının oluşturulmasını 
 * ve tüm sistemden erişilmesini sağlar.
 * 
 * @return UsbCommunication* Sınıfın singleton örneği.
 * 
 * @note ISR içinden ÇAĞRILMAMALIDIR (Muteks içerir).
 */
UsbCommunication* UsbCommunication::getInstance() {
    if (instance == nullptr) {
        if (usbMutex == nullptr) {
            usbMutex = xSemaphoreCreateMutex();
        }

        if(usbMutex != nullptr && xSemaphoreTake(usbMutex, portMAX_DELAY) == pdTRUE) {
            if (instance == nullptr) {
                instance = new UsbCommunication();
            }
            xSemaphoreGive(usbMutex);
        }
    }
    return instance;
}

/**
 * @brief USB iletişim görevlerini (Tasks) ve kuyrukları (Queues) başlatır.
 * 
 * UsbTxTask, UsbRxTask ve AIWorkerTask görevlerini oluşturur. İletişim için gerekli 
 * FreeRTOS kuyruklarını ilkinize eder.
 * 
 * @return bool Başlatma başarılıysa true döner.
 * 
 * @note Sistem açılışında bir kez çağrılmalıdır. RTOS scheduler başlamadan önce 
 *       çağrılması uygundur.
 */
bool UsbCommunication::initialize() {
    // Create queues
    txQueue = xQueueCreate(10, sizeof(UsbMessage*));
    rxQueue = xQueueCreate(10, sizeof(UsbMessage*));
    aiCommandQueue = xQueueCreate(5, sizeof(AICommandMsg));  // AI command queue

    if (!txQueue || !rxQueue || !aiCommandQueue) {
        return false;
    }

    // Create TX task
    BaseType_t txStatus = xTaskCreate(
        txTask,
        "UsbTxTask",
        512,
        this,
        tskIDLE_PRIORITY + 2,
        &txTaskHandle
    );

    // Create RX task - INCREASED stack for state transitions
    // CRITICAL: State machine changeState() creates tempState objects on stack
    // RealtimeInference_3Task::OnEnter() requires deep call stack
    BaseType_t rxStatus = xTaskCreate(
        rxTask,
        "UsbRxTask",
        4096,  // 16KB - needed for state transition stack depth (was 8KB, caused overflow)
        this,
        tskIDLE_PRIORITY + 2,
        &rxTaskHandle
    );

    // Create AI Worker task - Reasonable stack for AI operations
    BaseType_t aiStatus = xTaskCreate(
        aiWorkerTask,
        "AIWorkerTask",
        8192,  // 32KB stack (8192 words × 4 bytes) - sufficient for AI inference
        this,
        tskIDLE_PRIORITY + 3,
        &aiWorkerTaskHandle
    );

    // DEBUG: Check if AI Worker Task was created successfully
    if (aiStatus != pdPASS) {
        // Task creation failed - critical error
        if (txQueue) {
            char error_msg[128];
            snprintf(error_msg, sizeof(error_msg),
                    "CRITICAL: AI Worker Task creation failed! heap_free=%d",
                    (int)xPortGetFreeHeapSize());
            UsbMessage* msg = (UsbMessage*)pvPortMalloc(sizeof(UsbMessage));
            if (msg) {
                msg->message = (char*)pvPortMalloc(strlen(error_msg) + 1);
                if (msg->message) {
                    strcpy(msg->message, error_msg);
                    msg->priority = 0;
                    xQueueSend(txQueue, &msg, 0);
                }
            }
        }
    }

    initialized = (txStatus == pdPASS && rxStatus == pdPASS && aiStatus == pdPASS);

    if (initialized) {
        // Store original priorities to be restored after fire hose transfer
        originalRxPriority = uxTaskPriorityGet(rxTaskHandle);
        originalTxPriority = uxTaskPriorityGet(txTaskHandle);
        originalAiWorkerPriority = uxTaskPriorityGet(aiWorkerTaskHandle);
    }

    return initialized;
}

/**
 * @brief USB veri gönderimini yöneten FreeRTOS görevi.
 * 
 * `txQueue` üzerinden gelen mesajları bekler ve USB CDC arayüzü üzerinden PC'ye gönderir. 
 * USB donanımının meşguliyetini (TxState) kontrol ederek güvenli gönderim yapar.
 * 
 * @param parameters Görev parametreleri (UsbCommunication instance pointer).
 * @return None
 * 
 * @note Bu bir FreeRTOS görev döngüsüdür (infinite loop).
 */
void UsbCommunication::txTask(void* parameters) {
    UsbCommunication* instance = static_cast<UsbCommunication*>(parameters);
    UsbMessage* msg = nullptr;

    for (;;) {
        if (xQueueReceive(instance->txQueue, &msg, portMAX_DELAY) == pdPASS) {
            if (msg && msg->message) {
                // Get USB handle and check state
                extern USBD_HandleTypeDef hUsbDeviceFS;
                USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;

                if (hcdc && hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) {
                    // Wait for USB to be ready with timeout
                    uint32_t startTime = HAL_GetTick();
                    const uint32_t TIMEOUT_MS = 150;

                    while (hcdc->TxState != 0) {
                        if (HAL_GetTick() - startTime > TIMEOUT_MS) {
                            break;
                        }
                        vTaskDelay(pdMS_TO_TICKS(5));
                    }

                    if (hcdc->TxState == 0) {
                        uint32_t len = strlen(msg->message);
                        CDC_Transmit_FS((uint8_t*)msg->message, len);
                    }
                }

                free(msg->message);
                free(msg);
            }
        }
    }
}

/**
 * @brief USB veri alımını ve komut çözümlemeyi yöneten FreeRTOS görevi.
 * 
 * Circular buffer'dan gelen verileri okur, komutları ayrıştırır (parse) ve ilgili 
 * alt sistemlere yönlendirir. "Fire hose" modunda yüksek hızlı model transferini yönetir.
 * 
 * @param parameters Görev parametreleri.
 * @return None
 * 
 * @note En kritik komut işleme döngüsüdür. State machine geçişlerini tetikler.
 */
void UsbCommunication::rxTask(void* parameters) {
    UsbCommunication* instance = static_cast<UsbCommunication*>(parameters);
    static uint8_t temp_buffer[1024]; // Static to avoid stack overflow - large STR commands

    // AI model state will be accessed through static variable
    
    for (;;) {
        // Read data from circular buffer
        size_t bytes_read = CB_Read(&instance->circular_buffer, temp_buffer, sizeof(temp_buffer));

        // Skip retry logic completely in fire hose mode - process whatever data we got
        if (!fire_hose_mode && bytes_read > 0 && bytes_read < sizeof(temp_buffer)) {
            // Try additional reads to get complete command in one go (normal mode only)
            for (int retry = 0; retry < 5 && bytes_read < sizeof(temp_buffer) - 64; retry++) {
                vTaskDelay(pdMS_TO_TICKS(1)); // Small delay for more data to arrive
                size_t additional_space = sizeof(temp_buffer) - bytes_read;
                size_t extra_read = CB_Read(&instance->circular_buffer,
                                          temp_buffer + bytes_read, additional_space);
                if (extra_read == 0) break; // No more data
                bytes_read += extra_read;

                // Check if we got complete STR command
                if (bytes_read > 5 && strncmp((char*)temp_buffer, "<STR>", 5) == 0) {
                    temp_buffer[bytes_read] = '\0'; // Null terminate for check
                    if (temp_buffer[bytes_read-1] == '>') break; // Complete command
                }
            }
        }
        
        if (bytes_read > 0) {
            // Process the received data
            // Null-terminate for string processing
            if (bytes_read < sizeof(temp_buffer)) {
                temp_buffer[bytes_read] = '\0';
            } else {
                temp_buffer[sizeof(temp_buffer) - 1] = '\0';
            }
            
            // KRİTİK ÇÖZÜM: Fire hose mode aktifken HİÇBİR command processing yapma
            if (!fire_hose_mode && bytes_read >= 4) {  // String commands only when fire hose OFF
                // BUFFER OVERFLOW FIX: Ensure null termination within bounds
                if (bytes_read < sizeof(temp_buffer)) {
                    temp_buffer[bytes_read] = '\0';  // Safe null terminate
                } else {
                    temp_buffer[sizeof(temp_buffer) - 1] = '\0';  // Truncate safely
                }
                const char* command = (const char*)temp_buffer;
                
                // Handle legacy string commands
                if (strncmp(command, "check_esp32", 11) == 0) {
                    uint8_t rxVal = SPI_Check_Connection();
                    char msg[64];
                    if (rxVal == 0x55) {
                        snprintf(msg, sizeof(msg), "ESP32 Connected: YES (ACK: 0x55)");
                    } else {
                        snprintf(msg, sizeof(msg), "ESP32 Connected: NO (Rx: 0x%02X)", rxVal);
                    }
                    instance->sendStatusMessage("ESP32_STATUS", msg);
                    continue;

                // =====================================================================
                // MANUAL COMMAND SEQUENCE:
                // init → ready → registers → calibrate → load_model_binary →
                //   load_model_flash → read_model_nand → ai_init → run_inference_isr_real
                // =====================================================================

                } else if (strncmp(command, "INIT", 4) == 0 || strncmp(command, "init", 4) == 0) {
                    // Allowed from IDLE state only
                    StateMachine* sm = StateMachine::getInstance();
                    if (!sm || sm->getCurrentState() != STATE_IDLE) {
                        instance->sendStatusMessage("INIT_ERROR", "Must be in IDLE state to run INIT");
                        continue;
                    }
                    instance->sendStatusMessage("IDLE", "Processing INIT command");
                    sm->changeState(STATE_INIT);

                } else if (strncmp(command, "READY", 5) == 0 || strncmp(command, "ready", 5) == 0) {
                    // Allowed from INIT state only
                    StateMachine* sm = StateMachine::getInstance();
                    if (!sm || sm->getCurrentState() != STATE_INIT) {
                        instance->sendStatusMessage("READY_ERROR", "Must be in INIT state to go READY");
                        continue;
                    }
                    instance->sendStatusMessage("INIT", "Processing READY command");
                    sm->changeState(STATE_READY);

                } else if (strncmp(command, "registers", 9) == 0) {
                    // Allowed from READY state only
                    StateMachine* sm = StateMachine::getInstance();
                    if (!sm || sm->getCurrentState() != STATE_READY) {
                        instance->sendStatusMessage("REGISTERS_ERROR", "Must be in READY state (send 'ready' first)");
                        continue;
                    }
                    extern SPI_HandleTypeDef hspi4;
                    extern void Sensor_WriteAllRegisters(SPI_HandleTypeDef* hspi);
                    extern void Sensor_WriteRegister(SPI_HandleTypeDef* hspi, uint8_t addr, uint8_t val);
                    instance->sendStatusMessage("REGISTERS", "Writing radar registers...");
                    Sensor_WriteAllRegisters(&hspi4);
                    osDelay(400);
                    Sensor_WriteRegister(&hspi4, 0x08, 0x7A);
                    osDelay(10);
                    Sensor_WriteRegister(&hspi4, 0x09, 0x00);
                    osDelay(400);
                    Sensor_WriteRegister(&hspi4, 0x02, 0xFF);
                    osDelay(300);
                    instance->sendStatusMessage("REGISTERS", "Radar registers written. Send 'calibrate' next.");
                    continue;

                } else if (strncmp(command, "calibrate", 9) == 0 || strncmp(command, "CALIBRATE", 9) == 0) {
                    // Allowed from READY state only (registers must have been written)
                    StateMachine* sm = StateMachine::getInstance();
                    if (!sm || sm->getCurrentState() != STATE_READY) {
                        instance->sendStatusMessage("CALIBRATE_ERROR", "Must be in READY state (send 'registers' first)");
                        continue;
                    }
                    sm->changeState(STATE_CALIBRATE);
                    instance->sendStatusMessage("CALIBRATE", "Calibration started. Wait for completion then send 'load_model_binary'.");
                    memset(temp_buffer, 0, bytes_read);
                    bytes_read = 0;
                    continue;

                // =============================================================================
                // START COMMAND: Raw data streaming (for Python-side inference)
                // =============================================================================
                } else if (strncmp(command, "START", 5) == 0) {
                    instance->sendStatusMessage("STREAM", "START command received - initiating raw data streaming");
                    StateMachine* sm = StateMachine::getInstance();
                    if (sm) {
                        sm->changeState(STATE_STREAM);
                    }
                    // Buffer temizle ve döngüyü bitir
                    memset(temp_buffer, 0, bytes_read);
                    bytes_read = 0;
                    continue;

                // =============================================================================
                // COLLECT COMMAND: Raw data collection mode (no AI, just stream for recording)
                // Cleans up any running inference pipeline, then enters pure Stream mode
                // =============================================================================
                } else if (strncmp(command, "COLLECT", 7) == 0) {
                    instance->sendStatusMessage("COLLECT", "COLLECT command received - stopping inference, entering raw collection mode");

                    // Clean up ALL ISR inference modes before entering Stream
                    // changeState() does NOT call OnExit() on previous state, so flags stay dirty

                    // 1. ISR_Real mode cleanup (STATE_REALTIME_INFERENCE_ISR_REAL)
                    extern volatile bool g_isr_real_mode;
                    extern void ISR_Real_Pipeline_Cleanup(void);
                    if (g_isr_real_mode) {
                        ISR_Real_Pipeline_Cleanup();
                        instance->sendStatusMessage("COLLECT", "ISR_Real inference stopped");
                    }

                    // 2. ISR mode cleanup (STATE_REALTIME_INFERENCE_ISR)
                    extern volatile bool g_isr_mode_active;
                    extern volatile bool g_isr_inference_running;
                    if (g_isr_mode_active) {
                        g_isr_mode_active = false;
                        g_isr_inference_running = false;
                        instance->sendStatusMessage("COLLECT", "ISR inference stopped");
                    }

                    // 3. ISR_Test mode cleanup
                    extern volatile bool g_isr_test_mode;
                    if (g_isr_test_mode) {
                        g_isr_test_mode = false;
                        instance->sendStatusMessage("COLLECT", "ISR_Test mode stopped");
                    }

                    // Small delay to ensure ISR handlers see updated flags
                    osDelay(50);

                    StateMachine* sm = StateMachine::getInstance();
                    if (sm) {
                        sm->changeState(STATE_STREAM);
                    }
                    instance->sendStatusMessage("COLLECT", "Raw data streaming active - ready for collection");
                    memset(temp_buffer, 0, bytes_read);
                    bytes_read = 0;
                    continue;

                // =============================================================================
                // STOP_COLLECT COMMAND: Stop collection and return to IDLE state
                // =============================================================================
                } else if (strncmp(command, "STOP_COLLECT", 12) == 0) {
                    instance->sendStatusMessage("COLLECT", "STOP_COLLECT - returning to IDLE");

                    // ADC task'a graceful stop (vTaskDelete yok — USB CDC koruma)
                    extern volatile bool stopStreamingFlag;
                    extern TaskHandle_t adcSampleTaskHandle;
                    extern TaskHandle_t streamingTaskHandle;
                    extern ADC_HandleTypeDef hadc3;
                    stopStreamingFlag = true;
                    osDelay(500);
                    adcSampleTaskHandle = NULL;
                    streamingTaskHandle = NULL;
                    HAL_ADC_Stop(&hadc3);

                    StateMachine* sm = StateMachine::getInstance();
                    if (sm) {
                        sm->changeState(STATE_IDLE);
                    }
                    memset(temp_buffer, 0, bytes_read);
                    bytes_read = 0;
                    continue;

                // =============================================================================
                // STOP COMMAND: Stop streaming and return to READY state
                // =============================================================================
                } else if (strncmp(command, "STOP", 4) == 0) {
                    instance->sendStatusMessage("STREAM", "STOP command received - stopping data stream");

                    // ============================================================
                    // Stream::OnExit() changeState() tarafından çağrılmıyor.
                    // ADC task'a flag ile dur sinyali gönder ve kendi kendine
                    // çıkmasını bekle. vTaskDelete KULLANMA — task USB TX
                    // ortasında öldürülürse CDC endpoint hang olur.
                    // ============================================================
                    extern volatile bool stopStreamingFlag;
                    extern TaskHandle_t adcSampleTaskHandle;
                    extern TaskHandle_t streamingTaskHandle;
                    extern ADC_HandleTypeDef hadc3;

                    // 1. Flag'i set et — task while döngüsünden çıkıp
                    //    kendi kendini silecek (vTaskDelete(NULL))
                    stopStreamingFlag = true;

                    // 2. Task'ın mevcut frame'i + USB TX'i bitirip çıkmasını bekle
                    //    Worst case: 100ms notify timeout + ~5ms USB TX = ~105ms
                    //    Güvenlik marjı ile 500ms
                    osDelay(500);

                    // 3. Task kendini sildi, handle'ı temizle
                    adcSampleTaskHandle = NULL;
                    if (streamingTaskHandle != NULL) {
                        streamingTaskHandle = NULL;
                    }

                    // 4. ADC peripheral'ı güvenli state'e al
                    HAL_ADC_Stop(&hadc3);

                    // 5. State geçişi
                    StateMachine* sm = StateMachine::getInstance();
                    if (sm) {
                        sm->changeState(STATE_READY);
                    }

                    // Buffer temizle ve döngüyü bitir
                    memset(temp_buffer, 0, bytes_read);
                    bytes_read = 0;
                    continue;

                // =============================================================================
                // DIRECT RADAR REGISTER COMMANDS (Works instantaneously in ANY state)
                // =============================================================================
                } else if (strncmp(command, "SET_REG34:", 10) == 0 || strncmp(command, "set_reg34:", 10) == 0) {
                    const char* valStr = command + 10;
                    uint8_t val = (uint8_t)strtol(valStr, NULL, 16);
                    extern SPI_HandleTypeDef hspi4;
                    
                    Sensor_WriteRegister(&hspi4, 0x22, val);
                    osDelay(10);
                    uint8_t readback = Sensor_ReadRegister(&hspi4, 0x22);
                    
                    char msg[96];
                    snprintf(msg, sizeof(msg), "[RADAR_REG] Reg34 (0x22 Step Bin) -> 0x%02X (Readback: 0x%02X)", val, readback);
                    instance->sendStatusMessage("RADAR_REG", msg);
                    memset(temp_buffer, 0, bytes_read);
                    bytes_read = 0;
                    continue;

                } else if (strncmp(command, "SET_REG:", 8) == 0 || strncmp(command, "set_reg:", 8) == 0) {
                    const char* p = command + 8;
                    char* next = nullptr;
                    long reg = strtol(p, &next, 0);
                    if (next && *next == ':') {
                        long val = strtol(next + 1, NULL, 16);
                        uint8_t regAddr = (reg <= 0x3A) ? (uint8_t)reg : 0x22;
                        extern SPI_HandleTypeDef hspi4;
                        
                        Sensor_WriteRegister(&hspi4, regAddr, (uint8_t)val);
                        osDelay(10);
                        uint8_t readback = Sensor_ReadRegister(&hspi4, regAddr);
                        
                        char msg[96];
                        snprintf(msg, sizeof(msg), "[RADAR_REG] Reg 0x%02X -> 0x%02X (Readback: 0x%02X)", regAddr, (uint8_t)val, readback);
                        instance->sendStatusMessage("RADAR_REG", msg);
                    }
                    memset(temp_buffer, 0, bytes_read);
                    bytes_read = 0;
                    continue;

                } else if (strncmp(command, "READ_REG34", 10) == 0 || strncmp(command, "read_reg34", 10) == 0) {
                    extern SPI_HandleTypeDef hspi4;
                    uint8_t readback = Sensor_ReadRegister(&hspi4, 0x22);
                    char msg[64];
                    snprintf(msg, sizeof(msg), "[RADAR_REG] Current Reg34 (0x22) = 0x%02X", readback);
                    instance->sendStatusMessage("RADAR_REG", msg);
                    memset(temp_buffer, 0, bytes_read);
                    bytes_read = 0;
                    continue;

                } else if (strncmp(command, "ai_init", 7) == 0 || strncmp(command, "AI_INIT", 7) == 0) {
                    // DELEGATED TO AI WORKER TASK
                    instance->sendStatusMessage("AI_INIT", "Delegating ai_init to AI Worker Task");

                    AICommandMsg aiCmd;
                    aiCmd.cmd = AICommand::INIT_MODEL;
                    aiCmd.test_data = nullptr;
                    aiCmd.data_size = 0;

                    if (xQueueSend(instance->aiCommandQueue, &aiCmd, pdMS_TO_TICKS(100)) != pdPASS) {
                        instance->sendStatusMessage("AI_ERROR", "Failed to queue ai_init command");
                    } else {
                        instance->sendStatusMessage("AI_INIT", "ai_init queued - AI Worker will initialize");
                    }

                    continue;
                } else if (strncmp(command, "load_model_binary", 17) == 0) {
                    // Initialize monitoring for model loading
                    AIMonitoring::Initialize();
                    AIMonitoring::Reset();

                    AI_MONITOR_STEP("Model Load Request", 1);
                    instance->sendStatusMessage("LOAD_MODEL", "load_model_binary received - awaiting metadata");

                    // İYİLEŞTİRME: Metadata bekleme moduna geç, fire hose mode henüz aktif değil
                    fire_hose_mode = false;  // Henüz aktif değil!
                    fire_hose_bytes_received = 0;

                    // SDRAM henüz reset edilmez - metadata geldikten sonra reset edilecek
                    // SDRAM_ResetWriteState();  // Bu metadata sonrası yapılacak

                    AI_MONITOR_STEP("Awaiting Model Metadata", 2);
                    instance->sendStatusMessage("AWAITING_METADATA", "Ready for metadata (size, CRC32)");

                    // ÇÖZÜM: Command işlendikten sonra döngüyü bitir, metadata bekleme moduna geç
                    continue;
                } else if (strncmp(command, "metadata:", 9) == 0) {
                    AI_MONITOR_STEP("Processing Model Metadata", 3);

                    // YENİ: Metadata handling
                    // Format: "metadata:SIZE,CRC32" (örn: "metadata:4280700,0x12345678")
                    uint32_t file_size = 0;
                    uint32_t file_crc32 = 0;

                    // Parse metadata: SIZE,CRC32
                    const char* metadata_str = command + 9; // Skip "metadata:"
                    const char* comma_pos = strchr(metadata_str, ',');
                    if (comma_pos != NULL) {
                        // Parse size (before comma)
                        size_t size_len = comma_pos - metadata_str;
                        char size_str[32];
                        strncpy(size_str, metadata_str, size_len);
                        size_str[size_len] = '\0';

                        file_size = strtoul(size_str, NULL, 10);
                        file_crc32 = strtoul(comma_pos + 1, NULL, 16);  // Hex format

                        // Validate metadata ranges
                        if (file_size == 0 || file_size > 10*1024*1024) {  // Max 10MB model
                            AI_REPORT_ERROR("Metadata Validation", "Invalid model size");
                            instance->sendStatusMessage("METADATA_ERROR", "Model size out of range");
                            continue;
                        }

                        AI_MONITOR_STEP("Storing Model Metadata", 4);

                        // CRITICAL FIX: Store metadata properly before SDRAM reset
                        Metadata_t metadata;
                        memset(&metadata, 0, sizeof(metadata));
                        strncpy((char*)metadata.model_name, "fallower", 8);
                        metadata.model_name[7] = '\0';
                        metadata.version_major = 1;
                        metadata.version_minor = 0;
                        metadata.total_size = file_size;
                        metadata.crc32 = file_crc32;

                        // Store metadata BEFORE resetting SDRAM state
                        StoreMetadata(&metadata);

                        AI_MONITOR_STEP("Preparing SDRAM for Model", 5);

                        // Verify SDRAM model weights address
                        void* model_weights_addr = (void*)0xC0200000;  // AI weights with 2MB safety gap
                        if (!AI_VERIFY_ALIGNMENT(model_weights_addr, 4, "Model Weights SDRAM")) {
                            AI_REPORT_ERROR("SDRAM Setup", "Model weights address misaligned");
                            return;
                        }

                        // BEST PRACTICE: Clear the SDRAM region before writing new model data
                        instance->sendStatusMessage("SDRAM_CLEAR", "Clearing SDRAM for new model...");
                        memset(model_weights_addr, 0, file_size);
                        instance->sendStatusMessage("SDRAM_CLEAR", "SDRAM cleared successfully.");

                        // SDRAM yazma durumunu sıfırla (metadata aldıktan sonra)
                        SDRAM_ResetWriteState();

                        // CRITICAL: Boost rxTask priority for the duration of the transfer
                        // Use FreeRTOS native priorities. configMAX_PRIORITIES is defined in FreeRTOSConfig.h
                        vTaskPrioritySet(instance->rxTaskHandle, (configMAX_PRIORITIES - 1)); // Highest possible priority
                        vTaskPrioritySet(instance->txTaskHandle, tskIDLE_PRIORITY);
                        vTaskPrioritySet(instance->aiWorkerTaskHandle, tskIDLE_PRIORITY);
                        instance->sendStatusMessage("PRIORITY_CHANGE", "rxTask priority boosted for transfer.");

                        // CRITICAL FIX: Flush USB buffer before fire hose to prevent garbage
                        // Clear any remaining data in temp_buffer from this command
                        memset(temp_buffer, 0, sizeof(temp_buffer));
                        bytes_read = 0;

                        // Small delay to ensure USB buffer is clear
                        vTaskDelay(pdMS_TO_TICKS(10));

                        // Fire hose mode'u şimdi aktif et
                        fire_hose_mode = true;
                        fire_hose_bytes_received = 0;
                        fire_hose_chunk_count = 0;
                        fire_hose_expected_size = file_size;  // metadata'dan gelen beklenen boyut

                        instance->sendStatusMessage("FIRE_HOSE_DEBUG", "Fire hose mode activated - buffer flushed");

                        char msg[128];
                        snprintf(msg, sizeof(msg), "Metadata: Size=%u bytes, CRC32=0x%08X, Addr=0x%08lX",
                                (unsigned int)file_size, (unsigned int)file_crc32, (unsigned long)model_weights_addr);
                        AI_REPORT_SUCCESS("Metadata Processing", msg);
                        instance->sendStatusMessage("METADATA_OK", msg);
                        instance->sendStatusMessage("READY_FOR_DATA", "SDRAM ready, fire hose active");
                    } else {
                        AI_REPORT_ERROR("Metadata Parsing", "Invalid metadata format");
                        instance->sendStatusMessage("METADATA_ERROR", "Invalid metadata format");
                    }
                    
                    // Buffer temizle ve döngüyü bitir
                    memset(temp_buffer, 0, bytes_read);
                    bytes_read = 0;
                    continue;
                } else if (strncmp(command, "verify_model", 12) == 0) {
                    instance->sendStatusMessage("VERIFY", "Starting model verification task...");

                    // Fire hose mode'u kapat
                    fire_hose_mode = false;

                    // ModelInstallerTask'i başlat
                    extern void ModelInstallerTask(void* parameters);
                    BaseType_t result = xTaskCreate(
                        ModelInstallerTask,
                        "ModelInstaller", 
                        2048,  // Stack size - 8KB for non-AI tasks
                        NULL,  // Parameters
                        tskIDLE_PRIORITY + 3,  // High priority
                        NULL   // Task handle
                    );

                    if (result == pdPASS) {
                        char msg[64];
                        snprintf(msg, sizeof(msg), "Verification started - %u bytes received", 
                                (unsigned int)fire_hose_bytes_received);
                        instance->sendStatusMessage("VERIFY", msg);
                    } else {
                        instance->sendStatusMessage("VERIFY_ERROR", "Failed to create verification task");
                    }
                    
                    // KRİTİK: verify_model buffer'ını temizle - SDRAM'e yazılmasını engelle  
                    memset(temp_buffer, 0, bytes_read);
                    bytes_read = 0;
                    // ÇÖZÜM: Command işlendikten sonra döngüyü bitir, fire hose'a gitmesin
                    continue;
                } else if (strncmp(command, "AI_INFERENCE", 12) == 0 || strncmp(command, "ai_inference", 12) == 0) {
                    // Simple state change - can stay in rxTask (very lightweight)
                    // But for consistency, we could delegate to AI Worker too

                    if (network == AI_HANDLE_NULL) {
                        instance->sendStatusMessage("AI_ERROR", "Model not initialized");
                        continue;
                    }

                    instance->sendStatusMessage("AI_INFERENCE", "Model ready - AI inference mode activated");

                    // Change state to MODEL_READY
                    StateMachine* sm = StateMachine::getInstance();
                    if (sm) {
                        sm->changeState(STATE_MODEL_READY);
                    }

                    instance->sendStatusMessage("AI_INFERENCE", "Ready for radar data or 'run' command");
                    continue;
                } else if (strncmp(command, "run_radar", 9) == 0) {
                    // NEW: Run radar data pipeline with AI inference
                    instance->sendStatusMessage("RADAR_AI", "Starting radar data collection for AI inference");
                    
                    // Start NEW queue-based radar AI pipeline
                    instance->startRadarAIPipeline();
                    continue;
                    
                } else if (strncmp(command, "set_meta", 8) == 0) {
                    // Parse: set_meta 4280700 0x8E1EFA22
                    uint32_t size;
                    uint32_t crc32;
                    
                    int parsed = sscanf(command + 9, "%lu 0x%lx", &size, &crc32);
                    
                    if (parsed == 2) {
                        // Store metadata in StateMachine
                        Metadata_t metadata;
                        memset(&metadata, 0, sizeof(metadata));
                        strncpy((char*)metadata.model_name, "fallower", 7);
                        metadata.model_name[7] = '\0';  // Ensure null termination
                        metadata.version_major = 1;
                        metadata.version_minor = 0;
                        metadata.total_size = size;
                        metadata.crc32 = crc32;
                        
                        // Store metadata using global function
                        StoreMetadata(&metadata);
                        char msg[128];
                        snprintf(msg, sizeof(msg), "Metadata stored: fallower v1.0, %lu bytes, CRC: 0x%08lX", 
                                size, crc32);
                        instance->sendStatusMessage("METADATA", msg);
                    } else {
                        instance->sendStatusMessage("ERROR", "Invalid metadata format");
                    }
                    
                } else if (strncmp(command, "buffer_status", 13) == 0) {
                    // Report current SDRAM buffer status - SAFE VERSION
                    uint32_t total_bytes = GetTotalBytesWritten();
                    
                    char msg[128];
                    snprintf(msg, sizeof(msg), "Buffer: %lu bytes written", total_bytes);
                    instance->sendStatusMessage("BUFFER_STATUS", msg);
                    
                } else if (strncmp(command, "load_model_flash", 16) == 0) {
                    extern volatile bool g_fmc_high_priority_mode;
                    if (g_fmc_high_priority_mode) {
                        instance->sendStatusMessage("NAND_ERROR", "Action BLOCKED: FMC high-priority mode active (Radar active!)");
                        continue;
                    }
                    instance->sendStatusMessage("NAND", "Queueing NAND_OP_WRITE_MODEL command");
                    NAND_Operation_t op = NAND_OP_WRITE_MODEL;
                    if (xQueueSend(nandQueue, &op, (TickType_t)10) != pdPASS) {
                        instance->sendStatusMessage("NAND_ERROR", "Failed to queue NAND write operation.");
                    }
                    continue;
                } else if (strncmp(command, "read_model_nand", 15) == 0) {
                    extern volatile bool g_fmc_high_priority_mode;
                    if (g_fmc_high_priority_mode) {
                        instance->sendStatusMessage("NAND_ERROR", "Action BLOCKED: FMC high-priority mode active (Radar active!)");
                        continue;
                    }
                    instance->sendStatusMessage("NAND", "Queueing NAND_OP_READ_MODEL command");
                    NAND_Operation_t op = NAND_OP_READ_MODEL;
                    if (xQueueSend(nandQueue, &op, (TickType_t)10) != pdPASS) {
                        instance->sendStatusMessage("NAND_ERROR", "Failed to queue NAND read operation.");
                    }
                    continue;

                } else if (strncmp(command, "run_inference_isr_real", 22) == 0) {
                    // Precondition: ai_init must have succeeded (network handle valid)
                    if (network == AI_HANDLE_NULL) {
                        instance->sendStatusMessage("ISR_REAL_ERROR",
                            "Model not initialized - run 'ai_init' first");
                        continue;
                    }
                    // transitionTo() will kill calibrationTaskHandle if still running,
                    // preventing ADC3 deadlock between calibration and ISR mode.
                    instance->sendStatusMessage("ISR_REAL",
                        "Starting Real-time ISR inference (stride-13, 20MB circular buffer)");
                    StateMachine* stateMachine = StateMachine::getInstance();
                    if (stateMachine) {
                        stateMachine->transitionTo(STATE_REALTIME_INFERENCE_ISR_REAL);
                    }
                    continue;

                } else if (strncmp(command, "stop_inference", 14) == 0) {
                    // STOP REAL-TIME INFERENCE
                    instance->sendStatusMessage("RTINF_CMD", "Stopping real-time inference mode");

                    StateMachine* stateMachine = StateMachine::getInstance();
                    if (stateMachine) {
                        stateMachine->transitionTo(STATE_MODEL_READY);  // Return to idle
                    } else {
                        instance->sendStatusMessage("ERROR", "StateMachine not initialized");
                    }

                    // RESUME BACKGROUND TASKS
                    extern osThreadId_t nandTaskHandle;
                    extern osThreadId_t connectionCheckTaskHandle;

                    if (nandTaskHandle != NULL) {
                        osThreadResume(nandTaskHandle);
                    }
                    if (connectionCheckTaskHandle != NULL) {
                        osThreadResume(connectionCheckTaskHandle);
                        instance->sendStatusMessage("RTINF_CMD", "Background tasks resumed");
                    }

                    continue;

                } else if (strncmp(command, "inference_status", 16) == 0) {
                    // QUERY REAL-TIME INFERENCE STATUS
                    extern PipelineStats_t g_pipeline_stats;
                    extern QueueHandle_t frameQueue;
                    extern volatile bool is_inference_running;

                    char status[512];
                    UBaseType_t queue_count = (frameQueue != NULL) ? uxQueueMessagesWaiting(frameQueue) : 0;

                    snprintf(status, sizeof(status),
                        "[INF_STATUS] Running=%d, Produced=%lu, Consumed=%lu, "
                        "Inferences=%lu, Queue=%lu, Dropped=%lu, "
                        "AvgTime=%lums, MaxTime=%lums",
                        is_inference_running ? 1 : 0,
                        g_pipeline_stats.frames_produced,
                        g_pipeline_stats.frames_consumed,
                        g_pipeline_stats.inferences_done,
                        queue_count,
                        g_pipeline_stats.frames_dropped,
                        g_pipeline_stats.avg_inference_time_ms,
                        g_pipeline_stats.max_inference_time_ms
                    );
                    instance->sendMessage(status);

                    continue;

                } else if (strncmp(command, "run", 3) == 0) {
                    // DELEGATED TO AI WORKER TASK - rxTask just routes the command
                    instance->sendStatusMessage("AI_TEST", "Delegating inference to AI Worker Task");

                    AICommandMsg aiCmd;
                    aiCmd.cmd = AICommand::RUN_INFERENCE;
                    aiCmd.test_data = sit_2751_test_data;
                    aiCmd.data_size = AI_FALLOWER1_IN_1_SIZE_BYTES;

                    if (xQueueSend(instance->aiCommandQueue, &aiCmd, pdMS_TO_TICKS(100)) != pdPASS) {
                        instance->sendStatusMessage("AI_ERROR", "Failed to queue AI command");
                    } else {
                        instance->sendStatusMessage("AI_TEST", "Command queued - AI Worker will process");
                    }

                    continue;
                } else {
                    // SIMPLIFIED APPROACH: Direct STR command processing without multi-part complexity
                    bool isStartOfCommand = (strncmp(command, "<STR>", 5) == 0);
                    
                    if (isStartOfCommand) {
                        // Debug: Show STR command received with detailed info
                        char debugInfo[256];
                        size_t cmdLen = strlen(command);
                        char lastChar = (cmdLen > 0) ? command[cmdLen-1] : '?';
                        char secondLastChar = (cmdLen > 1) ? command[cmdLen-2] : '?';
                        bool hasR58 = (strstr(command, "<R58>") != NULL);
                        snprintf(debugInfo, sizeof(debugInfo), "STR: len=%d, last='%c%c', R58=%d", 
                                (int)cmdLen, secondLastChar, lastChar, hasR58 ? 1 : 0);
                        instance->sendStatusMessage("DEBUG", debugInfo);
                        
                        // Check if this buffer contains the complete command (ends with > possibly followed by \r\n)
                        bool endsWithMarker = (cmdLen > 10 && (
                            command[cmdLen-1] == '>' ||
                            command[cmdLen-2] == '>' ||
                            (cmdLen > 2 && command[cmdLen-3] == '>')  // handles >\r\n
                        ));
                        bool isComplete = endsWithMarker || (hasR58 && cmdLen > 500);
                        
                        if (isComplete) {
                            // SAFETY CHECK: Validate command length before processing
                            if (cmdLen > 2048) {
                                instance->sendStatusMessage("ERROR", "STR command too long - potential buffer overflow");
                            } else {
                                // Process complete STR command immediately
                                instance->sendStatusMessage("DEBUG", "Complete STR command - processing now");
                                instance->processSTRCommand(command);
                            }
                        } else {
                            // Start accumulating multi-part command
                            instance->sendStatusMessage("DEBUG", "Partial STR command - starting accumulation");
                            
                            // Clear and start fresh accumulation
                            memset(instance->gAccumulatedBuffer, 0, sizeof(instance->gAccumulatedBuffer));
                            instance->gAccumulatedLength = 0;
                            instance->gCommandInProgress = true;
                            
                            // Store first part
                            if (bytes_read < sizeof(instance->gAccumulatedBuffer) - 1) {
                                memcpy(instance->gAccumulatedBuffer, temp_buffer, bytes_read);
                                instance->gAccumulatedLength = bytes_read;
                                instance->gAccumulatedBuffer[instance->gAccumulatedLength] = '\0';
                            }
                        }
                    } else if (instance->gCommandInProgress) {
                        // Continue accumulating multi-part STR command
                        if (instance->gAccumulatedLength + bytes_read < sizeof(instance->gAccumulatedBuffer) - 1) {
                            memcpy(instance->gAccumulatedBuffer + instance->gAccumulatedLength, temp_buffer, bytes_read);
                            instance->gAccumulatedLength += bytes_read;
                            instance->gAccumulatedBuffer[instance->gAccumulatedLength] = '\0';
                        }
                        
                        // Check completion - command should end with > (possibly followed by \r\n)
                        size_t accLen = instance->gAccumulatedLength;
                        bool hasEndMarker = (accLen >= 1 && instance->gAccumulatedBuffer[accLen-1] == '>') ||
                                            (accLen >= 2 && instance->gAccumulatedBuffer[accLen-2] == '>') ||
                                            (accLen >= 3 && instance->gAccumulatedBuffer[accLen-3] == '>');
                        bool hasR58 = (strstr(instance->gAccumulatedBuffer, "<R58>") != NULL);
                        
                        if (hasEndMarker && (hasR58 || instance->gAccumulatedLength > 500)) {
                            // Process accumulated complete STR command
                            instance->sendStatusMessage("DEBUG", "Multi-part STR command complete - processing");
                            instance->processSTRCommand(instance->gAccumulatedBuffer);
                            
                            // Reset accumulation
                            memset(instance->gAccumulatedBuffer, 0, sizeof(instance->gAccumulatedBuffer));
                            instance->gAccumulatedLength = 0;
                            instance->gCommandInProgress = false;
                        }
                    } else {
                        // Process normal commands (INIT, READY, calibrate, ai_init etc.)
                        instance->processBasicCommand(command);
                    }
                    
                    // Buffer temizle ve döngüyü bitir
                    memset(temp_buffer, 0, bytes_read);
                    bytes_read = 0;
                    continue;
                }
            } else if (fire_hose_mode) {
                // **KRİTİK FİX**: Fire hose mode - SADECE binary data, HİÇBİR command check yapma
                // Tüm gelen data direkt SDRAM'e git
                    // Fire hose mode: Direct binary transfer without endianness modification
                    // Both PC (Intel/AMD) and STM32H7 (ARM) use little-endian format
                    // No byte swapping needed - preserves original float32 values

                    // DIRECT COPY: Write data exactly as received from PC
                    SDRAM_WriteChunk(temp_buffer, (uint16_t)bytes_read);

                    fire_hose_bytes_received += bytes_read;

                        // DEBUG: Log every 5000 chunks + LAST 5 chunks
                        fire_hose_chunk_count++;

                        bool should_log = (fire_hose_chunk_count % 5000 == 0) || (fire_hose_bytes_received >= 4280600);

                        if (should_log) {
                            char progress_msg[128];
                            snprintf(progress_msg, sizeof(progress_msg),
                                "FIRE_HOSE_PROGRESS: chunk#%u bytes_read=%u total=%u",
                                (unsigned int)fire_hose_chunk_count, (unsigned int)bytes_read, (unsigned int)fire_hose_bytes_received);
                            instance->sendStatusMessage("TRANSFER_DEBUG", progress_msg);
                        }

                        // Tamamlanma kontrolü: metadata'dan gelen beklenen boyuta ulaşıldı mı?
                        if (fire_hose_expected_size > 0 &&
                            fire_hose_bytes_received >= fire_hose_expected_size) {
                            instance->sendStatusMessage("DEBUG_HARDFAULT", "ENTERING completion block");

                            char trigger_msg[128];
                            snprintf(trigger_msg, sizeof(trigger_msg),
                                "TRANSFER_COMPLETE: received=%u expected=%u",
                                (unsigned int)fire_hose_bytes_received,
                                (unsigned int)fire_hose_expected_size);
                            instance->sendStatusMessage("TRANSFER_DEBUG", trigger_msg);

                            AI_MONITOR_STEP("Model Transfer Complete", 6);
                            instance->sendStatusMessage("TRANSFER_SUCCESS", "All data received");

                            // Fire hose mode'u kapat
                            fire_hose_mode = false;

                            // 1. Önce öncelikleri geri al — aiWorkerTask normal önceliğe
                            //    döndükten SONRA VERIFY_MODEL kuyruğa atılacak.
                            //    Bu sıralama HardFault'u önler: task düşük öncelikte
                            //    SDRAM'e erişmez.
                            vTaskPrioritySet(instance->rxTaskHandle, instance->originalRxPriority);
                            vTaskPrioritySet(instance->txTaskHandle, instance->originalTxPriority);
                            vTaskPrioritySet(instance->aiWorkerTaskHandle, instance->originalAiWorkerPriority);
                            instance->sendStatusMessage("PRIORITY_CHANGE", "Task priorities restored to normal.");

                            // 2. Cache flush + sistem stabilizasyonu için kısa gecikme
                            vTaskDelay(pdMS_TO_TICKS(100));

                            // 3. CRC + weight integrity doğrulamasını aiWorkerTask'a devret
                            AICommandMsg aiCmd;
                            aiCmd.cmd = AICommand::VERIFY_MODEL;
                            aiCmd.test_data = nullptr;
                            aiCmd.data_size = fire_hose_expected_size;

                            if (xQueueSend(instance->aiCommandQueue, &aiCmd, pdMS_TO_TICKS(200)) != pdPASS) {
                                instance->sendStatusMessage("AI_ERROR", "Failed to queue VERIFY_MODEL command");
                            } else {
                                instance->sendStatusMessage("VERIFY", "Model verification queued - aiWorkerTask will verify CRC and weights");
                            }
                        }
                    } else {
                        instance->sendStatusMessage("DATA_ERROR", "Failed to write to SDRAM");
                    }
            // Resume USB reception if it was paused
            instance->resumeRx();
        }

        // Yield the CPU briefly to allow other system tasks (like USB stack) to run,
        // preventing data corruption during high-speed transfers without causing a full 1ms delay.
        taskYIELD();
    }
}  // end rxTask



// Task: Poll the RAM-backed hardfault log and forward over USB from task context.
void UsbCommunication::hardFaultReporterTask(void* parameters) {
    UsbCommunication* instance = static_cast<UsbCommunication*>(parameters);
    for (;;) {
    // Check for a stored hardfault record (accessor provided by hardfault_report.cpp)
    const volatile void* raw = GetHardFaultLog();
        if (raw) {
            // Cast to known structure
            struct HF { uint32_t magic; uint32_t hfsr; uint32_t cfsr; uint32_t mmfar; uint32_t bfar; };
            const volatile HF* log = (const volatile HF*)raw;
            char buf[128];
            snprintf(buf, sizeof(buf), "HARDFAULT HFSR=0x%08X CFSR=0x%08X", (unsigned int)log->hfsr, (unsigned int)log->cfsr);
            instance->sendStatusMessage("HARDFAULT", buf);
            snprintf(buf, sizeof(buf), "MMFAR=0x%08X BFAR=0x%08X", (unsigned int)log->mmfar, (unsigned int)log->bfar);
            instance->sendStatusMessage("HARDFAULT", buf);

            // Clear the stored record so we don't spam
            ClearHardFaultLog();
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // check once per second
    }
}

void UsbCommunication::clearQueue() {
    if (!txQueue) return;

    UsbMessage* msg = nullptr;
    while (xQueueReceive(txQueue, &msg, 0) == pdPASS) {
        if (msg) {
            free(msg->message);
            free(msg);
        }
    }
}

bool UsbCommunication::queueTxMessage(const char* message, uint8_t priority) {
    if (!message || !txQueue) {
        return false;
    }

    if (uxQueueMessagesWaiting(txQueue) > 8) {
        UsbMessage* oldMsg = nullptr;
        for (int i = 0; i < 3; i++) {
            if (xQueueReceive(txQueue, &oldMsg, 0) == pdPASS && oldMsg) {
                free(oldMsg->message);
                free(oldMsg);
            }
        }
    }

    UsbMessage* msg = (UsbMessage*)malloc(sizeof(UsbMessage));
    if (!msg) {
        return false;
    }

    msg->message = strdup(message);
    msg->priority = priority;

    if (!msg->message) {
        free(msg);
        return false;
    }

    // Use priority to determine if message goes to front or back of queue
    BaseType_t queueResult;
    if (priority > 1) {
        queueResult = xQueueSendToFront(txQueue, &msg, pdMS_TO_TICKS(100));
    } else {
        queueResult = xQueueSendToBack(txQueue, &msg, pdMS_TO_TICKS(100));
    }

    if (queueResult != pdPASS) {
        free(msg->message);
        free(msg);
        return false;
    }

    return true;
}

bool UsbCommunication::sendMessage(const char* message) {
    return queueTxMessage(message, 0);  // Normal priority
}

bool UsbCommunication::sendStatusMessage(const char* stateName, const char* message) {
    char formattedMsg[MAX_USB_BUFFER_SIZE];
    snprintf(formattedMsg, MAX_USB_BUFFER_SIZE, "[%s] %s\r\n", stateName, message);
    return queueTxMessage(formattedMsg, 0);
}

bool UsbCommunication::sendData(uint8_t* data, uint32_t length) {
    if (data == nullptr || length == 0) {
        return false;  // Invalid data or length
    }

    extern USBD_HandleTypeDef hUsbDeviceFS;
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;

    if (hcdc == nullptr || hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED) {
        return false;  // USB not configured properly
    }

    uint32_t startTime = HAL_GetTick();
    const uint32_t TIMEOUT_MS = 100;

    // Send data without waiting for USB ready state, just attempt transmission
    if (CDC_Transmit_FS(data, length) == USBD_OK) {
        return true;  // Data successfully sent
    }

    // Retry after a short delay if transmission is not complete
    while (hcdc->TxState != 0) {
        if (HAL_GetTick() - startTime > TIMEOUT_MS) {
            return false;  // Timeout exceeded
        }
        vTaskDelay(pdMS_TO_TICKS(5));  // Small delay to avoid blocking other tasks
    }

    if (CDC_Transmit_FS(data, length) == USBD_OK) {
        return true;
    }

    return false;  // Transmission failed
}

// ISR-Safe Circular Buffer Write
void UsbCommunication::writeToCircularBuffer(uint8_t* data, uint32_t length) {
    if (!data || length == 0) {
        return;
    }

    // Try to write to circular buffer (ISR-safe)
    // CB_Write already checks free space internally with proper critical sections
    if (!CB_Write(&circular_buffer, data, (size_t)length)) {
        // Buffer full - pause USB reception (flow control)
        is_usb_rx_paused = true;
    }
}

void UsbCommunication::clearCircularBuffer() {
    // Clear circular buffer - critical section for ISR safety
    __disable_irq();
    CB_Clear(&circular_buffer);
    __enable_irq();
    
    // Resume reception if it was paused
    is_usb_rx_paused = false;
}

// Resume USB reception (called from main task)
void UsbCommunication::resumeRx() {
    extern USBD_HandleTypeDef hUsbDeviceFS;
    
    if (is_usb_rx_paused && CB_GetFreeSpace(&circular_buffer) > 256) {
        is_usb_rx_paused = false;
        USBD_CDC_ReceivePacket(&hUsbDeviceFS);  // Resume USB reception
    }
}

// Legacy addToRxQueue - kept for compatibility
void UsbCommunication::addToRxQueue(uint8_t* data, uint32_t length) {
    // Redirect to circular buffer
    writeToCircularBuffer(data, length);
}

// Fire hose mode yönetim fonksiyonları
void UsbCommunication::enableFireHoseMode() {
    fire_hose_mode = true;
    fire_hose_bytes_received = 0;
    // SDRAM reset will be done automatically
    sendStatusMessage("FIRE_HOSE", "Fire hose mode enabled");
}

void UsbCommunication::disableFireHoseMode() {
    fire_hose_mode = false;
    sendStatusMessage("FIRE_HOSE", "Fire hose mode disabled");
}

uint32_t UsbCommunication::getFireHoseBytesReceived() {
    return fire_hose_bytes_received;
}

// =============================================================================
// RADAR AI PIPELINE - QUEUE-BASED PRODUCER-CONSUMER IMPLEMENTATION
// =============================================================================

// Global variables for radar AI pipeline
static QueueHandle_t radar_frame_queue = NULL;
static TaskHandle_t producer_task_handle = NULL;
static TaskHandle_t consumer_task_handle = NULL;
static volatile bool radar_ai_active = false;
static volatile uint32_t frames_produced = 0;
static volatile uint32_t frames_consumed = 0;

// FIXED: Use only X-CUBE-AI managed buffers - no manual buffer allocation!
// X-CUBE-AI automatically manages all buffers in activations area: 0xC0000000 + 787200 bytes
// Weights: 0xC0200000 to 0xC0614F0C (4.28MB) - 2MB offset for safety
// Following STM32 standard practice: let X-CUBE-AI handle all memory management

// Test data buffer removed - now using local variables in functions to avoid scope issues

// Static task stacks to avoid heap allocation
// Place in RAM_D2 (288KB) to avoid RAM_D1 overflow (linker section .ram_d2)
__attribute__((section(".ram_d2"))) static StackType_t producer_stack[1024];  // 4KB for radar producer
static StaticTask_t producer_task_buffer;
__attribute__((section(".ram_d2"))) static StackType_t consumer_stack[8192];  // 32KB - FIXED: Matches xTaskCreateStatic size parameter
static StaticTask_t consumer_task_buffer;

void UsbCommunication::startRadarAIPipeline() {
    UsbCommunication* usb = UsbCommunication::getInstance();
    
    if (radar_ai_active) {
        if (usb) usb->sendStatusMessage("RADAR_AI", "Pipeline already running");
        return;
    }
    
    if (usb) usb->sendStatusMessage("RADAR_AI", "Initializing queue-based radar AI pipeline...");
    
    // Create frame queue - 32 frames capacity (30 + buffer)
    radar_frame_queue = xQueueCreate(32, sizeof(RadarFrame_t*));
    if (radar_frame_queue == NULL) {
        if (usb) usb->sendStatusMessage("RADAR_ERROR", "Failed to create frame queue");
        return;
    }
    
    // Initialize state
    radar_ai_active = true;
    frames_produced = 0;
    frames_consumed = 0;
    
    // Create Producer Task (High Priority) - Static allocation
    producer_task_handle = xTaskCreateStatic(
        radarProducerTask,
        "RadarProducer",
        1024,   // Stack size in words - 4KB
        NULL,  // Parameters
        tskIDLE_PRIORITY + 3,  // High priority
        producer_stack,        // Stack buffer
        &producer_task_buffer  // Task buffer
    );
    
    if (producer_task_handle == NULL) {
        if (usb) usb->sendStatusMessage("RADAR_ERROR", "Failed to create producer task");
        radar_ai_active = false;
        return;
    }
    
    // Create Consumer Task (Normal Priority) - Static allocation
    consumer_task_handle = xTaskCreateStatic(
        radarConsumerTask,
        "RadarConsumer", 
        8192,   // Stack size in words - 32KB for safety
        NULL,  // Parameters
        tskIDLE_PRIORITY + 2,  // Normal priority
        consumer_stack,        // Stack buffer
        &consumer_task_buffer  // Task buffer
    );
    
    if (consumer_task_handle == NULL) {
        if (usb) usb->sendStatusMessage("RADAR_ERROR", "Failed to create consumer task");
        radar_ai_active = false;
        return;
    }
    
    if (usb) usb->sendStatusMessage("RADAR_AI", "Queue-based pipeline started successfully");
}

void UsbCommunication::stopRadarAIPipeline() {
    radar_ai_active = false;
    
    // Clean up tasks
    if (producer_task_handle) {
        vTaskDelete(producer_task_handle);
        producer_task_handle = NULL;
    }
    
    if (consumer_task_handle) {
        vTaskDelete(consumer_task_handle);
        consumer_task_handle = NULL;
    }
    
    // Clean up queue
    if (radar_frame_queue) {
        vQueueDelete(radar_frame_queue);
        radar_frame_queue = NULL;
    }
}

// Producer Task - Based on Perform_Single_Frame_Reading_Test algorithm
void UsbCommunication::radarProducerTask(void* parameters) {
    UsbCommunication* usb = UsbCommunication::getInstance();
    
    // Use existing interrupt flags (from radar_adc_test.cpp)
    extern volatile uint8_t g_adcTestSsFlag;
    extern volatile uint8_t g_adcTestIntgClkFlag;
    extern ADC_HandleTypeDef hadc3;
    
    if (usb) usb->sendStatusMessage("RADAR_AI", "Producer task started - waiting for radar signals");
    
    // Memory pool for radar frames (static allocation) - MOVED TO RAM_D2
    __attribute__((section(".ram_d2")))
    static RadarFrame_t frame_pool[40];
    uint8_t pool_index = 0;
    
    while (radar_ai_active) {
        // Get frame from pool
        RadarFrame_t* frame = &frame_pool[pool_index];
        frame->count = 0;
        frame->timestamp = HAL_GetTick();
        frame->frame_number = frames_produced + 1;
        
        // Wait for SS signal to rise (frame start) - like Perform_Single_Frame_Reading_Test
        while (radar_ai_active && !g_adcTestSsFlag) {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        
        if (!radar_ai_active) break;
        
        // Wait for SS signal to fall (data collection start)
        while (radar_ai_active && g_adcTestSsFlag) {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        
        if (!radar_ai_active) break;
        
        // Clear flag and start collecting
        g_adcTestIntgClkFlag = 0;
        
        // Data collection loop - based on Perform_Single_Frame_Reading_Test
        while (radar_ai_active && g_adcTestSsFlag == 0 && frame->count < 688) {
            if (g_adcTestIntgClkFlag) {
                g_adcTestIntgClkFlag = 0;  // Clear flag
                
                HAL_ADC_Start(&hadc3);
                if (HAL_ADC_PollForConversion(&hadc3, 1) == HAL_OK) {
                    int16_t sample = (int16_t)(HAL_ADC_GetValue(&hadc3)) - 32768;
                    frame->samples[frame->count++] = sample;
                } else {
                    frame->samples[frame->count++] = 0;  // Default on error
                }
                HAL_ADC_Stop(&hadc3);
            }
            
            // Small yield to other tasks
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        
        // Frame completed - pad to 688 if needed
        while (frame->count < 688) {
            frame->samples[frame->count++] = 0;  // Zero padding
        }
        
        // Send frame pointer to queue
        RadarFrame_t* frame_ptr = frame;
        if (xQueueSend(radar_frame_queue, &frame_ptr, pdMS_TO_TICKS(10)) == pdPASS) {
            frames_produced++;
            pool_index = (pool_index + 1) % 40;  // Circular pool
            
            if (usb && frames_produced % 5 == 0) {
                char msg[64];
                snprintf(msg, sizeof(msg), "Produced frame %lu/30", frames_produced);
                usb->sendStatusMessage("RADAR_AI", msg);
            }
            
            // Stop after 30 frames
            if (frames_produced >= 30) {
                if (usb) usb->sendStatusMessage("RADAR_AI", "Producer completed - 30 frames collected");
                break;
            }
        } else {
            if (usb) usb->sendStatusMessage("RADAR_WARNING", "Queue full - frame dropped");
        }
    }
    
    // Task cleanup
    producer_task_handle = NULL;
    vTaskDelete(NULL);
}

// Normalize frame function - converted from Python oneri.md
void normalize_frame(int16_t* input, float* output, int count, float target_min = -1000.0f, float target_max = 1000.0f) {
    // Find min and max values
    int16_t min_val = input[0];
    int16_t max_val = input[0];
    
    for (int i = 1; i < count; i++) {
        if (input[i] < min_val) min_val = input[i];
        if (input[i] > max_val) max_val = input[i];
    }
    
    // Normalize to target range
    if (max_val - min_val < 1) {  // Avoid divide by zero
        for (int i = 0; i < count; i++) {
            output[i] = (float)input[i];  // Keep original if no variation
        }
    } else {
        for (int i = 0; i < count; i++) {
            float normalized = (float)(input[i] - min_val) / (max_val - min_val);  // Scale to [0,1]
            output[i] = normalized * (target_max - target_min) + target_min;      // Scale to [target_min, target_max]
        }
    }
}

// Consumer Task - Collects 30 frames, normalizes, runs AI, sends result
void UsbCommunication::radarConsumerTask(void* parameters) {
    UsbCommunication* usb = UsbCommunication::getInstance();
    
    if (usb) usb->sendStatusMessage("RADAR_AI", "Consumer task started - collecting frames for AI");

    // STM32 STANDARD: Create local input buffer for data collection
    // AI function will use X-CUBE-AI managed buffers internally
    // MOVED TO SDRAM to avoid RAM_D2 overflow (82KB buffer, RAM_D2 total was 616KB > 288KB limit)
    __attribute__((section(".sdram_data"))) __attribute__((aligned(32)))
    static float ai_input_buffer[20640];  // Local buffer for frame collection

    // CRITICAL FIX: Clear buffer on first use to prevent garbage data
    // Static buffers may contain residual memory from previous power cycles
    memset(ai_input_buffer, 0, sizeof(ai_input_buffer));

    RadarFrame_t* frame_ptr;
    int collected_frames = 0;
    
    while (radar_ai_active && collected_frames < 30) {
        // Wait for frame from producer
        if (xQueueReceive(radar_frame_queue, &frame_ptr, pdMS_TO_TICKS(1000)) == pdPASS) {
            
            // Copy raw samples to AI buffer (no normalization yet - will be done globally)
            int buffer_offset = collected_frames * 688;
            for (int i = 0; i < 688; i++) {
                ai_input_buffer[buffer_offset + i] = (float)frame_ptr->samples[i];
            }
            
            // DEBUG: Log first frame sample statistics
            if (collected_frames == 0 && usb) {
                int16_t min_sample = frame_ptr->samples[0];
                int16_t max_sample = frame_ptr->samples[0];
                for (int i = 1; i < 688; i++) {
                    if (frame_ptr->samples[i] < min_sample) min_sample = frame_ptr->samples[i];
                    if (frame_ptr->samples[i] > max_sample) max_sample = frame_ptr->samples[i];
                }
                char debug_msg[128];
                snprintf(debug_msg, sizeof(debug_msg), "First frame: min=%d, max=%d, first_5=[%d,%d,%d,%d,%d]",
                        min_sample, max_sample, 
                        frame_ptr->samples[0], frame_ptr->samples[1], frame_ptr->samples[2], 
                        frame_ptr->samples[3], frame_ptr->samples[4]);
                usb->sendStatusMessage("RADAR_DEBUG", debug_msg);
            }
            
            collected_frames++;
            frames_consumed++;
            
            if (usb && collected_frames % 5 == 0) {
                char msg[64];
                snprintf(msg, sizeof(msg), "Consumed frame %d/30", collected_frames);
                usb->sendStatusMessage("RADAR_AI", msg);
            }
            
        } else {
            if (usb) usb->sendStatusMessage("RADAR_WARNING", "Consumer timeout waiting for frame");
        }
    }
    
    if (collected_frames == 30) {
        if (usb) usb->sendStatusMessage("RADAR_AI", "30 frames collected - applying global normalization");
        
        // GLOBAL NORMALIZATION: Find min/max across all 20640 samples
        float global_min = ai_input_buffer[0];
        float global_max = ai_input_buffer[0];
        
        for (int i = 1; i < 20640; i++) {
            if (ai_input_buffer[i] < global_min) global_min = ai_input_buffer[i];
            if (ai_input_buffer[i] > global_max) global_max = ai_input_buffer[i];
        }
        
        // Apply global normalization to [-1000, 1000] range (same as PC model)
        float scale = 2000.0f / (global_max - global_min);
        for (int i = 0; i < 20640; i++) {
            ai_input_buffer[i] = -1000.0f + (ai_input_buffer[i] - global_min) * scale;
        }
        
        if (usb) {
            char norm_msg[128];
            int min_int = (int)(global_min);
            int max_int = (int)(global_max);
            snprintf(norm_msg, sizeof(norm_msg), "Global normalize: raw_min=%d, raw_max=%d", min_int, max_int);
            usb->sendStatusMessage("RADAR_AI", norm_msg);
            usb->sendStatusMessage("RADAR_AI", "Global normalization complete - running AI inference");
        }
        
        // DEBUG: Log AI input statistics after normalization
        if (usb) {
            float min_val = ai_input_buffer[0];
            float max_val = ai_input_buffer[0];
            float sum = 0.0f;
            for (int i = 0; i < 20640; i++) {
                if (ai_input_buffer[i] < min_val) min_val = ai_input_buffer[i];
                if (ai_input_buffer[i] > max_val) max_val = ai_input_buffer[i];
                sum += ai_input_buffer[i];
            }
            float avg = sum / 20640.0f;
            
            // Convert floats to int for printf
            int min_int = (int)(min_val * 1000);
            int max_int = (int)(max_val * 1000);
            int avg_int = (int)(avg * 1000);
            
            char ai_debug[128];
            snprintf(ai_debug, sizeof(ai_debug), "AI input: min=%d.%03d, max=%d.%03d, avg=%d.%03d",
                    min_int/1000, abs(min_int%1000),
                    max_int/1000, abs(max_int%1000),
                    avg_int/1000, abs(avg_int%1000));
            usb->sendStatusMessage("AI_DEBUG", ai_debug);
        }
        
        // Run AI inference with clean implementation
        extern float* ai_fallower_run_inference_direct(const float* input_data);
        float* ai_output = ai_fallower_run_inference_direct(ai_input_buffer);

        if (!ai_output) {
            if (usb) usb->sendStatusMessage("AI_ERROR", "Inference failed");
            return;
        }

        // Send results using direct float formatting
        char result_msg[256];
        snprintf(result_msg, sizeof(result_msg),
                "[%.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f]",
                (double)ai_output[0], (double)ai_output[1], (double)ai_output[2],
                (double)ai_output[3], (double)ai_output[4], (double)ai_output[5],
                (double)ai_output[6]);
        if (usb) usb->sendStatusMessage("AI_RESULT_SUCCESS", result_msg);

    } else {
        if (usb) usb->sendStatusMessage("RADAR_ERROR", "Failed to collect 30 frames for AI");
    }
    
    if (usb) usb->sendStatusMessage("RADAR_AI", "Consumer task completed - pipeline finished");
    
    // Stop pipeline
    radar_ai_active = false;
    
    // Task cleanup
    consumer_task_handle = NULL;
    vTaskDelete(NULL);
}

// REMOVED: Unused byte_swap_float32_array function
// Root cause will be determined by critical_weights_verification() first

// CRITICAL WEIGHTS INTEGRITY VERIFICATION (HIGHEST PRIORITY)
void critical_weights_verification(void) {
    UsbCommunication* usb = UsbCommunication::getInstance();
    float* weights = (float*)0xC0200000;

    // Statistical analysis - neural network weights should be reasonable
    float min_w = FLT_MAX, max_w = -FLT_MAX;
    int valid_count = 0, nan_count = 0, inf_count = 0;
    int extreme_count = 0; // Values outside expected range

    for (int i = 0; i < 1000; i++) { // Sample first 1000 weights
        if (isnan(weights[i])) {
            nan_count++;
        } else if (isinf(weights[i])) {
            inf_count++;
        } else {
            valid_count++;
            if (weights[i] < min_w) min_w = weights[i];
            if (weights[i] > max_w) max_w = weights[i];

            // Neural network weights typically in [-2.0, 2.0]
            if (weights[i] < -10.0f || weights[i] > 10.0f) {
                extreme_count++;
            }
        }
    }

    // CRITICAL: Log these results
    char stats_msg[256];
    snprintf(stats_msg, sizeof(stats_msg),
        "WEIGHTS_ANALYSIS: valid=%d nan=%d inf=%d extreme=%d min=%.6f max=%.6f",
        valid_count, nan_count, inf_count, extreme_count, min_w, max_w);
    if (usb) usb->sendStatusMessage("WEIGHTS_CRITICAL", stats_msg);

    // Sample specific values for pattern analysis
    char samples_msg[256];
    snprintf(samples_msg, sizeof(samples_msg),
        "WEIGHTS_SAMPLES: w[0]=%.6f w[1]=%.6f w[100]=%.6f w[1000]=%.6f",
        weights[0], weights[1], weights[100], weights[1000]);
    if (usb) usb->sendStatusMessage("WEIGHTS_SAMPLES", samples_msg);

    // Endianness corruption detection
    union { float f; uint32_t i; uint8_t b[4]; } test;
    test.f = weights[0];
    char endian_msg[256];
    snprintf(endian_msg, sizeof(endian_msg),
        "ENDIAN_CHECK: float=%.6f hex=0x%08lX bytes=[%02X,%02X,%02X,%02X]",
        test.f, test.i, test.b[0], test.b[1], test.b[2], test.b[3]);
    if (usb) usb->sendStatusMessage("ENDIAN_DEBUG", endian_msg);
}

// SYSTEMATIC DIAGNOSTICS FOR ROOT CAUSE VERIFICATION
void systematic_diagnostics(void) {
    UsbCommunication* usb = UsbCommunication::getInstance();

    // 1. CRITICAL: Weights integrity
    critical_weights_verification();

    // 2. System state verification
    char sys_msg[256];
    snprintf(sys_msg, sizeof(sys_msg),
        "SYSTEM_STATE: heap_free=%d fpu_enabled=%d",
        xPortGetFreeHeapSize(),
        ((SCB->CPACR & 0x00F00000) == 0x00F00000) ? 1 : 0);
    if (usb) usb->sendStatusMessage("SYS_STATE", sys_msg);

    // 3. Memory alignment check
    uint32_t weights_addr = (uint32_t)0xC0200000;

    char align_msg[256];
    snprintf(align_msg, sizeof(align_msg),
        "MEMORY_ALIGN: weights=%s",
        (weights_addr % 32 == 0) ? "OK" : "MISALIGNED");
    if (usb) usb->sendStatusMessage("ALIGN_CHECK", align_msg);
}

// OUTPUT VALIDATION AGAINST PC REFERENCE
void validate_ai_output_accuracy(float* output_data) {
    UsbCommunication* usb = UsbCommunication::getInstance();

    // Expected PC reference values from PC testing
    float expected[7] = {10.958415, -2.6598008, -3.7500937,
                        0.5547441, -3.0386488, -5.321173, -1.7327996};

    // Calculate accuracy
    float total_error = 0.0f;
    int valid_outputs = 0;

    for (int i = 0; i < 7; i++) {
        if (isfinite(output_data[i])) {
            valid_outputs++;
            float error = fabs(output_data[i] - expected[i]);
            float relative_error = error / fabs(expected[i]);
            total_error += relative_error;

            char accuracy_msg[256];
            snprintf(accuracy_msg, sizeof(accuracy_msg),
                "ACCURACY[%d]: got=%.6f expected=%.6f error=%.3f%%",
                i, output_data[i], expected[i], relative_error * 100.0f);
            if (usb) usb->sendStatusMessage("ACCURACY", accuracy_msg);
        }
    }

    float avg_error = (valid_outputs > 0) ? (total_error / valid_outputs) : 1.0f;

    char summary_msg[256];
    snprintf(summary_msg, sizeof(summary_msg),
        "VERIFICATION: valid_outputs=%d avg_error=%.3f%% status=%s",
        valid_outputs, avg_error * 100.0f,
        (avg_error < 0.05f) ? "PASS" : "FAIL"); // 5% tolerance
    if (usb) usb->sendStatusMessage("FINAL_RESULT", summary_msg);
}

// IMPROVED CACHE MANAGEMENT WITH ERROR HANDLING
void ai_inference_with_cache_management(ai_buffer* inputs, ai_buffer* outputs) {
    UsbCommunication* usb = UsbCommunication::getInstance();

    // 1. Ensure input data is in SDRAM (flush cache)
    uint32_t input_addr = (uint32_t)inputs[0].data;
    uint32_t input_size = inputs[0].size;

    if (input_addr >= 0xC0000000 && (input_addr % 32) == 0) { // SDRAM region and aligned
        // Align to 32-byte cache line boundary
        uint32_t aligned_addr = input_addr & ~0x1F;
        uint32_t aligned_size = (input_size + 31) & ~0x1F;

        SCB_CleanDCache_by_Addr((uint32_t*)aligned_addr, aligned_size);
        if (usb) usb->sendStatusMessage("CACHE_PRE", "Input cache cleaned");
    }

    // 2. Memory barriers
    __DSB(); // Data Synchronization Barrier
    __ISB(); // Instruction Synchronization Barrier

    // 3. AI Inference with protected execution
    __disable_irq();  // Critical section during inference
    ai_i32 result = ai_fallower1_run(network, inputs, outputs);
    __enable_irq();

    // 4. Invalidate output cache to see fresh data
    uint32_t output_addr = (uint32_t)outputs[0].data;
    uint32_t output_size = outputs[0].size;

    if (output_addr >= 0xC0000000 && (output_addr % 32) == 0) { // SDRAM region and aligned
        uint32_t aligned_addr = output_addr & ~0x1F;
        uint32_t aligned_size = (output_size + 31) & ~0x1F;

        SCB_InvalidateDCache_by_Addr((uint32_t*)aligned_addr, aligned_size);
        if (usb) usb->sendStatusMessage("CACHE_POST", "Output cache invalidated");
    }

    // 5. Final memory barriers
    __DSB(); // Data Synchronization Barrier
    __ISB(); // Instruction Synchronization Barrier

    char result_msg[128];
    snprintf(result_msg, sizeof(result_msg), "AI_INFERENCE_CACHE: result=%d", (int)result);
    if (usb) usb->sendStatusMessage("AI_RESULT", result_msg);
}

void ai_fallower_init_from_sdram() {
    UsbCommunication* usb = UsbCommunication::getInstance();

    // MANDATORY FIRST STEP: Weights integrity verification
    if (usb) usb->sendStatusMessage("AI_INIT", "Starting weights integrity verification");
    critical_weights_verification();

    // CRITICAL: Verify weights are loaded and valid before initialization
    float* weights_ptr = (float*)0xC0200000;

    // Quick sanity check for uninitialized weights
    if (weights_ptr[0] == 0.0f && weights_ptr[1] == 0.0f && weights_ptr[2] == 0.0f) {
        if (usb) usb->sendStatusMessage("AI_ERROR", "Weights appear uninitialized - load_model_binary required first");
        network = AI_HANDLE_NULL;
        return;
    }

    // Check for common corruption patterns
    uint32_t* word_weights = (uint32_t*)weights_ptr;
    if (word_weights[0] == 0xDEADBEEF || word_weights[0] == 0xFFFFFFFF) {
        if (usb) usb->sendStatusMessage("AI_ERROR", "Weights corrupted - reload model binary");
        network = AI_HANDLE_NULL;
        return;
    }

    if (usb) usb->sendStatusMessage("AI_INIT", "Weight validation passed - proceeding with model initialization");

    // CRITICAL FIX: SDRAM IS CACHEABLE! Must invalidate weights cache
    // NOT_SHAREABLE mode requires explicit cache invalidation for correct data
    // Model weights: 4,280,700 bytes at 0xC0200000
    if (usb) usb->sendStatusMessage("CACHE_DEBUG", "Invalidating weights cache (4.28MB) for NOT_SHAREABLE mode...");

    // Invalidate weights region to ensure CPU reads fresh data from SDRAM
    SCB_InvalidateDCache_by_Addr((uint32_t*)0xC0200000, 4280700);

    // Memory barriers to ensure invalidation completes
    __DSB();
    __ISB();

    // CRITICAL: 4.28MB is HUGE! Need longer delay for complete propagation
    // 10ms was too short - logit values were too low (1.0 instead of 8.7)
    // Increasing to 50ms to ensure ALL weights are fresh from SDRAM
    osDelay(50);  // 50ms for 4.28MB weights invalidation (INCREASED!)

    if (usb) usb->sendStatusMessage("CACHE_DEBUG", "Weights cache invalidation complete (50ms delay)");

    // Minimal logging to avoid stack/memory issues
    if (usb) usb->sendStatusMessage("AI_INIT", "Starting model initialization");

    // Setup memory pointers with proper alignment
    // CRITICAL: X-CUBE-AI requires 4-byte alignment for activations and weights
    ai_handle activations[] = { (ai_handle)0xC0000000 };  // SDRAM activations (aligned)
    ai_handle weights_handles[] = { (ai_handle)0xC0200000 };      // SDRAM weights (2MB offset)

    // CRITICAL FIX: Clear activation buffer before model initialization
    // Reason: Previous runs or garbage data in SDRAM can corrupt inference results
    // Activation buffer size: 790,688 bytes (from generate report)
    if (usb) usb->sendStatusMessage("AI_INIT", "Clearing activation buffer (790KB)...");
    memset((void*)0xC0000000, 0, 790688);
    if (usb) usb->sendStatusMessage("AI_INIT", "Activation buffer cleared successfully");

    // CRITICAL DEBUG: Log the addresses we're passing to X-CUBE-AI
    if (usb) {
        char addr_debug[128];
        snprintf(addr_debug, sizeof(addr_debug),
                "AI_ADDRESSES: activations=0x%08lX, weights=0x%08lX",
                (unsigned long)activations[0], (unsigned long)weights_handles[0]);
        usb->sendStatusMessage("AI_INIT_DEBUG", addr_debug);
    }

    // Create and initialize model with SDRAM memory
    ai_error err = ai_fallower1_create_and_init(&network, activations, weights_handles);

    if (err.type != AI_ERROR_NONE) {
        if (usb) {
            char error_debug[128];
            snprintf(error_debug, sizeof(error_debug),
                    "AI init failed: type=%d, code=%d - Check weights validity", (int)err.type, (int)err.code);
            usb->sendStatusMessage("AI_ERROR", error_debug);
        }
        network = AI_HANDLE_NULL;
        return;
    } else {
        if (usb) usb->sendStatusMessage("AI_SUCCESS", "Model initialized successfully - ready for inference");
    }

    // WEIGHTS DEBUG: Verify weights are at correct SDRAM address
    // CRITICAL FIX: DO NOT call ai_fallower1_data_weights_get() - it tries to copy from FLASH!
    // We load weights via USB Fire Hose directly to SDRAM 0xC0200000
    if (usb && network != AI_HANDLE_NULL) {
        // Directly check SDRAM weights address without calling X-CUBE-AI data functions
        ai_handle expected_weights_addr = (ai_handle)0xC0200000;

        char weights_debug[128];
        snprintf(weights_debug, sizeof(weights_debug),
                "AI_WEIGHTS_LOCATION: Using SDRAM address=0x%08lX (loaded via USB Fire Hose)",
                (unsigned long)expected_weights_addr);
        usb->sendStatusMessage("WEIGHTS_INFO", weights_debug);

        // Verify weights are valid (not zero or garbage)
        float* weights_check = (float*)expected_weights_addr;
        bool weights_valid = (weights_check[0] != 0.0f || weights_check[1] != 0.0f);

        if (weights_valid) {
            usb->sendStatusMessage("WEIGHTS_OK", "Weights present in SDRAM - ready for inference");
        } else {
            usb->sendStatusMessage("WEIGHTS_WARNING", "Weights may be uninitialized");
        }
    }

    if (usb) usb->sendStatusMessage("AI_SUCCESS", "AI model ready");
}

// CACHE MANAGEMENT: STM32H7 Cache Configuration Verification
void verify_cache_configuration(void) {
    UsbCommunication* usb = UsbCommunication::getInstance();

    // Check if D-Cache is enabled
    bool dcache_enabled = (SCB->CCR & SCB_CCR_DC_Msk) != 0;
    bool icache_enabled = (SCB->CCR & SCB_CCR_IC_Msk) != 0;

    char cache_config_msg[256];
    snprintf(cache_config_msg, sizeof(cache_config_msg),
        "CACHE_CONFIG: DCache=%s ICache=%s CacheType=0x%08lX",
        dcache_enabled ? "ENABLED" : "DISABLED",
        icache_enabled ? "ENABLED" : "DISABLED",
        (unsigned long)SCB->CTR);
    if (usb) usb->sendStatusMessage("CACHE_CONFIG", cache_config_msg);

    // Check MPU configuration for SDRAM region
    bool mpu_enabled = (MPU->CTRL & MPU_CTRL_ENABLE_Msk) != 0;
    uint32_t mpu_regions = (MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos;

    char mpu_config_msg[256];
    snprintf(mpu_config_msg, sizeof(mpu_config_msg),
        "MPU_CONFIG: enabled=%s regions=%lu",
        mpu_enabled ? "YES" : "NO", (unsigned long)mpu_regions);
    if (usb) usb->sendStatusMessage("MPU_CONFIG", mpu_config_msg);

    // Cache line size verification (ARM Cortex-M7 = 32 bytes)
    uint32_t cache_line_size = 32;  // Fixed for Cortex-M7
    char cache_line_msg[128];
    snprintf(cache_line_msg, sizeof(cache_line_msg),
        "CACHE_LINE: size=%lu bytes (Cortex-M7 fixed)", (unsigned long)cache_line_size);
    if (usb) usb->sendStatusMessage("CACHE_LINE", cache_line_msg);
}

// SAFE CACHE OPERATIONS: With error handling and bounds checking
bool safe_cache_clean_range(uint32_t addr, uint32_t size, const char* region_name) {
    UsbCommunication* usb = UsbCommunication::getInstance();

    // Validate SDRAM range
    if (addr < 0xC0000000 || addr >= 0xC2000000) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg),
            "CACHE_ERROR: %s addr=0x%08lX outside SDRAM range", region_name, (unsigned long)addr);
        if (usb) usb->sendStatusMessage("CACHE_ERROR", error_msg);
        return false;
    }

    // Check size bounds
    if (size == 0 || size > 32*1024*1024) {  // Max 32MB SDRAM
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg),
            "CACHE_ERROR: %s invalid size=%lu", region_name, (unsigned long)size);
        if (usb) usb->sendStatusMessage("CACHE_ERROR", error_msg);
        return false;
    }

    // Ensure 32-byte alignment
    uint32_t aligned_addr = addr & ~0x1F;
    uint32_t aligned_size = ((size + 31) & ~0x1F);

    // Bounds check after alignment
    if (aligned_addr + aligned_size > 0xC2000000) {
        aligned_size = 0xC2000000 - aligned_addr;
    }

    // Perform cache operation
    SCB_CleanDCache_by_Addr((uint32_t*)aligned_addr, aligned_size);

    char cache_msg[256];
    snprintf(cache_msg, sizeof(cache_msg),
        "CACHE_CLEAN: %s original(0x%08lX,%lu) aligned(0x%08lX,%lu)",
        region_name, (unsigned long)addr, (unsigned long)size,
        (unsigned long)aligned_addr, (unsigned long)aligned_size);
    if (usb) usb->sendStatusMessage("CACHE_CLEAN", cache_msg);

    return true;
}

bool safe_cache_invalidate_range(uint32_t addr, uint32_t size, const char* region_name) {
    UsbCommunication* usb = UsbCommunication::getInstance();

    // Same validation as clean operation
    if (addr < 0xC0000000 || addr >= 0xC2000000) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg),
            "CACHE_ERROR: %s addr=0x%08lX outside SDRAM range", region_name, (unsigned long)addr);
        if (usb) usb->sendStatusMessage("CACHE_ERROR", error_msg);
        return false;
    }

    if (size == 0 || size > 32*1024*1024) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg),
            "CACHE_ERROR: %s invalid size=%lu", region_name, (unsigned long)size);
        if (usb) usb->sendStatusMessage("CACHE_ERROR", error_msg);
        return false;
    }

    uint32_t aligned_addr = addr & ~0x1F;
    uint32_t aligned_size = ((size + 31) & ~0x1F);

    if (aligned_addr + aligned_size > 0xC2000000) {
        aligned_size = 0xC2000000 - aligned_addr;
    }

    SCB_InvalidateDCache_by_Addr((uint32_t*)aligned_addr, aligned_size);

    char cache_msg[256];
    snprintf(cache_msg, sizeof(cache_msg),
        "CACHE_INVALIDATE: %s original(0x%08lX,%lu) aligned(0x%08lX,%lu)",
        region_name, (unsigned long)addr, (unsigned long)size,
        (unsigned long)aligned_addr, (unsigned long)aligned_size);
    if (usb) usb->sendStatusMessage("CACHE_INVALIDATE", cache_msg);

    return true;
}

// REMOVED: Duplicate critical_weights_verification function - using original at line 1548

void verify_input_buffer_integrity(const float* input_data, uint32_t input_size) {
    UsbCommunication* usb = UsbCommunication::getInstance();

    if (!input_data || input_size == 0) {
        if (usb) usb->sendStatusMessage("INPUT_ERROR", "Input buffer is NULL or zero size");
        return;
    }

    // STEP 1: Memory region analysis
    uintptr_t input_addr = (uintptr_t)input_data;
    const char* memory_region = "UNKNOWN";

    if (input_addr >= 0x20000000 && input_addr < 0x20020000) {
        memory_region = "DTCMRAM";
    } else if (input_addr >= 0x24000000 && input_addr < 0x24080000) {
        memory_region = "RAM_D1";
    } else if (input_addr >= 0xC0000000 && input_addr < 0xC2000000) {
        memory_region = "SDRAM";
    }

    char region_msg[256];
    snprintf(region_msg, sizeof(region_msg),
        "INPUT_REGION: addr=0x%08lX size=%lu region=%s 4byte_aligned=%s 32byte_aligned=%s",
        (unsigned long)input_addr, (unsigned long)input_size, memory_region,
        ((input_addr & 0x3) == 0) ? "YES" : "NO",
        ((input_addr & 0x1F) == 0) ? "YES" : "NO");
    if (usb) usb->sendStatusMessage("INPUT_REGION", region_msg);

    // STEP 2: Data range validation
    float min_val = FLT_MAX, max_val = -FLT_MAX;
    uint32_t nan_count = 0, inf_count = 0, zero_count = 0;
    uint32_t in_range_count = 0;  // Expected range [-1000, 1000]

    uint32_t sample_size = (input_size > 1000) ? 1000 : input_size;
    for (uint32_t i = 0; i < sample_size; i++) {
        float val = input_data[i];

        if (isnan(val)) {
            nan_count++;
        } else if (isinf(val)) {
            inf_count++;
        } else {
            if (val == 0.0f) zero_count++;
            if (val >= -1000.0f && val <= 1000.0f) in_range_count++;
            if (val < min_val) min_val = val;
            if (val > max_val) max_val = val;
        }
    }

    char range_msg[256];
    snprintf(range_msg, sizeof(range_msg),
        "INPUT_ANALYSIS: min=%.3f max=%.3f nan=%lu inf=%lu zeros=%lu in_range=%lu/%lu",
        min_val, max_val, (unsigned long)nan_count, (unsigned long)inf_count,
        (unsigned long)zero_count, (unsigned long)in_range_count, (unsigned long)sample_size);
    if (usb) usb->sendStatusMessage("INPUT_ANALYSIS", range_msg);

    // STEP 3: First/last samples for pattern verification
    char samples_msg[256];
    snprintf(samples_msg, sizeof(samples_msg),
        "INPUT_SAMPLES: first=[%.3f,%.3f,%.3f] last=[%.3f,%.3f,%.3f]",
        input_data[0], input_data[1], input_data[2],
        input_data[input_size-3], input_data[input_size-2], input_data[input_size-1]);
    if (usb) usb->sendStatusMessage("INPUT_SAMPLES", samples_msg);

    // STEP 4: Memory corruption check around buffer
    if (input_size >= 4) {
        volatile uint32_t* before = (volatile uint32_t*)((uintptr_t)input_data - 16);
        volatile uint32_t* after = (volatile uint32_t*)((uintptr_t)input_data + input_size * sizeof(float) + 16);

        char corruption_msg[256];
        snprintf(corruption_msg, sizeof(corruption_msg),
            "INPUT_BOUNDARIES: before_16=0x%08lX after_16=0x%08lX buffer_size=%lu_bytes",
            (unsigned long)*before, (unsigned long)*after, (unsigned long)(input_size * sizeof(float)));
        if (usb) usb->sendStatusMessage("INPUT_BOUNDARIES", corruption_msg);
    }
}

// COMPREHENSIVE: Output Buffer Verification (SDRAM)
void verify_output_buffer_integrity(const float* output_data, uint32_t output_size) {
    UsbCommunication* usb = UsbCommunication::getInstance();

    if (!output_data || output_size == 0) {
        if (usb) usb->sendStatusMessage("OUTPUT_ERROR", "Output buffer is NULL or zero size");
        return;
    }

    // STEP 1: Memory region and alignment
    uintptr_t output_addr = (uintptr_t)output_data;
    char region_msg[256];
    snprintf(region_msg, sizeof(region_msg),
        "OUTPUT_REGION: addr=0x%08lX size=%lu SDRAM=%s 4byte_aligned=%s 32byte_aligned=%s",
        (unsigned long)output_addr, (unsigned long)output_size,
        (output_addr >= 0xC0000000 && output_addr < 0xC2000000) ? "YES" : "NO",
        ((output_addr & 0x3) == 0) ? "YES" : "NO",
        ((output_addr & 0x1F) == 0) ? "YES" : "NO");
    if (usb) usb->sendStatusMessage("OUTPUT_REGION", region_msg);

    // STEP 2: Output values analysis
    uint32_t nan_count = 0, inf_count = 0, extreme_count = 0;
    bool has_int32_pattern = false;

    for (uint32_t i = 0; i < output_size; i++) {
        float val = output_data[i];

        if (isnan(val)) {
            nan_count++;
        } else if (isinf(val)) {
            inf_count++;
        } else if (fabs(val) > 1000000.0f) {  // Astronomically large values
            extreme_count++;
        }

        // Check for INT32_MIN/MAX corruption pattern
        if (val == (float)INT32_MIN || val == (float)INT32_MAX) {
            has_int32_pattern = true;
        }
    }

    char analysis_msg[256];
    snprintf(analysis_msg, sizeof(analysis_msg),
        "OUTPUT_ANALYSIS: nan=%lu inf=%lu extreme=%lu int32_pattern=%s",
        (unsigned long)nan_count, (unsigned long)inf_count, (unsigned long)extreme_count,
        has_int32_pattern ? "YES" : "NO");
    if (usb) usb->sendStatusMessage("OUTPUT_ANALYSIS", analysis_msg);

    // STEP 3: Raw output values (all 7 elements for fallower model)
    if (output_size == 7) {
        char raw_msg[256];
        snprintf(raw_msg, sizeof(raw_msg),
            "OUTPUT_RAW: [%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f]",
            output_data[0], output_data[1], output_data[2], output_data[3],
            output_data[4], output_data[5], output_data[6]);
        if (usb) usb->sendStatusMessage("OUTPUT_RAW", raw_msg);

        // Integer scaled version (x1000) for easier reading
        char scaled_msg[256];
        snprintf(scaled_msg, sizeof(scaled_msg),
            "OUTPUT_SCALED: [%d,%d,%d,%d,%d,%d,%d] (x1000)",
            (int)(output_data[0] * 1000), (int)(output_data[1] * 1000),
            (int)(output_data[2] * 1000), (int)(output_data[3] * 1000),
            (int)(output_data[4] * 1000), (int)(output_data[5] * 1000),
            (int)(output_data[6] * 1000));
        if (usb) usb->sendStatusMessage("OUTPUT_SCALED", scaled_msg);
    }
}

// AI Model Inference Function - STM32 Standard: Returns X-CUBE-AI output buffer pointer
// No manual copying - use X-CUBE-AI managed buffers directly
// ============================================================================
// AI INFERENCE FUNCTION - CLEAN IMPLEMENTATION
// Following ST's reference implementation in st_ai_ws_fall128
// ============================================================================
float* ai_fallower_run_inference_direct(const float* input_data) {
    UsbCommunication* usb = UsbCommunication::getInstance();

    // === VALIDATION PHASE ===
    if (network == AI_HANDLE_NULL || !input_data) {
        if (usb) usb->sendStatusMessage("AI_ERROR", "Invalid parameters");
        return nullptr;
    }

    // === GET X-CUBE-AI BUFFERS ===
    // X-CUBE-AI manages all buffers - we just get pointers
    ai_buffer* ai_input = ai_fallower1_inputs_get(network, NULL);
    ai_buffer* ai_output = ai_fallower1_outputs_get(network, NULL);

    if (!ai_input || !ai_output || !ai_input[0].data || !ai_output[0].data) {
        if (usb) usb->sendStatusMessage("AI_ERROR", "X-CUBE-AI buffers not allocated");
        return nullptr;
    }

    // === ACTIVATION BUFFER CLEAR: DISABLED FOR PERFORMANCE ===
    // X-CUBE-AI automatically overwrites activation buffer during inference
    // Memset 790KB wastes significant time on non-cacheable SDRAM
    // REMOVED to improve inference speed

    // uint8_t* activation_buffer = (uint8_t*)0xC0000000;
    // const uint32_t activation_size = 790688;
    // memset(activation_buffer, 0, activation_size);  // DISABLED

    // === DEDICATED INPUT BUFFER (SAFE SDRAM REGION) ===
    // CRITICAL: Input buffer MUST NOT overlap with activations (0xC0000000-0xC00C10A0)
    // or weights (0xC0200000-0xC0614F0C)
    // or real-time buffers (.sdram_data @ 0xC0700000-0xC07F8000)
    // or test data (.sdram_test_data @ 0xC0800000)
    //
    // MEMORY MAP:
    // 0xC0000000-0xC00C10A0: Activation pool (790KB)
    // 0xC0200000-0xC0615E6C: Model weights (4.28MB)
    // 0xC0700000-0xC07F8000: .sdram_data (990KB) - frame_pool, circular_window, ai_input_buffer
    // 0xC0800000-0xC0900000: .sdram_test_data (~1MB)
    // 0xC0900000+: SAFE ZONE for dedicated_input_buffer
    //
    // Input shape: (1, 30, 688, 1) = 20640 float32 elements = 82560 bytes = 0x14280
    // FIX: Moved to 0xC0900000 to avoid ALL overlaps
    static float* dedicated_input_buffer = (float*)0xC0900000;  // Safe region after test data

    // DEBUG: Print buffer addresses
    // DISABLED: USB buffer overflow fix (10+ messages cause 700 bytes > 512 byte USB CDC TX buffer)
    // Root cause: HardFault in 3-task architecture when AI inference triggers
    /*
    if (usb) {
        char debug_msg[256];
        snprintf(debug_msg, sizeof(debug_msg),
            "[INPUT_SAFE] dedicated_input=0x%08lX, test_data=0x%08lX, size=%d",
            (unsigned long)dedicated_input_buffer, (unsigned long)input_data, AI_FALLOWER1_IN_1_SIZE_BYTES);
        usb->sendStatusMessage("DEBUG", debug_msg);

        // Print first 5 input values from FLASH test data
        snprintf(debug_msg, sizeof(debug_msg),
            "[INPUT_SOURCE] test_data[0-4]: %.2f, %.2f, %.2f, %.2f, %.2f",
            input_data[0], input_data[1], input_data[2], input_data[3], input_data[4]);
        usb->sendStatusMessage("DEBUG", debug_msg);
    }
    */

    // Copy test data to dedicated safe SDRAM region
    memcpy(dedicated_input_buffer, input_data, AI_FALLOWER1_IN_1_SIZE_BYTES);

    // DEBUG: Verify copy to dedicated buffer - CHECK FIRST 10 VALUES
    // DISABLED: USB buffer overflow fix
    /*
    if (usb) {
        char debug_msg[256];
        snprintf(debug_msg, sizeof(debug_msg),
            "[INPUT_COPIED] First 5: %.2f, %.2f, %.2f, %.2f, %.2f",
            dedicated_input_buffer[0], dedicated_input_buffer[1], dedicated_input_buffer[2],
            dedicated_input_buffer[3], dedicated_input_buffer[4]);
        usb->sendStatusMessage("DEBUG", debug_msg);

        snprintf(debug_msg, sizeof(debug_msg),
            "[INPUT_COPIED] Next 5: %.2f, %.2f, %.2f, %.2f, %.2f",
            dedicated_input_buffer[5], dedicated_input_buffer[6], dedicated_input_buffer[7],
            dedicated_input_buffer[8], dedicated_input_buffer[9]);
        usb->sendStatusMessage("DEBUG", debug_msg);

        // EXPECTED VALUES (WALK_1622): [271.89, -973.59, -1000.0, 830.03, -827.16, 822.00, ...]
        snprintf(debug_msg, sizeof(debug_msg),
            "[INPUT_VERIFY] Expected first 3 (WALK_1622): 271.89, -973.59, -1000.00");
        usb->sendStatusMessage("DEBUG", debug_msg);
    }
    */

    // Update ai_input buffer to point to dedicated safe buffer
    ai_input[0].data = (ai_handle)dedicated_input_buffer;

    // CRITICAL FIX: Clean the cache for the dedicated input buffer region
    // We just wrote to it (memcpy). We must FLUSH (Clean) to RAM, not Invalidate (Discard).
    // Invalidating here would throw away the memcpy data!
    SCB_CleanDCache_by_Addr((uint32_t*)dedicated_input_buffer, AI_FALLOWER1_IN_1_SIZE_BYTES);


    // DEBUG: Print ai_buffer structure details
    // DISABLED: USB buffer overflow fix
    /*
    if (usb) {
        char debug_msg[256];
        snprintf(debug_msg, sizeof(debug_msg),
            "[AI_BUFFER_CHECK] Using X-CUBE-AI generated shape [%d,%d,%d,%d]",
            (int)ai_input[0].shape.data[0], (int)ai_input[0].shape.data[1],
            (int)ai_input[0].shape.data[2], (int)ai_input[0].shape.data[3]);
        usb->sendStatusMessage("DEBUG", debug_msg);

        snprintf(debug_msg, sizeof(debug_msg),
            "[AI_BUFFER_CHECK] ai_input[0]: data=0x%08lX, size=%lu, format=0x%02lX",
            (unsigned long)ai_input[0].data, (unsigned long)ai_input[0].size, (unsigned long)ai_input[0].format);
        usb->sendStatusMessage("DEBUG", debug_msg);
    }
    */

    // === CACHE COHERENCY: Minimal Invalidation Strategy ===
    // CRITICAL: SDRAM uses Write-Through cache (MPU config)
    // Write-Through means:
    //   - Writes go to BOTH cache AND memory immediately
    //   - Model weights written by Fire Hose are already in memory
    //   - NO invalidation needed for weights! (they're fresh in cache)
    //
    // ONLY invalidate activation buffer (X-CUBE-AI reuses this):
    //   - Activation buffer: 0xC0000000 (790,688 bytes)
    //   - Align to cache line boundary (32 bytes)

    // Invalidate ONLY activation buffer (not entire 32MB!)
    // This reduces overhead from ~300ms to <1ms
    const uint32_t activation_size = 790688;
    const uint32_t cache_line_size = 32;
    const uint32_t aligned_size = ((activation_size + cache_line_size - 1) / cache_line_size) * cache_line_size;

    SCB_InvalidateDCache_by_Addr((uint32_t*)0xC0000000, aligned_size);

    // Memory barrier
    __DSB();
    __ISB();

    // === RUN INFERENCE ===
    // Start timing for pure AI inference (X-CUBE-AI run only)
    uint32_t ai_run_start_tick = HAL_GetTick();

    // DEBUG: Check stack before AI inference (HardFault protection)
    // DISABLED: USB buffer overflow fix
    /*
    UBaseType_t stack_remaining = uxTaskGetStackHighWaterMark(NULL);
    if (usb) {
        char stack_msg[64];
        snprintf(stack_msg, sizeof(stack_msg), "[PRE_AI] Stack free: %lu words", stack_remaining);
        usb->sendStatusMessage("DEBUG", stack_msg);
    }
    */

    // CRITICAL: Disable interrupts during inference to prevent priority inversion
    // taskENTER_CRITICAL();  // DISABLED - causes watchdog timeout

    ai_i32 batch = ai_fallower1_run(network, ai_input, ai_output);

    // taskEXIT_CRITICAL();  // DISABLED

    // Calculate pure AI inference time
    uint32_t ai_run_end_tick = HAL_GetTick();
    uint32_t ai_run_time_ms = ai_run_end_tick - ai_run_start_tick;
    (void)ai_run_time_ms;  // Suppress unused warning - used for debugging

    // DEBUG: Check stack after AI inference
    // DISABLED: USB buffer overflow fix
    /*
    stack_remaining = uxTaskGetStackHighWaterMark(NULL);
    if (usb) {
        char stack_msg[64];
        snprintf(stack_msg, sizeof(stack_msg), "[POST_AI] Stack free: %lu words", stack_remaining);
        usb->sendStatusMessage("DEBUG", stack_msg);
    }
    */

    // Report pure AI run time (MINIMAL LOGGING - only timing)
    // Report pure AI run time (MINIMAL LOGGING - only timing)
    /* SILENCED PER USER REQUEST
    if (usb) {
        char timing_msg[128];
        snprintf(timing_msg, sizeof(timing_msg),
                "AI_RUN_TIME: ai_fallower1_run() took %lu ms (pure inference)",
                (unsigned long)ai_run_time_ms);
        usb->sendStatusMessage("AI_RUN_TIME", timing_msg);
    }
    */

    if (batch != 1) {
        if (usb) {
            ai_error err = ai_fallower1_get_error(network);
            char msg[64];
            snprintf(msg, sizeof(msg), "Inference failed: type=%d code=%d", err.type, err.code);
            usb->sendStatusMessage("AI_ERROR", msg);
        }
        return nullptr;
    }

    // === VERIFY INPUT BUFFER NOT CORRUPTED ===
    // DISABLED: USB buffer overflow fix
    /*
    if (usb) {
        char debug_msg[256];
        float* input_check = (float*)ai_input[0].data;
        snprintf(debug_msg, sizeof(debug_msg),
            "[INPUT_AFTER_INFERENCE] First 3: %.2f, %.2f, %.2f (should be 271.89, -973.59, -1000.00)",
            input_check[0], input_check[1], input_check[2]);
        usb->sendStatusMessage("DEBUG", debug_msg);
    }
    */

    // === DEBUG OUTPUT BUFFER ===
    // DISABLED: USB buffer overflow fix
    /*
    if (usb) {
        char debug_msg[256];
        snprintf(debug_msg, sizeof(debug_msg),
            "[OUTPUT_BUFFER_CHECK] ai_output[0]: data=0x%08lX, size=%lu, format=0x%08lX",
            (unsigned long)ai_output[0].data, (unsigned long)ai_output[0].size, (unsigned long)ai_output[0].format);
        usb->sendStatusMessage("DEBUG", debug_msg);

        // Print raw bytes of output buffer (first 28 bytes = 7 floats)
        uint8_t* raw_bytes = (uint8_t*)ai_output[0].data;
        snprintf(debug_msg, sizeof(debug_msg),
            "[OUTPUT_RAW_BYTES] [0-7]: %02X %02X %02X %02X %02X %02X %02X %02X",
            raw_bytes[0], raw_bytes[1], raw_bytes[2], raw_bytes[3],
            raw_bytes[4], raw_bytes[5], raw_bytes[6], raw_bytes[7]);
        usb->sendStatusMessage("DEBUG", debug_msg);

        // Print as float32
        float* float_ptr = (float*)ai_output[0].data;
        snprintf(debug_msg, sizeof(debug_msg),
            "[OUTPUT_AS_FLOAT32] [0-2]: %.6f, %.6f, %.6f",
            float_ptr[0], float_ptr[1], float_ptr[2]);
        usb->sendStatusMessage("DEBUG", debug_msg);
    }
    */

    // === RETURN OUTPUT POINTER ===
    // X-CUBE-AI manages output buffer - just return pointer
    return (float*)ai_output[0].data;
}

// ============================================================================
// AI WORKER TASK - Dedicated task for heavy AI operations
// Prevents blocking rxTask during model loading and inference
// ============================================================================
void UsbCommunication::aiWorkerTask(void* parameters) {
    UsbCommunication* instance = static_cast<UsbCommunication*>(parameters);
    AICommandMsg cmd;

    // CRITICAL DEBUG: Task started verification
    char heap_msg[128];
    snprintf(heap_msg, sizeof(heap_msg), "AI Worker Task STARTED - heap_free=%d, stack_high_water=%d",
            (int)xPortGetFreeHeapSize(),
            (int)uxTaskGetStackHighWaterMark(NULL));
    instance->sendStatusMessage("AI_WORKER_DEBUG", heap_msg);

    instance->sendStatusMessage("AI_WORKER", "AI Worker Task started - waiting for commands");

    for (;;) {
        // Wait for command from rxTask
        if (xQueueReceive(instance->aiCommandQueue, &cmd, portMAX_DELAY) == pdPASS) {

            // DEBUG: Command received
            char cmd_msg[64];
            snprintf(cmd_msg, sizeof(cmd_msg), "AI Worker received command: %d", (int)cmd.cmd);
            instance->sendStatusMessage("AI_WORKER_DEBUG", cmd_msg);

            switch (cmd.cmd) {
                case AICommand::LOAD_MODEL:
                    instance->sendStatusMessage("AI_WORKER", "Loading model (this may take 60+ seconds)");
                    // Model loading happens via Fire Hose in rxTask (can't move due to USB dependency)
                    // This is just a placeholder for future optimization
                    instance->sendStatusMessage("AI_WORKER", "Model load command acknowledged");
                    break;

                case AICommand::INIT_MODEL:
                    instance->sendStatusMessage("AI_WORKER", "Initializing AI model");
                    {
                        instance->sendStatusMessage("AI_WORKER_DEBUG", "Calling ai_fallower_init_from_sdram()...");

                        // Call AI initialization function
                        extern void ai_fallower_init_from_sdram();
                        ai_fallower_init_from_sdram();

                        instance->sendStatusMessage("AI_WORKER_DEBUG", "ai_fallower_init_from_sdram() returned");

                        // Verify initialization
                        if (network != AI_HANDLE_NULL) {
                            instance->sendStatusMessage("AI_READY", "Model initialized successfully - ready for inference");
                        } else {
                            instance->sendStatusMessage("AI_ERROR", "Model initialization failed - network handle is NULL");
                        }
                    }
                    break;

                case AICommand::VERIFY_MODEL:
                    {
                        instance->sendStatusMessage("AI_WORKER", "Verifying model integrity...");
                        // The verification logic moved from rxTask
                        AI_MONITOR_STEP("Model Weights Verification", 7);
                        AI_START_TIMING("Model Verification");

                        void* model_weights_addr = (void*)0xC0200000;
                        uint32_t expected_size = cmd.data_size; // Use size from command

                        if (!AI_VERIFY_ALIGNMENT(model_weights_addr, 4, "Loaded Model Weights")) {
                            AI_REPORT_ERROR("Model Verification", "Model weights alignment failed");
                            instance->sendStatusMessage("VERIFY_ERROR", "Model weights misaligned in SDRAM");
                            break; // Exit case
                        }

                        AI_VERIFY_CACHE(model_weights_addr, expected_size, "Model Weights Region");
                        AIMonitoring::DumpMemoryRegion(model_weights_addr, expected_size, "Model Weights Start", 32);
                        AIMonitoring::DumpMemoryRegion((uint8_t*)model_weights_addr + expected_size - 32, 32, "Model Weights End", 32);

                        float* weights_analysis = (float*)model_weights_addr;
                        int total_weights = expected_size / sizeof(float);
                        int sample_size = 10000;
                        int nan_count = 0, inf_count = 0, extreme_count = 0;
                        float min_weight = 1000.0f, max_weight = -1000.0f;

                        for (int i = 0; i < sample_size && i < total_weights; i++) {
                            float w = weights_analysis[i];
                            if (isnan(w)) nan_count++;
                            else if (isinf(w)) inf_count++;
                            else if (fabs(w) > 50.0f) extreme_count++;
                            else {
                                if (w < min_weight) min_weight = w;
                                if (w > max_weight) max_weight = w;
                            }
                        }

                        char corruption_report[256];
                        snprintf(corruption_report, sizeof(corruption_report),
                            "WEIGHTS_CORRUPTION_CHECK: sample=%d nan=%d inf=%d extreme=%d min=%.6f max=%.6f",
                            sample_size, nan_count, inf_count, extreme_count, min_weight, max_weight);
                        instance->sendStatusMessage("WEIGHTS_CORRUPTION", corruption_report);

                        uint32_t* word_weights = (uint32_t*)model_weights_addr;
                        int endian_suspect = 0;
                        for (int i = 0; i < 1000; i++) {
                            uint32_t word = word_weights[i];
                            uint32_t reversed = __builtin_bswap32(word);
                            float original_float, reversed_float;
                            memcpy(&original_float, &word, 4);
                            memcpy(&reversed_float, &reversed, 4);
                            if (isfinite(reversed_float) && !isfinite(original_float)) endian_suspect++;
                            else if (isfinite(reversed_float) && isfinite(original_float) &&
                                      fabs(reversed_float) < 10.0f && fabs(original_float) > 100.0f) endian_suspect++;
                        }
                        char endian_report[256];
                        snprintf(endian_report, sizeof(endian_report),
                            "ENDIANNESS_CHECK: suspect_count=%d/1000 (>100 indicates corruption)",
                            endian_suspect);
                        instance->sendStatusMessage("ENDIANNESS_ANALYSIS", endian_report);

                        // ========== CRC32 VERIFICATION ==========
                        // Get metadata with expected CRC
                        extern const Metadata_t* GetMetadata(void);
                        extern uint32_t crc32_stm32(const uint8_t* data, uint32_t length);

                        // DEBUG: Test CRC algorithm with known data
                        const char* test_data = "test";
                        uint32_t test_crc = crc32_stm32((const uint8_t*)test_data, 4);
                        char test_msg[128];
                        snprintf(test_msg, sizeof(test_msg),
                            "CRC_TEST: 'test' = 0x%08X (expected 0x20AA3784)",
                            (unsigned int)test_crc);
                        instance->sendStatusMessage("CRC_DEBUG", test_msg);

                        const Metadata_t* metadata = GetMetadata();
                        if (metadata) {
                            uint32_t expected_crc = metadata->crc32;
                            // CRITICAL FIX: Use expected_size (from fire_hose_bytes_received) which includes chunk padding
                            uint32_t calculated_crc = crc32_stm32((const uint8_t*)model_weights_addr, expected_size);

                            char crc_msg[128];
                            snprintf(crc_msg, sizeof(crc_msg),
                                "CRC32: expected=0x%08X calculated=0x%08X size=%u (with chunk padding)",
                                (unsigned int)expected_crc, (unsigned int)calculated_crc, (unsigned int)expected_size);
                            instance->sendStatusMessage("CRC_VERIFY", crc_msg);

                            if (expected_crc != 0 && calculated_crc != expected_crc) {
                                char error_msg[128];
                                snprintf(error_msg, sizeof(error_msg),
                                    "CRC MISMATCH! Weights corrupted during transfer!");
                                AI_REPORT_ERROR("Model Verification", error_msg);
                                instance->sendStatusMessage("VERIFY_ERROR", error_msg);
                                break; // Exit verification with error
                            } else if (expected_crc == 0) {
                                char baseline_msg[128];
                                snprintf(baseline_msg, sizeof(baseline_msg),
                                    "CRC_BASELINE: No expected CRC, calculated=0x%08X",
                                    (unsigned int)calculated_crc);
                                instance->sendStatusMessage("CRC_INFO", baseline_msg);
                            } else {
                                AI_REPORT_SUCCESS("CRC Verification", "Weights integrity verified - CRC matches");
                            }
                        } else {
                            instance->sendStatusMessage("CRC_INFO", "No metadata available - skipping CRC verification");
                        }

                        AI_REPORT_SUCCESS("Model Verification", "Weights integrity verified");
                        AI_END_TIMING("Model Verification");

                        char msg[128];
                        snprintf(msg, sizeof(msg), "Model weights loaded - %u bytes @ 0x%08lX - ready for ai_init",
                                (unsigned int)expected_size, (unsigned long)model_weights_addr);
                        AI_REPORT_SUCCESS("Model Loading", msg);
                        instance->sendStatusMessage("MODEL_LOADED", msg);
                    }
                    break;

                case AICommand::RUN_INFERENCE:
                    instance->sendStatusMessage("AI_WORKER", "Running AI inference - entering critical section");
                    {
                        if (!cmd.test_data) {
                            instance->sendStatusMessage("AI_ERROR", "No test data provided");
                            break;
                        }

                        // Use mutex to protect AI inference - allows scheduler to run
                        if(xSemaphoreTake(instance->aiMutex, portMAX_DELAY) == pdTRUE)
                        {
                            // Start timing for inference
                            uint32_t inference_start_tick = HAL_GetTick();

                            float* ai_output = ai_fallower_run_inference_direct(cmd.test_data);

                            // Calculate inference time
                            uint32_t inference_end_tick = HAL_GetTick();
                            uint32_t inference_time_ms = inference_end_tick - inference_start_tick;

                            xSemaphoreGive(instance->aiMutex);

                            // Report inference time
                            char timing_msg[128];
                            snprintf(timing_msg, sizeof(timing_msg),
                                    "AI inference time: %lu ms",
                                    (unsigned long)inference_time_ms);
                            instance->sendStatusMessage("INFERENCE_TIME", timing_msg);

                            instance->sendStatusMessage("AI_WORKER", "AI inference completed - critical section exited");

                            if (!ai_output) {
                                instance->sendStatusMessage("AI_ERROR", "Inference failed");
                                break;
                            }

                            // Send results
                            // EXPECTED ONNX (WALK_1622): [-2.344, -4.556, -3.001, -0.650, 0.557, 0.438, 6.075]
                            // Class indices: FALL=0, GETUP=1, GETUPGR=2, SIT=3, STILLMOVE=4, STILLNOACT=5, WALK=6
                            // Expected: WALK class (index 6) should have highest value
                            char result[256];
                            snprintf(result, sizeof(result),
                                    "AI_RESULT_SUCCESS: [%.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f]",
                                    (double)ai_output[0], (double)ai_output[1], (double)ai_output[2],
                                    (double)ai_output[3], (double)ai_output[4], (double)ai_output[5],
                                    (double)ai_output[6]);
                            instance->sendStatusMessage("AI_RESULT_SUCCESS", result);

                            // CRITICAL: Check stack usage after inference
                            UBaseType_t stackRemaining = uxTaskGetStackHighWaterMark(NULL);
                            snprintf(result, sizeof(result),
                                    "AI_STACK_CHECK: high_water=%d bytes (32768 total, %d%% used)",
                                    (int)(stackRemaining * 4),  // Convert words to bytes
                                    (int)(100 - (stackRemaining * 400 / 32768)));
                            instance->sendStatusMessage("DEBUG", result);

                            instance->sendStatusMessage("AI_WORKER", "Results sent successfully");
                        }
                    }
                    break;

                default:
                    instance->sendStatusMessage("AI_ERROR", "Unknown AI command");
                    break;
            }
        }
    }
}

// REMOVED: Redundant test_sdram_weights_safety function
// Replaced by critical_weights_verification() for better diagnostics

// Forward declarations  
void radar_ai_standalone_task(void* parameters);
// Note: radar_ai_capture_frame currently unused - no forward declaration needed

// RADAR AI PIPELINE GLOBALS - TEMPORARY: Smaller buffer to test register command
__attribute__((section(".ram_d2")))
static float g_radar_ai_buffer[100];  // Small test buffer - RAM_D2
static volatile int g_radar_ai_frames_collected = 0;
static volatile bool g_radar_ai_active = false;
static TaskHandle_t radar_ai_task_handle = NULL;  // Task handle for management


void start_radar_ai_pipeline() {
    UsbCommunication* usb = UsbCommunication::getInstance();

    if (usb) usb->sendStatusMessage("RADAR_AI", "Starting standalone radar AI pipeline...");

    // Initialize AI pipeline state
    g_radar_ai_frames_collected = 0;
    g_radar_ai_active = true;
    memset(g_radar_ai_buffer, 0, sizeof(g_radar_ai_buffer));
    
    // Check if task already exists
    if (radar_ai_task_handle != NULL) {
        if (usb) usb->sendStatusMessage("RADAR_ERROR", "Radar AI task already running");
        return;
    }
    
    BaseType_t result = xTaskCreate(
        radar_ai_standalone_task,  // Task function
        "RadarAI",                 // Task name
        1024,                      // Minimal stack size
        NULL,                      // Parameters
        tskIDLE_PRIORITY + 2,      // Lower priority
        &radar_ai_task_handle      // Task handle
    );
    
    if (result != pdPASS) {
        char error_msg[128];
        snprintf(error_msg, sizeof(error_msg), "Failed to create radar AI task - error code: %d", (int)result);
        if (usb) usb->sendStatusMessage("RADAR_ERROR", error_msg);
        g_radar_ai_active = false;
        radar_ai_task_handle = NULL;
    } else {
        if (usb) usb->sendStatusMessage("RADAR_AI", "Task created successfully");
    }
}

// Standalone Radar AI Task - Copies Stream.cpp ADC collection logic
void radar_ai_standalone_task(void* parameters) {
    UsbCommunication* usb = UsbCommunication::getInstance();
    
    // Include radar flags and ADC from Stream.cpp
    extern ADC_HandleTypeDef hadc3;
    extern RadarFlags_t g_radarFlags;  // Use correct type from radar_flags.h
    
    if (usb) usb->sendStatusMessage("RADAR_AI", "Standalone task started - initializing radar collection");

    // Initialize radar flags (copied from Stream.cpp OnEnter)
    g_radarFlags.ssFlag = (HAL_GPIO_ReadPin(RADAR_SS_GPIO_Port, RADAR_SS_Pin) == GPIO_PIN_SET);
    g_radarFlags.intgClkFlag = 0;
    g_radarFlags.lastSSTime = HAL_GetTick();
    g_radarFlags.lastIntgTime = HAL_GetTick();
    
    // Local frame buffer for each frame (reduce size to save stack)
    #define LOCAL_MAX_FRAME_SAMPLES 512
    // MOVED TO RAM_D2 to avoid RAM_D1 overflow
    __attribute__((section(".ram_d2")))
    static int16_t frame_samples[LOCAL_MAX_FRAME_SAMPLES];  // Static to avoid stack
    int sample_count = 0;
    int frames_collected = 0;
    
    if (usb) usb->sendStatusMessage("RADAR_AI", "Waiting for radar SS signal...");
    
    // Use interrupt-driven flags instead of polling (like radar_adc_test.cpp)
    extern volatile uint8_t g_adcTestSsFlag;
    extern volatile uint8_t g_adcTestIntgClkFlag;
    
    uint32_t timeout = HAL_GetTick() + 5000;  // 5 second timeout
    while (HAL_GetTick() < timeout && frames_collected < 30) {
        
        // Use interrupt flags instead of direct GPIO polling
        bool current_ss = g_adcTestSsFlag;
        
        if (!g_radarFlags.ssFlag && current_ss) {
            // SS rising edge - frame starting
            g_radarFlags.ssFlag = current_ss;
            sample_count = 0;
            if (usb && frames_collected % 5 == 0) {
                char msg[64];
                snprintf(msg, sizeof(msg), "Frame %d/30 started", frames_collected + 1);
                usb->sendStatusMessage("RADAR_AI", msg);
            }
        }
        else if (g_radarFlags.ssFlag && !current_ss) {
            // SS falling edge - data collection phase
            g_radarFlags.ssFlag = current_ss;
            uint32_t frame_start_time = HAL_GetTick();
            sample_count = 0;
            
            // Collect samples during this frame using interrupt flags
            while (!current_ss && sample_count < LOCAL_MAX_FRAME_SAMPLES && HAL_GetTick() < (frame_start_time + 1000)) {
                
                // Wait for INTG_CLK interrupt flag (like radar_adc_test.cpp)
                if (g_adcTestIntgClkFlag) {
                    g_adcTestIntgClkFlag = 0; // Clear flag
                    
                    HAL_ADC_Start(&hadc3);
                    if (HAL_ADC_PollForConversion(&hadc3, 1) == HAL_OK) {
                        int16_t sample = (int16_t)(HAL_ADC_GetValue(&hadc3) - 32768);
                        frame_samples[sample_count++] = sample;
                    }
                    HAL_ADC_Stop(&hadc3);
                }
                
                // Update SS status from interrupt flag
                current_ss = g_adcTestSsFlag;
                
                // Small delay to prevent busy waiting
                vTaskDelay(pdMS_TO_TICKS(1));
            }
            
            // Frame completed - convert to float and store in AI buffer
            if (sample_count > 0) {
                int samples_to_copy = (sample_count < 688) ? sample_count : 688;
                
                for (int i = 0; i < samples_to_copy; i++) {
                    int buffer_idx = frames_collected * 688 + i;
                    if (buffer_idx < 20640) {
                        g_radar_ai_buffer[buffer_idx] = (float)(frame_samples[i]) / 32768.0f;
                    }
                }
                
                // Fill remaining with zeros
                for (int i = samples_to_copy; i < 688; i++) {
                    int buffer_idx = frames_collected * 688 + i;
                    if (buffer_idx < 20640) {
                        g_radar_ai_buffer[buffer_idx] = 0.0f;
                    }
                }
                
                frames_collected++;
                
                if (usb) {
                    char progress[64];
                    snprintf(progress, sizeof(progress), "Frame %d/30 collected (%d samples)", 
                            frames_collected, samples_to_copy);
                    usb->sendStatusMessage("RADAR_AI", progress);
                }
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(1));  // Small delay
    }
    
    g_radar_ai_active = false;
    
    if (frames_collected < 30) {
        if (usb) {
            char error_msg[64];
            snprintf(error_msg, sizeof(error_msg), "Timeout - collected only %d/30 frames", frames_collected);
            usb->sendStatusMessage("RADAR_ERROR", error_msg);
        }
        vTaskDelete(NULL);
        return;
    }
    
    if (usb) usb->sendStatusMessage("RADAR_AI", "30 frames collected - reshaping to 30x688 format");
    
    // RESHAPE: Ensure data is exactly 30x688 = 20,640 samples for AI model
    // Current g_radar_ai_buffer might have variable samples per frame
    // We need to reshape/interpolate to fixed 688 samples per frame
    
    // Use g_radar_ai_buffer directly for reshape to save stack memory
    // NOTE: temp_backup removed - was causing buffer overflow (100 vs 20640 elements)
    
    // Skip detailed analysis to save memory - just do simple reshape
    if (usb) usb->sendStatusMessage("RADAR_AI", "Reshaping frames to 30x688 format");
    
    // SIMPLE RESHAPE: Data is already in g_radar_ai_buffer
    // Just ensure exactly 30*688 = 20640 samples
    // No reshape needed if data was collected correctly
    
    // Verify buffer has expected data (simple sanity check)
    bool buffer_valid = false;
    for (int i = 0; i < 100; i++) {
        if (g_radar_ai_buffer[i] != 0.0f) {
            buffer_valid = true;
            break;
        }
    }
    
    if (!buffer_valid) {
        if (usb) usb->sendStatusMessage("RADAR_WARN", "Buffer appears empty - using zeros");
    }
    
    // No actual reshape loop needed - data should be in correct format
    // Just proceed to preprocessing
    
    if (usb) usb->sendStatusMessage("RADAR_AI", "Reshape complete - running AI inference");

    // Reshape operations completed

    // Run AI inference on reshaped data - STM32 standard approach
    extern float* ai_fallower_run_inference_direct(const float* input_data);
    float* ai_output = ai_fallower_run_inference_direct(g_radar_ai_buffer);
    
    // Send results
    if (ai_output) {
        char result_msg[256];
        int val[7];
        for (int i = 0; i < 7; i++) {
            val[i] = (int)(ai_output[i] * 1000.0f);
        }
        
        snprintf(result_msg, sizeof(result_msg), 
                "RADAR_AI_RESULT: [%d.%03d,%d.%03d,%d.%03d,%d.%03d,%d.%03d,%d.%03d,%d.%03d]",
                val[0]/1000, abs(val[0]%1000), val[1]/1000, abs(val[1]%1000),
                val[2]/1000, abs(val[2]%1000), val[3]/1000, abs(val[3]%1000),
                val[4]/1000, abs(val[4]%1000), val[5]/1000, abs(val[5]%1000),
                val[6]/1000, abs(val[6]%1000));
        if (usb) usb->sendStatusMessage("RADAR_AI", result_msg);
    } else {
        if (usb) usb->sendStatusMessage("RADAR_ERROR", "AI inference failed - nullptr returned");
    }
    
    if (usb) usb->sendStatusMessage("RADAR_AI", "Standalone pipeline completed");

    // Clear task handle before deleting
    radar_ai_task_handle = NULL;
    
    vTaskDelete(NULL);
}

// Removed old radar_ai_pipeline_task - using standalone version instead

// Hook function removed - we use standalone radar collection approach
// If needed later, can be re-added with proper FrameBuffer typedef

// FONKSİYON İMPLEMENTASYONLARI
void UsbCommunication::processBasicCommand(const char* buffer) {
    // This function handles all non-STR commands
    // OPTIMIZATION: Remove debug spam during binary transfers
    // During fire hose mode, binary data causes excessive debug messages
    // sendStatusMessage("DEBUG", "Unknown command ignored (not STR format)");
}

void UsbCommunication::processSTRCommand(const char* buffer) {
    // **SAFETY CHECK**: Validate input parameters
    if (!buffer) {
        sendStatusMessage("ERROR", "NULL buffer in processSTRCommand");
        return;
    }

    size_t bufferLen = strlen(buffer);
    if (bufferLen < 5 || bufferLen > 2048) {
        sendStatusMessage("ERROR", "Invalid STR command length");
        return;
    }
    
    // **FİX**: MemManage_Handler önleme - Stack-based parsing, NO HEAP ALLOCATION
    CommandType_t command = CMD_UPDATE_RADAR;
    
    // **KRİTİK FİX**: std::vector heap allocation yerine fixed stack array
    uint8_t regValues[64];  // Max 64 registers - stack safe
    size_t regCount = 0;

    // **KRİTİK FİX**: std::string heap allocation yerine direct C string parsing
    const char* bufferStr = buffer;
    size_t pos = 5; // Skip "<STR>"

    // **FİX**: C string parsing - NO std::string allocation
    while (pos < bufferLen && regCount < 64) {
        // Find "<R" pattern
        const char* start = strstr(bufferStr + pos, "<R");
        if (!start) break;
        
        // Find closing ">"
        const char* end = strchr(start, '>');
        if (!end) break;
        
        // Skip register address parsing - jump to value
        pos = (end - bufferStr) + 1;
        
        // Find next "<R" or end of buffer for value parsing
        const char* nextStart = strstr(bufferStr + pos, "<R");
        const char* valueEnd = nextStart ? nextStart : (bufferStr + bufferLen);
        
        // Extract value string - NO substr(), use direct pointer arithmetic
        size_t valueLen = valueEnd - (bufferStr + pos);
        if (valueLen > 0 && valueLen < 20) {  // Reasonable value length check
            char valueStr[20];
            strncpy(valueStr, bufferStr + pos, valueLen);
            valueStr[valueLen] = '\0';
            
            // Trim whitespace manually
            char* trimmed = valueStr;
            while (*trimmed == ' ' || *trimmed == '\t' || *trimmed == '\r' || *trimmed == '\n') trimmed++;
            
            // Parse hex value
            if (strlen(trimmed) > 0) {
                uint8_t value = (uint8_t)strtol(trimmed, NULL, 16);
                regValues[regCount++] = value;
            }
        }
        
        // Move to next register
        pos = nextStart ? (nextStart - bufferStr) : bufferLen;
    }

    // Debug message in oneri.md format
    char countMsg[64];
    snprintf(countMsg, sizeof(countMsg), "[DEBUG] Parsed %u register values", (unsigned int)regCount);
    sendStatusMessage("DEBUG", countMsg);

    // **CRITICAL FIX**: Use plain C array instead of std::vector to prevent HardFault
    // Forward to StateMachine with C array
    StateMachine* sm = StateMachine::getInstance();
    if (sm) {
        // Create temporary vector for compatibility (if StateMachine expects it)
        std::vector<uint8_t> regValuesVec;
        regValuesVec.reserve(regCount);
        for (size_t i = 0; i < regCount; i++) {
            regValuesVec.push_back(regValues[i]);
        }
        sm->processCommand(command, regValuesVec);
    }
}

