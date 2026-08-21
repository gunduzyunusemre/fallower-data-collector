/**
 * @file RealtimeInference_DMA.hpp
 * @brief DMA-Based Real-time Radar AI Inference Pipeline
 *
 * Architecture:
 * =============
 *
 *   SS_LOW ISR ──> Start ADC+DMA
 *                      │
 *   INTG_CLK ──> ADC3 Software Trigger (in ISR, ~20ns)
 *                      │
 *                  DMA Auto Transfer ──> dma_frame_buffer (RAM_D1)
 *                      │
 *   SS_HIGH ISR ──> Stop DMA + Copy to Batch Buffer + frame_idx++
 *                      │
 *                  30 frames ──> xTaskNotify ──> ProcessingTask
 *                                                    │
 *                                              Preprocessing + AI Inference
 *
 * Key Benefits:
 * - INTG_CLK ISR: Direct ADC Read (~1-2µs vs ~15µs polling in task)
 * - No context switch per sample (6720 → 0 per batch!)
 * - Zero wait-state access when using direct register
 * - CPU minimal overhead compared to task-based polling
 *
 * @author Ethem
 * @date 2025
 */

#ifndef REALTIME_INFERENCE_ISR_HPP
#define REALTIME_INFERENCE_ISR_HPP

#include "State.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <cstdint>

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

#define DMA_FRAME_SAMPLES      688   // Max samples per frame (matches model input)
#define DMA_WINDOW_FRAMES      30    // Frames per inference batch
#define DMA_NORM_MIN           -1000.0f
#define DMA_NORM_MAX           1000.0f

// MTI configuration
#define MTI_REF_AVG_NUM    4
#define MTI_TAG_AVG_NUM    2

// Buffer counts
#define DMA_BATCH_BUFFER_COUNT    3   // Triple buffer: 1 for ISR write, 1 in queue, 1 for Task read

// ============================================================================
// DMA SPECIFIC CONSTANTS
// ============================================================================

#define DMA_BUFFER_SIZE        DMA_FRAME_SAMPLES  // 688 half-words per frame

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * @brief DMA Frame Buffer - Single frame storage for DMA transfer
 * Located in RAM_D1 for fast DMA access
 * 32-byte aligned for cache coherency
 */
typedef struct {
    int16_t samples[DMA_BUFFER_SIZE];  // Raw ADC samples (16-bit)
} __attribute__((aligned(32))) DMAFrameBuffer_t;

/**
 * @brief DMA Batch Buffer - Holds 30 frames of raw data
 * Located in RAM_D1 for fast CPU access
 */
typedef struct {
    int16_t frames[DMA_WINDOW_FRAMES][DMA_FRAME_SAMPLES];  // [30][688] raw ADC
    uint16_t sample_counts[DMA_WINDOW_FRAMES];              // Actual samples per frame
    uint32_t timestamps[DMA_WINDOW_FRAMES];                 // Timestamp per frame
    uint8_t frame_count;                                    // Frames filled (0-30)
    uint32_t batch_id;                                      // Unique identifier
    uint8_t buffer_index;                                   // Which buffer (0 or 1)
    uint8_t padding[6];                                     // Explicit padding to 41472 bytes (Multiple of 32)
} __attribute__((aligned(32))) DMABatchBuffer_t;

/**
 * @brief AI Input Buffer - Preprocessed normalized data
 * Located in SDRAM (same as 3Task version for compatibility)
 */
typedef struct {
    float data[DMA_WINDOW_FRAMES][DMA_FRAME_SAMPLES];  // [30][688] normalized
    uint32_t batch_id;
} __attribute__((aligned(32))) DMAInputData_t;

/**
 * @brief DMA Pipeline Statistics
 */
