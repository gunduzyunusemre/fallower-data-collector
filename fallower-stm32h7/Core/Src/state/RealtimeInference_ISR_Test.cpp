/**
 * @file RealtimeInference_DMA_Test.cpp
 * @brief Test Mode Implementation - Code Duplication Strategy for Stability
 * 
 * Contains:
 * - Large SDRAM buffer allocation
 * - State lifecycle (OnEnter/OnExit)
 * - DUPLICATED logic from DMA_ProcessingTask to ensure main logic safety
 */

#include "RealtimeInference_ISR_Test.hpp"
#include "StateMachine.hpp"
#include "../UsbCommunication.hpp"
#include "main.h"    // For HAL_CRC, Timers
#include "../radar_flags.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include "cmsis_os.h"
#include "../radar_driver.hpp" // Added include for SPI functions

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================
volatile bool g_isr_test_mode = false;
volatile uint8_t g_isr_test_batch_idx = 0;

#define DMA_TEST_STRIDE 25   // Sliding Window Stride (Shift amount) - UPDATED TO 25


// SDRAM Allocation for 30 batches (~1.2MB) -> Total 6.3MB
// 2. STORAGE ARRAY
// Use SAFE .sdram_test_data section (Mapped to 0xC1000000)
__attribute__((section(".sdram_test_data"))) __attribute__((aligned(32)))
DMABatchBuffer_t g_isr_test_storage_array[DMA_TEST_BATCH_COUNT];

// Local AI Input Buffer for Test Mode (Isolated)
// Use SAFE .sdram_test_data section (Mapped to 0xC1000000 + Offset)
__attribute__((section(".sdram_test_data"))) __attribute__((aligned(32)))
DMAInputData_t g_isr_test_input_buffer;

DMABatchBuffer_t* g_isr_test_storage = g_isr_test_storage_array;

// External Dependencies
extern ADC_HandleTypeDef hadc3;
extern void ISR_Pipeline_Init(void);
extern void ISR_Pipeline_Cleanup(void);
extern volatile bool g_isr_inference_running;
extern volatile bool g_isr_mode_active;
extern float* ai_fallower_run_inference_direct(const float* input_data); // AI inference function
extern uint8_t ISR_GetPredictedClass(const float* output, float* confidence);
extern const char* const ISR_AI_CLASS_NAMES[];
extern uint8_t _ai_activation_start[];
extern uint8_t _ai_activation_end[];

// Reuse AI Input Buffer from main ISR logic (to save SDRAM/Complexity)
extern DMAInputData_t g_isr_ai_input_buffer;

// Task Handle
TaskHandle_t g_isr_test_task_handle = NULL;
extern osThreadId_t nandTaskHandle; // Extern handle for suspension

// ============================================================================
// STATE MACHINE HANDLERS
// ============================================================================

void RealtimeInference_ISR_Test::OnEnter() {
    UsbCommunication* usb = UsbCommunication::getInstance();
    if (usb) usb->sendMessage("[ISR_TEST] Entering Test Mode...\r\n");

    // 0. CRITICAL: Suspend NAND Task to preventing FMC Hardware Conflict
    // FMC controller is shared. Concurrent access causes HardFaults.
    if (nandTaskHandle != NULL) {
        vTaskSuspend((TaskHandle_t)nandTaskHandle);
    }

    // 1. Clear Test Storage
    // SDRAM uses Write-Back, so clean cache after memset
    memset(g_isr_test_storage_array, 0, sizeof(g_isr_test_storage_array));
    SCB_CleanDCache_by_Addr((uint32_t*)g_isr_test_storage_array, sizeof(g_isr_test_storage_array));
    __DSB();

    // 2. Initialize Globals (CRITICAL: Reset Index BEFORE enabling mode)
    g_isr_test_batch_idx = 0;
    __DSB(); // Ensure write completes

    // DIAGNOSTIC: Check Memory Layout (Verify No Overlap)
    if (usb) {
        char layout_msg[256];
        snprintf(layout_msg, sizeof(layout_msg), 
            "[ISR_TEST] MEMORY CHECK:\r\n"
            "  Storage: %p - %p (Size: %lu)\r\n"
            "  TestInputBuf: %p - %p (Size: %lu)\r\n",
            (void*)g_isr_test_storage_array, 
            (void*)((uint8_t*)g_isr_test_storage_array + sizeof(g_isr_test_storage_array)),
            (unsigned long)sizeof(g_isr_test_storage_array),
            (void*)&g_isr_test_input_buffer,
            (void*)((uint8_t*)&g_isr_test_input_buffer + sizeof(g_isr_test_input_buffer)),
            (unsigned long)sizeof(g_isr_test_input_buffer)
        );
        usb->sendMessage(layout_msg);
    }
    g_isr_test_mode = true;
    
    // 3. Initialize Pipeline (Reuse helper)
    ISR_Pipeline_Init();

    // 4. Set Run Flag BEFORE Task Creation (CRITICAL)
    g_isr_inference_running = true;
    g_isr_mode_active = true;

    // 5. Create Test Validation Task
    BaseType_t res = xTaskCreate(
        ISR_TestProcessingTask,
        "ISRTestProc",
        4096,           // 16KB stack
        NULL,
        osPriorityHigh,
        &g_isr_test_task_handle
    );

    if (res != pdPASS) {
        if (usb) usb->sendMessage("[ISR_TEST] ERROR: Task creation failed\r\n");
        return;
    }

    // 6. Start ADC (Reuse helper or direct register access code)
    // We can reuse the init sequence from RealtimeInference_DMA::OnEnter 
    // BUT we must be careful not to create the normal processing task.
    
    // --- COPY OF INIT SEQUENCE (Without Task Creation) ---
    HAL_ADC_Stop(&hadc3);
    uint32_t timeout = 10000;
    while ((hadc3.Instance->CR & ADC_CR_ADSTART) && timeout--);

    // Calibration
    if (HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET, ADC_DIFFERENTIAL_ENDED) != HAL_OK) {
        if (usb) usb->sendMessage("[ISR_TEST] Warning: ADC calibration failed\r\n");
    }

    // Enable ADC
    hadc3.Instance->ISR = ADC_ISR_ADRDY;
    hadc3.Instance->CR |= ADC_CR_ADEN;
    timeout = 10000;
    while (!(hadc3.Instance->ISR & ADC_ISR_ADRDY) && timeout--);

    if (timeout == 0) {
        if (usb) usb->sendMessage("[ISR_TEST] ERROR: ADC not ready\r\n");
        return;
    }

    if (usb) usb->sendMessage("[ISR_TEST] Collecting 30 batches... (Wait ~30s)\r\n");
}

