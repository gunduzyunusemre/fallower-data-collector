/**
 * @file RealtimeInference_DMA.cpp
 * @brief ISR-Based Real-time Radar AI Inference (Zero Context Switch)
 *
 * Architecture:
 * =============
 *   [EXTI ISR] ──direct──> [ADC Read in ISR] ──> [Batch Buffer (RAM_D1)]
 *                                                        │
 *                                              30 frames complete
 *                                                        │
 *                                                        ▼
 *                                              [xQueueSendFromISR]
 *                                                        │
 *                                                        ▼
 *                                              [ProcessingTask]
 *                                                   │
 *                                         Preprocess + AI Inference
 *                                                   │
 *                                                   ▼
 *                                              [USB Output]
 *
 * Key Optimization vs 3Task:
 * - 3Task: ISR → xTaskNotifyFromISR → Context Switch → Task → HAL_ADC_Start → Poll
 *          = ~20,160 context switches per batch (672 samples × 30 frames)
 *
 * - ISR Mode: ISR → Direct ADC Register Access → Store
 *             = 0 context switches per batch!
 *
 * Reference: Stream.cpp and RealtimeInference_3Task.cpp (proven working)
 *
 * @author Ethem
 * @date 2025
 */

#include "RealtimeInference_ISR.hpp"
#include "StateMachine.hpp"
#include "../UsbCommunication.hpp"
#include <cstring>
#include <cmath>
#include <stdio.h>
#include <climits>
#include "cmsis_os.h"
#include "main.h"
#include "cache_utils.hpp"
#include "../radar_flags.h"

// ============================================================================
// EXTERNAL DEPENDENCIES
// ============================================================================

extern float* ai_fallower_run_inference_direct(const float* input_data);
extern ADC_HandleTypeDef hadc3;
extern RadarFlags_t g_radarFlags;

extern uint8_t _ai_activation_start[];
extern uint8_t _ai_activation_end[];

// ============================================================================
// MEMORY ALLOCATION - SDRAM for large buffers (Write-Back cache policy)
// ============================================================================

// SDRAM: Triple Buffer for safe ISR→Task handoff
// Triple buffer ensures: 1 for ISR write, 1 in queue, 1 for Task read
// NOTE: SDRAM is Write-Back, so cache clean/invalidate is required
__attribute__((section(".sdram_data"))) __attribute__((aligned(32)))
DMABatchBuffer_t g_isr_batch_buffers[DMA_BATCH_BUFFER_COUNT];

// SDRAM: AI Input Buffer (only Task accesses, not ISR)
// Placed in SDRAM "Hot Zone" (0xC0700000) with Write-Back Write-Allocate cache policy
__attribute__((section(".sdram_data"))) __attribute__((aligned(32)))
DMAInputData_t g_isr_ai_input_buffer;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

ISR_PipelineStats_t g_isr_pipeline_stats = {0};
volatile bool g_isr_inference_running = false;

TaskHandle_t g_isr_producer_task_handle = NULL;  // Not used (ISR handles collection)
TaskHandle_t g_isr_processing_task_handle = NULL;
QueueHandle_t g_isr_batch_queue = NULL;

// ISR State (all volatile for ISR access)
volatile uint8_t g_isr_active_batch_idx = 0;
volatile uint8_t g_isr_current_frame_idx = 0;
volatile bool g_isr_frame_in_progress = false;
volatile bool g_isr_batch_ready = false;

// ============================================================================
// TEST MODE HOOKS (External)
// ============================================================================
extern volatile bool g_isr_test_mode;
extern volatile uint8_t g_isr_test_batch_idx;
extern DMABatchBuffer_t* g_isr_test_storage;
#define DMA_TEST_BATCH_COUNT 30

// SDRAM: Temporary buffer for MTI filtered data (82KB)
__attribute__((section(".sdram_data"))) __attribute__((aligned(32)))
static float s_isr_mti_filtered_window[DMA_WINDOW_FRAMES][DMA_FRAME_SAMPLES];

// ============================================================================
// HELPERS
// ============================================================================

/**
 * @brief Apply MTI Filter (Differential Moving Average)
 * Matches Python Implementation exactly
 */
