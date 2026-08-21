/**
 * @file radar_adc_test.cpp
 * @brief Radar interrupt handlers and ISR-to-Task notification routing
 *
 * This file handles EXTI interrupts from radar sensor:
 * - RADAR_SS_Pin: Sample Start signal (frame boundary)
 * - RADAR_INTG_CLK_Pin: Integration clock (sample trigger)
 *
 * ISR routes notifications to the appropriate task based on current mode:
 * - Stream mode: adcSampleTaskHandle
 * - AI Inference mode (3Task): g_producer_task_handle (context switch per sample)
 * - AI Inference mode (ISR): Direct ISR processing (NO context switch!)
 * - AI Inference mode (ISR_REAL): Stride-5 circular buffer mode
 */

#include "state/Stream.hpp"
#include "radar_adc_test.hpp"
#include "main.h"
#include "cmsis_os.h"
#include "UsbCommunication.hpp"
#include <stdio.h>
#include "radar_flags.h"
#include "state/RealtimeInference_3Task.hpp"
#include "state/RealtimeInference_ISR.hpp"
#include "state/RealtimeInference_ISR_Real.hpp"

// ============================================================================
// EXTERNAL TASK HANDLES
// ============================================================================

// Stream mode task
extern TaskHandle_t adcSampleTaskHandle;

// AI Inference mode task (3-task architecture)
// This is the primary handle for AI mode
extern TaskHandle_t g_producer_task_handle;

// Legacy 2-task handles (deprecated, kept for compatibility)
extern TaskHandle_t producerTaskHandle;
TaskHandle_t producerTaskHandle_3task = NULL;  // Alias for g_producer_task_handle

// ============================================================================
// ISR MODE FLAG
// ============================================================================

// When true, ISR handles ADC directly instead of notifying tasks
// This eliminates ~6720 context switches per batch!
volatile bool g_isr_mode_active = false;

// ============================================================================
// TEST VARIABLES
// ============================================================================

int16_t g_adcTestData[ADC_TEST_MAX_SAMPLES];
volatile uint16_t g_adcTestSampleIndex = 0;
volatile uint8_t g_adcTestSsFlag = 0;
volatile uint8_t g_adcTestIntgClkFlag = 0;

// ============================================================================
// TEST FUNCTION
// ============================================================================

/**
 * @brief Tek bir radar frame'i için örnekleme testi gerçekleştirir.
 * 
 * Bu test fonksiyonu, SS (Sample Start) sinyalini bekler ve ardından INTG_CLK 
 * sinyalini takip ederek ADC üzerinden örnekler toplar. Toplanan veriler `g_adcTestData` 
 * dizisine kaydedilir. Kullanıcıya USB üzerinden süreç bilgisi gönderir.
 * 
 * @param hadc Kullanılacak ADC handle'ı (örn. &hadc1).
 * @param timeout_ms İşlem için maksimum bekleme süresi (şu anki implementasyonda kullanılmamaktadır). [ms]
 * @return bool Test başarıyla tamamlandıysa true, hata oluştuysa false döner.
 * 
 * @note Bu fonksiyon polling (bekleme) döngüleri ve vTaskDelay içerdiği için 
 *       kesme (ISR) içinden ÇAĞRILAMAZ. Sadece düşük öncelikli test task'larında kullanılmalıdır.
 */
bool Perform_Single_Frame_Reading_Test(ADC_HandleTypeDef* hadc, uint32_t timeout_ms)
{
    UsbCommunication* usb = UsbCommunication::getInstance();
    (void)timeout_ms;

    usb->sendMessage("[ADC Test] Waiting for SS flag to rise...\r\n");
    vTaskDelay(pdMS_TO_TICKS(200));

    while (!g_adcTestSsFlag) {
        // Wait for SS to go high
    }

    while (g_adcTestSsFlag) {
        // Wait for SS to go low
    }

    g_adcTestIntgClkFlag = 0;
    g_adcTestSampleIndex = 0;

    while (1)
    {
        if (g_adcTestIntgClkFlag) {
            if (g_adcTestSsFlag == 0) {
                g_adcTestIntgClkFlag = 0;

                HAL_ADC_Start(hadc);
                if (HAL_ADC_PollForConversion(hadc, 1) == HAL_OK)
                {
                    int16_t sample = (int16_t)(HAL_ADC_GetValue(hadc)) - 32768;
                    // FIX F4: Array bounds check to prevent buffer overflow
                    if (g_adcTestSampleIndex < ADC_TEST_MAX_SAMPLES) {
                        g_adcTestData[g_adcTestSampleIndex] = sample;
                        g_adcTestSampleIndex++;
                    }
                }
                else {
                    // FIX F4: Array bounds check
                    if (g_adcTestSampleIndex < ADC_TEST_MAX_SAMPLES) {
                        g_adcTestData[g_adcTestSampleIndex] = 0;
                        g_adcTestSampleIndex++;
                    }
                }
                HAL_ADC_Stop(hadc);
            }
            else if (g_adcTestSsFlag == 1) {
                usb->sendMessage("[ADC Test] Frame finished. Samples collected: ");
                char count_str[16];  // FIX F3: Buffer overflow - increased size
                snprintf(count_str, sizeof(count_str), "%u\r\n", g_adcTestSampleIndex);
                usb->sendMessage(count_str);
                return true;
            }
        }
    }

    return false;
}