void RealtimeInference_ISR_Test::OnExit() {
    g_isr_test_mode = false;
    g_isr_mode_active = false;
    g_isr_inference_running = false;

    if (g_isr_test_task_handle != NULL) {
        vTaskDelete(g_isr_test_task_handle);
        g_isr_test_task_handle = NULL;
    }

    ISR_Pipeline_Cleanup();

    // Resume NAND Task
    if (nandTaskHandle != NULL) {
        vTaskResume((TaskHandle_t)nandTaskHandle);
    }
}

void RealtimeInference_ISR_Test::HandleEvent(Event* event, void* args) {
    (void)event;
    (void)args;
}

// ============================================================================
// TEST PROCESSING TASK -- DUPLICATED LOGIC
// ============================================================================

void ISR_TestProcessingTask(void* params) {
    (void)params;
    UsbCommunication* usb = UsbCommunication::getInstance();

    // Wait until collection is complete
    // ISR will set g_isr_inference_running = false when done
    uint32_t collection_start = HAL_GetTick();
    while (g_isr_inference_running) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    uint32_t collection_end = HAL_GetTick();
    
    // Explicit 50ms Delay as requested
    vTaskDelay(pdMS_TO_TICKS(50));

    if (usb) {
        char msg[128];
        snprintf(msg, sizeof(msg), "[ISR_TEST] Collection Complete. Time: %lu ms. Starting Process...\r\n", 
                 (collection_end - collection_start));
        usb->sendMessage(msg);
    }

    // ========================================================================
    // PHASE 1: DATA EXPORT (Send Raw Data to PC)
    // ========================================================================
    SendRawDataToPC();

    // ========================================================================
    // PHASE 2: MULTI-STRIDE INFERENCE TESTING
    // ========================================================================
    
    // Test 1: Stride 5
    RunInferenceLoop(5, "TEST_S5");

    // Test 2: Stride 10
    RunInferenceLoop(10, "TEST_S10");

    // Test 3: Stride 15
    RunInferenceLoop(15, "TEST_S15");

    if (usb) usb->sendMessage("[ISR_TEST] TEST COMPLETE. Returning to IDLE.\r\n");

    // Auto-exit test mode
    StateMachine* sm = StateMachine::getInstance();
    if (sm) {
        sm->changeState(STATE_IDLE);
    }

    vTaskDelete(NULL);
}

// ============================================================================
// HELPER IMPLEMENTATIONS
// ============================================================================

uint16_t ISR_Test_CalculateCRC16(uint8_t* data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) crc = (crc >> 1) ^ 0xA001;
            else crc >>= 1;
        }
    }
    return crc;
}

