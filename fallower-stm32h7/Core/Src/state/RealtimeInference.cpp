#include "RealtimeInference.hpp"
#include "StateMachine.hpp"
#include "../UsbCommunication.hpp"
#include <cstring>
#include <cmath>
#include <stdio.h>
#include <climits>
#include "cmsis_os.h"

// Radar capture includes (from Stream.cpp)
#include "../radar_flags.h"
#include "main.h"

// External AI function from UsbCommunication.cpp
extern float* ai_fallower_run_inference_direct(const float* input_data);

// External ADC and radar flags
extern ADC_HandleTypeDef hadc3;
extern RadarFlags_t g_radarFlags;

// Test data include (for validation)
#include "../data/sit_2751_test_data.h"

// ============================================================================
// MEMORY ORGANIZATION
// ============================================================================

// SDRAM: Frame pool (350KB @ 256 frames - MOVED from DTCMRAM due to size)
// CRITICAL: Was in DTCMRAM but overflowed (256×1400B = 350KB > 128KB DTCMRAM)
__attribute__((section(".sdram_data"))) __attribute__((aligned(32)))
static RadarFrame_t frame_pool[FRAME_POOL_SIZE];

// SDRAM: Circular window for normalized frames (82KB)
__attribute__((section(".sdram_data"))) __attribute__((aligned(32)))
static CircularWindow_t circular_window = {0};

// SDRAM: Linear window for AI input (82KB)
__attribute__((section(".sdram_data"))) __attribute__((aligned(32)))
static float linear_window[AI_WINDOW_FRAMES][FRAME_SAMPLES];

// SDRAM: Temporary buffers for MTI processing (124KB total)
__attribute__((section(".sdram_data"))) __attribute__((aligned(32)))
static int16_t raw_linear_window[AI_WINDOW_FRAMES][FRAME_SAMPLES];

__attribute__((section(".sdram_data"))) __attribute__((aligned(32)))
static float filtered_window[AI_WINDOW_FRAMES][FRAME_SAMPLES];

// FreeRTOS: Frame queue (SRAM - 128 bytes)
// CRITICAL: Must be GLOBAL (not static) for task functions to access
QueueHandle_t frameQueue = NULL;

// Task handles (global for GPIO interrupt access)
TaskHandle_t producerTaskHandle = NULL;
TaskHandle_t consumerTaskHandle = NULL;

// Global statistics and control
PipelineStats_t g_pipeline_stats = {0};
volatile bool is_inference_running = false;
volatile bool producer_paused = false;  // CRITICAL: Pause producer during AI inference to prevent CircularWindow corruption

// ============================================================================
// STATE MACHINE HANDLERS
// ============================================================================

