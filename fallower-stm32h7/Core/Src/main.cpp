/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/

#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <stdio.h>
#include <queue.h>
#include <cstring>
#include <memory>
#include <vector>  // For std::vector in AutoStartupTask

// Linker symbols for SDRAM test data copy
extern "C" {
    extern uint32_t _ssdram_test_data;
    extern uint32_t _esdram_test_data;
    extern uint32_t _lsdram_test_data;
}

// Update the state includes to be in the correct order
#include "state/StateFactory.hpp"  // Add this include to access StateFactory and StateType
#include "state/State.hpp"
#include "state/StateMachine.hpp"  // This should come after State.hpp
#include "state/Ready.hpp"

#include "main.h"
#include "sdramInitialization.hpp"

#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "UsbCommunication.hpp"
#include "nand_block_manager_enhanced.h"
#include "nand_model_storage.h"
#include "AutoStartConfig.h"  // Auto-startup configuration
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

typedef enum {PASSED = 0, FAILED = !PASSED} TestStatus_t;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi4;
ADC_HandleTypeDef hadc3;
DMA_HandleTypeDef hdma_spi1_tx;
SDRAM_HandleTypeDef hsdram1;
TIM_HandleTypeDef htim16;

NAND_HandleTypeDef hnand1;
nand_block_manager_t g_nand_manager;

// Watchdog Handle - Independent Watchdog for system recovery
// NOTE: Requires HAL_IWDG_MODULE_ENABLED in stm32h7xx_hal_conf.h
#ifdef HAL_IWDG_MODULE_ENABLED
IWDG_HandleTypeDef hiwdg1;
#endif

// FMC Mutex - Critical for protecting shared FMC hardware between SDRAM and NAND
SemaphoreHandle_t fmc_mutex;

/* Status variables */
__IO uint32_t uwWriteReadStatus = 0;

/* Counter index */
uint32_t uwIndex = 0;

/* USER CODE BEGIN PV */
uint8_t count = 0;

// Global flag for power-on NAND auto-load status
// This is set during startup and reported via USB when available
typedef enum {
    AUTOLOAD_NOT_ATTEMPTED = 0,
    AUTOLOAD_PENDING = 1,        // Waiting for AutoLoadTask to process
    AUTOLOAD_SUCCESS = 2,
    AUTOLOAD_FAILED_NO_MODEL = 3,
    AUTOLOAD_FAILED_FMC_ERROR = 4,
    AUTOLOAD_FAILED_READ_ERROR = 5
} AutoLoadStatus_t;

static AutoLoadStatus_t g_autoload_status = AUTOLOAD_NOT_ATTEMPTED;

/* USER CODE END PV */



// During initialization

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_FMC_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI4_Init(void);
static void MX_ADC3_Init(void);
void UsbHandleTask(void *argument);
void UsbStateInitTask(void* argument);
void ConnectionCheckTaskWrapper(void* argument);
void UsbInitTask(void* argument);  // Add this forward declaration
bool isValidData(uint16_t *spiRxBuffer);
HAL_StatusTypeDef Simple_NAND_Test(void);
void StartNANDTask(void *argument);

static void MX_IWDG1_Init(void);
void WatchdogTask(void* argument);

/* USER CODE BEGIN PFP */

// Queue for NAND commands
QueueHandle_t nandQueue;

osThreadId_t nandTaskHandle;
const osThreadAttr_t nandTask_attributes = {
    .name = "NANDTask",
    .stack_size = 4096 * 4, // 16KB stack for NAND operations
    .priority = (osPriority_t) osPriorityHigh,
};


osThreadId_t usbInitTaskHandle;
const osThreadAttr_t usbInitTask_attributes = {
    .name = "UsbInitTask",
    .stack_size = 512 * 4,
    .priority = (osPriority_t) osPriorityHigh,
};


