#ifndef RADAR_ADC_TEST_HPP_
#define RADAR_ADC_TEST_HPP_

#include "stm32h7xx_hal.h" // HAL kütüphanesi
#include <stdint.h>        // Standart integer tipleri
#include <stdbool.h>       // bool tipi için

// --- Test Veri Buffer ve Bayrakları ---
// Bu değişkenlere dışarıdan erişim gerekirse 'extern' olarak bırakılabilir,
// ama idealde bir sınıfın üyesi olmaları daha iyidir.
// Test amacıyla global tanımlama yapıyoruz.

#define ADC_TEST_MAX_SAMPLES 2048 // Örnekteki boyut

extern int16_t  g_adcTestData[ADC_TEST_MAX_SAMPLES]; // Test verisi için buffer
extern volatile uint16_t g_adcTestSampleIndex;       // Örnek indeksi
extern volatile uint8_t  g_adcTestSsFlag;            // Senkronizasyon bayrağı (SS)
extern volatile uint8_t  g_adcTestIntgClkFlag;       // Senkronizasyon bayrağı (INTG_CLK)

// --- Fonksiyon Prototipi ---

/**
 * @brief Radar sensöründen tek bir frame (çerçeve) ADC verisi okur (Test amaçlı).
 * Bu fonksiyon bloke edicidir ve bayrakların set edilmesini bekler.
 * @param hadc Kullanılacak ADC handle'ı (örn. &hadc3).
 * @param timeout_ms Bayrakların beklenmesi için maksimum süre (milisaniye).
 * @retval true Okuma başarılıysa, false timeout veya hata oluşursa.
 */
bool Perform_Single_Frame_Reading_Test(ADC_HandleTypeDef* hadc, uint32_t timeout_ms);

// --- EXTI Callback Bildirimi ---
// Bu callback'in başka bir yerde (örn. stm32h7xx_it.c) tanımlı OLMADIĞINDAN emin olun.
// Eğer orada tanımlıysa, bu satırı kaldırın ve oradaki callback'in
// g_adcTestSsFlag ve g_adcTestIntgClkFlag'ı set ettiğinden emin olun.
// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin); // Eğer it.c'de yoksa uncomment yapın

#endif /* RADAR_ADC_TEST_HPP_ */