static void ISR_ApplyMTIFilter(const int16_t input_window[DMA_WINDOW_FRAMES][DMA_FRAME_SAMPLES], 
                              float output_window[DMA_WINDOW_FRAMES][DMA_FRAME_SAMPLES]) {
    // Initial frames that are NOT filtered should be zeroed
    memset(output_window, 0, MTI_REF_AVG_NUM * DMA_FRAME_SAMPLES * sizeof(float));

    for (int i = MTI_REF_AVG_NUM; i < DMA_WINDOW_FRAMES; i++) {
        for (int b = 0; b < DMA_FRAME_SAMPLES; b++) {
            float ref_sum = 0;
            for (int k = i - MTI_REF_AVG_NUM; k < i; k++) {
                ref_sum += (float)input_window[k][b];
            }
            float ref_avg = ref_sum / MTI_REF_AVG_NUM;

            float tag_sum = 0;
            for (int k = i - MTI_TAG_AVG_NUM; k < i; k++) {
                tag_sum += (float)input_window[k][b];
            }
            float tag_avg = tag_sum / MTI_TAG_AVG_NUM;

            output_window[i][b] = tag_avg - ref_avg;
        }
    }
}


// Internal state
static volatile uint16_t s_isr_sample_count = 0;
static volatile bool s_isr_buffer_ready = true;      // FIX: Prevent race during buffer switch
static volatile bool s_isr_first_frame_synced = false; // FIX: Frame sync like 3Task
static volatile uint32_t s_isr_batch_id_counter = 0;

// ============================================================================
// STATIC TASK MEMORY - DTCMRAM (Fastest, Zero-Wait-State, Isolated)
// ============================================================================
// DTCMRAM (128KB total, ~64KB used by system stack)
// Benefits:
// - Zero wait-state access (fastest possible)
// - Cache bypass (no coherency issues)
// - Isolated from RAM_D1 congestion (3Task buffers)
// - Reserved for critical real-time tasks
__attribute__((section(".dtcmram"))) static StackType_t s_isr_processing_stack[4096];
__attribute__((section(".dtcmram"))) static StaticTask_t s_isr_processing_tcb;

__attribute__((section(".dtcmram"))) static StaticQueue_t s_isr_batch_queue_struct;
__attribute__((section(".dtcmram"))) static uint8_t s_isr_batch_queue_storage[DMA_BATCH_BUFFER_COUNT * sizeof(DMABatchBuffer_t*)];

// Unused but required by header
DMAFrameBuffer_t g_isr_frame_buffer;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void ISR_Pipeline_Init(void) {
    // Clear all buffers
    memset(g_isr_batch_buffers, 0, sizeof(g_isr_batch_buffers));
    memset(&g_isr_ai_input_buffer, 0, sizeof(g_isr_ai_input_buffer));
    memset(&g_isr_pipeline_stats, 0, sizeof(g_isr_pipeline_stats));

    // SDRAM uses Write-Back cache, so cache clean is required after memset
    Safe_SCB_CleanDCache_by_Addr((uint32_t*)g_isr_batch_buffers, sizeof(g_isr_batch_buffers));
    Safe_SCB_CleanDCache_by_Addr((uint32_t*)&g_isr_ai_input_buffer, sizeof(g_isr_ai_input_buffer));

    // Initialize buffer metadata
    for (int i = 0; i < DMA_BATCH_BUFFER_COUNT; i++) {
        g_isr_batch_buffers[i].buffer_index = i;
        g_isr_batch_buffers[i].frame_count = 0;
        g_isr_batch_buffers[i].batch_id = 0;
    }

    // Clear MTI buffer
    memset(s_isr_mti_filtered_window, 0, sizeof(s_isr_mti_filtered_window));
    Safe_SCB_CleanDCache_by_Addr((uint32_t*)s_isr_mti_filtered_window, sizeof(s_isr_mti_filtered_window));

    // Reset state
    g_isr_active_batch_idx = 0;
    g_isr_current_frame_idx = 0;
    g_isr_frame_in_progress = false;
    g_isr_batch_ready = false;
    s_isr_sample_count = 0;
    s_isr_buffer_ready = true;
    s_isr_first_frame_synced = false;
    s_isr_batch_id_counter = 0;

    // Stats
    g_isr_pipeline_stats.min_inference_time_ms = UINT32_MAX;
    g_isr_pipeline_stats.max_inference_time_ms = 0;
}

