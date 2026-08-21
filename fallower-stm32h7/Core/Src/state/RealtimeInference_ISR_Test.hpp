/**
 * @file RealtimeInference_DMA_Test.hpp
 * @brief Test Mode for DMA-Based Radar AI Inference
 * 
 * Logic:
 * 1. Collect 30 batches (900 frames) to SDRAM (g_dma_test_storage)
 * 2. Stop ADC/ISR
 * 3. Run DMA_TestProcessingTask to process stored batches
 * 
 * @author Ethem
 * @date 2025
 */

#ifndef REALTIME_INFERENCE_ISR_TEST_HPP
#define REALTIME_INFERENCE_ISR_TEST_HPP

#include "State.hpp"
#include "RealtimeInference_ISR.hpp"  // For data structures
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// ============================================================================
// CONFIGURATION
// ============================================================================
#define DMA_TEST_BATCH_COUNT    30   // 30 batches of 30 frames = 900 frames

// ============================================================================
// GLOBAL VARIABLES (Shared with ISR)
// ============================================================================
extern volatile bool g_isr_test_mode;
extern volatile uint8_t g_isr_test_batch_idx;
extern DMABatchBuffer_t* g_isr_test_storage; // Pointer to SDRAM array

// ============================================================================
// STATE CLASS
// ============================================================================
class RealtimeInference_ISR_Test : public State {
public:
    void OnEnter() override;
    void OnExit() override;
    void HandleEvent(Event* event, void* args) override;
    const char* getStateName() const override {
        return "REALTIME_INFERENCE_ISR_TEST";
    }
};

// ============================================================================
// TASK FUNCTIONS
// ============================================================================
/**
 * @brief Validation Task
 * Iterates through g_dma_test_storage and runs inference logic
 */
void ISR_TestProcessingTask(void* params);

// Helper Functions (Internal)
void SendRawDataToPC(void);
void RunInferenceLoop(uint8_t stride, const char* label);
uint16_t ISR_Test_CalculateCRC16(uint8_t* data, uint16_t length);

#endif // REALTIME_INFERENCE_DMA_TEST_HPP
