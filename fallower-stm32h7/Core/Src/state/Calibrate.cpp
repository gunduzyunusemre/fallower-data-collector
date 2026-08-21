#include "Calibrate.hpp"
#include "Event.hpp"
#include "StateMachine.hpp"
#include "StateFactory.hpp"
#include "../UsbCommunication.hpp"
#include "../radar_driver.hpp"
#include "../radar_adc_test.hpp"
#include <stdio.h>
#include "cmsis_os.h"

extern SPI_HandleTypeDef hspi4;
extern ADC_HandleTypeDef hadc3;

static const uint8_t MAX_CALIBRATION_ITERATIONS = 16;

// REG02 (Enable_3) bit 4 = EN_TX_CK_BUF ... NOTE: per datasheet the DC-offset
// calibration flow toggles "REG02<4>" as the TX enable bit. Adjust this mask
// if your register map differs.
static const uint8_t REG_ENABLE_3_ADDR = 0x02;
static const uint8_t TX_ENABLE_BIT     = (1 << 4);

// Made non-static for AutoStartupTask polling (extern in main.cpp)
TaskHandle_t calibrationTaskHandle = NULL;

// **KRİTİK FİX**: Static task allocation - heap tükenme problemi çözümü
// Place in RAM_D2 to avoid RAM_D1 overflow (8KB stack)
static StaticTask_t calibrationTaskTCB;
__attribute__((section(".ram_d2"))) static StackType_t calibrationTaskStack[2048];

void Calibrate::HandleEvent(Event* event, void* args) {
    if (!event) return;

    UsbCommunication* usb = UsbCommunication::getInstance();

    switch (event->GetType()) {
        case EventType::StartStreaming:
            if (usb) {
                usb->sendStatusMessage(getStateName(), "Starting stream mode");
            }
            break;

        default:
            if (usb) {
                usb->sendStatusMessage(getStateName(), "Unsupported event in Calibrate state");
            }
            break;
    }
}


// SIMPLIFIED VERSION from ornek_calisan (working example)
// Removed timeout protection - proven to work reliably during training
static int16_t calculate_frame_average()
{
    while (!g_adcTestSsFlag) {
    }

    while (g_adcTestSsFlag) {
    }

    g_adcTestIntgClkFlag = 0;
    g_adcTestSampleIndex = 0;

    while (1)
    {
        if (g_adcTestIntgClkFlag) {
            if (g_adcTestSsFlag == 0) {
                g_adcTestIntgClkFlag = 0;

                HAL_ADC_Start(&hadc3);
                if (HAL_ADC_PollForConversion(&hadc3, 1) == HAL_OK)
                {
                    int16_t sample = HAL_ADC_GetValue(&hadc3) - 32768;
                    g_adcTestData[g_adcTestSampleIndex] = sample;
                    g_adcTestSampleIndex++;
                }
                else {
                    g_adcTestData[g_adcTestSampleIndex] = 0;
                    g_adcTestSampleIndex++;
                }
                HAL_ADC_Stop(&hadc3);
            }
            else if (g_adcTestSsFlag == 1) {
                int32_t sum = 0;

                for (int i = 0; i < g_adcTestSampleIndex; i++) {
                    sum += g_adcTestData[i];
                }

                return sum / g_adcTestSampleIndex;
            }
        }
    }
}

// --- FIX: helpers to gate TX during calibration, per datasheet flow chart ---
static void Sensor_SetTxEnabled(bool enabled)
{
    uint8_t reg02 = Sensor_ReadRegister(&hspi4, REG_ENABLE_3_ADDR);

    if (enabled) {
        reg02 |= TX_ENABLE_BIT;
    } else {
        reg02 &= ~TX_ENABLE_BIT;
    }

    Sensor_WriteRegister(&hspi4, REG_ENABLE_3_ADDR, reg02);
}

