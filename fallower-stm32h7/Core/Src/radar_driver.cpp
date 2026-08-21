#include "radar_driver.hpp"
#include "cmsis_os.h"
#include "main.h"
#include <cstring>

/**
 * @brief Belirtilen adresteki radar register'ına veri yazar.
 * 
 * Bu fonksiyon, 16-bitlik bir SPI paketi oluşturarak radar sensörünün dahili register'larına
 * veri gönderir. Paket yapısı: [Başlangıç(1 bit) | R/W(1 bit, Yaz=0) | Adres(6 bit) | Veri(8 bit)].
 * 
 * @param hspi Kullanılacak SPI handle'ı (örn. &hspi4).
 * @param regAddress Yazılacak register adresi (0x00 - 0x3F). [birimsiz]
 * @param data Yazılacak 8-bit veri. [8-bit değer]
 * @return None
 * 
 * @note Bu fonksiyon HAL_SPI_Transmit (bloklamalı) kullandığı ve CS pinini manuel yönettiği için 
 *       kesme (ISR) içinden çağrılması önerilmez. FreeRTOS task'ları içinden çağrılmalıdır.
 */
void Sensor_WriteRegister(SPI_HandleTypeDef* hspi, uint8_t regAddress, uint8_t data)
{
  uint16_t packet = 0;

  // SPI paketini oluştur: 16 bit = | Başlangıç(1) | R/W(1) | Adres(6) | Veri(8) |
  packet |= (1 << 15);                // Başlangıç biti (1)
  packet |= (0 << 14);                // R/W biti (0: Yazma)
  packet |= (regAddress & 0x3F) << 8; // Adres bitleri (6 bit), sola kaydır
  packet |= data;                     // Veri bitleri (8 bit)

  // Chip Select (CS) pinini aktif et (LOW)
  HAL_GPIO_WritePin(RADAR_CS_PORT, RADAR_CS_PIN, GPIO_PIN_RESET);
  // osDelay(1); // Çok kısa gecikmeler gerekiyorsa eklenebilir, ancak SPI hızı düşükse gerekmeyebilir.
                // HAL_Delay yerine osDelay kullanıyoruz.

  // 16-bit veriyi SPI üzerinden gönder (HAL_SPI_Transmit 16 bit modunda 1 birim gönderir)
  // Dikkat: hspi handle'ının DataSize'ı SPI_DATASIZE_16BIT olarak ayarlanmış olmalı.
  // Eğer 8 bit ise, 2 byte olarak ayrı ayrı gönderim gerekebilir.
  // Örnek main.c'de 16 bit ayarlı olduğu varsayılarak:

  // MATCH ornek_calisan: Use HAL_MAX_DELAY (proven to work during training)
  // Note: Previous 100ms timeout could cause silent write failures!
  HAL_SPI_Transmit(hspi, (uint8_t*)&packet, 1, HAL_MAX_DELAY); // 1 * 16 bit gönder

  // Chip Select (CS) pinini pasif et (HIGH)
  // osDelay(1); // Çok kısa gecikmeler gerekiyorsa eklenebilir.
  HAL_GPIO_WritePin(RADAR_CS_PORT, RADAR_CS_PIN, GPIO_PIN_SET);
}

/**
 * @brief Belirtilen adresteki radar register'ından veri okur.
 * 
 * Bu fonksiyon, 16-bitlik bir SPI paketi göndererek (R/W=1) radar sensöründen veri talep eder.
 * SPI üzerinden eşzamanlı olarak gelen yanıtın düşük 8 bit'ini döndürür.
 * 
 * @param hspi Kullanılacak SPI handle'ı (örn. &hspi4).
 * @param regAddress Okunacak register adresi (0x00 - 0x3F). [birimsiz]
 * @return uint8_t Okunan 8-bit veri veya hata durumunda belirsiz değer. [8-bit değer]
 * 
 * @note Bu fonksiyon HAL_SPI_TransmitReceive (bloklamalı) kullandığı için kesme (ISR) 
 *       içinden çağrılması önerilmez.
 */
