#ifndef REALTIME_INFERENCE_HPP
#define REALTIME_INFERENCE_HPP

#include "State.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <cstdint>

// Configuration constants
#define FRAME_SAMPLES      688
#define FRAME_POOL_SIZE    256  // CRITICAL FIX: Increased from 32 to 256 to handle slow AI (4.4s inference @ 50Hz = 220 frames)
#define AI_WINDOW_FRAMES   30
#define NORM_MIN           -1000.0f
#define NORM_MAX           1000.0f
#define INFERENCE_STRIDE_V1   14  // Matches Python Configuration (53% overlap)

// MTI configuration
#define MTI_REF_AVG_NUM    4
#define MTI_TAG_AVG_NUM    2

// Frame structure for DTCMRAM (zero-wait access)
typedef struct {
    int16_t samples[FRAME_SAMPLES];
    uint32_t timestamp;
    uint32_t frame_number;
    uint8_t _padding[12];  // 16-byte alignment
} __attribute__((packed)) __attribute__((aligned(16))) RadarFrame_t;

// Circular window structure (SDRAM)
typedef struct {
    int16_t frames[AI_WINDOW_FRAMES][FRAME_SAMPLES];
    uint8_t write_idx;
    uint8_t count;
    uint8_t _padding[30];  // 32-byte cache line alignment
} __attribute__((aligned(32))) CircularWindow_t;

// Statistics structure
typedef struct {
    uint32_t frames_produced;
    uint32_t frames_consumed;
    uint32_t inferences_done;
    uint32_t inference_errors;
    uint32_t avg_inference_time_ms;
    uint32_t max_inference_time_ms;
    uint32_t frames_dropped;
    uint32_t producer_timeout_count;   // GPIO interrupt timeout count
    UBaseType_t producer_stack_hwm;     // Stack high water mark (words remaining)
    UBaseType_t consumer_stack_hwm;     // Stack high water mark (words remaining)
} PipelineStats_t;

extern PipelineStats_t g_pipeline_stats;

// Global control flags
extern volatile bool is_inference_running;
extern volatile bool producer_paused;  // Pause producer during AI inference

// Frame queue (for producer-consumer communication)
extern QueueHandle_t frameQueue;

// Task handles (for GPIO interrupt notification)
extern TaskHandle_t producerTaskHandle;
extern TaskHandle_t consumerTaskHandle;

// State class for real-time inference
class RealtimeInference : public State {
public:
    void OnEnter() override;
    void OnExit() override;
    void HandleEvent(Event* event, void* args) override { /* No-op */ }
    const char* getStateName() const override {
        return "REALTIME_INFERENCE";
    }
};

// Real-time task functions
void RadarProducerTask(void* params);
void InferenceConsumerTask(void* params);

// Helper functions
void PerFrameNormalize(const float* samples, float* output, uint16_t count);
void ApplyMTIFilter(const int16_t input_window[AI_WINDOW_FRAMES][FRAME_SAMPLES], float output_window[AI_WINDOW_FRAMES][FRAME_SAMPLES]);
void LinearizeCircularWindow(int16_t output[AI_WINDOW_FRAMES][FRAME_SAMPLES]);
void ProcessAIResult(const float* output);

// Test inference function (for validation)
void RunTestInference(const float* test_data);

#endif // REALTIME_INFERENCE_HPP