void ISR_Pipeline_Cleanup(void) {
    g_isr_inference_running = false;
    s_isr_buffer_ready = false;

    // Stop ADC properly
    if (hadc3.Instance->CR & ADC_CR_ADSTART) {
        hadc3.Instance->CR |= ADC_CR_ADSTP;
        volatile uint32_t timeout = 10000;
        while ((hadc3.Instance->CR & ADC_CR_ADSTP) && timeout--);
    }

    hadc3.Instance->CR |= ADC_CR_ADDIS;
    volatile uint32_t timeout = 10000;
    while ((hadc3.Instance->CR & ADC_CR_ADEN) && timeout--);

    hadc3.Instance->ISR = ADC_ISR_ADRDY | ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR;

    if (g_isr_processing_task_handle) {
        vTaskDelete(g_isr_processing_task_handle);
        g_isr_processing_task_handle = NULL;
    }

    if (g_isr_batch_queue) {
        vQueueDelete(g_isr_batch_queue);
        g_isr_batch_queue = NULL;
    }

    ISR_RestoreADC();
}

bool ISR_ConfigureADC(void) {
    // Not used - we use direct register access
    return true;
}

void ISR_RestoreADC(void) {
    HAL_ADC_Stop_DMA(&hadc3);
    HAL_ADC_Stop(&hadc3);
    for (volatile int i = 0; i < 1000; i++);
}

uint8_t ISR_GetPredictedClass(const float* output, float* confidence) {
    if (!output) {
        *confidence = 0.0f;
        return ISR_AI_CLASS_COUNT - 1;
    }

    uint8_t max_idx = 0;
    float max_val = output[0];

    for (uint8_t i = 1; i < ISR_AI_CLASS_COUNT; i++) {
        if (output[i] > max_val) {
            max_val = output[i];
            max_idx = i;
        }
    }

    float sum = 0.0f;
    for (uint8_t i = 0; i < ISR_AI_CLASS_COUNT; i++) {
        sum += expf(output[i] - max_val);
    }

    if (sum > 0.0f) {
        *confidence = (1.0f / sum) * 100.0f;
        if (*confidence > 99.0f) *confidence = 99.0f;
        if (*confidence < 1.0f) *confidence = 1.0f;
    } else {
        *confidence = 50.0f;
    }

    return max_idx;
}

// ============================================================================
// ISR HANDLERS - Called from radar_adc_test.cpp
// These mirror the logic from RadarProducerTask_3Task but run directly in ISR
// ============================================================================

/**
 * @brief Handle SS_LOW (frame start) - mirrors 3Task SS_LOW handling
 */
void ISR_HandleSSLow(void) {
    if (!g_isr_inference_running) return;
    if (!s_isr_buffer_ready) return;  // Buffer switch in progress
    if (!s_isr_first_frame_synced) return;  // Not yet synced
    if (g_isr_current_frame_idx >= DMA_WINDOW_FRAMES) return;

    // CRITICAL FIX: Clear frame to prevent stale data
    // This is the KEY difference from 3Task that was causing accuracy issues
    DMABatchBuffer_t* batch = &g_isr_batch_buffers[g_isr_active_batch_idx];
    memset(batch->frames[g_isr_current_frame_idx], 0, sizeof(batch->frames[0]));  // 1376 bytes (~5µs)

    // Reset sample counter (like 3Task Line 479)
    if (s_isr_sample_count == 0) {
        batch->timestamps[g_isr_current_frame_idx] = HAL_GetTick();
    }

    g_isr_frame_in_progress = true;
    g_isr_pipeline_stats.isr_transfers_started++;
}

/**
 * @brief Handle SS_HIGH (frame end) - mirrors 3Task SS_HIGH handling
 */