void RealtimeInference::OnEnter() {
    UsbCommunication* usb = UsbCommunication::getInstance();
    usb->sendStatusMessage("RTINF", "Real-time inference mode activated");

    // Check available heap memory
    size_t free_heap = xPortGetFreeHeapSize();
    char heap_msg[128];
    snprintf(heap_msg, sizeof(heap_msg), "Free heap: %u bytes (need ~24KB for tasks+queue)", (unsigned int)free_heap);
    usb->sendStatusMessage("RTINF_MEM", heap_msg);

    if (free_heap < 25000) {
        usb->sendStatusMessage("ERROR", "Insufficient heap memory for real-time inference!");
        return;
    }

    // Reset statistics
    memset(&g_pipeline_stats, 0, sizeof(g_pipeline_stats));

    // Reset circular window
    memset(&circular_window, 0, sizeof(circular_window));

    // CRITICAL FIX: Clear linear window to prevent garbage data in first inference
    memset(&linear_window, 0, sizeof(linear_window));

    // Initialize radar flags (from Stream.cpp OnEnter)
    g_radarFlags.ssFlag = (HAL_GPIO_ReadPin(RADAR_SS_GPIO_Port, RADAR_SS_Pin) == GPIO_PIN_SET);
    g_radarFlags.intgClkFlag = 0;
    g_radarFlags.lastSSTime = HAL_GetTick();
    g_radarFlags.lastIntgTime = HAL_GetTick();

    // Create frame queue (32 frame pointers)
    if (frameQueue == NULL) {
        frameQueue = xQueueCreate(FRAME_POOL_SIZE, sizeof(RadarFrame_t*));
        if (frameQueue == NULL) {
            usb->sendStatusMessage("ERROR", "Failed to create frame queue");
            return;
        }

        // Register queue name for FreeRTOS debugger visibility
        #if (configQUEUE_REGISTRY_SIZE > 0)
        vQueueAddToRegistry(frameQueue, "FrameQueue");
        #endif

        usb->sendStatusMessage("RTINF", "Frame queue created successfully");
    } else {
        usb->sendStatusMessage("RTINF", "Frame queue already exists");
    }

    // Set running flag
    is_inference_running = true;

    // Create producer task (ADC capture)
    if (producerTaskHandle == NULL) {
        BaseType_t result = xTaskCreate(
            RadarProducerTask,
            "RadarProducer",
            2048,  // Stack size
            NULL,
            osPriorityAboveNormal,  // AŞAMA 1 FIX: High priority to prevent frame loss during inference
            &producerTaskHandle
        );

        if (result != pdPASS) {
            usb->sendStatusMessage("ERROR", "Failed to create producer task");
            is_inference_running = false;
            return;
        }

        // CRITICAL FIX: Give producer task time to start and enter xTaskNotifyWait()
        // Without this delay, GPIO interrupts may arrive before task is ready
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Create consumer task (AI inference)
    if (consumerTaskHandle == NULL) {
        BaseType_t result = xTaskCreate(
            InferenceConsumerTask,
            "InferenceConsumer",
            16384,  // CRITICAL FIX: 64KB stack for AI Conv2D + MobileNetV3 layers (was 32KB due to stack overflow)
            NULL,
            osPriorityNormal,  // Normal priority
            &consumerTaskHandle
        );

        if (result != pdPASS) {
            usb->sendStatusMessage("ERROR", "Failed to create consumer task");
            is_inference_running = false;
            if (producerTaskHandle) {
                vTaskDelete(producerTaskHandle);
                producerTaskHandle = NULL;
            }
            return;
        }

        // CRITICAL FIX: Give consumer task time to start
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    usb->sendStatusMessage("RTINF", "Producer and Consumer tasks started");
}

void RealtimeInference::OnExit() {
    UsbCommunication* usb = UsbCommunication::getInstance();

    // Stop tasks
    is_inference_running = false;

    // Wait for tasks to complete
    vTaskDelay(pdMS_TO_TICKS(100));

    // Delete tasks
    if (producerTaskHandle) {
        vTaskDelete(producerTaskHandle);
        producerTaskHandle = NULL;
    }

    if (consumerTaskHandle) {
        vTaskDelete(consumerTaskHandle);
        consumerTaskHandle = NULL;
    }

    // Delete queue
    if (frameQueue) {
        vQueueDelete(frameQueue);
        frameQueue = NULL;
    }

    // Send final statistics
    char msg[256];
    snprintf(msg, sizeof(msg),
        "Stats: Produced=%lu Consumed=%lu Inferences=%lu Errors=%lu Dropped=%lu Timeouts=%lu",
        g_pipeline_stats.frames_produced,
        g_pipeline_stats.frames_consumed,
        g_pipeline_stats.inferences_done,
        g_pipeline_stats.inference_errors,
        g_pipeline_stats.frames_dropped,
        g_pipeline_stats.producer_timeout_count
    );
    usb->sendStatusMessage("RTINF_STATS", msg);

    // Send timing statistics
    char timing_msg[128];
    snprintf(timing_msg, sizeof(timing_msg),
        "Timing: Avg=%lums Max=%lums",
        g_pipeline_stats.avg_inference_time_ms,
        g_pipeline_stats.max_inference_time_ms
    );
    usb->sendStatusMessage("RTINF_TIMING", timing_msg);

    // Send stack usage statistics
    char stack_msg[256];
    snprintf(stack_msg, sizeof(stack_msg),
        "Stack: Producer=%lu words free, Consumer=%lu words free",
        g_pipeline_stats.producer_stack_hwm,
        g_pipeline_stats.consumer_stack_hwm
    );
    usb->sendStatusMessage("RTINF_STACK", stack_msg);

    usb->sendStatusMessage("RTINF", "Inference mode deactivated");
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void PerFrameNormalize(const float* samples, float* output, uint16_t count) {
    // Find min/max for THIS FRAME
    float min_val = samples[0];
    float max_val = samples[0];

    for (uint16_t i = 1; i < count; i++) {
        if (samples[i] < min_val) min_val = samples[i];
        if (samples[i] > max_val) max_val = samples[i];
    }

    float range = max_val - min_val;

    // Handle zero-range case (constant signal)
    if (range < 1e-6f) {
        memset(output, 0, count * sizeof(float));
    } else {
        // Normalize to [0,1] then scale to [-1000, 1000]
        for (uint16_t i = 0; i < count; i++) {
            float norm_01 = (samples[i] - min_val) / range;
            output[i] = -1000.0f + norm_01 * 2000.0f;
        }
    }
}

void ApplyMTIFilter(const int16_t input_window[AI_WINDOW_FRAMES][FRAME_SAMPLES], float output_window[AI_WINDOW_FRAMES][FRAME_SAMPLES]) {
    // Note: Python logic:
    // for i in range(ref_avg_num, num_frames):
    //     ref_avg_data = np.mean(data[i - ref_avg_num:i, :], axis=0)
    //     tag_avg_data = np.mean(data[i - tag_avg_num:i, :], axis=0)
    //     diff_moving_avg_filtered_data[i, :] = tag_avg_data - ref_avg_data

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

void LinearizeCircularWindow(int16_t output[AI_WINDOW_FRAMES][FRAME_SAMPLES]) {
    // Oldest frame is at write_idx (chronologically first)
    uint8_t read_start = circular_window.write_idx;

    // Copy frames in chronological order
    for (int i = 0; i < AI_WINDOW_FRAMES; i++) {
        uint8_t src_idx = (read_start + i) % AI_WINDOW_FRAMES;
        memcpy(output[i],
               circular_window.frames[src_idx],
               FRAME_SAMPLES * sizeof(int16_t));
    }
}

void ProcessAIResult(const float* output) {
    UsbCommunication* usb = UsbCommunication::getInstance();

    if (!output) {
        return;
    }

    // Find max class
    int max_class = 0;
    float max_logit = output[0];
    for (int i = 1; i < 7; i++) {
        if (output[i] > max_logit) {
            max_logit = output[i];
            max_class = i;
        }
    }

    // Calculate softmax for confidence (numerically stable version)
    // Subtract max logit to prevent overflow: exp(x - max) / sum(exp(x - max))
    float exp_sum = 0.0f;
    float exp_vals[7];
    for (int i = 0; i < 7; i++) {
        exp_vals[i] = expf(output[i] - max_logit);  // Prevent overflow
        exp_sum += exp_vals[i];
    }
    float confidence = exp_vals[max_class] / exp_sum;

    // Class names
    const char* classes[] = {
        "FALL", "GETUP", "GETUPGR", "SIT",
        "STILLMOVE", "STILLNOACT", "WALK"
    };

    // Send result in parse-friendly format
    char result[256];
    snprintf(result, sizeof(result),
        "<AI_RESULT>class=%s,confidence=%.3f,logit=%.3f,frame=%lu\r\n",
        classes[max_class],
        confidence,
        max_logit,
        g_pipeline_stats.inferences_done + 1
    );
    usb->sendMessage(result);

    // Send all probabilities for debugging
    char probs[512];
    int offset = snprintf(probs, sizeof(probs), "[AI_PROBS] ");
    for (int i = 0; i < 7; i++) {
        float prob = exp_vals[i] / exp_sum;
        offset += snprintf(probs + offset, sizeof(probs) - offset,
            "%s:%.3f ", classes[i], prob);
    }
    snprintf(probs + offset, sizeof(probs) - offset, "\r\n");
    usb->sendMessage(probs);
}

// ============================================================================
// PRODUCER TASK (Radar Capture)
// ============================================================================

void RadarProducerTask(void* params) {
    UsbCommunication* usb = UsbCommunication::getInstance();
    usb->sendStatusMessage("PRODUCER", "Radar producer task started - Real ADC capture");

    // CRITICAL: Verify frameQueue exists
    if (frameQueue == NULL) {
        usb->sendStatusMessage("PRODUCER_ERROR", "CRITICAL: frameQueue is NULL! Cannot continue.");
        vTaskDelete(NULL);
        return;
    }

    char queue_info[128];
    UBaseType_t queue_spaces = uxQueueSpacesAvailable(frameQueue);
    snprintf(queue_info, sizeof(queue_info),
        "Frame queue OK: %lu slots available", queue_spaces);
    usb->sendStatusMessage("PRODUCER", queue_info);

    uint32_t notificationValue;
    uint8_t pool_index = 0;
    uint32_t frame_counter = 0;
    uint16_t sample_count = 0;
    uint32_t frame_start_time = 0;

    // CRITICAL FIX: Don't wait for initial SS_HIGH/SS_LOW!
    // Radar is already streaming, jump directly into main loop
    // and synchronize with whatever event comes next
    usb->sendStatusMessage("PRODUCER", "Entering main data collection loop (auto-sync)");

    while (is_inference_running) {
        if (xTaskNotifyWait(0, ULONG_MAX, &notificationValue, pdMS_TO_TICKS(100)) == pdTRUE) {

            // INTG_CLK event - take ADC sample (Line 245-259)
            if ((notificationValue & INTG_CLK_EVENT) != 0) {
                if (g_radarFlags.ssFlag == 0) {  // Only sample when SS is low
                    HAL_ADC_Start(&hadc3);

                    if (HAL_ADC_PollForConversion(&hadc3, 1) == HAL_OK) {
                        // Get ADC value and convert to signed int16
                        int16_t sample = (int16_t)(HAL_ADC_GetValue(&hadc3) - 32768);

                        if (sample_count < FRAME_SAMPLES) {
                            frame_pool[pool_index].samples[sample_count++] = sample;
                        }
                    }

                    HAL_ADC_Stop(&hadc3);
                }
            }

            // SS high - frame completed (Line 262-286)
            if ((notificationValue & SS_HIGH_EVENT) != 0) {
                // DEBUG: Log SS_HIGH event
                if (frame_counter < 3) {
                    char msg[64];
                    snprintf(msg, sizeof(msg), "[EVENT] SS_HIGH received, samples=%u", sample_count);
                    usb->sendStatusMessage("PRODUCER_DEBUG", msg);
                }

                // CRITICAL FIX: Skip first incomplete frame (started mid-cycle)
                // Only process frames with full 688 samples
                if (sample_count >= FRAME_SAMPLES) {
                    // Frame is already 688 samples, no padding needed

                    // Fill frame metadata
                    frame_pool[pool_index].timestamp = frame_start_time;
                    frame_pool[pool_index].frame_number = ++frame_counter;

                    // CRITICAL: Cache coherency for SDRAM (Producer writes, Consumer reads)
                    // Clean D-Cache to ensure data is written to SDRAM before Consumer reads
                    RadarFrame_t* frame_ptr = &frame_pool[pool_index];
                    SCB_CleanDCache_by_Addr((uint32_t*)frame_ptr, sizeof(RadarFrame_t));

                    // Send frame pointer to consumer queue (10ms timeout to prevent blocking)
                    BaseType_t queue_result = xQueueSend(frameQueue, &frame_ptr, pdMS_TO_TICKS(10));

                    if (queue_result != pdPASS) {
                        g_pipeline_stats.frames_dropped++;

                        // DEBUG: First 10 drops
                        if (g_pipeline_stats.frames_dropped <= 10) {
                            char msg[64];
                            snprintf(msg, sizeof(msg), "QUEUE SEND FAILED! Dropped frame #%lu",
                                    g_pipeline_stats.frames_dropped);
                            usb->sendStatusMessage("PRODUCER_ERROR", msg);
                        }

                        // Periodic warning (every 100 drops)
                        static uint32_t last_drop_warning = 0;
                        if (g_pipeline_stats.frames_dropped >= last_drop_warning + 100) {
                            last_drop_warning = g_pipeline_stats.frames_dropped;
                            char msg[64];
                            snprintf(msg, sizeof(msg), "WARNING: %lu frames dropped!",
                                    g_pipeline_stats.frames_dropped);
                            usb->sendStatusMessage("PRODUCER_WARN", msg);
                        }
                    } else {
                        g_pipeline_stats.frames_produced++;

                        // Debug: Report first 35 frames to see inference trigger
                        if (g_pipeline_stats.frames_produced <= 35) {
                            char msg[64];
                            snprintf(msg, sizeof(msg), "Produced %lu frames (samples=%u)",
                                    g_pipeline_stats.frames_produced, sample_count);
                            usb->sendStatusMessage("PRODUCER", msg);
                        }
                    }

                    // Yield to allow the consumer to run (no delay)
                    taskYIELD();

                    // Move to next buffer in pool (circular)
                    pool_index = (pool_index + 1) % FRAME_POOL_SIZE;

                    // Reset for next frame
                    sample_count = 0;
                    frame_start_time = HAL_GetTick();
                } else {
                    // Incomplete frame (< 688 samples) - skip and reset
                    if (sample_count > 0) {
                        char msg[80];
                        snprintf(msg, sizeof(msg), "Skipping incomplete frame (%u samples < 688)", sample_count);
                        usb->sendStatusMessage("PRODUCER_DEBUG", msg);
                    }
                    sample_count = 0;
                    frame_start_time = HAL_GetTick();
                }
            }

            // SS low - new frame starting (Line 288-293)
            if ((notificationValue & SS_LOW_EVENT) != 0) {
                // DEBUG: Log SS_LOW event
                if (g_pipeline_stats.frames_produced < 3) {
                    usb->sendStatusMessage("PRODUCER_DEBUG", "[EVENT] SS_LOW received");
                }

                if (sample_count == 0) {
                    frame_start_time = HAL_GetTick();
                }
            }
        } else {
            // Timeout in main loop - likely no radar activity
            g_pipeline_stats.producer_timeout_count++;

            // DEBUG: Report first few timeouts to diagnose radar issue
            if (g_pipeline_stats.producer_timeout_count <= 10) {
                char msg[128];
                snprintf(msg, sizeof(msg),
                    "TIMEOUT #%lu: No GPIO events (waiting for INTG_CLK/SS_HIGH)",
                    g_pipeline_stats.producer_timeout_count);
                usb->sendStatusMessage("PRODUCER_DEBUG", msg);
            }

            // Periodic stack monitoring (every timeout = 100ms)
            static uint32_t stack_check_counter = 0;
            if (++stack_check_counter >= 50) {  // Check every 5 seconds (50 * 100ms)
                stack_check_counter = 0;

                // Get stack high water mark (minimum free stack ever reached)
                g_pipeline_stats.producer_stack_hwm = uxTaskGetStackHighWaterMark(NULL);

                // Calculate usage percentage
                uint32_t stack_total = 2048;  // From task creation
                uint32_t stack_used = stack_total - g_pipeline_stats.producer_stack_hwm;
                uint32_t usage_percent = (stack_used * 100) / stack_total;

                if (usage_percent > 85) {
                    // WARNING: Stack usage above 85%
                    char msg[128];
                    snprintf(msg, sizeof(msg),
                        "STACK WARNING: Producer %lu%% used (%lu/%lu words)",
                        usage_percent, stack_used, stack_total);
                    usb->sendStatusMessage("PRODUCER_STACK", msg);
                }
            }
        }
    }

    usb->sendStatusMessage("PRODUCER", "Radar producer task stopped");
    vTaskDelete(NULL);
}

// ============================================================================
// CONSUMER TASK (AI Inference Pipeline)
// ============================================================================

void InferenceConsumerTask(void* params) {
    UsbCommunication* usb = UsbCommunication::getInstance();
    usb->sendStatusMessage("CONSUMER", "Inference consumer task started");

    // CRITICAL: Verify frameQueue exists
    if (frameQueue == NULL) {
        usb->sendStatusMessage("CONSUMER_ERROR", "CRITICAL: frameQueue is NULL! Cannot continue.");
        vTaskDelete(NULL);
        return;
    }

    UBaseType_t queue_waiting = uxQueueMessagesWaiting(frameQueue);
    char queue_info[128];
    snprintf(queue_info, sizeof(queue_info),
        "Frame queue OK: %lu messages waiting", queue_waiting);
    usb->sendStatusMessage("CONSUMER", queue_info);

    RadarFrame_t* frame_ptr;
    static uint8_t frames_since_last_inference = 0;

    while (is_inference_running) {
        // Wait for frame from producer
        if (xQueueReceive(frameQueue, &frame_ptr, portMAX_DELAY) == pdPASS) {

            // CRITICAL: Cache coherency for SDRAM (Consumer reads, Producer wrote)
            // Invalidate D-Cache to ensure fresh data is read from SDRAM
            SCB_InvalidateDCache_by_Addr((uint32_t*)frame_ptr, sizeof(RadarFrame_t));

            // 1. STORE RAW SAMPLES to circular buffer
            uint8_t target_idx = circular_window.write_idx;
            memcpy(circular_window.frames[target_idx],
                   frame_ptr->samples,
                   FRAME_SAMPLES * sizeof(int16_t));

            // 2. Update circular window index
            circular_window.write_idx = (circular_window.write_idx + 1) % AI_WINDOW_FRAMES;
            if (circular_window.count < AI_WINDOW_FRAMES) {
                circular_window.count++;
            }

            g_pipeline_stats.frames_consumed++;

            // DEBUG: Report first 30 frames, then every 50
            if (g_pipeline_stats.frames_consumed <= 30 || g_pipeline_stats.frames_consumed % 50 == 0) {
                char msg[80];
                snprintf(msg, sizeof(msg), "Consumed %lu frames, window count=%u",
                        g_pipeline_stats.frames_consumed, circular_window.count);
                usb->sendStatusMessage("CONSUMER", msg);
            }

            // 3. Run inference when window is full + stride condition
            if (circular_window.count >= AI_WINDOW_FRAMES) {
                // DEBUG: Window full
                if (g_pipeline_stats.inferences_done == 0) {
                    char msg[80];
                    snprintf(msg, sizeof(msg), "Window FULL! count=%u, frames_since_last=%u",
                            circular_window.count, frames_since_last_inference);
                    usb->sendStatusMessage("CONSUMER_DEBUG", msg);
                }

                bool is_first_inference = (g_pipeline_stats.inferences_done == 0);
                frames_since_last_inference++;

                // STRIDE CHECK: Run on first fill, then every N frames
                if (is_first_inference || frames_since_last_inference >= INFERENCE_STRIDE_V1) {
                    frames_since_last_inference = 0;  // Reset counter

                    usb->sendStatusMessage("CONSUMER", "Running AI inference (30 frames ready)...");

                    uint32_t start_time = HAL_GetTick();

                    // 3a. LINEARIZE circular buffer
                    LinearizeCircularWindow(raw_linear_window);

                    // 3b. APPLY MTI FILTER
                    ApplyMTIFilter(raw_linear_window, filtered_window);

                    // 3c. PER-FRAME NORMALIZE filtered data
                    for (int i = 0; i < AI_WINDOW_FRAMES; i++) {
                        PerFrameNormalize(filtered_window[i], linear_window[i], FRAME_SAMPLES);
                    }

                    // DEBUG: Check input data validity before inference
                    char debug_msg[128];
                    snprintf(debug_msg, sizeof(debug_msg),
                        "[PRE_INF] Input[0-2]: %.2f %.2f %.2f | Filtered[0-2]: %.2f %.2f %.2f",
                        linear_window[MTI_REF_AVG_NUM][0], linear_window[MTI_REF_AVG_NUM][1], linear_window[MTI_REF_AVG_NUM][2],
                        filtered_window[MTI_REF_AVG_NUM][0], filtered_window[MTI_REF_AVG_NUM][1], filtered_window[MTI_REF_AVG_NUM][2]);
                    usb->sendStatusMessage("CONSUMER_DEBUG", debug_msg);

                    // 3b. CACHE COHERENCY (CRITICAL!)
                    SCB_CleanDCache_by_Addr(
                        (uint32_t*)linear_window,
                        sizeof(linear_window)
                    );
                    __DSB();
                    __ISB();

                    // 3c. AI INFERENCE
                    usb->sendStatusMessage("CONSUMER_DEBUG", "Calling ai_fallower_run_inference_direct...");

                    float* output = ai_fallower_run_inference_direct(
                        (const float*)linear_window
                    );

                    usb->sendStatusMessage("CONSUMER_DEBUG", "AI inference returned");

                    uint32_t elapsed_ms = HAL_GetTick() - start_time;

                    // 3d. Process result
                    if (output) {
                        ProcessAIResult(output);

                        // Update statistics
                        g_pipeline_stats.inferences_done++;
                        uint32_t total_time =
                            g_pipeline_stats.avg_inference_time_ms *
                            (g_pipeline_stats.inferences_done - 1) + elapsed_ms;
                        g_pipeline_stats.avg_inference_time_ms =
                            total_time / g_pipeline_stats.inferences_done;

                        if (elapsed_ms > g_pipeline_stats.max_inference_time_ms) {
                            g_pipeline_stats.max_inference_time_ms = elapsed_ms;
                        }

                        // Stack monitoring after AI inference (highest memory usage point)
                        g_pipeline_stats.consumer_stack_hwm = uxTaskGetStackHighWaterMark(NULL);

                        // Check every 10 inferences
                        if (g_pipeline_stats.inferences_done % 10 == 0) {
                            uint32_t stack_total = 16384;  // 64KB stack (from task creation)
                            uint32_t stack_used = stack_total - g_pipeline_stats.consumer_stack_hwm;
                            uint32_t usage_percent = (stack_used * 100) / stack_total;

                            if (usage_percent > 85) {
                                // CRITICAL WARNING: AI stack usage high!
                                char msg[128];
                                snprintf(msg, sizeof(msg),
                                    "STACK CRITICAL: Consumer %lu%% used (%lu/%lu words) - Consider increasing!",
                                    usage_percent, stack_used, stack_total);
                                usb->sendStatusMessage("CONSUMER_STACK", msg);
                            } else if (usage_percent > 70) {
                                // Informational: Stack usage report
                                char msg[128];
                                snprintf(msg, sizeof(msg),
                                    "Consumer stack: %lu%% used (%lu words free)",
                                    usage_percent, g_pipeline_stats.consumer_stack_hwm);
                                usb->sendStatusMessage("CONSUMER_INFO", msg);
                            }
                        }

                    } else {
                        usb->sendStatusMessage("AI_ERROR", "Inference returned NULL");
                        g_pipeline_stats.inference_errors++;
                    }
                }
            }
        }
    }

    usb->sendStatusMessage("CONSUMER", "Inference consumer task stopped");
    vTaskDelete(NULL);
}

// ============================================================================
// TEST INFERENCE FUNCTION (for validation with sit_2751_test_data)
// ============================================================================

void RunTestInference(const float* test_data) {
    UsbCommunication* usb = UsbCommunication::getInstance();

    if (!test_data) {
        usb->sendStatusMessage("ERROR", "Test data is NULL");
        return;
    }

    usb->sendStatusMessage("TEST_INF", "Starting test inference with sit_2751_test_data");

    uint32_t start_time = HAL_GetTick();

    // CRITICAL: Cache coherency
    // Test data is in Flash (.rodata), but still good practice
    SCB_CleanDCache_by_Addr((uint32_t*)test_data, AI_WINDOW_FRAMES * FRAME_SAMPLES * sizeof(float));
    __DSB();
    __ISB();

    // Run AI inference
    float* output = ai_fallower_run_inference_direct(test_data);

    uint32_t elapsed_ms = HAL_GetTick() - start_time;

    if (output) {
        // Process result
        ProcessAIResult(output);

        // Update statistics
        g_pipeline_stats.inferences_done++;

        uint32_t total_time =
            g_pipeline_stats.avg_inference_time_ms * (g_pipeline_stats.inferences_done - 1) + elapsed_ms;
        g_pipeline_stats.avg_inference_time_ms = total_time / g_pipeline_stats.inferences_done;

        if (elapsed_ms > g_pipeline_stats.max_inference_time_ms) {
            g_pipeline_stats.max_inference_time_ms = elapsed_ms;
        }

        // Send timing info
        char msg[128];
        snprintf(msg, sizeof(msg),
            "Inference completed in %lums (avg=%lums, max=%lums)",
            elapsed_ms,
            g_pipeline_stats.avg_inference_time_ms,
            g_pipeline_stats.max_inference_time_ms
        );
        usb->sendStatusMessage("TEST_INF", msg);

    } else {
        usb->sendStatusMessage("ERROR", "AI inference returned NULL");
        g_pipeline_stats.inference_errors++;
    }
}