uint8_t Sensor_ReadRegister(SPI_HandleTypeDef* hspi, uint8_t regAddress)
{
  uint16_t packet = 0;
  uint16_t response = 0;

  // SPI paketini oluştur: 16 bit = | Başlangıç(1) | R/W(1) | Adres(6) | Dummy(8) |
  packet |= (1 << 15);                // Başlangıç biti (1)
  packet |= (1 << 14);                // R/W biti (1: Okuma)
  packet |= (regAddress & 0x3F) << 8; // Adres bitleri (6 bit), sola kaydır
  // Veri kısmı okuma için önemli değil (dummy)

  // Chip Select (CS) pinini aktif et (LOW)
  HAL_GPIO_WritePin(RADAR_CS_PORT, RADAR_CS_PIN, GPIO_PIN_RESET);
  // osDelay(1);

  // 16-bit komutu gönder ve 16-bit yanıt al
  // Dikkat: hspi handle'ının DataSize'ı SPI_DATASIZE_16BIT olarak ayarlanmış olmalı.
  HAL_SPI_TransmitReceive(hspi, (uint8_t*)&packet, (uint8_t*)&response, 1, HAL_MAX_DELAY); // 1 * 16 bit gönder/al

  // Chip Select (CS) pinini pasif et (HIGH)
  // osDelay(1);
  HAL_GPIO_WritePin(RADAR_CS_PORT, RADAR_CS_PIN, GPIO_PIN_SET);

  // Yanıtın sadece düşük 8 bit'i (veri kısmı) geri döndürülür
  return (uint8_t)(response & 0xFF);
}

/**
 * @brief Radar sensörünü donanımsal olarak resetler.
 * 
 * RADAR_RST pinini önce LOW'a çekerek sensörü reset durumuna sokar, ardından HIGH'a 
 * çekerek normal çalışmaya dönmesini sağlar. Reset döngüsü sırasında sensörün 
 * kendi iç sistemlerini hazırlaması için gecikmeler uygulanır.
 * 
 * @return None
 * 
 * @note Bu fonksiyon osDelay kullandığı için kesinlikle kesme (ISR) içinden ÇAĞRILAMAZ. 
 *       Sadece RTOS task'ları içinden çağrılmalıdır.
 */
void Sensor_Reset(void)
{
  // Reset pinini LOW yap
  HAL_GPIO_WritePin(RADAR_RST_PORT, RADAR_RST_PIN, GPIO_PIN_RESET);
  // HAL_Delay(10); // HAL_Delay yerine osDelay kullan
  osDelay(10);   // 10ms bekle

  // Reset pinini HIGH yap
  HAL_GPIO_WritePin(RADAR_RST_PORT, RADAR_RST_PIN, GPIO_PIN_SET);
  // HAL_Delay(100); // HAL_Delay yerine osDelay kullan
  osDelay(100);  // 100ms bekle (sensörün stabil olması için önemli olabilir)
}


/*
static const uint8_t kSensorRegDefaultValues[] = {
	0xFF, 0xFD, 0xEF, 0xC0, 0xC1, 0x87, 0x01, 0x0F,  // R00-R07: WORKING VALUES!
	0x00, 0x00, 0x01, 0x20, 0x20, 0xE7, 0x00, 0x01,  // R08-R15
	0x00, 0x01, 0x8A, 0x00, 0x03, 0x06, 0x02, 0x63,
	0x63, 0x00, 0x00, 0x00, 0x63, 0x00, 0x63, 0x03,
	0x00, 0x7A, 0x2C, 0xFA, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
	0x40, 0x10, 0x04, 0x04, 0x01, 0x01, 0x01, 0x00,
	0x00, 0xC0, 0x00
  };


  // ÖNCEKİ DEĞERLER
*/


static const uint8_t kSensorRegAddresses[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A
};

static const uint8_t kSensorRegDefaultValues[] = {
	0xFF, 0xFD, 0xEF, 0xC0, 0xC1, 0x87, 0x01, 0x0F,  // R00-R07: WORKING VALUES!
	0x00, 0x00, 0x01, 0x20, 0x20, 0xE7, 0x00, 0x01,  // R08-R15
	0x00, 0x01, 0x8A, 0x00, 0x03, 0x06, 0x02, 0x63,
	0x63, 0x00, 0x00, 0x00, 0x63, 0x00, 0x63, 0x03,
	0x00, 0x7A, 0x2C, 0xFA, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
	0x40, 0x10, 0x04, 0x04, 0x01, 0x01, 0x01, 0x00,
	0x00, 0xC0, 0x00
};


/*
    0xFF, 0xFD, 0xFF, 0xC0, 0xC1,
	0x07, //reg05 ---- gain_control_1
	0x01, //reg06 ---- gain_control_2
	0x0F,  //reg07
    0x80, 0x00, //reg08, reg09
	0x01, 0x20, 0x20, 0xE7, 0x00, 0x01,  // R08-R15
    0x00, 0x01, 0x8A, 0x00, 0x03, 0x06, 0x02, 0x63,
    0x63, 0x00, 0x00, 0x00, 0x63, 0x00, 0x63, 0x03,
    0x00,
	0x7A, //reg33 ---- start_bin (0 noktası 0x7A)
	0x1A, //reg34 ---- step_bin, start+step=target
    0xFF, //reg35 ---- integration
    0x01, //reg36
	0x00, //reg37 ---- resolution
    0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
    0x40, 0x10, 0x04, 0x04, 0x01, 0x01, 0x01, 0x00,
    0x00, 0xC0, 0x00
 */

