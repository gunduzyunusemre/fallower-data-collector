// Stream.hpp
#ifndef SRC_STATE_STREAM_HPP_
#define SRC_STATE_STREAM_HPP_

#include "State.hpp"
#include "../radar_flags.h"

// Burada çift tampon mekanizması için sabitleri tanımlayın
#define MAX_FRAME_SAMPLES 2048
#define BUFFER_COUNT 2

// Frame buffer yapısını tanımlayın
typedef struct {
    int16_t samples[MAX_FRAME_SAMPLES];
    uint16_t count;
    uint32_t captureTime;
    uint32_t endTime;
    uint32_t frameNumber;
} FrameBuffer;

// Stream durumu sınıfı
class Stream : public State {
public:
    void HandleEvent(Event* event, void* args) override;
    const char* getStateName() const override { return "Stream"; }
    void OnEnter() override;
    void OnExit() override;
};

// Global değişkenler
extern TaskHandle_t adcSampleTaskHandle;
extern TaskHandle_t streamingTaskHandle;
extern volatile bool stopStreamingFlag;
extern FrameBuffer frameBuffers[BUFFER_COUNT];
extern volatile uint8_t activeBufferIndex;
extern volatile uint8_t processingBufferIndex;

// Fonksiyon prototipleri
void ADC_SamplingTask(void* pvParameters);
void frame_buffer_init(FrameBuffer* buffer);
void send_frame_direct(UsbCommunication* usb, FrameBuffer* buffer);
void sendStreamMessage(const char* message);
uint16_t calculateCRC16(uint8_t* data, uint16_t length);

#endif /* SRC_STATE_STREAM_HPP_ */