void ISR_HandleSSHigh(void) {
    if (!g_isr_inference_running) return;

    // First SS_HIGH after start = sync point (like 3Task Phase 1)
    if (!s_isr_first_frame_synced) {
        s_isr_first_frame_synced = true;
        s_isr_sample_count = 0;
        g_isr_current_frame_idx = 0;
        return;
    }

    if (!s_isr_buffer_ready) return;

    // Only process if we have samples (like 3Task Line 412)
    if (s_isr_sample_count > 0 && g_isr_current_frame_idx < DMA_WINDOW_FRAMES) {
        DMABatchBuffer_t* batch = &g_isr_batch_buffers[g_isr_active_batch_idx];

        // Record frame metadata
        batch->sample_counts[g_isr_current_frame_idx] = s_isr_sample_count;
        batch->timestamps[g_isr_current_frame_idx] = HAL_GetTick();

        g_isr_current_frame_idx++;
        s_isr_sample_count = 0;  // Reset for next frame
        g_isr_pipeline_stats.total_frames_collected++;
        g_isr_pipeline_stats.isr_transfers_completed++;

        // Check if batch complete (30 frames)
        if (g_isr_current_frame_idx >= DMA_WINDOW_FRAMES) {
            batch->frame_count = g_isr_current_frame_idx;
            batch->batch_id = ++s_isr_batch_id_counter;

            // ================================================================
            // TEST MODE DIVERSION
            // ================================================================
            if (g_isr_test_mode) {
                // Copy to test storage
                if (g_isr_test_batch_idx < DMA_TEST_BATCH_COUNT) {
                     // Direct copy to SDRAM storage (using Write-Back, will need flush later)
                     memcpy(&g_isr_test_storage[g_isr_test_batch_idx], batch, sizeof(DMABatchBuffer_t));
                     // CRITICAL FIX: Flush D-Cache to SDRAM after copy
                     // Without this, SendRawDataToPC reads stale data from SDRAM
                     Safe_SCB_CleanDCache_by_Addr((uint32_t*)&g_isr_test_storage[g_isr_test_batch_idx], sizeof(DMABatchBuffer_t));
                     g_isr_test_batch_idx++;
                }

                // If collection complete, STOP EVERYTHING
                if (g_isr_test_batch_idx >= DMA_TEST_BATCH_COUNT) {
                    g_isr_inference_running = false; // Stop ISR processing
                    // Stop ADC Hardware
                    hadc3.Instance->CR |= ADC_CR_ADSTP;
                    hadc3.Instance->CR |= ADC_CR_ADDIS;
                }

                // Reset batch for next iteration (if any)
                g_isr_active_batch_idx = 0; // Use only buffer 0 for simplicity or rotate
                g_isr_current_frame_idx = 0;
                s_isr_sample_count = 0;
                
                // Clear buffer for next run
                memset(batch->sample_counts, 0, sizeof(batch->sample_counts));
                
                return; // SKIP QUEUE SEND
            }
            // ================================================================

            // CRITICAL: Flush D-Cache to RAM (RAM_D1 uses Write-Back policy!)
            // Without this, Task reads stale data from RAM while fresh data sits in cache!
            // This is THE bug causing wrong AI predictions (SIT instead of STILLNOACT)
            Safe_SCB_CleanDCache_by_Addr((uint32_t*)batch, sizeof(DMABatchBuffer_t));

            // Send to processing task
            if (g_isr_batch_queue != NULL) {
                DMABatchBuffer_t* batch_ptr = batch;
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;

                if (xQueueSendFromISR(g_isr_batch_queue, &batch_ptr, &xHigherPriorityTaskWoken) == pdTRUE) {
                    g_isr_pipeline_stats.batches_completed++;
                } else {
                    g_isr_pipeline_stats.batch_queue_full_drops++;
                }

                // Switch buffer (like 3Task Lines 446-467)
                s_isr_buffer_ready = false;  // Block sampling during switch

                g_isr_active_batch_idx = (g_isr_active_batch_idx + 1) % DMA_BATCH_BUFFER_COUNT;
                DMABatchBuffer_t* new_batch = &g_isr_batch_buffers[g_isr_active_batch_idx];

                // Clear new buffer metadata (not full memset - too slow for ISR)
                new_batch->frame_count = 0;
                memset(new_batch->sample_counts, 0, sizeof(new_batch->sample_counts));

                g_isr_current_frame_idx = 0;
                s_isr_buffer_ready = true;  // Resume sampling

                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
    }

    g_isr_frame_in_progress = false;
    g_isr_pipeline_stats.last_frame_samples = s_isr_sample_count;
}

/**
 * @brief Handle INTG_CLK (ADC sample) - Direct register access, NO context switch!
 *
 * This is the KEY optimization:
 * - 3Task: ISR → notify → context switch → task → HAL_ADC_Start → poll = ~15µs
 * - DMA Mode: ISR → direct register → poll → store = ~2-3µs
 */
void ISR_HandleIntgClk(void) {
    if (!g_isr_inference_running) return;
    if (!s_isr_first_frame_synced) return;
    if (!s_isr_buffer_ready) return;
    if (!g_isr_frame_in_progress) return;
    if (g_radarFlags.ssFlag != 0) return;  // Only sample when SS is LOW

    g_isr_pipeline_stats.intg_clk_triggers++;

    uint8_t frame_idx = g_isr_current_frame_idx;
    uint16_t sample_idx = s_isr_sample_count;

    if (frame_idx >= DMA_WINDOW_FRAMES || sample_idx >= DMA_FRAME_SAMPLES) {
        g_isr_pipeline_stats.isr_overruns++;
        return;
    }

    // ========================================================================
    // Direct ADC Read (mirrors HAL_ADC_Start + PollForConversion + GetValue)
    // ========================================================================

    // Step 1: Clear pending flags (critical for clean conversion)
    hadc3.Instance->ISR = ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR;

    // Step 2: Start conversion
    hadc3.Instance->CR |= ADC_CR_ADSTART;

    // Step 3: Wait for EOC with timeout
    // ADC3 @ 8MHz, ~2.25µs conversion, 300 iterations = ~1.5µs margin
    volatile uint32_t timeout = 300;
    while (!(hadc3.Instance->ISR & ADC_ISR_EOC)) {
        if (--timeout == 0) {
            g_isr_pipeline_stats.isr_overruns++;
            hadc3.Instance->CR |= ADC_CR_ADSTP;
            return;
        }
    }

    // Step 4: Read data (clears EOC automatically)
    uint32_t raw = hadc3.Instance->DR;

    // Step 5: Convert to signed (differential ADC, center=32768)
    int16_t sample = (int16_t)(raw) - 32768;

    // Step 6: Store in batch buffer
    DMABatchBuffer_t* batch = &g_isr_batch_buffers[g_isr_active_batch_idx];
    batch->frames[frame_idx][sample_idx] = sample;

    s_isr_sample_count++;
}

// ============================================================================
// STATE MACHINE HANDLERS
// ============================================================================

void RealtimeInference_ISR::OnEnter() {
    UsbCommunication* usb = UsbCommunication::getInstance();

    // ========================================================================
    // STEP 1: Stop conflicting tasks
    // ========================================================================
    extern TaskHandle_t adcSampleTaskHandle;
    extern TaskHandle_t streamingTaskHandle;
    extern TaskHandle_t g_producer_task_handle;
    extern TaskHandle_t g_processing_task_handle;

    if (adcSampleTaskHandle != NULL) {
        vTaskDelete(adcSampleTaskHandle);
        adcSampleTaskHandle = NULL;
    }
    if (streamingTaskHandle != NULL) {
        vTaskDelete(streamingTaskHandle);
        streamingTaskHandle = NULL;
    }
    if (g_producer_task_handle != NULL) {
        vTaskDelete(g_producer_task_handle);
        g_producer_task_handle = NULL;
    }
    if (g_processing_task_handle != NULL) {
        vTaskDelete(g_processing_task_handle);
        g_processing_task_handle = NULL;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    // ========================================================================
    // STEP 2: Initialize pipeline
    // ========================================================================
    ISR_Pipeline_Init();

    // ========================================================================
    // STEP 3: Prepare ADC for ISR-based direct register access
    // ========================================================================
    HAL_ADC_Stop(&hadc3);

    uint32_t timeout = 10000;
    while ((hadc3.Instance->CR & ADC_CR_ADSTART) && timeout--);

    // Calibration
    if (HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET, ADC_DIFFERENTIAL_ENDED) != HAL_OK) {
        if (usb) usb->sendMessage("[DMA] WARNING: ADC calibration failed\r\n");
    }

    // Enable ADC
    hadc3.Instance->ISR = ADC_ISR_ADRDY;
    hadc3.Instance->CR |= ADC_CR_ADEN;

    timeout = 10000;
    while (!(hadc3.Instance->ISR & ADC_ISR_ADRDY) && timeout--);

    if (timeout == 0) {
        if (usb) usb->sendMessage("[DMA] ERROR: ADC not ready!\r\n");
        return;
    }

    if (usb) usb->sendMessage("[DMA] ADC enabled and ready\r\n");

    // ========================================================================
    // STEP 4: Create queue (Static allocation in DTCMRAM)
    // ========================================================================
    if (g_isr_batch_queue == NULL) {
        g_isr_batch_queue = xQueueCreateStatic(
            DMA_BATCH_BUFFER_COUNT,
            sizeof(DMABatchBuffer_t*),
            s_isr_batch_queue_storage,
            &s_isr_batch_queue_struct
        );
        
        if (g_isr_batch_queue != NULL) {
            vQueueAddToRegistry(g_isr_batch_queue, "ISRBatchQ"); // DEBUG: Show in IDE
        } else {
            if (usb) usb->sendMessage("[ISR] ERROR: Queue creation failed!\r\n");
        }
    } else {
        xQueueReset(g_isr_batch_queue);
    }

    // ========================================================================
    // STEP 5: Initialize radar flags (same as Stream.cpp)
    // ========================================================================
    g_radarFlags.ssFlag = (HAL_GPIO_ReadPin(RADAR_SS_GPIO_Port, RADAR_SS_Pin) == GPIO_PIN_SET);
    g_radarFlags.intgClkFlag = 0;

    // ========================================================================
    // STEP 6: Heap check
    // ========================================================================
    size_t free_heap = xPortGetFreeHeapSize();
    if (usb) {
        char msg[64];
        snprintf(msg, sizeof(msg), "[DMA] Free heap: %u bytes\r\n", (unsigned int)free_heap);
        usb->sendMessage(msg);
    }

    // ========================================================================
    // STEP 7: Enable ISR mode BEFORE creating task (CRITICAL!)
    // ========================================================================
    // Task checks g_isr_inference_running immediately on start.
    // MUST be true before task creation or task exits instantly!
    g_isr_inference_running = true;

    extern volatile bool g_isr_mode_active;
    g_isr_mode_active = true;

    if (usb) {
        usb->sendMessage("[ISR] Flags set - creating task...\r\n");
    }

    // ========================================================================
    // STEP 8: Create processing task (Static allocation in DTCMRAM)
    // ========================================================================
    if (g_isr_processing_task_handle == NULL) {
        // DIAGNOSTIC: Verify pointers before task creation
        if (usb) {
            char diag[128];
            snprintf(diag, sizeof(diag), 
                "[ISR] Creating task: Stack=%p TCB=%p Size=%u\r\n",
                (void*)s_isr_processing_stack, 
                (void*)&s_isr_processing_tcb,
                (unsigned int)sizeof(s_isr_processing_stack));
            usb->sendMessage(diag);
        }
        
        g_isr_processing_task_handle = xTaskCreateStatic(
            ISR_ProcessingTask,
            "ISRProc",
            4096,           // Stack Size (Words) -> 16KB
            NULL,
            osPriorityHigh,
            s_isr_processing_stack,
            &s_isr_processing_tcb
        );
        
        if (g_isr_processing_task_handle == NULL) {
            if (usb) {
                usb->sendMessage("[ISR] *** CRITICAL ERROR: Task creation failed! ***\r\n");
                usb->sendMessage("[ISR] Possible causes:\r\n");
                usb->sendMessage("[ISR]   - DTCMRAM full or misconfigured\r\n");
                usb->sendMessage("[ISR]   - Stack/TCB pointer invalid\r\n");
                usb->sendMessage("[ISR]   - Priority issue\r\n");
            }
        } else {
            if (usb) {
                char success[64];
                snprintf(success, sizeof(success), 
                    "[ISR] Task created successfully! Handle=%p\r\n",
                    (void*)g_isr_processing_task_handle);
                usb->sendMessage(success);
            }
        }
    }

    vTaskDelay(pdMS_TO_TICKS(50));

    if (usb) {
        usb->sendMessage("[DMA] Pipeline started - waiting for first SS_HIGH sync...\r\n");
    }
}

void RealtimeInference_ISR::OnExit() {
    extern volatile bool g_isr_mode_active;
    g_isr_mode_active = false;

    ISR_Pipeline_Cleanup();

    UsbCommunication* usb = UsbCommunication::getInstance();
    if (usb) {
        char msg[256];
        snprintf(msg, sizeof(msg),
            "[ISR] Stopped. Batches:%lu Inf:%lu IntgClk:%lu Overruns:%lu Drops:%lu\r\n",
            g_isr_pipeline_stats.batches_completed,
            g_isr_pipeline_stats.inferences_done,
            g_isr_pipeline_stats.intg_clk_triggers,
            g_isr_pipeline_stats.isr_overruns,
            g_isr_pipeline_stats.batch_queue_full_drops
        );
        usb->sendMessage(msg);
    }
}

void RealtimeInference_ISR::HandleEvent(Event* event, void* args) {
    (void)event;
    (void)args;
}

// ============================================================================
// PROCESSING TASK - Same preprocessing as 3Task for consistency
// ============================================================================

void ISR_ProcessingTask(void* params) {
    (void)params;

    UsbCommunication* usb = UsbCommunication::getInstance();
    DMABatchBuffer_t* received_batch;
    bool first_batch_skipped = false;

    if (usb) {
        usb->sendMessage("[ISR] Processing task started\r\n");
    }

    // SAFETY CHECK: Queue must exist
    if (g_isr_batch_queue == NULL) {
        if (usb) usb->sendMessage("[ISR] CRITICAL ERROR: g_isr_batch_queue is NULL! Task aborting.\r\n");
        vTaskDelete(NULL);
        return;
    }

    while (g_isr_inference_running) {
        if (xQueueReceive(g_isr_batch_queue, &received_batch, pdMS_TO_TICKS(2000)) == pdTRUE) {

            // Skip first batch (startup sync issue, like 3Task)
            if (!first_batch_skipped) {
                first_batch_skipped = true;
                if (usb) {
                    usb->sendMessage("[ISR] Skipping first batch (sync)\r\n");
                }
                continue;
            }

            // CRITICAL: SDRAM uses Write-Back cache
            // Must use CleanInvalidate (not just Invalidate) to flush dirty data first
            Safe_SCB_CleanInvalidateDCache_by_Addr((uint32_t*)received_batch, sizeof(DMABatchBuffer_t));

            // ================================================================
            // STEP 1: APPLY MTI FILTER (Python ile uyumlu)
            // ================================================================
            ISR_ApplyMTIFilter(received_batch->frames, s_isr_mti_filtered_window);

            // ================================================================
            // STEP 2: PER-FRAME NORMALIZE (MİTİ sonrası)
            // ================================================================
            uint8_t zero_count_frames = 0;
            
            for (uint8_t f = 0; f < DMA_WINDOW_FRAMES; f++) {
                // Pass 1: Find min/max of filtered data
                float frame_min = 1e9f;
                float frame_max = -1e9f;
                
                for (uint16_t s = 0; s < DMA_FRAME_SAMPLES; s++) {
                    float val = s_isr_mti_filtered_window[f][s];
                    if (val < frame_min) frame_min = val;
                    if (val > frame_max) frame_max = val;
                }
                
                // Pass 2: Normalize
                float range = frame_max - frame_min;
                if (range < 1e-6f) {
                    memset(g_isr_ai_input_buffer.data[f], 0, DMA_FRAME_SAMPLES * sizeof(float));
                } else {
                    const float inv_range = 1.0f / range;
                    for (uint16_t s = 0; s < DMA_FRAME_SAMPLES; s++) {
                        float norm_01 = (s_isr_mti_filtered_window[f][s] - frame_min) * inv_range;
                        g_isr_ai_input_buffer.data[f][s] = DMA_NORM_MIN + (norm_01 * (DMA_NORM_MAX - DMA_NORM_MIN));
                    }
                }
            }

            g_isr_ai_input_buffer.batch_id = received_batch->batch_id;

            // Cache clean AI input buffer (SDRAM is Write-Back)
            Safe_SCB_CleanDCache_by_Addr((uint32_t*)&g_isr_ai_input_buffer, sizeof(DMAInputData_t));

            // ================================================================
            // AI INFERENCE
            // ================================================================
            // CRITICAL: Explicit barrier before activation invalidate
            // Prevents CPU/compiler from reordering the two cache operations
            __DSB();  // Memory barrier - ensures AI input fully flushed before activation touch
            
            uint32_t activation_size = (uint32_t)_ai_activation_end - (uint32_t)_ai_activation_start;
            if (activation_size > 0 && activation_size < 2*1024*1024) {
                Safe_SCB_CleanInvalidateDCache_by_Addr((uint32_t*)_ai_activation_start, activation_size);
                // NOTE: __ISB() NOT needed! ISB is for instruction cache/pipeline flush only
                // Using ISB here causes unnecessary CPU stall and can disrupt timing
            }

            uint32_t start_time = HAL_GetTick();

            float* output = ai_fallower_run_inference_direct((const float*)g_isr_ai_input_buffer.data);

            uint32_t end_time = HAL_GetTick();
            uint32_t duration = (end_time >= start_time) ?
                                (end_time - start_time) :
                                (UINT32_MAX - start_time + end_time + 1);

            // CRITICAL NOTE: Do NOT invalidate here!
            // The AI inference (CPU) just wrote the output to the activation buffer.
            // If we Invalidate now, we discard the calculation results in the cache!
            // The CPU can read its own dirty cache correctly.
            // If we needed to send this to DMA/USB, we would use SCB_CleanDCache.
            // SCB_CleanDCache_by_Addr((uint32_t*)output, 32); // Optional: good for debug visibility

            // Update stats
            g_isr_pipeline_stats.batches_processed++;
            g_isr_pipeline_stats.last_inference_time_ms = duration;
            if (duration < g_isr_pipeline_stats.min_inference_time_ms) {
                g_isr_pipeline_stats.min_inference_time_ms = duration;
            }
            if (duration > g_isr_pipeline_stats.max_inference_time_ms) {
                g_isr_pipeline_stats.max_inference_time_ms = duration;
            }

            // ================================================================
            // OUTPUT RESULT
            // ================================================================
            if (output && usb) {
                // NOTE: No need to invalidate output!
                // The CPU just wrote it to cache. CPU can read its own dirty cache.
                // Invalidating would discard the result we just calculated!

                float confidence;
                uint8_t predicted_class = ISR_GetPredictedClass(output, &confidence);
                const char* class_name = ISR_AI_CLASS_NAMES[predicted_class];

                char msg[256];
                snprintf(msg, sizeof(msg),
                    "result: [%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f] -> %s(%.0f%%)\r\n",
                    output[0], output[1], output[2], output[3],
                    output[4], output[5], output[6],
                    class_name, confidence
                );
                usb->sendMessage(msg);

                // Stats every 5 inferences
                if (g_isr_pipeline_stats.inferences_done % 5 == 0) {
                    uint16_t batch_min = 999, batch_max = 0;
                    for (uint8_t f = 0; f < DMA_WINDOW_FRAMES; f++) {
                        uint16_t cnt = received_batch->sample_counts[f];
                        if (cnt < batch_min) batch_min = cnt;
                        if (cnt > batch_max) batch_max = cnt;
                    }

                    char stats_msg[192];
                    snprintf(stats_msg, sizeof(stats_msg),
                        "[ISR_STATS] B:%lu Inf:%lu Time:%lums Samp:%u-%u Zero:%d Overruns:%lu\r\n",
                        g_isr_pipeline_stats.batches_completed,
                        g_isr_pipeline_stats.inferences_done,
                        duration,
                        batch_min, batch_max, zero_count_frames,
                        g_isr_pipeline_stats.isr_overruns);
                    usb->sendMessage(stats_msg);
                }

                g_isr_pipeline_stats.inferences_done++;
            } else {
                g_isr_pipeline_stats.inference_errors++;
            }
        } else {
            // TIMEOUT / HEARTBEAT DIAGNOSTIC
            // If we are here, queue receive timed out (2s).
            // This means ISRs are not pushing data. Let's see why.
            if (usb) {
                char diag_msg[256];
                snprintf(diag_msg, sizeof(diag_msg),
                    "[ISR_DIAG] Waiting... Frames:%u/30 Samples:%u IntgClk:%lu Sync:%d BatchId:%lu\r\n",
                    g_isr_current_frame_idx,
                    s_isr_sample_count,
                    g_isr_pipeline_stats.intg_clk_triggers,
                    s_isr_first_frame_synced,
                    s_isr_batch_id_counter
                );
                usb->sendMessage(diag_msg);
            }
        }
    }

    vTaskDelete(NULL);
}
