
#include <radarHandler.hpp>


#define MAX_NUM_SAMPLES 2048

int16_t adcData[MAX_NUM_SAMPLES];
volatile uint16_t sampleIndex = 0;
volatile uint8_t ssFlag = 0;
volatile uint8_t intgClkFlag = 0;
int16_t frameAverage = 0;
volatile int8_t transmitFinished = 1;

//extern osSemaphoreId_t spiSemaphoreHandle;
extern ADC_HandleTypeDef hadc3;
//extern osThreadId_t spiTransmitReceiveTaskHandle;

int16_t calculate_frame_average(void);
void collectSample(void);

/* Definitions for streamTask */
osThreadId_t streamTaskHandle;
const osThreadAttr_t streamTask_attributes = {
  .name = "streamTask",
  .stack_size = 1700 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};

/**
 * @brief Tek bir radar frame'inin ADC ortalamasını hesaplar (DC Offset tespiti için).
 * 
 * SS ve INTG_CLK sinyallerini bekleyerek ham ADC verilerini toplar ve frame sonrasında 
 * ortalama değeri döndürür.
 * 
 * @return int16_t Frame ortalaması (ADC biriminde, -32768 ile 32767 arası). [ADC değeri]
 * 
 * @note Polling (beklemeli) çalıştığı için ISR içinden ÇAĞRILAMAZ.
 */
int16_t radarHandler::calculate_frame_average()
{
	while (!ssFlag) {
	// wait for ssFalg to be set
	}

	while (ssFlag) {
	// wait for ssFalg to be reset
	}

	intgClkFlag = 0;
	sampleIndex = 0;

	while (1)
	{
		if (intgClkFlag) {
			if (ssFlag == 0) {
				intgClkFlag = 0;

				HAL_ADC_Start(&hadc3);
				if (HAL_ADC_PollForConversion(&hadc3, 1) == HAL_OK)
				{
					int16_t sample = HAL_ADC_GetValue(&hadc3) - 32768;
					adcData[sampleIndex] = sample;
					sampleIndex++;
				}
				else{
					adcData[sampleIndex] = 0;
					sampleIndex++;
				}
				HAL_ADC_Stop(&hadc3);
			}

			else if (ssFlag == 1) {

				int32_t sum = 0;

				for (int i = 0; i < sampleIndex; i++) {
					sum += adcData[i];
				}

				frameAverage = sum / sampleIndex;
				int16_t average = 0;
				sum = 0;
				sampleIndex = 0;


				if(frameAverage>32)
				{
					average = 256;
				}
				else if (frameAverage<-32)
				{
					average = 512;
				}
				else
				{
					average = 0;
				}
				frameAverage = 0;

				// Use average to suppress unused variable warning
				(void)average;  // Suppress unused variable warning

				break;
			}
		}
	}

	return 0;  // Add missing return statement
}

/**
 * @brief Radar verilerini sürekli akıtan (streaming) görevi başlatır.
 * 
 * @param frameCount Kaç frame veri toplanacağı. [adet]
 * @return int Başarılıysa 0, hata durumunda 1 döner.
 */
int radarHandler::startStreamingTask(int frameCount)
{
	/* creation of streamTask */
	if (streamTaskHandle == NULL) {
		static int persistentFrameCount = frameCount;
		streamTaskHandle = osThreadNew(radarHandler::streamTaskCallback, &persistentFrameCount, &streamTask_attributes);

		if (streamTaskHandle == NULL) {
				printf("Error: Failed to create stream task.\n");
				return 1;
		}
		else{
			return 0;
		}
	}
	return 1; // Task zaten mevcut
}

void radarHandler::stopStreamingTask()
{

	if (streamTaskHandle != NULL) {

		osStatus_t status = osThreadTerminate(streamTaskHandle);

		if (status == osOK) {
			printf("Streaming task stopped successfully.\n");
		} else {
			printf("Error: Failed to terminate streaming task.\n");
		}
		streamTaskHandle = NULL;

	} else {
        printf("Error: Streaming task handle is NULL.\n");
    }

}



/**
 * @brief Veri akışı (Streaming) yapan FreeRTOS görev callback fonksiyonu.
 * 
 * ADC üzerinden verileri toplar, çift tamponlama (double buffering) kullanarak 
 * verileri DMA üzerinden SPI (ESP32) hattına basar.
 * 
 * @param argument Görev parametreleri (Frame sayısı).
 * @return None
 * 
 * @note Yüksek öncelikli bir görevdir; zamanlama hassastır.
 */
void radarHandler::streamTaskCallback(void *argument)
{
    int frameCount = *(int *)argument;
    int currentFrame = 0;
    uint16_t sampleCount = 0;

    int16_t spiTxBuffer1[700] = {0};
    int16_t spiTxBuffer2[700] = {0};

    int16_t sample = 0;
    int16_t *currentBuffer = spiTxBuffer1;
    int16_t *nextBuffer = spiTxBuffer2;
    // int8_t i=0; // Kullanılmayan değişken kaldırıldı

    //osThreadSuspend(spiTransmitReceiveTaskHandle);

    while (currentFrame < frameCount)
    {
        while (!ssFlag) {
        }

        while (ssFlag) {
        }

        intgClkFlag = 0;

        //prepare_label("<DTA>");


        while (1)
        {
            if (intgClkFlag)
            {
                intgClkFlag = 0;

                if (ssFlag == 0)
                {
                    HAL_ADC_Start(&hadc3);

                    if (HAL_ADC_PollForConversion(&hadc3, (TickType_t)1) == HAL_OK) {
                        sample = HAL_ADC_GetValue(&hadc3) - 32768;  // Optional offset adjustment
                    }
                    else {
                        sample = 0;
                    }

                    HAL_ADC_Stop(&hadc3);

                    currentBuffer[sampleCount] = sample;
                    sampleCount++;
                }

                if (ssFlag == 1)
                {
                    //osSemaphoreAcquire(spiSemaphoreHandle, portMAX_DELAY);

                    HAL_GPIO_WritePin(GPIOA, SPI1_CS_Pin, GPIO_PIN_RESET);

                    HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)currentBuffer, sampleCount);


                    sampleCount = 0;
                    currentFrame++;
                    std::swap(currentBuffer, nextBuffer);

                    break;
                }
            }
        }

        if (currentFrame >= frameCount)
        {
            //prepare_label("<STP>");
            // StopStreamingEvent event;  // COMMENTED: Event.hpp not included
            //g_stateMachine.processEvent(&event); // StateMachine kullan

            break;
        }
    }

    //osThreadResume(spiTransmitReceiveTaskHandle);
}








 void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
 {
		HAL_GPIO_WritePin(GPIOA, SPI1_CS_Pin, GPIO_PIN_SET);
 }


void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{

	HAL_GPIO_WritePin(GPIOA, SPI1_CS_Pin, GPIO_PIN_SET);
	//osSemaphoreRelease(spiSemaphoreHandle);
}


  void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
  {
      uint32_t errorCode = HAL_SPI_GetError(hspi);

      printf("SPI Error Callback: Error Code = %lu\n", errorCode);

      if (errorCode != HAL_SPI_ERROR_NONE)
      {
          HAL_SPI_DeInit(hspi);
          HAL_SPI_Init(hspi);
      }
  }

