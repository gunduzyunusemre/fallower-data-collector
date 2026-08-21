#ifndef RADAR_DRIVER_HPP_
#define RADAR_DRIVER_HPP_

#include "stm32h7xx_hal.h" // HAL kütüphanesi fonksiyonları ve tipleri için
#include <stdint.h>        // Standart integer tipleri için (uint8_t, uint16_t)

// --- Pin Tanımlamaları ---
// Bu tanımlamaları projenizdeki main.h veya gpio konfigürasyonunuza göre
// güncelleyebilirsiniz veya fonksiyonlara parametre olarak geçirebilirsiniz.
// Şimdilik örnek main.c'deki gibi bırakıyorum.
#define RADAR_CS_PORT    GPIOB // RADAR Chip Select Portu
#define RADAR_CS_PIN     RADAR_SPI_nCS_Pin // RADAR Chip Select Pini (CubeMX'teki isme göre)
#define RADAR_RST_PORT   GPIOB // RADAR Reset Portu
#define RADAR_RST_PIN    RADAR_SPI_RST_Pin // RADAR Reset Pini (CubeMX'teki isme göre)

// --- Fonksiyon Prototipleri ---

/**
 * @brief Belirtilen adresteki radar register'ına veri yazar.
 * @param hspi: Kullanılacak SPI handle'ı (örn. &hspi4).
 * @param regAddress: Yazılacak register adresi (0x00 - 0x3F).
 * @param data: Yazılacak 8-bit veri.
 * @retval None
 */
void Sensor_WriteRegister(SPI_HandleTypeDef* hspi, uint8_t regAddress, uint8_t data);

/**
 * @brief Belirtilen adresteki radar register'ından veri okur.
 * @param hspi: Kullanılacak SPI handle'ı (örn. &hspi4).
 * @param regAddress: Okunacak register adresi (0x00 - 0x3F).
 * @retval uint8_t: Okunan 8-bit veri.
 */
uint8_t Sensor_ReadRegister(SPI_HandleTypeDef* hspi, uint8_t regAddress);

/**
 * @brief Radar sensörünü donanımsal olarak resetler.
 * @retval None
 */
void Sensor_Reset(void);

/**
 * @brief Radar sensörünün başlangıç register değerlerini yazar.
 * @param hspi: Kullanılacak SPI handle'ı (örn. &hspi4).
 * @retval None
 */
void Sensor_WriteAllRegisters(SPI_HandleTypeDef* hspi);
void Sensor_WriteDCOffset(SPI_HandleTypeDef* hspi, uint8_t valueForReg08, uint8_t valueForReg09);

/**
 * @brief Checks connection with ESP32 via SPI1.
 * @retval uint8_t: The byte received from ESP32 (Expecting 0x55).
 */
uint8_t SPI_Check_Connection(void);
void SPI_Send_Inference_Result(uint8_t class_id, float confidence);




#endif /* RADAR_DRIVER_HPP_ */