typedef struct {
    // Counters
    uint32_t total_frames_collected;
    uint32_t batches_completed;
    uint32_t batches_processed;
    uint32_t inferences_done;

    // ISR specific
    uint32_t isr_transfers_started;
    uint32_t isr_transfers_completed;
    uint32_t isr_overruns;           // Polling couldn't keep up
    uint32_t intg_clk_triggers;      // Total INTG_CLK events

    // Error counters
    uint32_t batch_queue_full_drops;
    uint32_t inference_errors;

    // Timing
    uint32_t last_inference_time_ms;
    uint32_t min_inference_time_ms;
    uint32_t max_inference_time_ms;
    uint32_t last_frame_samples;     // Debug: samples in last frame
} ISR_PipelineStats_t;

// ============================================================================
// AI CLASS LABELS (same as 3Task for consistency)
// ============================================================================
#define ISR_AI_CLASS_COUNT 7

static const char* const ISR_AI_CLASS_NAMES[ISR_AI_CLASS_COUNT] = {
    "FALL",
    "GETUP",
    "GETUPGR",
    "SIT",
    "STILLMOVE",
    "STILLNOACT",
    "WALK"
};

// ============================================================================
// GLOBAL VARIABLES (extern declarations)
// ============================================================================

// Statistics
extern ISR_PipelineStats_t g_isr_pipeline_stats;

// Control flag
extern volatile bool g_isr_inference_running;

// Task handles
extern TaskHandle_t g_isr_producer_task_handle;
extern TaskHandle_t g_isr_processing_task_handle;

// Queue for batch transfer
extern QueueHandle_t g_isr_batch_queue;

// DMA Frame buffer (single, for active transfer)
extern DMAFrameBuffer_t g_dma_frame_buffer;

// Batch Buffers (RAM_D1) - Double buffered
extern DMABatchBuffer_t g_isr_batch_buffers[DMA_BATCH_BUFFER_COUNT];

// AI Input Buffer (SDRAM)
extern DMAInputData_t g_isr_ai_input_buffer;

// ============================================================================
// DMA STATE FLAGS (for ISR communication)
// ============================================================================

// Volatile flags for ISR-Task communication
extern volatile uint8_t g_isr_active_batch_idx;      // Which batch buffer is active
extern volatile uint8_t g_isr_current_frame_idx;     // Current frame index (0-29)
extern volatile bool g_isr_frame_in_progress;        // True during SS_LOW to SS_HIGH
extern volatile bool g_isr_batch_ready;              // Set when 30 frames complete

// ============================================================================
// STATE CLASS
// ============================================================================

class RealtimeInference_ISR : public State {
public:
    void OnEnter() override;
    void OnExit() override;
    void HandleEvent(Event* event, void* args) override;
    const char* getStateName() const override {
        return "REALTIME_INFERENCE_ISR";
    }
};

// ============================================================================
// TASK FUNCTIONS
// ============================================================================

/**
 * @brief DMA Processing Task - Handles preprocessing and inference
 *
 * Waits for batch notification from ISR.
 * Performs preprocessing (padding, normalization).
 * Runs AI inference.
 * Outputs result via USB.
 */
void ISR_ProcessingTask(void* params);

// ============================================================================
// ISR HANDLERS (called from radar_adc_test.cpp)
// ============================================================================

/**
 * @brief Handle SS_LOW event (frame start) in ISR mode
 */
void ISR_HandleSSLow(void);

/**
 * @brief Handle SS_HIGH event (frame end) in ISR mode
 */
void ISR_HandleSSHigh(void);

/**
 * @brief Handle INTG_CLK event in ISR mode
 * Triggers single ADC conversion via software
 * ~2ms execution time (direct register access)
 */
void ISR_HandleIntgClk(void);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Initialize DMA pipeline
 */
void ISR_Pipeline_Init(void);

/**
 * @brief Cleanup DMA pipeline resources
 */
void DMA_Pipeline_Cleanup(void);

/**
 * @brief Configure ADC3 for DMA mode
 * Called during OnEnter
 */
bool ISR_ConfigureADC(void);

/**
 * @brief Restore ADC3 to polling mode
 * Called during OnExit
 */
void ISR_RestoreADC(void);

/**
 * @brief Get predicted class from AI output (same as 3Task)
 */
uint8_t ISR_GetPredictedClass(const float* output, float* confidence);

#endif // REALTIME_INFERENCE_DMA_HPP