void SendRawDataToPC(void) {
    UsbCommunication* usb = UsbCommunication::getInstance();
    if (!usb) return;

    usb->sendMessage("[ISR_TEST] DATA TRANSFER START\r\n");
    vTaskDelay(pdMS_TO_TICKS(100)); // Allow PC to prepare

    // CRITICAL FIX: Invalidate D-Cache before reading from SDRAM
    // Ensures CPU reads fresh data from SDRAM, not stale cache lines
    SCB_InvalidateDCache_by_Addr((uint32_t*)g_isr_test_storage_array, sizeof(g_isr_test_storage_array));
    __DSB();

    // Buffer for packet construction - STATIC, ALIGNED, and DOUBLE BUFFERED
    // Double buffering ensures we don't overwrite the buffer while USB DMA is sending it.
    static uint8_t packet_buffers[2][2048] __attribute__((aligned(32))); 
    uint8_t buf_idx = 0;
    
    // uint32_t packet_size = 20 + (DMA_FRAME_SAMPLES * sizeof(int16_t)) + 5; // Unused
    uint32_t total_frames = DMA_TEST_BATCH_COUNT * DMA_WINDOW_FRAMES; // 900 frames

    for (uint32_t i = 0; i < total_frames; i++) {
        // Select buffer
        uint8_t* current_buffer = packet_buffers[buf_idx];

        uint32_t batch_idx = i / DMA_WINDOW_FRAMES;
        uint32_t frame_idx = i % DMA_WINDOW_FRAMES;
        
        DMABatchBuffer_t* batch = &g_isr_test_storage_array[batch_idx];

        // 1. Header
        memcpy(current_buffer, "<DTA>", 4);

        // 2. Metadata
        uint32_t* pMeta = (uint32_t*)(current_buffer + 4);
        pMeta[0] = i; // Global Frame ID
        pMeta[1] = batch->timestamps[frame_idx]; // Capture Start Time
        pMeta[2] = 0; // Duration (approx)

        uint16_t* pCount = (uint16_t*)(current_buffer + 16);
        *pCount = batch->sample_counts[frame_idx];  

        // 3. CRC Calculation (Header + Meta)
        uint16_t crc = ISR_Test_CalculateCRC16(current_buffer + 4, 14);
        *(uint16_t*)(current_buffer + 18) = crc;

        // 4. Data Payload
        // Use actual sample count to avoid sending garbage/zeros at the end
        memcpy(current_buffer + 20, batch->frames[frame_idx], batch->sample_counts[frame_idx] * sizeof(int16_t));

        // 5. End Marker
        memcpy(current_buffer + 20 + (batch->sample_counts[frame_idx] * sizeof(int16_t)), "<END>", 5);

        // CRITICAL FIX: Flush Packet Buffer from Cache to RAM for USB DMA
        SCB_CleanDCache_by_Addr((uint32_t*)current_buffer, 2048); 
        __DSB();

        // Send Packet
        // Recalculate packet size based on dynamic count
        uint32_t actual_packet_size = 20 + (batch->sample_counts[frame_idx] * sizeof(int16_t)) + 5;
        usb->sendData(current_buffer, actual_packet_size);
        
        // Toggle buffer
        buf_idx = !buf_idx;

        // Flow control
        if (i % 10 == 0) {
            vTaskDelay(pdMS_TO_TICKS(1)); 
        }
    }

    // No free needed
    // vPortFree(packet_buffer);
    vTaskDelay(pdMS_TO_TICKS(100));
    usb->sendMessage("[ISR_TEST] DATA TRANSFER COMPLETE\r\n");
    vTaskDelay(pdMS_TO_TICKS(100));
}