#define SENSOR_NUM_REGISTERS \
    (sizeof(kSensorRegAddresses) / sizeof(kSensorRegAddresses[0]))

/* Derleme zamanında iki dizinin uzunluğu tutarlı mı diye kontrol (opsiyonel ama faydalı) */
_Static_assert(sizeof(kSensorRegAddresses) == sizeof(kSensorRegDefaultValues),
               "regAddresses ve regValues uzunluklari uyusmuyor!");

/* ---- Ortak yazma yardımcı fonksiyonu ---- */
static void Sensor_WriteRegisterArray(SPI_HandleTypeDef* hspi,
                                       const uint8_t* addresses,
                                       const uint8_t* values,
                                       uint8_t count)
{
    for (uint8_t i = 0; i < count; i++) {
        Sensor_WriteRegister(hspi, addresses[i], values[i]);
        osDelay(10);
    }
}

/**
 * @brief Radar sensörünün başlangıç register değerlerini toplu olarak yazar.
 */
void Sensor_WriteAllRegisters(SPI_HandleTypeDef* hspi)
{
    Sensor_WriteRegisterArray(hspi, kSensorRegAddresses,
                               kSensorRegDefaultValues,
                               SENSOR_NUM_REGISTERS);
}

/**
 * @brief Belirtilen özel register değerleriyle radar DC offset register'larını yazar.
 */
void Sensor_WriteDCOffset(SPI_HandleTypeDef* hspi, uint8_t valueForReg08, uint8_t valueForReg09)
{
    Sensor_WriteRegister(hspi, 0x08, valueForReg08);
    osDelay(5);
    Sensor_WriteRegister(hspi, 0x09, valueForReg09);
    osDelay(5);
    Sensor_WriteRegister(hspi, 0x02, 0xFF);
    osDelay(50);
}









/**
 * @brief ESP32 ile SPI bağlantısını kontrol eder (Handshake/Ping).
 * 
 * 5-byte'lık bir kontrol paketi gönderir ve ESP32'den gelen yanıtı döndürür.
 * Paket formatı: [SYNC(0xA5) | CMD(0x10) | Dummy | Dummy | END(0x5A)].
 * 
 * @return uint8_t ESP32'den dönen ilk byte (Genellikle 0x55 veya 0xA5). Hata durumunda 0xFF döner. [status code]
 * 
 * @note Bu fonksiyon bloklamalı HAL_SPI_TransmitReceive kullandığı için ISR içinden çağrılmamalıdır.
 */
uint8_t SPI_Check_Connection(void)
{
  uint8_t txPacket[5] = {0xA5, 0x10, 0x00, 0x00, 0x5A};
  uint8_t rxPacket[5] = {0};

  // Chip Select (CS) - PA8 (SPI1_CS_Pin)
  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
  
  // Transmit and Receive simultaneously (5 bytes)
  HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi1, txPacket, rxPacket, 5, 100);

  // Chip Select High
  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);

  if (status != HAL_OK) {
      return 0xFF; // SPI Error
  }
  
  // Return the byte at index 2 (Payload from ACK?) 
  // ESP32 Logic: If it receives PING, it sends previous ACK. 
  // For simpliciy, return the first valid non-zero byte or specific index.
  // Assuming ESP32 shifts out something. Let's return rxPacket[2] or rxPacket[0] if meaningful.
  // Actually, ESP32 sends ACK_PING (0x55) as first byte in previous logic. 
  // With 5 bytes, let's see what happens. Just return rxPacket[0] for now to match legacy check.
  return rxPacket[0];
}

/**
 * @brief AI çıkarım sonucunu SPI üzerinden ESP32'ye gönderir.
 * 
 * Sınıf ID'si ve güven değerini içeren 5-byte'lık paketi ESP32'ye iletir.
 * Paket formatı: [SYNC(0xA5) | CMD(0x21) | ClassID | Confidence | END(0x5A)].
 * 
 * @param class_id Tespit edilen nesne/hareket sınıfı (0-N). [id]
 * @param confidence Modelin tahmin güveni (0.0-100.0 arası beklenir). [% float]
 * @return None
 * 
 * @note Bu fonksiyon bloklamalı SPI transferi yaptığı için yüksek öncelikli döngülerde gecikme 
 *       yaratabilir. ISR içinden çağrılması önerilmez.
 */
void SPI_Send_Inference_Result(uint8_t class_id, float confidence)
{
  // Packet: [SYNC(0xA5), CMD(0x21), CLASS, CONF, END(0x5A)]
  uint8_t conf_val = (uint8_t)(confidence); // 0-100 expected
  uint8_t txPacket[5] = {0xA5, 0x21, class_id, conf_val, 0x5A};
  uint8_t rxPacket[5] = {0}; // Dummy rx

  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_TransmitReceive(&hspi1, txPacket, rxPacket, 5, 100);
  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
}

