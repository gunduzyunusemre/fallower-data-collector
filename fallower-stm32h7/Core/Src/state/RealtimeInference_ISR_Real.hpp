/**
 * @file RealtimeInference_DMA_Real.hpp
 * @brief Real-time Stride-14 AI Inference with 20MB Circular Buffer
 * 
 * Architecture:
 *   ISR → Circular Buffer (20MB) → Queue → Processing Task → Inference → USB
 * 
 * @author Ethem
 * @date 2026
 */

#ifndef REALTIME_INFERENCE_ISR_REAL_HPP
#define REALTIME_INFERENCE_ISR_REAL_HPP

#include "State.hpp"
#include "RealtimeInference_ISR.hpp"  // Reuse DMA structures
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// ============================================================================
// CONFIGURATION
// ============================================================================

// Circular buffer: ~20MB = ~14,500 frames (~16 minutes capacity)
#define CIRCULAR_BUFFER_FRAMES      14500
#define CIRCULAR_FRAME_SAMPLES      688

// Sliding window parameters
#define REAL_WINDOW_FRAMES          30      // Frames per inference window
#define REAL_STRIDE                 13      // Stride between windows (Matches Python Phase 3)

// MTI configuration (Matches Python differential_moving_average)
#define MTI_REF_AVG_NUM             4
#define MTI_TAG_AVG_NUM             2

// Queue depth for pending windows
#define WINDOW_QUEUE_DEPTH          200     // Max pending windows before drop

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * @brief Single frame in circular buffer
 * Compact structure to maximize buffer capacity
 */
typedef struct {
    int16_t samples[CIRCULAR_FRAME_SAMPLES];  // Raw ADC data
    uint16_t sample_count;                     // Actual samples collected
    uint32_t timestamp;                        // Capture time (HAL_GetTick)
    uint8_t padding[26];                       // Align to 1408 bytes (32-byte multiple)
} __attribute__((aligned(32))) CircularFrame_t;

// CRITICAL: Verify alignment and sizing for Cache Line safety (32-byte)
#ifdef __cplusplus
static_assert(sizeof(CircularFrame_t) % 32 == 0, "CircularFrame_t must be 32-byte multiple for Cache safety");
static_assert(((sizeof(CircularFrame_t)) & 31) == 0, "CircularFrame_t size must be power-of-2 or 32-byte multiple");
#endif

/**
 * @brief Window queue item - lightweight pointer to circular buffer
 */
typedef struct {
    uint32_t start_frame_idx;   // Global frame index (not modulo)
    uint32_t window_id;         // Sequential window ID
} WindowQueueItem_t;

/**
 * @brief Real-time pipeline statistics
 */
typedef struct {
    // Frame collection
    uint32_t total_frames_collected;
    uint32_t frames_overwritten;
    
    // Window processing  
    uint32_t windows_queued;
    uint32_t windows_processed;
    uint32_t windows_dropped;       // Queue full (legacy)
    uint32_t windows_invalid;       // Data overwritten before read
    uint32_t windows_skipped;       // Catch-up skip count
    
    // Status
    bool buffer_full;               // Buffer reached capacity
    
    // Timing
    uint32_t last_inference_ms;
    uint32_t max_queue_depth;
    
    // Errors
    uint32_t inference_errors;
} RealPipelineStats_t;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

extern volatile bool g_isr_real_mode;
extern volatile bool g_isr_real_running;
extern volatile uint32_t g_real_write_idx;      // ISR write position
extern volatile uint32_t g_real_total_frames;   // Total frames collected

extern CircularFrame_t* g_circular_buffer;      // SDRAM pointer
extern QueueHandle_t g_window_queue;
extern RealPipelineStats_t g_real_stats;

// ============================================================================
// STATE CLASS
// ============================================================================

class RealtimeInference_ISR_Real : public State {
public:
    void OnEnter() override;
    void OnExit() override;
    void HandleEvent(Event* event, void* args) override;
    const char* getStateName() const override {
        return "REALTIME_INFERENCE_ISR_REAL";
    }
};

// ============================================================================
// FUNCTIONS
// ============================================================================

// ISR Handlers (called from radar_adc_test.cpp)
void ISR_Real_HandleSSLow(void);
void ISR_Real_HandleSSHigh(void);
void ISR_Real_HandleIntgClk(void);

// Task
void ISR_Real_ProcessingTask(void* params);

// Helpers
void ISR_Real_Pipeline_Init(void);
void ISR_Real_Pipeline_Cleanup(void);

#endif // REALTIME_INFERENCE_ISR_REAL_HPP
