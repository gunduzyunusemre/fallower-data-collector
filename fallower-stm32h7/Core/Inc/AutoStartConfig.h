#ifndef AUTOSTART_CONFIG_H
#define AUTOSTART_CONFIG_H

#include <stdint.h>  // For uint8_t

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// FALLOWER DEVICE - Task & Timing Configuration
// ============================================================================

// ============================================================================
// TIMING CONFIGURATION (milliseconds)
// ============================================================================

// Scheduler stabilization delay after osKernelStart
#define AUTOLOAD_STABILIZATION_MS      500

// Maximum wait time for NAND model loading
#define AUTOLOAD_TIMEOUT_MS            15000

// AI engine initialization timeout
#define AI_INIT_TIMEOUT_MS             5000

// Calibration completion wait time
// NOTE: Calibration typically takes 13-15 iterations (~15-20 seconds)
#define CALIBRATION_TIMEOUT_MS         20000

// Delay between retry attempts
#define RETRY_DELAY_MS                 500

// ============================================================================
// WATCHDOG CONFIGURATION
// ============================================================================

// Independent Watchdog timeout in seconds
// System will reset if not refreshed within this period
#define IWDG_TIMEOUT_SEC               30

// Refresh interval (must be less than timeout)
// Recommended: timeout / 2
#define IWDG_REFRESH_INTERVAL_MS       15000

// ============================================================================
// TASK STACK SIZES (words, multiply by 4 for bytes)
// All stacks are placed in RAM_D2 (.ram_d2 section)
// ============================================================================

// AutoLoadTask: NAND FMC switching, SDRAM operations
// Required: ~6KB, Allocated: 8KB (with safety margin)
#define AUTOLOAD_TASK_STACK_SIZE       2048

// WatchdogTask: Only HAL_IWDG_Refresh call
// Minimal requirements, 2KB is more than enough
#define WATCHDOG_TASK_STACK_SIZE       512

// ============================================================================
// RETRY CONFIGURATION
// ============================================================================

// NAND read retry count before giving up
#define MAX_NAND_RETRY_COUNT           2

#ifdef __cplusplus
}
#endif

#endif // AUTOSTART_CONFIG_H
