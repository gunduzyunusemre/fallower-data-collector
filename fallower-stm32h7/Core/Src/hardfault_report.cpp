// Minimal, reentrant-safe hardfault report writer.
// Do NOT call RTOS, heap, USB or other non-reentrant APIs from here.
// This file provides a simple RAM-backed log that can be read and
// transmitted later from a normal task.

#include <stdint.h>

typedef struct {
    uint32_t magic;   // set to 0xDEADBEEF when a report is present
    uint32_t hfsr;
    uint32_t cfsr;
    uint32_t mmfar;
    uint32_t bfar;
    uint32_t reserved[3]; // reserved for future stacked regs if needed
} HardFaultLog_t;

// Global log in RAM. Volatile because it may be written from exception context
static volatile HardFaultLog_t g_hardfault_log = {0};

// Called from HardFault handler (exception context). MUST be simple and
// reentrant-safe (no C library, no RTOS, no peripheral drivers that may
// depend on interrupts or locks).
extern "C" void ReportHardFaultRegs(uint32_t hfsr, uint32_t cfsr, uint32_t mmfar, uint32_t bfar) {
    // Write the fields into the global log. Order writes so magic is set last
    // to indicate a complete record.
    g_hardfault_log.hfsr = hfsr;
    g_hardfault_log.cfsr = cfsr;
    g_hardfault_log.mmfar = mmfar;
    g_hardfault_log.bfar = bfar;
    // Mark record present
    g_hardfault_log.magic = 0xDEADBEEFu;
}

// Accessor for normal task context. Returns pointer to the log (read-only)
// or NULL if no report is present.
extern "C" const volatile HardFaultLog_t* GetHardFaultLog(void) {
    if (g_hardfault_log.magic == 0xDEADBEEFu) {
        return &g_hardfault_log;
    }
    return (const volatile HardFaultLog_t*)0;
}

// Clear the stored report (call from task context after sending/processing).
extern "C" void ClearHardFaultLog(void) {
    // Clear magic first to avoid race where a reader sees partial data.
    g_hardfault_log.magic = 0;
    g_hardfault_log.hfsr = 0;
    g_hardfault_log.cfsr = 0;
    g_hardfault_log.mmfar = 0;
    g_hardfault_log.bfar = 0;
}