osThreadId_t connectionCheckTaskHandle;
const osThreadAttr_t connectionCheckTask_attributes = {
    .name = "ConnCheckTask",
    .stack_size = 512 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

// ============================================================================
// AUTO-START TASK STATIC MEMORY (RAM_D2 Section)
// Static allocation prevents heap fragmentation and guarantees memory placement
// ============================================================================

// WatchdogTask: IWDG refresh (2KB stack)
__attribute__((section(".ram_d2"))) 
static StackType_t watchdogTaskStack[WATCHDOG_TASK_STACK_SIZE];
static StaticTask_t watchdogTaskTCB;
osThreadId_t watchdogTaskHandle;

// AutoStartupTask removed - using manual USB command sequence instead

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
HAL_StatusTypeDef Simple_NAND_Test() {
    NAND_IDTypeDef nand_id;
    if (HAL_NAND_Read_ID(&hnand1, &nand_id) != HAL_OK) {
        return HAL_ERROR;
    }
    
    // NOTE: These IDs can be changed depending on the exact NAND chip model
    // Maker: 0x01=Spansion/Cypress, Device: 0xF2=S34ML02G2, 0xDA=S34ML01G2
    if (nand_id.Maker_Id != 0x01 || (nand_id.Device_Id != 0xF2 && nand_id.Device_Id != 0xDA)) {
        return HAL_ERROR;
    }
    return HAL_OK;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
/**
 * @brief Uygulamanın giriş noktası (Entry Point).
 * 
 * Donanımı (Clock, MPU, Cache, FMC) ilkinize eder, RTOS nesnelerini ve görevlerini 
 * (Tasks) oluşturur ve scheduler'ı başlatır.
 * 
 * @return int Normal şartlarda dönmez.
 */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* Enable CPU Cache for Performance ----------------------------------------*/
  /* ✅ CACHE ENABLED with Write-Back No Allocate mode                       */
  /* Cache strategy:                                                          */
  /*   - I-Cache: Speeds up code execution (instruction fetch)               */
  /*   - D-Cache: Speeds up SDRAM data access (10-15x for AI inference)      */
  /*   - Write-Back No Allocate: Cache buffering with coherency control      */
  /*   - MPU configured: TEX=0, C=1, B=1 (Write-Back No Write Allocate)      */
  /*   - Cache operations: Clean/Invalidate at task boundaries               */

  // Enable Instruction Cache (I-Cache) - Safe in all modes
  SCB_EnableICache();

  // Enable Data Cache (D-Cache)
  SCB_EnableDCache();

  /* USER CODE END SysInit */

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_FMC_Init();
  
  /* USER CODE BEGIN 2 */
  // SDRAM Initialization Sequence
  FMC_SDRAM_CommandTypeDef Command;
  SDRAM_Initialization_Sequence(&hsdram1, &Command);
  
  // NAND Flash Initialization
  if (NAND_BlockManager_Init(&g_nand_manager, &hnand1) != HAL_OK) {
      Error_Handler();
  }

  if (Simple_NAND_Test() != HAL_OK) {
      Error_Handler();
  }

  // POWER-ON AUTO-LOAD: DISABLED - Causes MemManage_Handler during startup
  // Root Cause: FMC switching or SDRAM access before scheduler starts
  // TODO: Debug MemManage_Handler, then re-enable auto-load
  // Workaround: Use 'read_model_nand' command after boot to manually load from NAND

  // TEMPORARILY DISABLED - UNCOMMENT AFTER FIXING MemManage_Handler
  /*
  // Step 1: Switch FMC from SDRAM mode to NAND mode
  if (FMC_ReInitNAND() == HAL_OK) {

      // Step 2: Try to read model from NAND to SDRAM
      HAL_StatusTypeDef read_result = ReadModelFromNAND_ToSDRAM();

      // Step 3: Switch FMC back to SDRAM mode (CRITICAL - always restore!)
      // This must happen regardless of read success/failure
      if (FMC_RestoreSDRAM() != HAL_OK) {
          // CRITICAL ERROR: Cannot restore SDRAM - system unusable
          g_autoload_status = AUTOLOAD_FAILED_FMC_ERROR;
          Error_Handler();
      }

      // Step 4: Set auto-load status based on result
      if (read_result == HAL_OK) {
          // Model successfully loaded from NAND Flash
          // AI inference will be ready after startup
          g_autoload_status = AUTOLOAD_SUCCESS;
      } else {
          // No model in NAND or read failed
          // User needs to load model manually via load_model_binary
          g_autoload_status = AUTOLOAD_FAILED_NO_MODEL;
      }

  } else {
      // FMC_ReInitNAND failed - NAND hardware issue
      // System continues without auto-load (not critical)
      g_autoload_status = AUTOLOAD_FAILED_FMC_ERROR;
  }
  */

  // Manual command sequence is used - no auto-load at startup
  g_autoload_status = AUTOLOAD_NOT_ATTEMPTED;


  // TEMPORARILY DISABLED: Copy test data from FLASH to SDRAM (system stability)
  // uint32_t* src = &_lsdram_test_data;
  // uint32_t* dst = &_ssdram_test_data;
  // uint32_t size = (uint32_t)&_esdram_test_data - (uint32_t)&_ssdram_test_data;

  // // Word-aligned copy for better performance
  // for (uint32_t i = 0; i < size / 4; i++) {
  //     dst[i] = src[i];
  // }
  /* USER CODE END 2 */
  
  MX_SPI1_Init();
  MX_ADC3_Init();
  MX_SPI4_Init();
  
  // Initialize Independent Watchdog (before scheduler starts!)
  MX_IWDG1_Init();




  //osThreadId_t connectionCheckTaskHandle = osThreadNew(ConnectionCheckTaskWrapper, NULL, &connectionCheckTask_attributes);


  /* Init scheduler */
  osKernelInitialize();





  (void)osThreadNew(UsbInitTask, NULL, &usbInitTask_attributes);

  // ❌ CRITICAL FIX: REMOVED vTaskDelay() before osKernelStart()
  // Reason: vTaskDelay() requires scheduler to be running (osKernelStart())
  // Calling vTaskDelay() before scheduler starts causes system freeze
  // This prevented osKernelStart() from being reached, blocking all tasks
  // including StartNANDTask (load_model_flash would freeze here)
  // Solution: Delay moved to UsbInitTask (Line 615) where scheduler is active
  // Reference: nand.md - FreeRTOS task lifecycle analysis
  // vTaskDelay(pdMS_TO_TICKS(100)); // ← REMOVED - CAUSES SYSTEM FREEZE!

  StateMachine::getInstance()->changeState(STATE_IDLE);

  (void)osThreadNew(ConnectionCheckTaskWrapper, NULL, &connectionCheckTask_attributes);


  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  // CRITICAL: Create FMC mutex to protect shared FMC hardware (SDRAM + NAND Flash)
  // This prevents race conditions when high-priority NAND task interrupts SDRAM operations
  fmc_mutex = xSemaphoreCreateMutex();
  if (fmc_mutex == NULL) {
      // Critical failure - cannot proceed without FMC protection
      Error_Handler();
  }
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of spiTransmitReceiveQueue */


  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  nandQueue = xQueueCreate(5, sizeof(NAND_Operation_t));

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  //UsbTaskHandle = osThreadNew(UsbHandleTask, NULL, &UsbTask_attributes);
  nandTaskHandle = osThreadNew(StartNANDTask, NULL, &nandTask_attributes);


  /* creation of spiTransmitReceiveTask */
  //spiTransmitReceiveTaskHandle = osThreadNew(spiTransmitReceiveCallback, NULL, &spiTransmitReceiveTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  
  // ============================================================================
  // AUTO-START TASKS (Static allocation in RAM_D2)
  // ============================================================================

  // Manual command sequence: init → ready → registers → calibrate →
  //   load_model_binary → load_model_flash → read_model_nand → ai_init → run_inference_isr_real

  // WatchdogTask - IWDG refresh (runs forever, highest priority)
  watchdogTaskHandle = (osThreadId_t)xTaskCreateStatic(
      WatchdogTask,
      "WatchdogTask",
      WATCHDOG_TASK_STACK_SIZE,
      NULL,
      osPriorityRealtime,
      watchdogTaskStack,
      &watchdogTaskTCB
  );
  
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();



  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
/**
 * @brief Sistem saat yapılandırması (400MHz @ VOS0).
 * 
 * İşlemciyi maksimum performans moduna (400MHz) ayarlar ve ilgili otobüs (bus) 
 * bölücülerini yapılandırır.
 * 
 * @return None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;


  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;  // 8-16MHz range
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;    // AHB = 200MHz
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;   // APB3 = 100MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;   // APB1 = 100MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;   // APB2 = 100MHz
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;   // APB4 = 100MHz

  // CRITICAL: Flash latency must match CPU frequency
  // 400MHz @ VOS0 requires FLASH_LATENCY_4 (per datasheet Table 16)
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FMC|RCC_PERIPHCLK_SPI1
                              |RCC_PERIPHCLK_CKPER;

  // PLL2 Configuration optimized for 80MHz SDRAM (STABILIZATION)
  //   VCO_IN  = HSE / PLL2M = 32MHz / 2 = 16MHz
  //   VCO_OUT = VCO_IN × PLL2N = 16MHz × 20 = 320MHz
  //   FMC_CLK = VCO_OUT / PLL2R = 320MHz / 2 = 160MHz
  //   SPI_CLK = VCO_OUT / PLL2P = 320MHz / 2 = 160MHz
  PeriphClkInitStruct.PLL2.PLL2M = 2;
  PeriphClkInitStruct.PLL2.PLL2N = 20;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.FmcClockSelection = RCC_FMCCLKSOURCE_PLL2;
  PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSI;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */

  /** Common config
  */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV8;
  hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc3.Init.LowPowerAutoWait = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc3.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  hadc3.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc3.Init.OversamplingMode = DISABLE;
  hadc3.Init.Oversampling.Ratio = 1;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.SingleDiff = ADC_DIFFERENTIAL_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT; // CRITICAL FIX: ESP32 uses 8-bit SPI
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64; // Adjusted for 200MHz PLL2P (~3.1MHz)
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 0x0;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi1.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI4_Init(void)
{

  /* USER CODE BEGIN SPI4_Init 0 */

  /* USER CODE END SPI4_Init 0 */

  /* USER CODE BEGIN SPI4_Init 1 */

  /* USER CODE END SPI4_Init 1 */
  /* SPI4 parameter configuration*/
  hspi4.Instance = SPI4;
  hspi4.Init.Mode = SPI_MODE_MASTER;
  hspi4.Init.Direction = SPI_DIRECTION_2LINES;
  hspi4.Init.DataSize = SPI_DATASIZE_16BIT;
  hspi4.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi4.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi4.Init.NSS = SPI_NSS_SOFT;
  hspi4.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi4.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi4.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi4.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi4.Init.CRCPolynomial = 0x0;
  hspi4.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi4.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi4.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi4.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi4.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi4.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi4.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi4.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi4.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi4.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI4_Init 2 */

  /* USER CODE END SPI4_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_NAND_PCC_TimingTypeDef ComSpaceTiming = {0};
  FMC_NAND_PCC_TimingTypeDef AttSpaceTiming = {0};
  FMC_SDRAM_TimingTypeDef SdramTiming = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the NAND1 memory initialization sequence
  */
  hnand1.Instance = FMC_NAND_DEVICE;
  /* hnand1.Init */
  hnand1.Init.NandBank = FMC_NAND_BANK3;
  hnand1.Init.Waitfeature = FMC_NAND_WAIT_FEATURE_DISABLE;
  hnand1.Init.MemoryDataWidth = FMC_NAND_MEM_BUS_WIDTH_8;
  hnand1.Init.EccComputation = FMC_NAND_ECC_ENABLE;
  hnand1.Init.ECCPageSize = FMC_NAND_ECC_PAGE_SIZE_512BYTE;
  // CRITICAL FIX: Using conservative timing from gemini reference project
  // Original values (11,15,8,23) were too aggressive and causing data corruption
  // Gemini values (20,30,15,40) are 1.5-1.7x slower but ensure NAND reliability
  hnand1.Init.TCLRSetupTime = 3;    // Was 1, increased to 3 (CLE-to-RE delay)
  hnand1.Init.TARSetupTime = 3;     // Was 1, increased to 3 (ALE-to-RE delay)
  /* hnand1.Config */
  hnand1.Config.PageSize = 2048;
  hnand1.Config.SpareAreaSize = 64;
  hnand1.Config.BlockSize = 64;
  hnand1.Config.BlockNbr = 2048;
  hnand1.Config.PlaneNbr = 2;
  hnand1.Config.PlaneSize = 1024;
  hnand1.Config.ExtraCommandEnable = ENABLE;
  /* ComSpaceTiming - Conservative values for S34ML02G2 reliability */
  ComSpaceTiming.SetupTime = 20;        // Was 11, increased by 1.8x for safety
  ComSpaceTiming.WaitSetupTime = 30;    // Was 15, increased by 2x for erase operations
  ComSpaceTiming.HoldSetupTime = 15;    // Was 8, increased by 1.9x for data hold
  ComSpaceTiming.HiZSetupTime = 40;     // Was 23, increased by 1.7x for bus release
  /* AttSpaceTiming - Match ComSpaceTiming for consistency */
  AttSpaceTiming.SetupTime = 20;        // Was 11, conservative for attribute space
  AttSpaceTiming.WaitSetupTime = 30;    // Was 15, ensures ready/busy signal stability
  AttSpaceTiming.HoldSetupTime = 15;    // Was 8, prevents attribute space glitches
  AttSpaceTiming.HiZSetupTime = 40;     // Was 23, safe bus tristate timing

  if (HAL_NAND_Init(&hnand1, &ComSpaceTiming, &AttSpaceTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /** Perform the SDRAM1 memory initialization sequence
  */
  hsdram1.Instance = FMC_SDRAM_DEVICE;
  /* hsdram1.Init */
  hsdram1.Init.SDBank = FMC_SDRAM_BANK1;
  hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_9;
  hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_13;
  hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
  hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;  // CL3 for 80MHz stability
  hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
  hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
  hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0; // SAFE MODE: 0 Delay
  /* SdramTiming - AS4C32M16SB-7TCN ABSOLUTELY CONSERVATIVE for boot debug */
  SdramTiming.LoadToActiveDelay = 2;        // tMRD: 14ns min
  SdramTiming.ExitSelfRefreshDelay = 7;     // tXSR: 70ns min
  SdramTiming.SelfRefreshTime = 5;          // tRAS: 42ns min
  SdramTiming.RowCycleDelay = 7;            // tRC: 60ns min
  SdramTiming.WriteRecoveryTime = 3;        // tWR: 14ns min
  SdramTiming.RPDelay = 3;                  // tRP: 21ns min
  SdramTiming.RCDDelay = 3;                 // tRCD: 21ns min

  if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */

  /* USER CODE END FMC_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(FMC_nWP_GPIO_Port, FMC_nWP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  // CRITICAL FIX: SPI CS should be High (Deselected) by default
  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, RADAR_SPI_nCS_Pin|RADAR_LED_Pin|RADAR_SPI_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : RADAR_SS_Pin */
  GPIO_InitStruct.Pin = RADAR_SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(RADAR_SS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RADAR_INTG_CLK_Pin */
  GPIO_InitStruct.Pin = RADAR_INTG_CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(RADAR_INTG_CLK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : FMC_nWP_Pin */
  GPIO_InitStruct.Pin = FMC_nWP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(FMC_nWP_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI1_CS_Pin */
  GPIO_InitStruct.Pin = SPI1_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI1_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RADAR_SPI_nCS_Pin RADAR_LED_Pin RADAR_SPI_RST_Pin */
  GPIO_InitStruct.Pin = RADAR_SPI_nCS_Pin|RADAR_LED_Pin|RADAR_SPI_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  /* EXTI interrupt init*/
  // CRITICAL FIX F8: Priority 5 matches configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
  // This allows safe usage of FreeRTOS API functions (xTaskNotifyFromISR)
  HAL_NVIC_SetPriority(RADAR_SS_EXTI_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(RADAR_SS_EXTI_IRQn);

  HAL_NVIC_SetPriority(RADAR_INTG_CLK_EXTI_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(RADAR_INTG_CLK_EXTI_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void StartNANDTask(void *argument) {
  NAND_Operation_t operation;
  UsbCommunication* usb = UsbCommunication::getInstance(); // Get USB instance once

  // DEBUG: Task başladığını bildir
  if (usb) {
    usb->sendStatusMessage("NAND_TASK", "StartNANDTask STARTED - waiting for commands");
  }

  for(;;) {
    // Wait for a command on the queue (blocking call - no spam log needed)
    if (xQueueReceive(nandQueue, &operation, portMAX_DELAY) == pdPASS) {
      // DEBUG: Queue item alındı
      if (usb) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Received operation: %d", (int)operation);
        usb->sendStatusMessage("NAND_DEBUG", msg);
      }

      // CRITICAL: Acquire FMC mutex before ANY FMC operations (NAND or SDRAM)
      // Check high priority mode FIRST as a fail-safe
      extern volatile bool g_fmc_high_priority_mode;
      if (g_fmc_high_priority_mode) {
          if (usb) {
              usb->sendStatusMessage("NAND_ABORT", "Radar inference active! NAND operation blocked.");
          }
          continue; 
      }

      if (xSemaphoreTake(fmc_mutex, pdMS_TO_TICKS(30000)) == pdTRUE) {

        HAL_StatusTypeDef result = HAL_ERROR;
        if (operation == NAND_OP_WRITE_MODEL) {
            usb->sendStatusMessage("NAND_TASK", "=== NAND WRITE STARTED ===");
            usb->sendStatusMessage("NAND_STRATEGY", "Using BLOCK-LEVEL FMC switching strategy");

            // Model is assumed to be at 0xC0200000, size is hardcoded for now
            uint8_t* model_data = (uint8_t*)0xC0200000;
            uint32_t model_size = 4280700;

            // CRITICAL FIX: WriteModelToNAND now handles FMC switching internally
            // Strategy: Read block from SDRAM → Switch to NAND → Write → Switch to SDRAM → Repeat
            // This ensures SDRAM is NEVER accessed while FMC is in NAND mode
            result = WriteModelToNAND(model_data, model_size, "fallower_model");

            usb->sendStatusMessage("NAND_TASK", "=== NAND WRITE COMPLETED ===");

            // VERIFICATION: Clear SDRAM model region after NAND write
            // This proves NAND write was successful (not just reading from SDRAM cache)
            // User can verify with 'read_model_nand' command to reload from NAND
            if (result == HAL_OK) {
                usb->sendStatusMessage("NAND_VERIFY", "Clearing SDRAM model region for verification...");

                // Clear 4.28MB SDRAM region @ 0xC0200000
                memset((void*)0xC0200000, 0x00, model_size);

                // Memory barrier to ensure write completes
                __DSB();
                __ISB();

                usb->sendStatusMessage("NAND_VERIFY", "✅ SDRAM cleared - model now ONLY in NAND Flash");
                usb->sendStatusMessage("NAND_VERIFY", "To verify NAND write: use 'read_model_nand' command");
                usb->sendStatusMessage("NAND_VERIFY", "Memory Browser @ 0xC0200000 should show all zeros");
            }

        } else if (operation == NAND_OP_READ_MODEL) {
            usb->sendStatusMessage("NAND_TASK", "=== NAND READ STARTED ===");
            usb->sendStatusMessage("NAND_STRATEGY", "Using optimized FMC switching (per-block)");

            // CRITICAL FIX: ReadModelFromNAND_ToSDRAM now handles ALL FMC switching internally
            // Strategy: Read block from NAND → Switch to SDRAM → Write to SDRAM → Switch back to NAND → Repeat
            // No need to pre-initialize FMC here - function handles everything
            result = ReadModelFromNAND_ToSDRAM();

            // VERIFICATION: Report success and suggest verification
            if (result == HAL_OK) {
                usb->sendStatusMessage("NAND_VERIFY", "✅ Model loaded from NAND to SDRAM @ 0xC0200000");
                usb->sendStatusMessage("NAND_VERIFY", "Verification: Check Memory Browser @ 0xC0200000");
                usb->sendStatusMessage("NAND_VERIFY", "Should see model weights (non-zero FP32 data)");
                usb->sendStatusMessage("NAND_VERIFY", "AI test: use 'ai_inference' then 'run' commands");
            }
        }

        // CRITICAL: Ensure SDRAM mode is restored after ALL operations
        // ReadModelFromNAND_ToSDRAM ends in SDRAM mode, but we verify to be safe
        // WriteModelToNAND ends in SDRAM mode, but we verify to be safe
        usb->sendStatusMessage("NAND_TASK", "Verifying SDRAM configuration...");
        if (FMC_RestoreSDRAM() != HAL_OK) {
            usb->sendStatusMessage("NAND_ERROR", "Failed to restore SDRAM config!");
            result = HAL_ERROR;  // Override result if restore fails
        } else {
            usb->sendStatusMessage("NAND_TASK", "SDRAM configuration confirmed");
        }

        // CRITICAL: Release FMC mutex after operations complete
        xSemaphoreGive(fmc_mutex);

        if (result == HAL_OK) {
            usb->sendStatusMessage("NAND_TASK", "Operation completed successfully.");
        } else {
            usb->sendStatusMessage("NAND_TASK", "Operation failed.");
        }

      } else {
        // Mutex timeout - critical error (deadlock or very slow operation)
        usb->sendStatusMessage("NAND_CRITICAL", "Could not acquire FMC mutex within 30s! Possible deadlock.");
      }
    }
  }
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_UsbHandleTask */
/**
  * @brief  Function implementing the UsbTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_UsbHandleTask */
void UsbHandleTask(void *argument)
{
  /* USB handled in Init state now */
  /* Just keep this task idle */
  for(;;)
  {
    vTaskDelay(pdMS_TO_TICKS(60000)); // 1 dakika bekle
  }
}
 /* MPU Configuration */


void ConnectionCheckTaskWrapper(void* argument) {
    while (1) {
        UsbCommunication* usbComm = UsbCommunication::getInstance();
        if (usbComm && usbComm->isReady()) {

        	extern USBD_HandleTypeDef hUsbDeviceFS;
        	bool currentConnStatus = (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED);


            HAL_GPIO_WritePin(GPIOB, RADAR_LED_Pin,
                currentConnStatus ? GPIO_PIN_SET : GPIO_PIN_RESET);


            static bool prevConnStatus = false;
            if (currentConnStatus != prevConnStatus) {
                prevConnStatus = currentConnStatus;

                if (currentConnStatus) {

                    usbComm->sendStatusMessage("System", "USB connected, waiting for commands");
                }

            }
        }

        // 500ms bekle
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// Define UsbInitTask function before it's used
void UsbInitTask(void* argument) {

	HAL_GPIO_WritePin(GPIOB, RADAR_LED_Pin, GPIO_PIN_SET);
    vTaskDelay(pdMS_TO_TICKS(100));
    HAL_GPIO_WritePin(GPIOB, RADAR_LED_Pin, GPIO_PIN_RESET);
    vTaskDelay(pdMS_TO_TICKS(100));

    MX_USB_DEVICE_Init();

    // LED blink twice
    for(int i=0; i<2; i++) {
        HAL_GPIO_WritePin(GPIOB, RADAR_LED_Pin, GPIO_PIN_SET);
        vTaskDelay(pdMS_TO_TICKS(100));
        HAL_GPIO_WritePin(GPIOB, RADAR_LED_Pin, GPIO_PIN_RESET);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Initialize USB communication
    UsbCommunication* usb = UsbCommunication::getInstance();
    if (usb) {
        // Initialize USB communication
        usb->initialize();

        // Check if initialization was successful using isReady() instead of initializeQueues()
        if (!usb->isReady()) {
            // Initialization error - flash LED to indicate
            HAL_GPIO_WritePin(GPIOB, RADAR_LED_Pin, GPIO_PIN_SET);
            vTaskDelay(pdMS_TO_TICKS(100));
            HAL_GPIO_WritePin(GPIOB, RADAR_LED_Pin, GPIO_PIN_RESET);
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        // Send initialization complete message
        // NOTE: These messages will be queued even if PC not connected
        // They'll be sent when USB connection is established
        usb->sendStatusMessage("SYSTEM", "USB initialized. System is in IDLE state.");

        // Report power-on NAND auto-load status
        switch (g_autoload_status) {
            case AUTOLOAD_SUCCESS:
                usb->sendStatusMessage("NAND_AUTOLOAD", "Model auto-loaded from NAND Flash to SDRAM @ 0xC0200000");
                usb->sendStatusMessage("NAND_AUTOLOAD", "AI inference ready! Use 'ai_inference' then 'run' to test.");
                break;

            case AUTOLOAD_FAILED_NO_MODEL:
                usb->sendStatusMessage("NAND_AUTOLOAD", "No model found in NAND Flash");
                usb->sendStatusMessage("NAND_AUTOLOAD", "Use 'load_model_binary' to upload model to SDRAM");
                usb->sendStatusMessage("NAND_AUTOLOAD", "Then use 'load_model_flash' to save to NAND for persistence");
                break;

            case AUTOLOAD_FAILED_FMC_ERROR:
                usb->sendStatusMessage("NAND_AUTOLOAD", "FMC initialization failed - NAND auto-load skipped");
                usb->sendStatusMessage("NAND_AUTOLOAD", "Use 'load_model_binary' to load model directly to SDRAM");
                break;

            case AUTOLOAD_FAILED_READ_ERROR:
                usb->sendStatusMessage("NAND_AUTOLOAD", "NAND read error - model corrupted or incomplete");
                usb->sendStatusMessage("NAND_AUTOLOAD", "Use 'load_model_flash' to re-write model to NAND");
                break;

            case AUTOLOAD_NOT_ATTEMPTED:
            default:
                // Manual command sequence - no auto-load
                usb->sendStatusMessage("SYSTEM", "=== MANUAL COMMAND SEQUENCE ===");
                usb->sendStatusMessage("SYSTEM", "1) init  2) ready  3) registers  4) calibrate");
                usb->sendStatusMessage("SYSTEM", "5) load_model_binary  6) load_model_flash  7) read_model_nand");
                usb->sendStatusMessage("SYSTEM", "8) ai_init  9) run_inference_isr_real");
                break;
        }

        // Give USB tasks brief time to queue messages, then exit
        // Even if PC not connected, task must complete to free resources
        vTaskDelay(pdMS_TO_TICKS(100));

    }

    // Delete task when done
    vTaskDelete(NULL);
}


void MPU_Config(void)
{

	MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the main SRAM region (RAM_D1) - WRITE-THROUGH for ISR/DMA safety */
  /* CRITICAL FIX: Changed from Write-Back (B=1) to Write-Through (B=0)
   *
   * Write-Back (TEX=0, C=1, B=1): CPU writes go to cache first, may not reach RAM
   *   → ISR writes cached, Task reads stale RAM data = DATA CORRUPTION!
   *
   * Write-Through (TEX=0, C=1, B=0): CPU writes go to BOTH cache AND RAM
   *   → ISR writes visible in RAM immediately, Task reads fresh data
   *   → ~5-10% slower but SAFE for ISR→Task data flow
   */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x24000000; // RAM_D1 start
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;  // Single core, no need for shareable
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;      // C=1: Read caching enabled
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE; // B=0: Write-Through (not Write-Back!)
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Configure the SDRAM region - WRITE-BACK NO ALLOCATE for performance */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0xC0000000; // SDRAM start
  MPU_InitStruct.Size = MPU_REGION_SIZE_32MB; // Keep 32MB (model needs it)
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0; // Write-Back No Allocate: TEX=0, C=1, B=1
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE; // CRITICAL: BUFFERABLE for Write-Back No Allocate
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* AI HOT-ZONE: DISABLED - Write-Allocate caused 126ms REGRESSION (779→905ms)
   * Root cause: 790KB activation buffer vs 16KB D-Cache = constant evictions
   * Write-Allocate adds allocation overhead on every miss
   * Write-Back No-Allocate (Region 1 default) is faster for this use case
   */

  /* Configure the FMC/NAND region (CRITICAL for NAND Flash access!) */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0x80000000; // FMC Bank 3 (NAND Flash)
  MPU_InitStruct.Size = MPU_REGION_SIZE_256MB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0; // Device Memory
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE; // NAND requires NOT_BUFFERABLE
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Configure the peripheral region */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER3;
  MPU_InitStruct.BaseAddress = 0x40000000; // AHB/APB peripherals
  MPU_InitStruct.Size = MPU_REGION_SIZE_512MB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0; // Device Memory
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Configure RAM_D2 region (CRITICAL for DMA Processing Task Stack!) */
  /* If this is missing, accessing stack in RAM_D2 (0x30000000) will fault if
     default memory map disallows execution or access */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER4;
  MPU_InitStruct.BaseAddress = 0x30000000; // RAM_D2 start
  MPU_InitStruct.Size = MPU_REGION_SIZE_256KB; // 288KB total, but 256KB safe region size
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE; // Buffer only, no code
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;      // Enable cache
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE; // Write-Through for safety (matches RAM_D1)
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/* USER CODE BEGIN FREERTOS_STACK_OVERFLOW_HOOK */
/**
 * @brief FreeRTOS Stack Overflow Hook - CRITICAL for AI inference stability
 * This function is called when FreeRTOS detects stack overflow (configCHECK_FOR_STACK_OVERFLOW = 2)
 * @param xTask: Handle of the task that overflowed
 * @param pcTaskName: Name of the task that overflowed
 */
extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    // CRITICAL: System halt on stack overflow - AI inference safety measure
    // Log the error and halt system to prevent data corruption

    // Attempt to send error message via USB (if still functional)
    #include "UsbCommunication.hpp"
    UsbCommunication* usb = UsbCommunication::getInstance();
    if (usb) {
        char overflow_msg[256];
        snprintf(overflow_msg, sizeof(overflow_msg),
            "CRITICAL_STACK_OVERFLOW: Task=%s - System halted for safety",
            pcTaskName ? pcTaskName : "UNKNOWN");
        usb->sendStatusMessage("STACK_OVERFLOW", overflow_msg);

        // Give USB a moment to send the message
        HAL_Delay(100);
    }

    // Flash LED to indicate critical error
    while(1) {
        // Toggle LED if available
        #ifdef LED3_Pin
        HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
        #endif
        HAL_Delay(100); // Fast flash to indicate critical error
    }
}
/* USER CODE END FREERTOS_STACK_OVERFLOW_HOOK */

// ============================================================================
// IWDG INITIALIZATION
// Independent Watchdog for system recovery from hangs
// NOTE: Requires HAL_IWDG_MODULE_ENABLED - add IWDG via STM32CubeMX to enable
// ============================================================================
#ifdef HAL_IWDG_MODULE_ENABLED
static void MX_IWDG1_Init(void)
{
    // CRITICAL: Enable LSI oscillator (IWDG requires it)
    __HAL_RCC_LSI_ENABLE();
    
    // Wait for LSI to be ready (maximum ~100us typical)
    uint32_t tickstart = HAL_GetTick();
    while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY) == RESET) {
        if ((HAL_GetTick() - tickstart) > 100) {
            // LSI timeout - continue without watchdog (not critical)
            return;
        }
    }
    
    hiwdg1.Instance = IWDG1;
    hiwdg1.Init.Prescaler = IWDG_PRESCALER_256;  // 32kHz / 256 = 125Hz
    hiwdg1.Init.Window = IWDG_WINDOW_DISABLE;
    hiwdg1.Init.Reload = (125 * IWDG_TIMEOUT_SEC); // 30s = 3750
    
    if (HAL_IWDG_Init(&hiwdg1) != HAL_OK) {
        // Watchdog init failed - not critical, system continues
    }
}
#else
static void MX_IWDG1_Init(void)
{
    // IWDG not enabled - stub function
    // To enable watchdog, add IWDG peripheral via STM32CubeMX
}
#endif

// ============================================================================
// WATCHDOG TASK: Periodic IWDG Refresh
// Runs at highest priority to ensure watchdog is always fed
// ============================================================================
void WatchdogTask(void* argument)
{
    (void)argument;
    
    for (;;) {
#ifdef HAL_IWDG_MODULE_ENABLED
        HAL_IWDG_Refresh(&hiwdg1);
#endif
        osDelay(IWDG_REFRESH_INTERVAL_MS);
    }
}

// AutoLoadTask and AutoStartupTask removed.
// Use manual USB commands: init → ready → registers → calibrate →
//   load_model_binary → load_model_flash → read_model_nand → ai_init → run_inference_isr_real

#if 0  // REMOVED - kept only for reference
void AutoLoadTask(void* argument)
{
    (void)argument;
    
    // Wait for scheduler to stabilize
    osDelay(AUTOLOAD_STABILIZATION_MS);
    
    UsbCommunication* usb = UsbCommunication::getInstance();
    if (usb) {
        usb->sendStatusMessage("AUTOLOAD", "Starting NAND model load...");
    }
    
    // FMC mutex protection for NAND operations
    if (xSemaphoreTake(fmc_mutex, pdMS_TO_TICKS(AUTOLOAD_TIMEOUT_MS)) == pdTRUE) {
        
        bool success = false;
        
        for (int retry = 0; retry < MAX_NAND_RETRY_COUNT && !success; retry++) {
            
            // Switch FMC to NAND mode
            if (FMC_ReInitNAND() == HAL_OK) {
                
                HAL_StatusTypeDef result = ReadModelFromNAND_ToSDRAM();
                
                // ALWAYS restore SDRAM mode (critical!)
                if (FMC_RestoreSDRAM() == HAL_OK) {
                    if (result == HAL_OK) {
                        g_autoload_status = AUTOLOAD_SUCCESS;
                        success = true;
                        if (usb) {
                            usb->sendStatusMessage("AUTOLOAD", "Model loaded from NAND!");
                        }
                    } else {
                        g_autoload_status = AUTOLOAD_FAILED_NO_MODEL;
                        if (usb) {
                            usb->sendStatusMessage("AUTOLOAD", "No model in NAND");
                        }
                        break; // No point retrying if no model exists
                    }
                } else {
                    g_autoload_status = AUTOLOAD_FAILED_FMC_ERROR;
                    if (usb) {
                        usb->sendStatusMessage("AUTOLOAD", "FMC restore failed!");
                    }
                }
            } else {
                g_autoload_status = AUTOLOAD_FAILED_FMC_ERROR;
            }
            
            if (!success && retry < MAX_NAND_RETRY_COUNT - 1) {
                osDelay(RETRY_DELAY_MS);
            }
        }
        
        xSemaphoreGive(fmc_mutex);
    } else {
        g_autoload_status = AUTOLOAD_FAILED_FMC_ERROR;
        if (usb) {
            usb->sendStatusMessage("AUTOLOAD", "FMC mutex timeout");
        }
    }
    
    // Task completed - delete itself
    vTaskDelete(NULL);
}

void AutoStartupTask(void* argument)  // UNUSED - kept inside #if 0 block
{
    (void)argument;

    UsbCommunication* usb = UsbCommunication::getInstance();
    StateMachine* sm = StateMachine::getInstance();

    // Wait for system stabilization
    osDelay(1000);

    if (usb) {
        usb->sendStatusMessage("AUTOSTARTUP", "Starting automatic initialization...");
    }

    // ========================================================================
    // STEP 1: Wait for AutoLoadTask to complete
    // ========================================================================
    uint32_t timeout = AUTOLOAD_TIMEOUT_MS;
    uint32_t elapsed = 0;
    while (g_autoload_status == AUTOLOAD_PENDING && elapsed < timeout) {
        osDelay(100);
        elapsed += 100;
    }

    if (g_autoload_status != AUTOLOAD_SUCCESS) {
        if (usb) {
            usb->sendStatusMessage("AUTOSTARTUP", "Model load failed - manual load required");
        }
        vTaskDelete(NULL);
        return;
    }

    if (usb) usb->sendStatusMessage("AUTOSTARTUP", "Step 1/7: Model loaded");

    // ========================================================================
    // STEP 2: STATE_INIT (wait for initialization to complete)
    // ========================================================================
    osDelay(200);
    sm->changeState(STATE_INIT);
    if (usb) usb->sendStatusMessage("AUTOSTARTUP", "Step 2/7: INIT - waiting for completion...");

    // Wait for INIT to complete - takes about 4 seconds typically
    // Poll state machine for INIT completion (it's sync so just wait a bit)
    osDelay(5000);  // INIT takes ~4s (reset + registers + ADC test)
    if (usb) usb->sendStatusMessage("AUTOSTARTUP", "Step 2/7: INIT complete");

    // ========================================================================
    // STEP 3: STATE_READY
    // ========================================================================
    sm->changeState(STATE_READY);
    if (usb) usb->sendStatusMessage("AUTOSTARTUP", "Step 3/7: READY");
    osDelay(500);

    // ========================================================================
    // STEP 4: Radar Register Configuration
    // ========================================================================
    if (usb) usb->sendStatusMessage("AUTOSTARTUP", "Step 4/7: Configuring radar...");

    std::vector<uint8_t> regValues(RADAR_REGISTERS, RADAR_REGISTERS + RADAR_REGISTER_COUNT);
    sm->processCommand(CMD_UPDATE_RADAR, regValues);

    // Wait for register update + ADC test to complete
    osDelay(2000);
    if (usb) usb->sendStatusMessage("AUTOSTARTUP", "Radar configured (59 registers)");

    // ========================================================================
    // STEP 5: STATE_CALIBRATE (POLL for completion!)
    // ========================================================================
    sm->changeState(STATE_CALIBRATE);
    if (usb) usb->sendStatusMessage("AUTOSTARTUP", "Step 5/7: CALIBRATE - waiting for completion...");

    // Poll for calibration task to complete (calibrationTaskHandle becomes NULL)
    // The task sets itself to NULL when done (Calibrate.cpp line 208)
    extern TaskHandle_t calibrationTaskHandle;
    uint32_t calib_timeout = 60000;  // Maximum 60 seconds for calibration
    uint32_t calib_elapsed = 0;

    while (calibrationTaskHandle != NULL && calib_elapsed < calib_timeout) {
        osDelay(500);
        calib_elapsed += 500;

        // Send progress every 5 seconds
        if (calib_elapsed % 5000 == 0) {
            char msg[64];
            snprintf(msg, sizeof(msg), "Calibration in progress... (%lus)", calib_elapsed / 1000);
            if (usb) usb->sendStatusMessage("AUTOSTARTUP", msg);
        }
    }

    if (calibrationTaskHandle == NULL) {
        if (usb) usb->sendStatusMessage("AUTOSTARTUP", "Step 5/7: CALIBRATE complete");
    } else {
        if (usb) usb->sendStatusMessage("AUTOSTARTUP", "WARNING: Calibration timeout - proceeding anyway");
    }

    // ========================================================================
    // STEP 6: ESP32 Connection Check (REQUIRED!)
    // ========================================================================
    if (usb) usb->sendStatusMessage("AUTOSTARTUP", "Step 6/7: Checking ESP32...");

    extern uint8_t SPI_Check_Connection(void);
    bool esp32_connected = false;

    for (int retry = 0; retry < MAX_ESP32_RETRY_COUNT; retry++) {
        uint8_t esp32_status = SPI_Check_Connection();
        if (esp32_status == 0x55) {
            esp32_connected = true;
            if (usb) usb->sendStatusMessage("AUTOSTARTUP", "ESP32 connected!");
            break;
        }
        if (usb) {
            char msg[64];
            snprintf(msg, sizeof(msg), "ESP32 attempt %d/%d failed (0x%02X)",
                     retry + 1, MAX_ESP32_RETRY_COUNT, esp32_status);
            usb->sendStatusMessage("AUTOSTARTUP", msg);
        }
        osDelay(RETRY_DELAY_MS);
    }

    // ESP32 connection is REQUIRED - abort if not connected
    if (!esp32_connected) {
        if (usb) {
            usb->sendStatusMessage("AUTOSTARTUP", "ESP32 NOT CONNECTED - Inference aborted!");
            usb->sendStatusMessage("AUTOSTARTUP", "Please check ESP32 connection and restart device");
        }
        vTaskDelete(NULL);
        return;
    }

    // ========================================================================
    // STEP 7: AI Initialization + Start Inference
    // ========================================================================
    if (usb) usb->sendStatusMessage("AUTOSTARTUP", "Step 7/7: AI_INIT + Inference...");

    // Initialize AI model directly (extern function from UsbCommunication.hpp)
    extern void ai_fallower_init_from_sdram();
    ai_fallower_init_from_sdram();

    osDelay(1000); // Wait for AI initialization to complete
    if (usb) usb->sendStatusMessage("AUTOSTARTUP", "AI initialized");

    // Suspend background tasks for clean FMC access during inference
    if (nandTaskHandle != NULL) {
        osThreadSuspend(nandTaskHandle);
    }
    if (connectionCheckTaskHandle != NULL) {
        osThreadSuspend(connectionCheckTaskHandle);
    }

    osDelay(100);

    // Start DMA_Real mode (Stride-14 continuous inference)
    sm->transitionTo(STATE_REALTIME_INFERENCE_ISR_REAL);

    osDelay(500);

    if (usb) {
        usb->sendStatusMessage("AUTOSTARTUP",
            "SYSTEM READY - Inference running (stride-14, continuous)");
    }

    // Task completed - delete itself
    vTaskDelete(NULL);
}
#endif  // REMOVED AutoLoadTask + AutoStartupTask
