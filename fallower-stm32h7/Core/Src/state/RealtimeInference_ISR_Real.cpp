/**
 * @file RealtimeInference_DMA_Real.cpp
 * @brief Real-time Stride-14 AI Inference Implementation
 * 
 * 20MB Circular Buffer + CATCH-UP Inference (NO QUEUE DROP!)
 *
 * Architecture:
 * - ISR: Writes ALL frames to circular buffer (NEVER drops)
 * - Task: Processes with stride-13 (REAL_STRIDE=13), skips ahead if behind
 * 
 * @author Ethem
 * @date 2026
 */

#include "RealtimeInference_ISR_Real.hpp"
#include "StateMachine.hpp"
#include "../UsbCommunication.hpp"
#include "main.h"
#include "../radar_flags.h"
#include "../radar_driver.hpp"  // For SPI_Send_Inference_Result
#include <cmath>
#include <cstdio>
#include <cstring>
#include "cmsis_os.h"
#include "cache_utils.hpp"

// ============================================================================
// EXTERNAL DEPENDENCIES
// ============================================================================

extern ADC_HandleTypeDef hadc3;
extern RadarFlags_t g_radarFlags;
extern float* ai_fallower_run_inference_direct(const float* input_data);
extern uint8_t _ai_activation_start[];
extern uint8_t _ai_activation_end[];

// Reuse helpers from DMA module
extern uint8_t ISR_GetPredictedClass(const float* output, float* confidence);
extern const char* const ISR_AI_CLASS_NAMES[];

// ============================================================================
// MEMORY ALLOCATION - 20MB Circular Buffer in SDRAM
// ============================================================================

__attribute__((section(".sdram_data"))) __attribute__((aligned(32)))
static CircularFrame_t s_circular_buffer_storage[CIRCULAR_BUFFER_FRAMES];

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

volatile bool g_isr_real_mode = false;
volatile bool g_isr_real_running = false;
volatile uint32_t g_real_write_idx = 0;
volatile uint32_t g_real_total_frames = 0;

CircularFrame_t* g_circular_buffer = s_circular_buffer_storage;
QueueHandle_t g_window_queue = NULL;  // Kept for compatibility but NOT USED
RealPipelineStats_t g_real_stats = {0};

// Task handle
static TaskHandle_t s_real_processing_task = NULL;

// ISR state
static volatile uint16_t s_real_sample_count = 0;
static volatile bool s_real_frame_in_progress = false;
static volatile bool s_real_first_sync = false;

// Wraparound tracking (must reset between runs - NOT static inside functions)
static bool s_wraparound_notified = false;
static bool s_wraparound_msg_sent = false;

// Static task memory
__attribute__((section(".dtcmram"))) static StackType_t s_real_task_stack[4096];
__attribute__((section(".dtcmram"))) static StaticTask_t s_real_task_tcb;

// AI Input buffer (reuse from ISR module)
extern DMAInputData_t g_isr_ai_input_buffer;

// SDRAM: Temporary buffer for MTI filtered data (82KB)
__attribute__((section(".sdram_data"))) __attribute__((aligned(32)))
static float s_mti_filtered_window[REAL_WINDOW_FRAMES][CIRCULAR_FRAME_SAMPLES];

// SDRAM: Linearized raw window for MTI (42KB)
__attribute__((section(".sdram_data"))) __attribute__((aligned(32)))
static CircularFrame_t s_linear_raw_window[REAL_WINDOW_FRAMES];

