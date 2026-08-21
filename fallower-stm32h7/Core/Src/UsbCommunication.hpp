#ifndef USB_COMMUNICATION_HPP_
#define USB_COMMUNICATION_HPP_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <cstdint>
#include <vector>
#include "circular_buffer.h"
#include "protocol_spec.h"
#include "state/StateMachine.hpp"

// X-CUBE-AI includes for ai_i32 type definition
extern "C" {
#include "ai_platform.h"
#include "fallower1.h"
}

#define MAX_USB_BUFFER_SIZE 512

class UsbCommunication {
private:
    static UsbCommunication* instance;
    static SemaphoreHandle_t usbMutex;
    static SemaphoreHandle_t aiMutex; // Mutex for protecting AI inference
    QueueHandle_t txQueue;
    QueueHandle_t rxQueue;
    TaskHandle_t txTaskHandle;
    TaskHandle_t rxTaskHandle;
    bool initialized;

    // AI Worker Task - Dedicated for heavy AI operations
    TaskHandle_t aiWorkerTaskHandle;
    QueueHandle_t aiCommandQueue;  // Commands from rxTask to AI worker

    // AI Command Types
    enum class AICommand {
        LOAD_MODEL,
        INIT_MODEL,
        VERIFY_MODEL, // Command to trigger post-transfer verification
        RUN_INFERENCE
    };

    typedef struct {
        AICommand cmd;
        const float* test_data;  // For RUN_INFERENCE
        uint32_t data_size;
    } AICommandMsg;
    
    // Circular Buffer for ISR-safe data reception
    CircularBuffer_t circular_buffer;
    volatile bool is_usb_rx_paused;
    
    // Circular Buffer Storage
    uint8_t usb_buffer[4096];   // Internal buffer for circular buffer
    
    // Register Command Accumulation (from oneri.md)
    char gAccumulatedBuffer[2048];  // Buffer for accumulating multi-part STR commands
    size_t gAccumulatedLength;      // Current length in accumulation buffer
    bool gCommandInProgress;        // Flag indicating STR command is being accumulated

    // Store original task priorities
    UBaseType_t originalRxPriority;
    UBaseType_t originalTxPriority;
    UBaseType_t originalAiWorkerPriority;

    typedef struct {
        char* message;
        uint8_t priority;
    } UsbMessage;

    UsbCommunication();
    ~UsbCommunication();

    static void txTask(void* parameters);
    static void rxTask(void* parameters);
    static void aiWorkerTask(void* parameters);  // NEW: Dedicated AI task
    static void hardFaultReporterTask(void* parameters);
    bool queueTxMessage(const char* message, uint8_t priority);
    void processBasicCommand(const char* buffer);
    void processSTRCommand(const char* buffer);
    
public:
    static UsbCommunication* getInstance();
    bool initialize();
    
    // Fire hose mode kontrolü
    void enableFireHoseMode();
    void disableFireHoseMode(); 
    uint32_t getFireHoseBytesReceived();
    
    // Radar AI Pipeline - Queue-based producer-consumer
    typedef struct {
        int16_t samples[688];  // 671 actual + 17 padding = 688
        uint16_t count;        // Actual sample count (671)
        uint32_t timestamp;    // Frame capture timestamp
        uint32_t frame_number; // Frame sequence number
    } RadarFrame_t;
    
    void startRadarAIPipeline();
    void stopRadarAIPipeline();
    static void radarProducerTask(void* parameters);
    static void radarConsumerTask(void* parameters);
    
    bool sendMessage(const char* message);
    bool sendStatusMessage(const char* stateName, const char* message);
    void processReceivedData(const uint8_t* data, uint32_t length);
    void addToRxQueue(uint8_t* data, uint32_t length);
    bool isReady() const { return initialized; }
    void clearQueue();
    bool sendData(uint8_t* data, uint32_t length);
    
    // Circular Buffer Methods
    void writeToCircularBuffer(uint8_t* data, uint32_t length);
    void clearCircularBuffer();
    bool isRxPaused() const { return is_usb_rx_paused; }
    void resumeRx();  // Resume USB reception
};

// AI and Memory Test Functions
bool test_sdram_weights_safety();
// STM32 Standard AI Inference - returns X-CUBE-AI buffer pointer
extern float* ai_fallower_run_inference_direct(const float* input_data);
extern void ai_fallower_init_from_sdram();
extern void critical_weights_verification(void);

// C-linkage functions for USB callbacks
extern "C" {
    void UsbDataReceivedCallback(uint8_t* data, uint32_t length);
    void UsbRxQueueAddData(uint8_t* data, uint32_t length);
}

#endif /* USB_COMMUNICATION_HPP_ */
