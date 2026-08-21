/**
 * @file RealtimeInference_3Task.cpp
 * @brief Robust 3-Task Real-time Radar AI Inference Pipeline
 * 
 * Architecture:
 * =============
 * 
 *   [ISR] ──notifications──> [RadarProducerTask] ──queue──> [WindowManagerTask] ──queue──> [AIInferenceTask]
 *                                   │                              │                              │
 *                            Batch Buffer A/B              AI Input Buffer A/B              USB Output
 *                              (RAM_D1)                        (SDRAM)                     (Result only)
 * 
 * Key Design Decisions:
 * - Queue-based communication (not binary semaphores) prevents signal loss
 * - Double buffering at each stage prevents data override
 * - Only AIInferenceTask outputs logs (clean USB output)
 * - Critical sections protect buffer swaps
 * - Cache operations ensure data coherency
 * 
 * @author Ethem
 * @date 2025
 */

#include "RealtimeInference_3Task.hpp"
#include "StateMachine.hpp"
#include "../UsbCommunication.hpp"
#include <cstring>
#include <cmath>
#include <stdio.h>
#include <climits> // Fixed: Include for ULONG_MAX
#include "cmsis_os.h"
#include "main.h"
#include "../radar_flags.h" // Source of Truth for Events

// ============================================================================
// EXTERNAL DEPENDENCIES
// ============================================================================

// AI inference function
extern float* ai_fallower_run_inference_direct(const float* input_data);

// Hardware handles
extern ADC_HandleTypeDef hadc3;
extern RadarFlags_t g_radarFlags;

// Linker symbols for AI activation buffer (cache management)
extern uint8_t _ai_activation_start[];
extern uint8_t _ai_activation_end[];

// ============================================================================
// MEMORY ALLOCATION
// ============================================================================

// RAM_D1: Batch Buffers for raw data collection (~82KB total)
__attribute__((section(".ram_d1"))) __attribute__((aligned(32)))
BatchBuffer_t g_batch_buffers[BATCH_BUFFER_COUNT];

// SDRAM: AI Input Buffer for preprocessed data (~82KB)
__attribute__((section(".sdram_data"))) __attribute__((aligned(32)))
AIInputData_t g_ai_input_buffer;  // Single buffer (no double buffering)

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// Statistics
PipelineStats_3Task_t g_pipeline_stats_3task = {0}; // Fixed: Renamed to avoid conflicts

// Control flag
volatile bool g_inference_running = false;

// Queues
QueueHandle_t g_batch_queue = NULL;      // BatchBuffer_t* pointers

// Task handles
TaskHandle_t g_producer_task_handle = NULL;
TaskHandle_t g_processing_task_handle = NULL;

// Buffer management (cache-line aligned to prevent false sharing)
__attribute__((aligned(32))) static volatile uint8_t s_active_batch_idx = 0;
static volatile bool s_buffer_ready = true;  // FIX: Prevent race condition during memset

#define AI_CLASS_COUNT 7

// SDRAM: Temporary buffer for MTI filtered data (82KB)
__attribute__((section(".sdram_data"))) __attribute__((aligned(32)))
static float s_mti_filtered_window[AI_WINDOW_FRAMES][FRAME_SAMPLES];

// ============================================================================
// STATIC TASK MEMORY (RAM_D2)
// ============================================================================
__attribute__((section(".ram_d2"))) static StackType_t s_producer_stack[2048];
__attribute__((section(".ram_d2"))) static StackType_t s_processing_task_stack[6144];  // Merged task needs larger stack

static StaticTask_t s_producer_tcb;
static StaticTask_t s_processing_task_tcb;

// Static queue storage
static StaticQueue_t s_batch_queue_struct;
static uint8_t s_batch_queue_storage[BATCH_QUEUE_DEPTH * sizeof(BatchBuffer_t*)];

/**
 * @brief Apply MTI Filter (Differential Moving Average)
 * Matches Python Implementation exactly
 */
