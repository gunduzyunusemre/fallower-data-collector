#include "Stream.hpp"
#include "Event.hpp"
#include "StateMachine.hpp"
#include "../UsbCommunication.hpp"
#include "../radar_adc_test.hpp"
#include <stdio.h>
#include <cstring>
#include "cmsis_os.h"
#include <climits>
#include "main.h"
#include "../radar_flags.h"

// Global değişkenler
TaskHandle_t adcSampleTaskHandle = NULL;
TaskHandle_t streamingTaskHandle = NULL;
volatile bool stopStreamingFlag = false;
// CRITICAL FIX: Moved back to RAM_D1 (default) for MPU coverage
// RAM_D2 had undefined memory access behavior without MPU configuration
// RAM_D1 has proper MPU Region configuration (strongly-ordered access)
// NOTE: Cache alignment removed as D-Cache is DISABLED (matches reference project)
FrameBuffer frameBuffers[BUFFER_COUNT];  // Default location: RAM_D1 (0x24000000)
volatile uint8_t activeBufferIndex = 0;
volatile uint8_t processingBufferIndex = 1;
volatile uint32_t frameCounter = 0;

extern SPI_HandleTypeDef hspi4;
extern ADC_HandleTypeDef hadc3;

#define STREAM_EVT_STOP (1 << 0)

// Yardımcı fonksiyonlar
void sendStreamMessage(const char* message) {
    UsbCommunication* usb = UsbCommunication::getInstance();
    if (usb) {
        usb->sendStatusMessage("STREAM", message);
    }
}

void frame_buffer_init(FrameBuffer* buffer) {
    if (buffer) {
        buffer->count = 0;
        buffer->captureTime = 0;
        buffer->endTime = 0;
        buffer->frameNumber = 0;
    }
}

uint16_t calculateCRC16(uint8_t* data, uint16_t length) {
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i];

        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

void send_frame_direct(UsbCommunication* usb, FrameBuffer* buffer) {
    if (!usb || !buffer || buffer->count == 0) return;

    // Tüm verinin toplam boyutunu hesapla
    uint32_t totalSize = 20 + (buffer->count * sizeof(int16_t)) + 5; // header + data + end marker

    // Tek bir büyük buffer oluştur
    uint8_t* completeBuffer = (uint8_t*)malloc(totalSize);
    if (!completeBuffer) return; // Bellek kontrolü

    // Header ve meta verileri kopyala
    memcpy(completeBuffer, "<DTA>", 4);

    // Meta verileri ekle
    uint32_t* pMeta = (uint32_t*)(completeBuffer + 4);
    pMeta[0] = buffer->frameNumber;
    pMeta[1] = buffer->captureTime;
    pMeta[2] = buffer->endTime - buffer->captureTime;

    // Örnek sayısını ekle
    uint16_t* pCount = (uint16_t*)(completeBuffer + 16);
    *pCount = buffer->count;

    // CRC hesapla
    uint16_t crc = calculateCRC16(completeBuffer + 4, 14);
    *(uint16_t*)(completeBuffer + 18) = crc;

    // ADC verilerini kopyala
    memcpy(completeBuffer + 20, buffer->samples, buffer->count * sizeof(int16_t));

    // Bitiş işaretleyicisini kopyala
    memcpy(completeBuffer + 20 + (buffer->count * sizeof(int16_t)), "<END>", 5);

    // Tüm veriyi tek seferde gönder
    usb->sendData(completeBuffer, totalSize);

    // Belleği temizle
    free(completeBuffer);
}
// Stream sınıf metodları
void Stream::HandleEvent(Event* event, void* args) {
    if (!event) return;

    switch (event->GetType()) {
        case EventType::StopStreaming:
            {
                sendStateMessage("Stopping streaming...");
                stopStreamingFlag = true;

                UsbCommunication* usb = UsbCommunication::getInstance();
                if (usb) {
                    const char* stop_marker = "<STP>";
                    usb->sendData((uint8_t*)stop_marker, strlen(stop_marker));
                }

                osDelay(100);

                if (streamingTaskHandle != NULL) {
                    vTaskDelete(streamingTaskHandle);
                    streamingTaskHandle = NULL;
                }

                if (adcSampleTaskHandle != NULL) {
                    vTaskDelete(adcSampleTaskHandle);
                    adcSampleTaskHandle = NULL;
                }

                StateMachine::getInstance()->changeState(STATE_IDLE);
            }
            break;

        // Handle missing enumeration values to suppress warnings
        case EventType::Initialize:
        case EventType::Ready:
        case EventType::Calibrate:
        case EventType::StartStreaming:
            // These events are not handled in Stream state
            sendStateMessage("Event not handled in Stream state");
            break;

        default:
            sendStateMessage("Unknown event in Stream state");
            break;
    }
}