// ============================================================================
// GPIO INTERRUPT CALLBACK
// ============================================================================
/**
 * @brief EXTI interrupt handler for radar signals
 *
 * Routes notifications to the appropriate handler based on current mode:
 *
 * 1. ISR Mode (g_isr_mode_active = true):
 *    - SS_LOW/HIGH: Call ISR_HandleSSLow/High() directly
 *    - INTG_CLK: Call ISR_HandleIntgClk() - direct ADC read (~1µs, NO context switch!)
 *
 * 2. 3Task Mode (g_producer_task_handle != NULL):
 *    - All events: xTaskNotifyFromISR() → context switch → task handles ADC
 *
 * 3. Stream Mode (adcSampleTaskHandle != NULL):
 *    - All events: xTaskNotifyFromISR() → context switch → task handles ADC
 *
 * ISR mode eliminates ~6720 context switches per batch (672 samples × 30 frames × 0.33)
 */
/**
 * @brief Radar sinyalleri için EXTI kesme yakalayıcı (Interrupt Callback).
 * 
 * Bu fonksiyon, RADAR_SS ve RADAR_INTG_CLK pinlerinden gelen donanımsal kesmeleri işler.
 * Sistemin o anki moduna (DMA, DMA_REAL, 3Task veya Stream) göre ilgili işleyiciye
 * (handler) yönlendirme yapar veya RTOS task'larına bildirim (notification) gönderir.
 * 
 * @param GPIO_Pin Kesmeyi tetikleyen pin numarası.
 * @return None
 * 
 * @note Bu fonksiyon BİR KESME (ISR) FONKSİYONUDUR. İçerisinde bloklamalı işlemler, 
 *       uzun döngüler veya RTOS delay fonksiyonları kesinlikle kullanılmamalıdır.
 *       Hız kritik bir noktadır; DMA modunda doğrudan ADC okuması yaparak bağlam anahtarlamasını 
 *       (context switch) minimize eder.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (GPIO_Pin == RADAR_SS_Pin)
    {
        // Read current pin state
        uint8_t pinState = HAL_GPIO_ReadPin(RADAR_SS_GPIO_Port, RADAR_SS_Pin);

        // Update global flags for all systems
        g_radarFlags.ssFlag = (pinState == GPIO_PIN_SET) ? 1 : 0;
        g_adcTestSsFlag = g_radarFlags.ssFlag;

        // ============================================================
        // ISR_REAL MODE: Real-time Stride-5 with circular buffer
        // ============================================================
        if (g_isr_real_mode) {
            if (pinState == GPIO_PIN_SET) {
                ISR_Real_HandleSSHigh();
            } else {
                ISR_Real_HandleSSLow();
            }
            return;
        }

        // ============================================================
        // ISR MODE: Direct ISR handling (NO context switch!)
        // ============================================================
        if (g_isr_mode_active) {
            if (pinState == GPIO_PIN_SET) {
                // SS_HIGH: Frame end
                ISR_HandleSSHigh();
            } else {
                // SS_LOW: Frame start
                ISR_HandleSSLow();
            }
            return;  // Don't notify tasks in ISR mode
        }

        // ============================================================
        // LEGACY MODE: Task notification (context switch per event)
        // ============================================================
        uint32_t notification = (pinState == GPIO_PIN_SET) ? SS_HIGH_EVENT : SS_LOW_EVENT;

        // Route to active task (mutually exclusive)
        if (adcSampleTaskHandle != NULL) {
            xTaskNotifyFromISR(adcSampleTaskHandle, notification, eSetBits, &xHigherPriorityTaskWoken);
        }
        else if (g_producer_task_handle != NULL) {
            xTaskNotifyFromISR(g_producer_task_handle, notification, eSetBits, &xHigherPriorityTaskWoken);
        }
    }
    else if (GPIO_Pin == RADAR_INTG_CLK_Pin)
    {
        // Update global flags
        g_radarFlags.intgClkFlag = 1;
        g_adcTestIntgClkFlag = 1;

        // ============================================================
        // ISR_REAL MODE: Real-time Stride-5 with circular buffer
        // ============================================================
        if (g_isr_real_mode) {
            ISR_Real_HandleIntgClk();
            return;
        }

        // ============================================================
        // ISR MODE: Direct ADC read in ISR (~1µs vs ~15µs with task)
        // This is the KEY optimization - eliminates 6720 context switches!
        // ============================================================
        if (g_isr_mode_active) {
            ISR_HandleIntgClk();
            return;  // Don't notify tasks in ISR mode
        }

        // ============================================================
        // LEGACY MODE: Task notification (context switch per sample)
        // ============================================================
        if (adcSampleTaskHandle != NULL) {
            xTaskNotifyFromISR(adcSampleTaskHandle, INTG_CLK_EVENT, eSetBits, &xHigherPriorityTaskWoken);
        }
        else if (g_producer_task_handle != NULL) {
            xTaskNotifyFromISR(g_producer_task_handle, INTG_CLK_EVENT, eSetBits, &xHigherPriorityTaskWoken);
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