static void ApplyMTIFilter_3Task(const int16_t input_window[AI_WINDOW_FRAMES][FRAME_SAMPLES], 
                               float output_window[AI_WINDOW_FRAMES][FRAME_SAMPLES]) {
    // Initial frames that are NOT filtered should be zeroed
    memset(output_window, 0, MTI_REF_AVG_NUM * FRAME_SAMPLES * sizeof(float));

    for (int i = MTI_REF_AVG_NUM; i < AI_WINDOW_FRAMES; i++) {
        for (int b = 0; b < FRAME_SAMPLES; b++) {
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

void Pipeline_Init(void) {
    // Clear all buffers
    memset(g_batch_buffers, 0, sizeof(g_batch_buffers));
    memset(&g_ai_input_buffer, 0, sizeof(g_ai_input_buffer));  // Single buffer
    memset(&g_pipeline_stats_3task, 0, sizeof(g_pipeline_stats_3task));
    memset(s_mti_filtered_window, 0, sizeof(s_mti_filtered_window));
    
    // FIX BUG #2: Cache clean after memset to ensure D-Cache coherency
    // Without this, first batches may read stale data from cache
    SCB_CleanDCache_by_Addr((uint32_t*)g_batch_buffers, sizeof(g_batch_buffers));
    SCB_CleanDCache_by_Addr((uint32_t*)&g_ai_input_buffer, sizeof(g_ai_input_buffer));
    __DSB();  // Ensure cache operations complete before proceeding
    
    // Initialize buffer indices
    for (int i = 0; i < BATCH_BUFFER_COUNT; i++) {
        g_batch_buffers[i].buffer_index = i;
        g_batch_buffers[i].frame_count = 0;
        g_batch_buffers[i].batch_id = 0;
    }
    
    // Initialize single AI buffer
    g_ai_input_buffer.batch_id = 0;
    
    // Reset buffer management
    s_active_batch_idx = 0;
    
    // Initialize stats
    g_pipeline_stats_3task.min_inference_time_ms = UINT32_MAX;
    g_pipeline_stats_3task.max_inference_time_ms = 0;
}

void Pipeline_Cleanup(void) {
    g_inference_running = false;
    
    // Delete tasks
    if (g_producer_task_handle) {
        vTaskDelete(g_producer_task_handle);
        g_producer_task_handle = NULL;
    }
    if (g_processing_task_handle) {
        vTaskDelete(g_processing_task_handle);
        g_processing_task_handle = NULL;
    }
    
    // Delete queue
    if (g_batch_queue) {
        vQueueDelete(g_batch_queue);
        g_batch_queue = NULL;
    }
}

uint8_t GetPredictedClass(const float* output, float* confidence) {
    if (!output) {
        *confidence = 0.0f;
        return AI_CLASS_COUNT - 1; // UNKNOWN
    }
    
    uint8_t max_idx = 0;
    float max_val = output[0];
    
    for (uint8_t i = 1; i < AI_CLASS_COUNT; i++) {
        if (output[i] > max_val) {
            max_val = output[i];
            max_idx = i;
        }
    }
    
    // Softmax approximation for confidence
    // confidence = exp(max) / sum(exp(all)) = 1 / sum(exp(x - max))
    float sum = 0.0f;
    for (uint8_t i = 0; i < AI_CLASS_COUNT; i++) {
        // expf(x - max_val) is numerically stable since x - max_val <= 0
        // FIX F17: Clamp to prevent extreme underflow (expf(-88.0) is approx 0)
        float diff = output[i] - max_val;
        if (diff < -88.0f) diff = -88.0f; 
        sum += expf(diff);
    }
    
    if (sum > 0.0f) {
        *confidence = (1.0f / sum) * 100.0f;
        // Clamp to reasonable range
        if (*confidence > 99.0f) *confidence = 99.0f;
        if (*confidence < 1.0f) *confidence = 1.0f;
    } else {
        *confidence = 50.0f; // Uncertain
    }
    
    return max_idx;
}

// ============================================================================
// STATE MACHINE HANDLERS
// ============================================================================

void RealtimeInference_3Task::OnEnter() {
    UsbCommunication* usb = UsbCommunication::getInstance();
    
    // ========================================================================
    // STEP 1: Stop conflicting tasks (Stream mode)
    // ========================================================================
    extern TaskHandle_t adcSampleTaskHandle;
    extern TaskHandle_t streamingTaskHandle;
    
    if (adcSampleTaskHandle != NULL) {
        vTaskDelete(adcSampleTaskHandle);
        adcSampleTaskHandle = NULL;
    }
    if (streamingTaskHandle != NULL) {
        vTaskDelete(streamingTaskHandle);
        streamingTaskHandle = NULL;
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // ========================================================================
    // STEP 2: Initialize pipeline
    // ========================================================================
    Pipeline_Init();
    
    // ========================================================================
    // STEP 3: Create queue (pointer-based for efficiency)
    // ========================================================================
    if (g_batch_queue == NULL) {
        g_batch_queue = xQueueCreateStatic(
            BATCH_QUEUE_DEPTH,
            sizeof(BatchBuffer_t*),
            s_batch_queue_storage,
            &s_batch_queue_struct
        );
    } else {
        xQueueReset(g_batch_queue);
    }
    
    // ========================================================================
    // STEP 4: Initialize radar flags (same as Stream.cpp)
    // ========================================================================
    g_radarFlags.ssFlag = (HAL_GPIO_ReadPin(RADAR_SS_GPIO_Port, RADAR_SS_Pin) == GPIO_PIN_SET);
    g_radarFlags.intgClkFlag = 0;
    
    // ========================================================================
    // STEP 5: Set running flag BEFORE creating tasks
    // ========================================================================
    g_inference_running = true;
    
    // ========================================================================
    // DIAGNOSTIC: Check heap before creating tasks
    // ========================================================================
    size_t free_heap = xPortGetFreeHeapSize();
    if (usb) {
        char msg[64];
        snprintf(msg, sizeof(msg), "[HEAP] Free heap: %u bytes\r\n", (unsigned int)free_heap);
        usb->sendMessage(msg);
    }
    
    if (free_heap < 10240) {  // Less than 10KB free
        if (usb) {
            usb->sendMessage("[ERROR] Low heap! Tasks may fail!\r\n");
        }
    }
    
    // ========================================================================
    // STEP 6: Create tasks in order (lowest priority first)
    // ========================================================================
    
    // Processing and Inference Task (High Priority) - Creates first, waits for data
    if (g_processing_task_handle == NULL) {
        g_processing_task_handle = xTaskCreateStatic(
            ProcessingAndInferenceTask,
            "ProcInfer",
            6144,  // Larger stack for merged task
            NULL,
            osPriorityHigh,
            s_processing_task_stack,
            &s_processing_task_tcb
        );
    }
    
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Radar Producer Task (Realtime Priority) - Creates last, starts immediately
    // This is the handle ISR will send notifications to
    if (g_producer_task_handle == NULL) {
        g_producer_task_handle = xTaskCreateStatic(
            RadarProducerTask_3Task,
            "RadarProd",
            2048,
            NULL,
            osPriorityRealtime,
            s_producer_stack,
            &s_producer_tcb
        );
    }
    
    // Update external handle for ISR routing
    extern TaskHandle_t producerTaskHandle_3task;
    producerTaskHandle_3task = g_producer_task_handle;
    
    if (usb) {
        usb->sendMessage("[AI] Pipeline started\r\n");
    }
}

void RealtimeInference_3Task::OnExit() {
    // Update external handle to prevent ISR sending to deleted task
    extern TaskHandle_t producerTaskHandle_3task;
    producerTaskHandle_3task = NULL;
    
    Pipeline_Cleanup();
    
    UsbCommunication* usb = UsbCommunication::getInstance();
    if (usb) {
        char msg[128];
        snprintf(msg, sizeof(msg), 
            "[AI] Stopped. Batches:%lu Inferences:%lu Drops:%lu\r\n",
            g_pipeline_stats_3task.batches_completed,
            g_pipeline_stats_3task.inferences_done,
            g_pipeline_stats_3task.batch_queue_full_drops + g_pipeline_stats_3task.ai_queue_full_drops
        );
        usb->sendMessage(msg);
    }
}

void RealtimeInference_3Task::HandleEvent(Event* event, void* args) {
    (void)event;
    (void)args;
    // Events handled by StateMachine
}

// ============================================================================
// TASK 1: RADAR PRODUCER
// ============================================================================
/**
 * This task mirrors ADC_SamplingTask logic exactly for proven reliability.
 * Key differences:
 * - Collects 30 frames before signaling (not 1)
 * - Uses queue instead of direct processing
 * - Double buffers batch data
 */

void RadarProducerTask_3Task(void* params) {
    (void)params;
    
    uint32_t notificationValue;
    uint8_t current_frame_idx = 0;
    uint16_t current_sample_count = 0;
    static uint32_t s_batch_id_counter = 0;
    
    // Get pointer to active batch buffer
    BatchBuffer_t* active_batch = &g_batch_buffers[s_active_batch_idx];
    active_batch->frame_count = 0;
    
    // ========================================================================
    // PHASE 1: Wait for first SS_HIGH (same as ADC_SamplingTask)
    // ========================================================================
    while (g_inference_running) {
        if (xTaskNotifyWait(0, ULONG_MAX, &notificationValue, pdMS_TO_TICKS(1000)) == pdTRUE) {
            if ((notificationValue & SS_HIGH_EVENT) != 0) {
                break;
            }
        }
    }
    
    // ========================================================================
    // PHASE 2: Wait for first SS_LOW (frame start alignment)
    // ========================================================================
    while (g_inference_running) {
        if (xTaskNotifyWait(0, ULONG_MAX, &notificationValue, pdMS_TO_TICKS(1000)) == pdTRUE) {
            if ((notificationValue & SS_LOW_EVENT) != 0) {
                current_sample_count = 0;
                current_frame_idx = 0;
                active_batch->timestamps[0] = HAL_GetTick();
                break;
            }
        }
    }
    
    // ========================================================================
    // PHASE 3: Main collection loop (mirrors ADC_SamplingTask logic)
    // ========================================================================
    while (g_inference_running) {
        if (xTaskNotifyWait(0, ULONG_MAX, &notificationValue, pdMS_TO_TICKS(100)) == pdTRUE) {
            
            // ----------------------------------------------------------------
            // INTG_CLK Event: Collect ADC sample
            // ----------------------------------------------------------------
            if ((notificationValue & INTG_CLK_EVENT) != 0) {
                // FIX F2: Check s_buffer_ready inside critical section to prevent race condition
                bool can_sample = false;
                taskENTER_CRITICAL();
                can_sample = (g_radarFlags.ssFlag == 0 && current_frame_idx < AI_WINDOW_FRAMES && s_buffer_ready);
                taskEXIT_CRITICAL();

                if (can_sample) {
                    HAL_ADC_Start(&hadc3);

                    if (HAL_ADC_PollForConversion(&hadc3, 1) == HAL_OK) {
                        int16_t sample = (int16_t)(HAL_ADC_GetValue(&hadc3)) - 32768;

                        // CRITICAL SECTION: Protect buffer write from race conditions
                        taskENTER_CRITICAL();
                        if (current_frame_idx < AI_WINDOW_FRAMES &&
                            current_sample_count < FRAME_SAMPLES && s_buffer_ready) {
                            active_batch->frames[current_frame_idx][current_sample_count] = sample;
                            current_sample_count++;
                        } else {
                            // Buffer overflow detected - skip this sample
                            g_pipeline_stats_3task.inference_errors++;
                        }
                        taskEXIT_CRITICAL();
                    }

                    HAL_ADC_Stop(&hadc3);
                }
            }
            
            // ----------------------------------------------------------------
            // SS_HIGH Event: Frame complete
            // ----------------------------------------------------------------
            if ((notificationValue & SS_HIGH_EVENT) != 0) {
                // FIX F2: Check s_buffer_ready inside critical section
                bool can_finish_frame = false;
                taskENTER_CRITICAL();
                can_finish_frame = (current_sample_count > 0 && current_frame_idx < AI_WINDOW_FRAMES && s_buffer_ready);
                taskEXIT_CRITICAL();

                if (can_finish_frame) {
                    // Record frame metadata
                    active_batch->sample_counts[current_frame_idx] = current_sample_count;
                    active_batch->timestamps[current_frame_idx] = HAL_GetTick();
                    
                    current_frame_idx++;
                    current_sample_count = 0; // RESET COUNT HERE FOR NEXT FRAME
                    g_pipeline_stats_3task.total_frames_collected++;
                    
                    // --------------------------------------------------------
                    // CHECK: Batch complete (30 frames)?
                    // --------------------------------------------------------
                    if (current_frame_idx >= AI_WINDOW_FRAMES) {
                        active_batch->frame_count = current_frame_idx;
                        active_batch->batch_id = ++s_batch_id_counter;
                        
                        // Flush cache to ensure data is visible to other tasks
                        SCB_CleanDCache_by_Addr((uint32_t*)active_batch, sizeof(BatchBuffer_t));
                        __DSB();
                        
                        // Send pointer to WindowManager via queue
                        BatchBuffer_t* batch_ptr = active_batch;
                        if (xQueueSend(g_batch_queue, &batch_ptr, 0) != pdTRUE) {
                            // Queue full - this batch will be overwritten
                            g_pipeline_stats_3task.batch_queue_full_drops++;
                        } else {
                            g_pipeline_stats_3task.batches_completed++;
                        }
                        
                        // ========================================================
                        // CRITICAL SECTION: Only swap pointers (FAST!)
                        // memset moved outside to prevent ISR starvation
                        // ========================================================
                        // FIX: Set buffer NOT ready before swap to prevent race condition
                        taskENTER_CRITICAL();
                        s_buffer_ready = false;  // Block sample collection
                        s_active_batch_idx = (s_active_batch_idx + 1) % BATCH_BUFFER_COUNT;
                        active_batch = &g_batch_buffers[s_active_batch_idx];
                        taskEXIT_CRITICAL();
                        
                        // Clear buffer (safe now - s_buffer_ready is false)
                        // CRITICAL: Must clear frames to prevent stale data!
                        // Frame memset for clean slate (prevents stale data)
                        active_batch->frame_count = 0;
                        memset(active_batch->frames, 0, sizeof(active_batch->frames));  // 41KB - essential!
                        memset(active_batch->sample_counts, 0, sizeof(active_batch->sample_counts));
                        
                        // Ensure memset completes and cache is clean
                        SCB_CleanDCache_by_Addr((uint32_t*)active_batch, sizeof(BatchBuffer_t));
                        __DSB();  // Memory barrier - ensures all writes complete
                        
                        current_frame_idx = 0;
                        
                        // FIX: Now buffer is ready for new data
                        s_buffer_ready = true;
                    }
                }
            }
            
            // ----------------------------------------------------------------
            // SS_LOW Event: New frame starting
            // ----------------------------------------------------------------
            if ((notificationValue & SS_LOW_EVENT) != 0) {
                // FIX F2: Check s_buffer_ready inside critical section
                bool can_start_frame = false;
                taskENTER_CRITICAL();
                can_start_frame = (current_sample_count == 0 && current_frame_idx < AI_WINDOW_FRAMES && s_buffer_ready);
                taskEXIT_CRITICAL();

                if (can_start_frame) {
                    active_batch->timestamps[current_frame_idx] = HAL_GetTick();
                }
            }
        }
    }
    
    vTaskDelete(NULL);
}

// ============================================================================
// TASK 2: PROCESSING AND INFERENCE (Merged WindowManager + AIInference)
// ============================================================================
/**
 * Merged task that combines preprocessing and AI inference.
 * Benefits:
 * - Eliminates race condition (single buffer, sequential access)
 * - Saves ~164KB SDRAM (one less buffer)
 * - Simpler architecture (no inter-stage queue)
 * 
 * Trade-off:
 * - No parallel preprocessing (acceptable: 1s inference < 2s batch interval)
 */

void ProcessingAndInferenceTask(void* params) {
    (void)params;
    
    UsbCommunication* usb = UsbCommunication::getInstance();
    BatchBuffer_t* received_batch;
    
    while (g_inference_running) {
        // ====================================================================
        // STEP 1: Wait for batch from RadarProducer (blocking with timeout)
        // ====================================================================
        if (xQueueReceive(g_batch_queue, &received_batch, pdMS_TO_TICKS(2000)) == pdTRUE) {
            
            // ----------------------------------------------------------------
            // FIX: Skip first batch (may have incomplete samples)
            // First batch timing is off due to startup synchronization
            // ----------------------------------------------------------------
            static bool s_first_batch_skipped = false;
            if (!s_first_batch_skipped) {
                s_first_batch_skipped = true;
                if (usb) {
                    usb->sendMessage("[INFO] Skipping first batch (startup sync issue)\r\n");
                }
                continue;  // Skip to next batch
            }
            
            // ----------------------------------------------------------------
            // Cache invalidate received batch to ensure fresh data
            // ----------------------------------------------------------------
            SCB_InvalidateDCache_by_Addr((uint32_t*)received_batch, sizeof(BatchBuffer_t));
            __DSB();
            __ISB();
            
            // ================================================================
            // DIAGNOSTIC: Log sample counts for first 10 frames
            // (Conditional to reduce overhead)
            // ================================================================
            #ifdef DEBUG_INFERENCE
            if (usb && received_batch->batch_id <= 2) {             char msg[256];
                int offset = snprintf(msg, sizeof(msg), "[BATCH_%lu] Samples: ", received_batch->batch_id);
                for (uint8_t f = 0; f < 10 && f < AI_WINDOW_FRAMES; f++) {
                    offset += snprintf(msg + offset, sizeof(msg) - offset, "%d,", 
                                      received_batch->sample_counts[f]);
                }
                snprintf(msg + offset - 1, sizeof(msg) - offset + 1, "\r\n");  // Replace last comma
                usb->sendMessage(msg);
            }
            #endif
            
            // ================================================================
            // DIAGNOSTIC: Log raw data for first batch (Python comparison)
            // (Conditional to reduce overhead)
            // ================================================================
            #ifdef DEBUG_INFERENCE
            if (usb && received_batch->batch_id <= 2) {
                char msg[256];
                int offset = snprintf(msg, sizeof(msg), "[RAW_B%lu_F0] First 10: ", 
                                    received_batch->batch_id);
                for (int i = 0; i < 10 && i < received_batch->sample_counts[0]; i++) {
                    offset += snprintf(msg + offset, sizeof(msg) - offset, "%d,", 
                                     received_batch->frames[0][i]);
                }
                usb->sendMessage(msg);
            }
            #endif
            
            // ================================================================
            // STEP 2: APPLY MTI FILTER (Python ile uyumlu)
            // ================================================================
            ApplyMTIFilter_3Task(received_batch->frames, s_mti_filtered_window);

            // ================================================================
            // STEP 3: PER-FRAME NORMALIZE (MİTİ sonrası)
            // ================================================================
            uint8_t zero_count_frames = 0;
            
            for (uint8_t f = 0; f < AI_WINDOW_FRAMES; f++) {
                // Pass 1: Find min/max of filtered data
                float frame_min = 1e9f;
                float frame_max = -1e9f;
                
                for (uint16_t s = 0; s < FRAME_SAMPLES; s++) {
                    float val = s_mti_filtered_window[f][s];
                    if (val < frame_min) frame_min = val;
                    if (val > frame_max) frame_max = val;
                }
                
                // Pass 2: Normalize
                float range = frame_max - frame_min;
                if (range < 1e-6f) {
                    memset(g_ai_input_buffer.data[f], 0, FRAME_SAMPLES * sizeof(float));
                } else {
                    const float inv_range = 1.0f / range;
                    for (uint16_t s = 0; s < FRAME_SAMPLES; s++) {
                        float norm_01 = (s_mti_filtered_window[f][s] - frame_min) * inv_range;
                        g_ai_input_buffer.data[f][s] = NORM_MIN + (norm_01 * (NORM_MAX - NORM_MIN));
                    }
                }
            }
            
            g_ai_input_buffer.batch_id = received_batch->batch_id;
            
            // ================================================================
            // DIAGNOSTIC: Log normalization parameters
            // (Conditional to reduce overhead)
            // ================================================================
            #ifdef DEBUG_INFERENCE
            if (usb) {
                char msg[64];
                snprintf(msg, sizeof(msg), 
                    "[NORM_%lu] per-frame, zeros=%d\r\n",
                    g_ai_input_buffer.batch_id, zero_count_frames);
                usb->sendMessage(msg);
            }
            #endif
            
            // ----------------------------------------------------------------
            // Flush ai_input_buffer to SDRAM (cache clean)
            // ----------------------------------------------------------------
            SCB_CleanDCache_by_Addr((uint32_t*)&g_ai_input_buffer, sizeof(AIInputData_t));
            __DSB();  // Ensure AI input flush completes
            
            // ================================================================
            // STEP 4: Run Inference (cache already clean from STEP 3)
            // ================================================================
            // NOTE: No need to invalidate - we just wrote this data
            // Single-core system, same task context
            
            // CRITICAL: Memory barrier before activation touch
            __DSB();  // Prevents reordering of cache operations
            
            // CRITICAL: Validate and invalidate AI activation buffer
            uint32_t activation_size = (uint32_t)_ai_activation_end - (uint32_t)_ai_activation_start;
            if (activation_size > 0 && activation_size < 2*1024*1024) {  // Sanity: 0 < size < 2MB
                SCB_CleanInvalidateDCache_by_Addr((uint32_t*)_ai_activation_start, activation_size);
                __DSB();  // Data Sync Barrier only - ISB not needed for cache ops
            } else if (usb && activation_size > 0) {
                char msg[64];
                snprintf(msg, sizeof(msg), "[ERROR] Bad activation size: %lu\r\n", activation_size);
                usb->sendMessage(msg);
            }
            
            // ================================================================
            // STEP 5: RUN INFERENCE
            // ================================================================
            uint32_t start_time = HAL_GetTick();
            
            float* output = ai_fallower_run_inference_direct((const float*)g_ai_input_buffer.data);
            
            // CRITICAL: Handle timestamp overflow (49-day uptime)
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
            
            // Update statistics
            g_pipeline_stats_3task.batches_processed++;
            g_pipeline_stats_3task.last_inference_time_ms = duration;
            if (duration < g_pipeline_stats_3task.min_inference_time_ms) {
                g_pipeline_stats_3task.min_inference_time_ms = duration;
            }
            if (duration > g_pipeline_stats_3task.max_inference_time_ms) {
                g_pipeline_stats_3task.max_inference_time_ms = duration;
            }
            
            // ================================================================
            // STEP 6: OUTPUT RESULT (ONLY LOG FROM THIS TASK)
            // ================================================================
            if (output && usb) {
                // Invalidate output cache
                SCB_InvalidateDCache_by_Addr((uint32_t*)output, 32);
                
                // Get predicted class
                float confidence;
                uint8_t predicted_class = GetPredictedClass(output, &confidence);
                const char* class_name = AI_CLASS_NAMES[predicted_class];
                
                // Format: result: [logits] -> CLASS (XX%)
                char msg[256];
                snprintf(msg, sizeof(msg),
                    "result: [%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f] -> %s(%.0f%%)\\r\\n",
                    output[0], output[1], output[2], output[3], 
                    output[4], output[5], output[6],
                    class_name, confidence
                );
                usb->sendMessage(msg);
                
                // DIAGNOSTIC: Log stats every 5 inferences
                // Now includes sample diagnostics
                // static uint16_t s_last_min_samples = 999;
                // static uint16_t s_last_max_samples = 0;
                
                // Calculate min/max samples for this batch
                uint16_t batch_min = 999, batch_max = 0;
                for (uint8_t f = 0; f < AI_WINDOW_FRAMES; f++) {
                    uint16_t cnt = received_batch->sample_counts[f];
                    if (cnt < batch_min) batch_min = cnt;
                    if (cnt > batch_max) batch_max = cnt;
                }
                // s_last_min_samples = batch_min;
                // s_last_max_samples = batch_max;
                
                if (g_pipeline_stats_3task.inferences_done % 5 == 0) {
                    char stats_msg[160];
                    snprintf(stats_msg, sizeof(stats_msg),
                        "[STATS] B:%lu Inf:%lu D:%lu Samp:%u-%u Zero:%d\r\n",
                        g_pipeline_stats_3task.batches_completed,
                        g_pipeline_stats_3task.inferences_done,
                        g_pipeline_stats_3task.batch_queue_full_drops,
                        batch_min, batch_max, zero_count_frames);
                    usb->sendMessage(stats_msg);
                }
                
                g_pipeline_stats_3task.inferences_done++;
            } else {
                g_pipeline_stats_3task.inference_errors++;
            }
        }
    }
    
    vTaskDelete(NULL);
}