void Stream::OnEnter() {
    sendStateMessage("Stream state entered. Starting streaming task...");

    // ========================================================================
    // CRITICAL FIX: Stop any AI inference tasks that might be running
    // Race condition fix: Multiple tasks were processing radar data simultaneously
    // ========================================================================
    extern TaskHandle_t producerTaskHandle;        // 2-task AI version (deprecated)
    extern TaskHandle_t producerTaskHandle_3task;  // 3-task AI version (ProducterTask)
    extern TaskHandle_t g_processing_task_handle;  // 2-task AI version (merged WindowManager+AIInference)

    // Delete AI inference tasks if they exist (both old and new architectures)
    if (producerTaskHandle != NULL) {
        vTaskDelete(producerTaskHandle);
        producerTaskHandle = NULL;
        sendStateMessage("Stopped deprecated 2-task producer (was conflicting with stream)");
    }
    if (producerTaskHandle_3task != NULL) {
        vTaskDelete(producerTaskHandle_3task);
        producerTaskHandle_3task = NULL;
        sendStateMessage("Stopped 3-task producer (was conflicting with stream)");
    }
    if (g_processing_task_handle != NULL) {
        vTaskDelete(g_processing_task_handle);
        g_processing_task_handle = NULL;
        sendStateMessage("Stopped processing+inference task (was conflicting with stream)");
    }

    // Small delay to ensure tasks are fully deleted before starting stream
    vTaskDelay(pdMS_TO_TICKS(100));

    // g_radarFlags'ı başlat - mevcut durumu oku
    g_radarFlags.ssFlag = (HAL_GPIO_ReadPin(RADAR_SS_GPIO_Port, RADAR_SS_Pin) == GPIO_PIN_SET);
    g_radarFlags.intgClkFlag = 0;
    g_radarFlags.lastSSTime = HAL_GetTick();
    g_radarFlags.lastIntgTime = HAL_GetTick();

    // Durumu sıfırla
    stopStreamingFlag = false;
    frameCounter = 0;

    // Tamponları başlat
    for (int i = 0; i < BUFFER_COUNT; i++) {
        frame_buffer_init(&frameBuffers[i]);
    }

    // Tampon indekslerini ayarla
    activeBufferIndex = 0;
    processingBufferIndex = 1;

    // ADC Sampling task'ı oluştur
    BaseType_t result = xTaskCreate(
        ADC_SamplingTask,       // Task fonksiyonu
        "ADCSampling",          // Task adı
        2048,                   // Stack boyutu
        NULL,                   // Task'a geçirilecek parametre
        osPriorityAboveNormal,  // Öncelik (yüksek öncelik)
        &adcSampleTaskHandle    // Task handle
    );
    
    if (result != pdPASS) {
        sendStateMessage("ERROR: Failed to create ADC sampling task");
        StateMachine::getInstance()->changeState(STATE_IDLE);
    }
}

void Stream::OnExit() {
    sendStateMessage("Stream state exited. Stopping streaming task...");

    // Streaming durumunu sonlandır
    stopStreamingFlag = true;

    // Task'ı bekle ve sil
    if (adcSampleTaskHandle != NULL) {
        vTaskDelay(pdMS_TO_TICKS(100)); // Task'ın sonlanmasını bekle
        vTaskDelete(adcSampleTaskHandle);
        adcSampleTaskHandle = NULL;
    }

    if (streamingTaskHandle != NULL) {
        vTaskDelete(streamingTaskHandle);
        streamingTaskHandle = NULL;
    }
}