// Helper for MTI
static void ISR_ApplyMTIFilter(const CircularFrame_t* window, float output[REAL_WINDOW_FRAMES][CIRCULAR_FRAME_SAMPLES]) {
    // Zero out initial frames
    memset(output, 0, MTI_REF_AVG_NUM * CIRCULAR_FRAME_SAMPLES * sizeof(float));

    for (int i = MTI_REF_AVG_NUM; i < REAL_WINDOW_FRAMES; i++) {
        for (int b = 0; b < CIRCULAR_FRAME_SAMPLES; b++) {
            float ref_sum = 0;
            for (int k = i - MTI_REF_AVG_NUM; k < i; k++) {
                ref_sum += (float)window[k].samples[b];
            }
            float ref_avg = ref_sum / MTI_REF_AVG_NUM;

            float tag_sum = 0;
            for (int k = i - MTI_TAG_AVG_NUM; k < i; k++) {
                tag_sum += (float)window[k].samples[b];
            }
            float tag_avg = tag_sum / MTI_TAG_AVG_NUM;

            output[i][b] = tag_avg - ref_avg;
        }
    }
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void ISR_Real_Pipeline_Init(void) {
    memset(&g_real_stats, 0, sizeof(g_real_stats));
    
    g_real_write_idx = 0;
    g_real_total_frames = 0;
    s_real_sample_count = 0;
    s_real_frame_in_progress = false;
    s_real_first_sync = false;
    s_wraparound_notified = false;
    s_wraparound_msg_sent = false;
    
    // Clear first few frames
    memset(s_circular_buffer_storage, 0, sizeof(CircularFrame_t) * 100);
    Safe_SCB_CleanDCache_by_Addr((uint32_t*)s_circular_buffer_storage, sizeof(CircularFrame_t) * 100);
}

void ISR_Real_Pipeline_Cleanup(void) {
    g_isr_real_running = false;
    g_isr_real_mode = false;
    
    // Stop ADC
    if (hadc3.Instance->CR & ADC_CR_ADSTART) {
        hadc3.Instance->CR |= ADC_CR_ADSTP;
        volatile uint32_t timeout = 10000;
        while ((hadc3.Instance->CR & ADC_CR_ADSTP) && timeout--);
    }
    hadc3.Instance->CR |= ADC_CR_ADDIS;
    
    // Delete task
    if (s_real_processing_task) {
        vTaskDelete(s_real_processing_task);
        s_real_processing_task = NULL;
    }
}

// ============================================================================
// ISR HANDLERS - JUST WRITE TO BUFFER, NO QUEUE!
// ============================================================================

void ISR_Real_HandleSSLow(void) {
    if (!g_isr_real_running) return;
    if (!s_real_first_sync) return;
    
    // ═══════════════════════════════════════════════════════════
    // CRITICAL FIX: REMOVED buffer full check!
    // Old code: if (g_real_total_frames >= CIRCULAR_BUFFER_FRAMES) return;
    // 
    // PROBLEM: Stopping ISR caused crash because:
    //   - Radar hardware continues sending INTG_CLK interrupts
    //   - ADC operations conflict without data collection
    //   - Result: HardFault or system freeze
    // 
    // SOLUTION: Allow wraparound (circular behavior)
    //   - ISR continues writing indefinitely
    //   - Modulo operation (line 130) handles wraparound
    //   - Task has overwrite protection (lines 253-276)
    //   - Oldest data gets overwritten (expected circular behavior)
    // ═══════════════════════════════════════════════════════════
    
    // Start new frame
    s_real_sample_count = 0;
    s_real_frame_in_progress = true;
    
    uint32_t idx = g_real_write_idx % CIRCULAR_BUFFER_FRAMES;  // Wraparound via modulo
    g_circular_buffer[idx].timestamp = HAL_GetTick();
}

void ISR_Real_HandleSSHigh(void) {
    if (!g_isr_real_running) return;
    
    // First SS_HIGH = sync point
    if (!s_real_first_sync) {
        s_real_first_sync = true;
        s_real_sample_count = 0;
        return;
    }
    
    if (!s_real_frame_in_progress) return;
    if (s_real_sample_count == 0) return;
    
    uint32_t idx = g_real_write_idx % CIRCULAR_BUFFER_FRAMES;
    g_circular_buffer[idx].sample_count = s_real_sample_count;
    
    // ═══════════════════════════════════════════════════════════
    // CRITICAL FIX: Cache Clean BEFORE incrementing write index
    // Ensures data is physically in SDRAM before Task can see new index
    // ═══════════════════════════════════════════════════════════
    Safe_SCB_CleanDCache_by_Addr((uint32_t*)&g_circular_buffer[idx], sizeof(CircularFrame_t));
    
    // Wraparound notification (one-time per run - reset in ISR_Real_Pipeline_Init)
    if (g_real_total_frames >= CIRCULAR_BUFFER_FRAMES && !s_wraparound_notified) {
        s_wraparound_notified = true;
        g_real_stats.buffer_full = true;  // Repurpose flag as "wraparound active"
        // Data collection CONTINUES - buffer now wraps around (oldest data overwritten)
    }
    
    g_real_write_idx++;
    g_real_total_frames++;
    g_real_stats.total_frames_collected++;
    s_real_frame_in_progress = false;
    
    // NO QUEUE! Task drives itself with REAL_STRIDE=13
}

void ISR_Real_HandleIntgClk(void) {
    if (!g_isr_real_running) return;
    if (!s_real_first_sync) return;
    if (!s_real_frame_in_progress) return;
    if (g_radarFlags.ssFlag != 0) return;
    
    // ═══════════════════════════════════════════════════════════
    // CRITICAL: Double-check with local copy (interrupt-safe)
    // ═══════════════════════════════════════════════════════════
    uint16_t current_count = s_real_sample_count;
    if (current_count >= CIRCULAR_FRAME_SAMPLES) return;
    
    // Direct ADC read
    hadc3.Instance->ISR = ADC_ISR_EOC | ADC_ISR_EOS | ADC_ISR_OVR;
    hadc3.Instance->CR |= ADC_CR_ADSTART;
    
    volatile uint32_t timeout = 300;
    while (!(hadc3.Instance->ISR & ADC_ISR_EOC)) {
        if (--timeout == 0) return;
    }
    
    uint32_t raw = hadc3.Instance->DR;
    int16_t sample = (int16_t)(raw) - 32768;
    
    uint32_t idx = g_real_write_idx % CIRCULAR_BUFFER_FRAMES;
    
    // ═══════════════════════════════════════════════════════════
    // CRITICAL: Bounds check BEFORE write protection
    // To prevent overwriting next fields if count corruption occurred
    // ═══════════════════════════════════════════════════════════
    if (current_count < CIRCULAR_FRAME_SAMPLES) {
        g_circular_buffer[idx].samples[current_count] = sample;
        s_real_sample_count = current_count + 1;
    }
}

// ============================================================================
// PROCESSING TASK - SELF-DRIVEN STRIDE-5, CATCHES UP IF BEHIND
// ============================================================================

void ISR_Real_ProcessingTask(void* params) {
    (void)params;
    
    UsbCommunication* usb = UsbCommunication::getInstance();
    
    // Track our own position
    uint32_t next_window_start = 0;  // Start of next window to process
    uint32_t window_id = 0;
    
    // Temporary linearized window for MTI (uses global SDRAM buffer)
    // No local static to save RAM_D1 space
    
    if (usb) {
        char msg[128];
        snprintf(msg, sizeof(msg), 
            "[REAL] Started - Buffer:%d frames (~%dMB) Stride:%d (NO DROP!)\r\n",
            CIRCULAR_BUFFER_FRAMES, 
            (int)(CIRCULAR_BUFFER_FRAMES * sizeof(CircularFrame_t) / (1024*1024)),
            REAL_STRIDE);
        usb->sendMessage(msg);
    }
    
    // Wait for first 30 frames
    while (g_isr_real_running && g_real_total_frames < REAL_WINDOW_FRAMES) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    if (usb) usb->sendMessage("[REAL] First window ready, starting inference...\r\n");
    
    while (g_isr_real_running) {
        
        // Calculate available frames
        uint32_t current_total = g_real_total_frames;
        uint32_t oldest_valid = (current_total > CIRCULAR_BUFFER_FRAMES) ?
                                (current_total - CIRCULAR_BUFFER_FRAMES) : 0;
        
        // Wait until we have enough data for a window
        if (next_window_start + REAL_WINDOW_FRAMES > current_total) {
            // Not enough frames yet, wait
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }
        
        // Check if our target window data was OVERWRITTEN (buffer wraparound)
        // ONLY skip if data is lost - otherwise just stay behind with REAL_STRIDE=13
        if (next_window_start < oldest_valid) {
            // Data overwritten! Must skip to oldest valid + align to stride
            uint32_t skipped_frames = oldest_valid - next_window_start;
            
            // Align to stride boundary
            next_window_start = oldest_valid;
            uint32_t remainder = next_window_start % REAL_STRIDE;
            if (remainder != 0) {
                next_window_start += (REAL_STRIDE - remainder);
            }
            
            g_real_stats.windows_skipped += (skipped_frames / REAL_STRIDE);
            
            if (usb) {
                char msg[128];
                snprintf(msg, sizeof(msg), 
                    "[REAL] BUFFER OVERFLOW - Data lost! Skipped %lu frames to %lu\r\n",
                    (unsigned long)skipped_frames, (unsigned long)next_window_start);
                usb->sendMessage(msg);
            }
            continue;
        }
        
        // Normal case: Process this window (even if we're behind - that's OK!)
        
        // ================================================================
        // PREPROCESSING
        // ================================================================
        bool window_valid = true;
        
        // 1. LINEARIZE and Validate sample counts
        for (uint8_t f = 0; f < REAL_WINDOW_FRAMES; f++) {
            uint32_t frame_idx = (next_window_start + f) % CIRCULAR_BUFFER_FRAMES;
            CircularFrame_t* src = &g_circular_buffer[frame_idx];

            Safe_SCB_InvalidateDCache_by_Addr((uint32_t*)src, sizeof(CircularFrame_t));

            if (src->sample_count != CIRCULAR_FRAME_SAMPLES) {
                if (usb) {
                    char err[128];
                    snprintf(err, sizeof(err), "[REAL_ERR] Win%lu dropped! Frame%lu has %u samples\r\n",
                        (unsigned long)window_id, (unsigned long)frame_idx, src->sample_count);
                    usb->sendMessage(err);
                }
                window_valid = false;
                break;
            }
            
            // Linearize to temporary buffer for MTI
            memcpy(&s_linear_raw_window[f], src, sizeof(CircularFrame_t));
        }
        
        if (!window_valid) {
            next_window_start += REAL_STRIDE;
            window_id++;
            continue;
        }

        // 2. APPLY MTI FILTER (Matches Python differential_moving_average)
        ISR_ApplyMTIFilter(s_linear_raw_window, s_mti_filtered_window);

        // 3. PER-FRAME NORMALIZE filtered data
        for (uint8_t f = 0; f < REAL_WINDOW_FRAMES; f++) {
            float frame_min = s_mti_filtered_window[f][0];
            float frame_max = s_mti_filtered_window[f][0];

            for (uint16_t s = 1; s < CIRCULAR_FRAME_SAMPLES; s++) {
                if (s_mti_filtered_window[f][s] < frame_min) frame_min = s_mti_filtered_window[f][s];
                if (s_mti_filtered_window[f][s] > frame_max) frame_max = s_mti_filtered_window[f][s];
            }

            float range = frame_max - frame_min;
            if (range < 1e-6f) {
                memset(g_isr_ai_input_buffer.data[f], 0, CIRCULAR_FRAME_SAMPLES * sizeof(float));
            } else {
                const float inv_range = 1.0f / range;
                for (uint16_t s = 0; s < CIRCULAR_FRAME_SAMPLES; s++) {
                    float norm_01 = (s_mti_filtered_window[f][s] - frame_min) * inv_range;
                    g_isr_ai_input_buffer.data[f][s] = -1000.0f + norm_01 * 2000.0f;
                }
            }
        }
        
        if (!window_valid) {
            // Skip this window, move to next stride
            next_window_start += REAL_STRIDE;
            window_id++;
            continue;
        }
        
        g_isr_ai_input_buffer.batch_id = window_id;
        Safe_SCB_CleanDCache_by_Addr((uint32_t*)&g_isr_ai_input_buffer, sizeof(DMAInputData_t));

        // ================================================================
        // AI INFERENCE
        // ================================================================
        // NOTE: Activation buffer cache ops moved to ai_fallower_run_inference_direct
        // Removed redundant CleanInvalidate here (was double operation)
        __DSB();

        uint32_t start_time = HAL_GetTick();
        float* output = ai_fallower_run_inference_direct((const float*)g_isr_ai_input_buffer.data);
        uint32_t end_time = HAL_GetTick();
        uint32_t duration = end_time - start_time;
        
        g_real_stats.last_inference_ms = duration;
        g_real_stats.windows_processed++;
        
        // ================================================================
        // OUTPUT
        // ================================================================
        if (output && usb) {
            float confidence;
            uint8_t predicted_class = ISR_GetPredictedClass(output, &confidence);
            const char* class_name = ISR_AI_CLASS_NAMES[predicted_class];
            
            // Calculate lag (how far behind we are)
            uint32_t current_frame = g_real_total_frames;
            int32_t lag = (int32_t)(current_frame - (next_window_start + REAL_WINDOW_FRAMES));
            
            char msg[192];
            snprintf(msg, sizeof(msg),
                "[REAL] Win%lu (%lu-%lu) → %s(%.0f%%) T:%lums Lag:%ld\r\n",
                (unsigned long)window_id,
                (unsigned long)next_window_start,
                (unsigned long)(next_window_start + REAL_WINDOW_FRAMES - 1),
                class_name, confidence,
                (unsigned long)duration,
                (long)lag);
            usb->sendMessage(msg);
            
            // Send inference result to ESP32 via SPI (Phase 2 Integration)
            // Packet: [SYNC(0xA5), CMD(0x21), CLASS_ID, CONFIDENCE, END(0x5A)]
            // Non-blocking with 100ms timeout - minimal latency impact (~1ms)
            SPI_Send_Inference_Result(predicted_class, confidence);
            
            // Stats every 50 inferences (reduced to prevent USB flood)
            if (g_real_stats.windows_processed % 50 == 0) {
                char stats[160];
                snprintf(stats, sizeof(stats),
                    "[REAL_STATS] Frames:%lu/%d Wins:%lu Lag:%ld%s\r\n",
                    (unsigned long)g_real_stats.total_frames_collected,
                    CIRCULAR_BUFFER_FRAMES,
                    (unsigned long)g_real_stats.windows_processed,
                    (long)lag,
                    g_real_stats.buffer_full ? " WRAPAROUND" : "");
                usb->sendMessage(stats);
            }
            
            // DEBUG: Check frame integrity every 100 windows
            if (g_real_stats.windows_processed % 100 == 0) {
                 uint32_t bad_frames = 0;
                 for (uint8_t f = 0; f < REAL_WINDOW_FRAMES; f++) {
                    uint32_t f_idx = (next_window_start + f) % CIRCULAR_BUFFER_FRAMES;
                    if (g_circular_buffer[f_idx].sample_count != CIRCULAR_FRAME_SAMPLES) {
                        bad_frames++;
                    }
                 }
                 if (bad_frames > 0) {
                     char dbg[100];
                     snprintf(dbg, sizeof(dbg), "[REAL_WARN] Win%lu has %lu incomplete frames!\r\n", 
                        (unsigned long)window_id, (unsigned long)bad_frames);
                     usb->sendMessage(dbg);
                 }
            }
            
            // Wraparound notification (one-time per run - reset in ISR_Real_Pipeline_Init)
            if (g_real_stats.buffer_full && !s_wraparound_msg_sent) {
                s_wraparound_msg_sent = true;
                usb->sendMessage("[REAL] *** WRAPAROUND MODE - Buffer cycling, oldest data overwritten (infinite runtime) ***\r\n");
            }
        } else {
            g_real_stats.inference_errors++;
        }
        
        // Move to next window (REAL_STRIDE = 13)
        next_window_start += REAL_STRIDE;
        window_id++;
        
        // Small yield
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    
    // Final stats
    if (usb) {
        char msg[192];
        snprintf(msg, sizeof(msg),
            "[REAL] Stopped. Frames:%lu Wins:%lu Skip:%lu\r\n",
            (unsigned long)g_real_stats.total_frames_collected,
            (unsigned long)g_real_stats.windows_processed,
            (unsigned long)g_real_stats.windows_skipped);
        usb->sendMessage(msg);
    }
    
    vTaskDelete(NULL);
}

// ============================================================================
// STATE HANDLERS
// ============================================================================

void RealtimeInference_ISR_Real::OnEnter() {
    UsbCommunication* usb = UsbCommunication::getInstance();
    
    // Stop conflicting tasks
    extern TaskHandle_t adcSampleTaskHandle;
    extern TaskHandle_t streamingTaskHandle;
    extern TaskHandle_t g_isr_processing_task_handle;
    
    if (adcSampleTaskHandle) { vTaskDelete(adcSampleTaskHandle); adcSampleTaskHandle = NULL; }
    if (streamingTaskHandle) { vTaskDelete(streamingTaskHandle); streamingTaskHandle = NULL; }
    if (g_isr_processing_task_handle) { vTaskDelete(g_isr_processing_task_handle); g_isr_processing_task_handle = NULL; }
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Initialize pipeline
    ISR_Real_Pipeline_Init();
    
    // Configure ADC
    HAL_ADC_Stop(&hadc3);
    uint32_t timeout = 10000;
    while ((hadc3.Instance->CR & ADC_CR_ADSTART) && timeout--);
    
    if (HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET, ADC_DIFFERENTIAL_ENDED) != HAL_OK) {
        if (usb) usb->sendMessage("[REAL] Warning: ADC calibration failed\r\n");
    }
    
    hadc3.Instance->ISR = ADC_ISR_ADRDY;
    hadc3.Instance->CR |= ADC_CR_ADEN;
    timeout = 10000;
    while (!(hadc3.Instance->ISR & ADC_ISR_ADRDY) && timeout--);
    
    // Initialize radar flags
    g_radarFlags.ssFlag = (HAL_GPIO_ReadPin(RADAR_SS_GPIO_Port, RADAR_SS_Pin) == GPIO_PIN_SET);
    g_radarFlags.intgClkFlag = 0;
    
    // Set running flags
    g_isr_real_running = true;
    g_isr_real_mode = true;
    
    // Create processing task
    s_real_processing_task = xTaskCreateStatic(
        ISR_Real_ProcessingTask,
        "RealProc",
        4096,
        NULL,
        osPriorityHigh,
        s_real_task_stack,
        &s_real_task_tcb
    );
    
    if (usb) {
        usb->sendMessage("[REAL] Mode activated (NO QUEUE - CATCH-UP) - waiting for radar sync...\r\n");
    }
}

void RealtimeInference_ISR_Real::OnExit() {
    ISR_Real_Pipeline_Cleanup();
    
    UsbCommunication* usb = UsbCommunication::getInstance();
    if (usb) {
        usb->sendMessage("[REAL] Mode deactivated\r\n");
    }
}

void RealtimeInference_ISR_Real::HandleEvent(Event* event, void* args) {
    (void)event;
    (void)args;
}