static void calibrationTaskFunction(void* parameter) {
    UsbCommunication* usb = UsbCommunication::getInstance();
    usb->sendStatusMessage("CALIBRATE", "Calibration task started");

    // MATCH ornek_calisan: Longer initial delay for radar stabilization
    vTaskDelay(pdMS_TO_TICKS(1000));

    if (usb) {
        usb->sendStatusMessage("CALIBRATE", "Starting calibration process...");
    }

    // --- FIX: TX power down BEFORE measuring DC offset (datasheet flow) ---
    // Prevents TX-to-RX coupling from polluting the DC-offset measurement.
    Sensor_SetTxEnabled(false);
    usb->sendStatusMessage("CALIBRATE", "TX powered down for calibration");
    vTaskDelay(pdMS_TO_TICKS(20)); // let the front-end settle after TX off

    // Kalibrasyon değişkenlerini başlat
    // --- FIX: start from mid-scale (0x8000) instead of 0xFFFF ---
    // 0x8000 matches a standard SAR search starting point and roughly aligns
    // with the datasheet default (REG08=0x80 coarse mid-code, REG09=0x00).
    uint16_t dc_offset_cont_reg = 0x8000; // Başlangıç DC offset değeri (mid-scale)
    uint8_t iteration __attribute__((unused)) = 0;
    const uint16_t MAX_TOTAL_ITERATIONS = 1000; // Maksimum toplam iterasyon sayısı (sonsuz döngüden korunmak için)
    bool calibration_success = false;

    // İlk register değerlerini ayarla
    Sensor_WriteDCOffset(&hspi4, (dc_offset_cont_reg >> 8) & 0xFF, dc_offset_cont_reg & 0xFF);

    // Register değerlerini oku ve göster
    uint8_t reg08 = Sensor_ReadRegister(&hspi4, 0x08);
    uint8_t reg09 = Sensor_ReadRegister(&hspi4, 0x09);

    char regValues[100];
    sprintf(regValues, "Initial registers: 0x08=0x%02X, 0x09=0x%02X, DC_OFFSET=0x%04X",
            reg08, reg09, dc_offset_cont_reg);
    usb->sendStatusMessage("CALIBRATE", regValues);

    // Kalibrasyon döngüsü - kalibrasyon başarılı olana kadar devam et
    // Ancak sonsuz döngüye girmemek için maksimum toplam iterasyon sayısı belirle
    uint16_t total_iterations = 0;

    while (!calibration_success && total_iterations < MAX_TOTAL_ITERATIONS) {
        char iterMsg[64];
        sprintf(iterMsg, "Calibration iteration %u", total_iterations + 1);
        usb->sendStatusMessage("CALIBRATE", iterMsg);
        vTaskDelay(pdMS_TO_TICKS(50));

        // Frame ortalamasını ölç - statik fonksiyonu doğrudan çağır
        int16_t frameAverage = calculate_frame_average();

        char avgMsg[64];
        sprintf(avgMsg, "Frame average value: %d", frameAverage);
        usb->sendStatusMessage("CALIBRATE", avgMsg);
        vTaskDelay(pdMS_TO_TICKS(50));

        // --- FIX: monotonically shrinking SAR step, no modulo reset ---
        // Step halves every iteration (bit 15 down to bit 0), then stays at 1
        // if convergence hasn't happened yet, instead of jumping back to
        // 0x8000 every 16 iterations.
        uint16_t adjustment = (total_iterations < 16)
            ? (uint16_t)(1u << (15 - total_iterations))
            : 1u;

        // Kalibrasyon mantığı - Direkt frame ortalamasını değerlendir
        if (frameAverage >= -320 && frameAverage <= 320) {
            // İdeal durum - kalibrasyon tamamlandı
            calibration_success = true;
            usb->sendStatusMessage("CALIBRATE", "Calibration completed successfully!");
        } else if (frameAverage > 32) {
            // Ortalama çok yüksek, DC offset'i artır
            dc_offset_cont_reg += adjustment;

            char adjMsg[64];
            sprintf(adjMsg, "Average too high, increasing DC offset by 0x%04X", adjustment);
            usb->sendStatusMessage("CALIBRATE", adjMsg);
        } else if (frameAverage < -32) {
            // Ortalama çok düşük, DC offset'i azalt
            dc_offset_cont_reg -= adjustment;

            char adjMsg[64];
            sprintf(adjMsg, "Average too low, decreasing DC offset by 0x%04X", adjustment);
            usb->sendStatusMessage("CALIBRATE", adjMsg);
        }

        if (!calibration_success) {
            // Yeni DC offset değerlerini yaz
            Sensor_WriteDCOffset(&hspi4, (dc_offset_cont_reg >> 8) & 0xFF, dc_offset_cont_reg & 0xFF);

            char regMsg[64];
            sprintf(regMsg, "Updated DC offset: 0x%04X", dc_offset_cont_reg);
            usb->sendStatusMessage("CALIBRATE", regMsg);
        }

        // İterasyon sayacını artır
        total_iterations++;

        // İterasyonlar arası bekle
        vTaskDelay(pdMS_TO_TICKS(200));

        // Her MAX_CALIBRATION_ITERATIONS iterasyonda bir durum bilgisi gönder
        if (total_iterations % MAX_CALIBRATION_ITERATIONS == 0) {
            char statusMsg[64];
            sprintf(statusMsg, "Completed %u iterations. Still calibrating...", total_iterations);
            usb->sendStatusMessage("CALIBRATE", statusMsg);
            vTaskDelay(pdMS_TO_TICKS(500)); // Biraz daha uzun bekle
        }
    }

    // Kalibrasyon sonucu
    if (calibration_success) {
        char finalMsg[100];
        sprintf(finalMsg, "Calibration successful after %u iterations. Final DC offset: 0x%04X",
                total_iterations, dc_offset_cont_reg);
        usb->sendStatusMessage("CALIBRATE", finalMsg);
    } else {
        usb->sendStatusMessage("CALIBRATE", "Calibration did not converge within maximum allowed iterations");
        usb->sendStatusMessage("CALIBRATE", "Using best effort DC offset value");
    }

    // Son kalibrasyon değerlerini göster
    reg08 = Sensor_ReadRegister(&hspi4, 0x08);
    reg09 = Sensor_ReadRegister(&hspi4, 0x09);

    sprintf(regValues, "Final registers: 0x08=0x%02X, 0x09=0x%02X (0x%04X)",
            reg08, reg09, (reg08 << 8) | reg09);
    usb->sendStatusMessage("CALIBRATE", regValues);

    // --- FIX: TX enable AFTER calibration completes (datasheet flow) ---
    Sensor_SetTxEnabled(true);
    usb->sendStatusMessage("CALIBRATE", "TX re-enabled after calibration");

    usb->sendStatusMessage("CALIBRATE", "Calibration process completed. Send START command to begin streaming.");

    // Task tamamlandığında kendini sonlandır
    calibrationTaskHandle = NULL;
    vTaskDelete(NULL);
}

void Calibrate::OnEnter() {
    sendStateMessage("Calibrate state entered. Starting calibration task...");

    if (calibrationTaskHandle != NULL) {
        vTaskDelete(calibrationTaskHandle);
        calibrationTaskHandle = NULL;
    }

    // **KRİTİK FİX**: xTaskCreateStatic kullan - heap tükenmesi çözümü
    calibrationTaskHandle = xTaskCreateStatic(
        calibrationTaskFunction,  // Task fonksiyonu
        "CalibTask",              // Task adı
        2048,                     // Stack boyutu (words)
        NULL,                     // Parametre
        tskIDLE_PRIORITY + 2,     // Öncelik
        calibrationTaskStack,     // Stack buffer
        &calibrationTaskTCB       // TCB buffer
    );

    if (calibrationTaskHandle == NULL) {
        sendStateMessage("ERROR: Failed to create calibration task!");
    }
}