// ADC Örnekleme Task'ı
void ADC_SamplingTask(void* pvParameters) {
    UsbCommunication* usb = UsbCommunication::getInstance();
    uint32_t notificationValue;

    // DIAGNOSTIC: Task started
    if (usb) {
        usb->sendStatusMessage("STREAM_DIAG", "ADC_SamplingTask started, waiting for SS_HIGH");
    }

    // SS yükselmesini bekle (Frame başlangıcı)
    while (1) {
        if (xTaskNotifyWait(0, ULONG_MAX, &notificationValue, pdMS_TO_TICKS(1000)) == pdTRUE) {
            if ((notificationValue & SS_HIGH_EVENT) != 0) {
                if (usb) {
                    usb->sendStatusMessage("STREAM_DIAG", "SS_HIGH received, waiting for SS_LOW");
                }
                break;
            }
        }

        if (stopStreamingFlag) {
            // CRITICAL: Handle'ı NULL'la ÖNCE — ISR silinmiş task'a
            // notification göndermesin (use-after-free koruması)
            taskENTER_CRITICAL();
            adcSampleTaskHandle = NULL;
            taskEXIT_CRITICAL();
            vTaskDelete(NULL);
            return;
        }
    }

    // SS düşmesini bekle (Veri toplama başlangıcı)
    while (1) {
        if (xTaskNotifyWait(0, ULONG_MAX, &notificationValue, pdMS_TO_TICKS(1000)) == pdTRUE) {
            if ((notificationValue & SS_LOW_EVENT) != 0) {
                frameBuffers[activeBufferIndex].count = 0;
                frameBuffers[activeBufferIndex].captureTime = HAL_GetTick();
                break;
            }
        }

        if (stopStreamingFlag) {
            taskENTER_CRITICAL();
            adcSampleTaskHandle = NULL;
            taskEXIT_CRITICAL();
            vTaskDelete(NULL);
            return;
        }
    }

    // Ana veri toplama döngüsü
    while (!stopStreamingFlag) {
        if (xTaskNotifyWait(0, ULONG_MAX, &notificationValue, pdMS_TO_TICKS(100)) == pdTRUE) {

            // INTG_CLK olayı - ADC örneği al
            if ((notificationValue & INTG_CLK_EVENT) != 0) {
                if (g_radarFlags.ssFlag == 0) {
                    HAL_ADC_Start(&hadc3);

                    if (HAL_ADC_PollForConversion(&hadc3, 1) == HAL_OK) {
                        int16_t sample = (int16_t)(HAL_ADC_GetValue(&hadc3) - 32768);

                        if (frameBuffers[activeBufferIndex].count < MAX_FRAME_SAMPLES) {
                            frameBuffers[activeBufferIndex].samples[frameBuffers[activeBufferIndex].count++] = sample;
                        }
                    }

                    HAL_ADC_Stop(&hadc3);
                }
            }

            // SS yükseldi - frame tamamlandı
            if ((notificationValue & SS_HIGH_EVENT) != 0) {
                if (frameBuffers[activeBufferIndex].count > 0) {
                    frameBuffers[activeBufferIndex].endTime = HAL_GetTick();
                    frameBuffers[activeBufferIndex].frameNumber = ++frameCounter;

                    // DIAGNOSTIC: Frame completed
                    if (usb && frameCounter % 50 == 0) {
                        char msg[64];
                        snprintf(msg, sizeof(msg), "Frame #%lu: %d samples",
                                frameCounter, frameBuffers[activeBufferIndex].count);
                        usb->sendStatusMessage("STREAM_DIAG", msg);
                    }

                    taskENTER_CRITICAL();

                    // Tampon indekslerini değiştir
                    uint8_t tempIndex = activeBufferIndex;
                    activeBufferIndex = processingBufferIndex;
                    processingBufferIndex = tempIndex;

                    taskEXIT_CRITICAL();

                    frameBuffers[activeBufferIndex].count = 0;
                    frameBuffers[activeBufferIndex].captureTime = HAL_GetTick();

                    if (usb) {
                        send_frame_direct(usb, &frameBuffers[processingBufferIndex]);
                    }
                }
            }

            // SS düştü - yeni frame başlıyor
            if ((notificationValue & SS_LOW_EVENT) != 0) {
                if (frameBuffers[activeBufferIndex].count == 0) {
                    frameBuffers[activeBufferIndex].captureTime = HAL_GetTick();
                }
            }
        }
    }

    // CRITICAL: Handle'ı NULL'la ÖNCE — ISR silinmiş task'a
    // notification göndermesin (use-after-free koruması)
    taskENTER_CRITICAL();
    adcSampleTaskHandle = NULL;
    taskEXIT_CRITICAL();

    // ADC'yi temiz bırak (HAL_ADC_Start/Stop arasında olabilirdik)
    HAL_ADC_Stop(&hadc3);

    vTaskDelete(NULL);
}
