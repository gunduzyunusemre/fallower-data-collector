#ifndef STATE_MACHINE_HPP_
#define STATE_MACHINE_HPP_

#include "FreeRTOS.h"
#include "semphr.h"
#include <cstdint>
#include <vector>
#include "protocol_spec.h"


// Define states
typedef enum {
    STATE_IDLE,
    STATE_INIT,
    STATE_READY,
    STATE_CALIBRATE,
    STATE_STREAM,
    STATE_AWAITING_METADATA,
    STATE_LOADING_DATA,
    STATE_VERIFYING_DATA,
    STATE_MODEL_READY,
    STATE_REALTIME_INFERENCE,         // AŞAMA 1: 2-Task (Priority Fixed)
    STATE_REALTIME_INFERENCE_3TASK,   // AŞAMA 2: 3-Task Architecture
    STATE_REALTIME_INFERENCE_ISR,     // AŞAMA 3: ISR Mode (Zero Context Switch)
    STATE_REALTIME_INFERENCE_ISR_TEST, // AŞAMA 4: ISR Test Mode (Collet -> Stop -> Process)
    STATE_REALTIME_INFERENCE_ISR_REAL, // AŞAMA 5: Real-time Stride-5 (20MB Circular Buffer)
    STATE_ERROR
} SystemState_t;

// Define command types (from oneri.md)
typedef enum {
    CMD_INIT = 0,
    CMD_READY = 1,
    CMD_CALIBRATE = 2,
    CMD_UPDATE_RADAR = 3,
    CMD_START_STREAM = 4,
    CMD_STOP_STREAM = 5,
    CMD_GET_STATUS = 6
} CommandType_t;


class StateMachine {
private:
    static StateMachine* instance;
    static SemaphoreHandle_t stateMutex;
    SystemState_t currentState;

    StateMachine();

public:
    static StateMachine* getInstance();
    SystemState_t getCurrentState();
    bool changeState(SystemState_t newState);
    bool processPacket(PacketType_t type, const uint8_t* payload, uint16_t len, const Metadata_t* metadata = nullptr);
    bool processCommand(CommandType_t command, const std::vector<uint8_t>& regValues); // From oneri.md
    const char* getStateName() const;

    // State transition helper
    void transitionTo(SystemState_t newState);
};

// PROMPT 4: SDRAM and Model Management Functions
bool SDRAM_WriteChunk(const uint8_t* data, uint16_t len);
void SDRAM_ResetWriteState();
void StoreMetadata(const Metadata_t* metadata);
const Metadata_t* GetMetadata(void);
void ModelInstallerTask(void* parameters);

// SDRAM Status Monitoring
uint32_t GetTotalBytesWritten(void);

extern volatile bool g_fmc_high_priority_mode;

#endif /* STATE_MACHINE_HPP_ */
