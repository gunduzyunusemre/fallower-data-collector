#include "Init.hpp"
#include "Event.hpp"
#include "StateMachine.hpp"
#include "../UsbCommunication.hpp"
#include <stdio.h>

#include "main.h"           // HAL handle'ları (hspi4, hadc3) ve pin tanımları için
#include "cmsis_os.h"       // osDelay fonksiyonu için
#include "../radar_driver.hpp" // Radar SPI fonksiyonları için
#include "../radar_adc_test.hpp" // ADC test okuma fonksiyonu için

extern SPI_HandleTypeDef hspi4;
extern ADC_HandleTypeDef hadc3;
// --------------------------------------------------

void Init::HandleEvent(Event* event, void* args) {
    if (!event) return;
    sendStateMessage("Unexpected event received while in Init state.");
    (void)args;
}


void Init::OnEnter() {
    sendStateMessage("Init state entered. Starting Radar Initialization...");
    vTaskDelay(pdMS_TO_TICKS(50)); // Zaten mevcut

    sendStateMessage("Resetting Radar...");
    vTaskDelay(pdMS_TO_TICKS(50)); // Zaten mevcut
    Sensor_Reset();
    osDelay(500);
    sendStateMessage("Radar Reset Complete.");
    vTaskDelay(pdMS_TO_TICKS(50)); // Eklendi

    sendStateMessage("Writing Radar Registers...");
    vTaskDelay(pdMS_TO_TICKS(50)); // Eklendi
    Sensor_WriteAllRegisters(&hspi4);
    osDelay(400);

    sendStateMessage("Writing specific configuration registers...");
    vTaskDelay(pdMS_TO_TICKS(50)); // Eklendi
    Sensor_WriteRegister(&hspi4, 0x08, 0x80);
    osDelay(10);
    Sensor_WriteRegister(&hspi4, 0x09, 0x00);
    osDelay(400);
    Sensor_WriteRegister(&hspi4, 0x02, 0xFF);
    osDelay(300);
    sendStateMessage("Radar Register Configuration Finished.");
    vTaskDelay(pdMS_TO_TICKS(50)); // Eklendi

    sendStateMessage("Performing single frame ADC reading test...");
    vTaskDelay(pdMS_TO_TICKS(50)); // Eklendi
    uint32_t adc_timeout = 5000;
    bool adc_test_successful = Perform_Single_Frame_Reading_Test(&hadc3, adc_timeout);

    if (adc_test_successful) {
        sendStateMessage("ADC Test Frame Reading Completed Successfully.");
        vTaskDelay(pdMS_TO_TICKS(50)); // Eklendi
    } else {
        sendStateMessage("ADC Test Frame Reading FAILED (Timeout or Error). Check EXTI flags/ADC.");
        vTaskDelay(pdMS_TO_TICKS(50)); // Eklendi
    }

    sendStateMessage("Initialization Sequence Complete. Send READY command to proceed.");
    vTaskDelay(pdMS_TO_TICKS(50)); // Eklendi
}