void RunInferenceLoop(uint8_t stride, const char* label) {
    UsbCommunication* usb = UsbCommunication::getInstance();
    if (!usb) return;

    // CRITICAL FIX: Invalidate D-Cache before reading from SDRAM
    SCB_InvalidateDCache_by_Addr((uint32_t*)g_isr_test_storage_array, sizeof(g_isr_test_storage_array));
    __DSB();

    char start_msg[64];
    snprintf(start_msg, sizeof(start_msg), "[%s] Stride: %d Start...\r\n", label, stride);
    usb->sendMessage(start_msg);

    // Verify total frames (Should be 900)
    uint32_t total_frames = DMA_TEST_BATCH_COUNT * DMA_WINDOW_FRAMES;
    uint32_t window_count = 0;

    // 30 batches * 30 frames/batch = 900
    if (total_frames == 0 || total_frames > 2000) {
        usb->sendMessage("[EMB_ERROR] Invalid Total Frames count!\r\n");
        return;
    }

    // Safety: Calculate max safe start index
    uint32_t max_start_index = (total_frames >= DMA_WINDOW_FRAMES) ? (total_frames - DMA_WINDOW_FRAMES) : 0;

    for (uint32_t start_frame = 0; start_frame <= max_start_index; start_frame += stride) {

        // Safety Break: Prevent infinite loop or overflow
        if (window_count > 200) {
             usb->sendMessage("[EMB_ERROR] Window Count Limit Exceeded! Force Break.\r\n");
             break;
        }
        
        // STREAM PROCESSING OPTIMIZATION:
        // Read directly from g_isr_test_storage_array -> Process -> g_isr_test_input_buffer
        
        float temp_frame[DMA_FRAME_SAMPLES];

        for (uint16_t f = 0; f < DMA_WINDOW_FRAMES; f++) {
            // 1. Locate Source Frame
            uint32_t start_frame_abs = start_frame + f;
            uint32_t storage_batch_idx = start_frame_abs / DMA_WINDOW_FRAMES;
            uint32_t storage_frame_idx = start_frame_abs % DMA_WINDOW_FRAMES;

            DMABatchBuffer_t* src_batch = &g_isr_test_storage_array[storage_batch_idx];
            int16_t* src_frame_data = src_batch->frames[storage_frame_idx];
            uint16_t count = src_batch->sample_counts[storage_frame_idx];
            
            if (count == 0) count = 1;

            // 2. Preprocess Frame (Scale + Pad + Normalize)
            // Pass 1: Scale + find min/max
            float frame_min = 1e9f;
            float frame_max = -1e9f;

            for (uint16_t s = 0; s < DMA_FRAME_SAMPLES; s++) {
                int16_t raw = src_frame_data[s];

                float val = floorf((float)raw * 0.0625f);
                temp_frame[s] = val;
                if (val < frame_min) frame_min = val;
                if (val > frame_max) frame_max = val;
            }

            // Pass 2: Normalize
            float range = frame_max - frame_min;
            if (range < 1e-6f) range = 1.0f;

            for (uint16_t s = 0; s < DMA_FRAME_SAMPLES; s++) {
                float norm = (temp_frame[s] - frame_min) / range;
                g_isr_test_input_buffer.data[f][s] = DMA_NORM_MIN + (norm * (DMA_NORM_MAX - DMA_NORM_MIN));
            }


        }
        
        g_isr_test_input_buffer.batch_id = window_count;

        // Cache Maintenance for TEST Input Buffer
        SCB_CleanDCache_by_Addr((uint32_t*)&g_isr_test_input_buffer, sizeof(DMAInputData_t));
        __DSB();



        // AI Inference
        __DSB();
        uint32_t activation_size = (uint32_t)_ai_activation_end - (uint32_t)_ai_activation_start;
        if (activation_size > 0 && activation_size < 2*1024*1024) {
             SCB_CleanInvalidateDCache_by_Addr((uint32_t*)_ai_activation_start, activation_size);
             __DSB();
        }

        uint32_t start_time = HAL_GetTick();
        float* output = ai_fallower_run_inference_direct((const float*)g_isr_test_input_buffer.data);
        uint32_t end_time = HAL_GetTick();
        uint32_t duration = end_time - start_time;

        // Reporting
        if (output && usb) {
            float confidence;
            uint8_t predicted_class = ISR_GetPredictedClass(output, &confidence);
            const char* class_name = ISR_AI_CLASS_NAMES[predicted_class];

            // Timestamp Calc: 30 frames = 2.0s (Corrected Logic)
            // Stride 5 (5 frames) = 5 * (2/30) = 0.33s
            float start_sec = (float)start_frame * (2.0f / 30.0f);
            float end_sec = start_sec + 2.0f;

            char msg[512];
            // Format: [EMB_S5] 0.33-2.33 Win1 ...
            // Using %.2f to match user request (0.33, 0.66)
            // Use logical check to print integers cleanly if needed?
            // User examples: "0-2", "1-3", "0.33-2.33".
            // %.2g might be ideal but %.2f is safer for embedded snprintf support.
            // Using %.2f for consistency.
            snprintf(msg, sizeof(msg),
                "[EMB_%s] %.2f-%.2f Win%lu (%lu-%lu):[%.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f] ->%s(%d%%) Time:%lums\r\n",
                (stride == 5 ? "S5" : (stride == 10 ? "S10" : "S15")),
                start_sec, end_sec,
                (unsigned long)window_count,
                (unsigned long)start_frame,
                (unsigned long)(start_frame + DMA_WINDOW_FRAMES - 1),
                output[0], output[1], output[2], output[3], output[4], output[5], output[6],
                class_name, (int)confidence,
                duration
            );
            usb->sendMessage(msg);

            // SPI TRANSFER TO ESP32 (Added in Phase 2)
            SPI_Send_Inference_Result(predicted_class, confidence);
        }
        window_count++;

        // CRITICAL FIX: Increased delay to 50ms to prevent USB Starvation / Watchdog Trigger
        // Inference takes ~875ms, which is very long. We must yield significantly
        // to let IDLE task (Watchdog) and USB tasks run.
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
