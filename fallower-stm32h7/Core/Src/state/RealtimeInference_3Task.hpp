#ifndef REALTIME_INFERENCE_3TASK_HPP
#define REALTIME_INFERENCE_3TASK_HPP

#include "State.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <cstdint>

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

#define FRAME_SAMPLES      688  // Model input size (radar sends 688 samples directly)
#define AI_WINDOW_FRAMES   30
#define NORM_MIN           -1000.0f
#define NORM_MAX           1000.0f

// MTI configuration
#define MTI_REF_AVG_NUM    4
#define MTI_TAG_AVG_NUM    2

// Buffer counts for pipeline stages
#define BATCH_BUFFER_COUNT    3   // Triple buffer for raw data collection

// Queue depths
#define BATCH_QUEUE_DEPTH     3   // Max pending batches for Processing task

// ============================================================================
// EVENT DEFINITIONS
// ============================================================================
// Macros now defined in radar_flags.h - DO NOT REDEFINE HERE
// #define SS_HIGH_EVENT   (1 << 0)
// #define SS_LOW_EVENT    (1 << 1)
// #define INTG_CLK_EVENT  (1 << 2)

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * @brief Batch Buffer - Holds raw radar frames
 * Located in RAM_D1 for fast CPU access
 */
typedef struct {
    int16_t frames[AI_WINDOW_FRAMES][FRAME_SAMPLES];  // [30][688] raw ADC data
    uint16_t sample_counts[AI_WINDOW_FRAMES];         // Actual samples per frame
    uint32_t timestamps[AI_WINDOW_FRAMES];            // Timestamp per frame
    uint8_t frame_count;                              // Number of frames filled (0-30)
    uint32_t batch_id;                                // Unique batch identifier
    uint8_t buffer_index;                             // Which buffer this is (0 or 1)
} __attribute__((aligned(32))) BatchBuffer_t;

/**
 * @brief AI Input Buffer - Holds preprocessed normalized data
 * Located in SDRAM for large storage
 * NOTE: Single buffer (no double buffering) since preprocessing and inference
 *       are now sequential in the same task
 */
typedef struct {
    float data[AI_WINDOW_FRAMES][FRAME_SAMPLES];      // [30][688] normalized floats
    uint32_t batch_id;                                // Source batch identifier
} __attribute__((aligned(32))) AIInputData_t;

/**
 * @brief Pipeline Statistics
 */
typedef struct {
    // Counters
    uint32_t total_frames_collected;
    uint32_t batches_completed;
    uint32_t batches_processed;
    uint32_t inferences_done;

    // Error counters
    uint32_t batch_queue_full_drops;
    uint32_t ai_queue_full_drops;
    uint32_t inference_errors;

    // Timing
    uint32_t last_inference_time_ms;
    uint32_t min_inference_time_ms;
    uint32_t max_inference_time_ms;

    // Current state
    uint8_t current_batch_buffer;
    uint8_t current_ai_buffer;
} PipelineStats_3Task_t;

// ============================================================================
// AI CLASS LABELS
// ============================================================================
#define AI_CLASS_COUNT 7

static const char* const AI_CLASS_NAMES[AI_CLASS_COUNT] = {
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
extern PipelineStats_3Task_t g_pipeline_stats_3task;

// Control flag
extern volatile bool g_inference_running;

// Queue for inter-task communication
extern QueueHandle_t g_batch_queue;      // Producer -> ProcessingAndInference

// Task handles
extern TaskHandle_t g_producer_task_handle;
extern TaskHandle_t g_processing_task_handle;

// Batch Buffers (RAM_D1) - Double buffered
extern BatchBuffer_t g_batch_buffers[BATCH_BUFFER_COUNT];

// AI Input Buffer (SDRAM) - Single buffer
extern AIInputData_t g_ai_input_buffer;

// ============================================================================
// STATE CLASS
// ============================================================================

class RealtimeInference_3Task : public State {
public:
    void OnEnter() override;
    void OnExit() override;
    void HandleEvent(Event* event, void* args) override;
    const char* getStateName() const override {
        return "REALTIME_INFERENCE_3TASK";
    }
};

// ============================================================================
// TASK FUNCTIONS
// ============================================================================

/**
 * @brief RadarProducerTask - REALTIME PRIORITY
 *
 * Collects ADC samples from radar via task notifications.
 * Fills batch buffers with 30 frames of 688 samples each.
 * Uses double buffering to prevent data loss during batch handoff.
 *
 * Flow: ISR notifications -> ADC read -> Batch buffer -> Queue to WindowManager
 */
void RadarProducerTask_3Task(void* params);

/**
 * @brief ProcessingAndInferenceTask - HIGH PRIORITY
 *
 * Merged task that combines WindowManager and AIInference functionality.
 * Receives completed batches from Producer via queue.
 * Performs preprocessing (padding, normalization to -1000..+1000).
 * Copies normalized data to single ai_input_buffer.
 * Runs MobileNetV3-Small inference.
 * Outputs result via USB.
 *
 * Flow: Batch queue -> Preprocess -> ai_input_buffer -> Inference -> USB output
 *
 * Benefits:
 * - Eliminates race condition between preprocessing and inference
 * - Saves memory (~164KB SDRAM - one less buffer)
 * - Simpler architecture (no inter-stage queue)
 *
 * Trade-off:
 * - No parallel preprocessing (acceptable: 1s inference < 2s batch interval)
 */
void ProcessingAndInferenceTask(void* params);

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Initialize all pipeline buffers and queues
 */
void Pipeline_Init(void);

/**
 * @brief Cleanup pipeline resources
 */
void Pipeline_Cleanup(void);

/**
 * @brief Get prediction class from AI output
 * @param output AI output array (7 elements)
 * @param confidence Output: confidence value of predicted class
 * @return Index of predicted class (0-6)
 */
uint8_t GetPredictedClass(const float* output, float* confidence);

#endif // REALTIME_INFERENCE_3TASK_HPP
