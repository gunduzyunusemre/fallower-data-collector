#ifndef AI_MONITORING_HPP
#define AI_MONITORING_HPP

#include <stdint.h>
#include <stdbool.h>
#include "UsbCommunication.hpp"

// AI Pipeline Monitoring & Debugging System
// Systematic tracking of AI inference pipeline failures

class AIMonitoring {
public:
    // Monitoring flags and counters
    struct MonitoringState {
        uint32_t step_counter;
        uint32_t error_count;
        uint32_t cache_invalidations;
        uint32_t alignment_checks;
        bool weights_verified;
        bool cache_coherent;
        bool data_format_valid;
        char last_error[128];
    };

    // Initialize monitoring system
    static void Initialize();

    // Step-by-step pipeline monitoring
    static bool MonitorStep(const char* step_name, uint32_t step_id);

    // Memory alignment verification
    static bool VerifyMemoryAlignment(void* ptr, uint32_t expected_alignment, const char* description);

    // Cache coherency monitoring
    static bool VerifyCacheCoherency(void* buffer, uint32_t size, const char* buffer_name);

    // Data format validation
    static bool ValidateDataFormat(const float* data, uint32_t count, float expected_min, float expected_max, const char* data_name);

    // Model weights integrity check
    static bool VerifyModelWeights(void* weights_ptr, uint32_t weights_size, uint32_t expected_crc);

    // Input tensor shape validation
    static bool ValidateInputTensor(const float* input, uint32_t batch, uint32_t channels, uint32_t height, uint32_t width);

    // Output tensor validation
    static bool ValidateOutputTensor(const float* output, uint32_t output_classes);

    // Memory dump for debugging (first/last N bytes)
    static void DumpMemoryRegion(void* ptr, uint32_t size, const char* region_name, uint32_t dump_bytes = 16);

    // Cache management verification
    static void VerifyAndFixCacheCoherency(void* buffer, uint32_t size, const char* operation);

    // Performance timing
    static void StartTiming(const char* operation_name);
    static void EndTiming(const char* operation_name);

    // Error reporting
    static void ReportError(const char* function, const char* error_description);
    static void ReportSuccess(const char* function, const char* success_description);

    // Get monitoring state for external analysis
    static const MonitoringState& GetState();

    // Reset monitoring counters
    static void Reset();

    // Public logging helper for external use
    static void Log(const char* category, const char* message);

private:
    static MonitoringState state;
    static uint32_t timing_start;
    static UsbCommunication* usb_comm;

    // Calculate CRC32 for integrity check
    static uint32_t CalculateCRC32(const void* data, uint32_t length);
};

// Monitoring macros for easy integration
#define AI_MONITOR_STEP(step_name, step_id) \
    AIMonitoring::MonitorStep(step_name, step_id)

#define AI_VERIFY_ALIGNMENT(ptr, align, desc) \
    AIMonitoring::VerifyMemoryAlignment(ptr, align, desc)

#define AI_VERIFY_CACHE(buffer, size, name) \
    AIMonitoring::VerifyCacheCoherency(buffer, size, name)

#define AI_VALIDATE_DATA(data, count, min_val, max_val, name) \
    AIMonitoring::ValidateDataFormat(data, count, min_val, max_val, name)

#define AI_START_TIMING(op) AIMonitoring::StartTiming(op)
#define AI_END_TIMING(op) AIMonitoring::EndTiming(op)

#define AI_REPORT_ERROR(func, error) AIMonitoring::ReportError(func, error)
#define AI_REPORT_SUCCESS(func, success) AIMonitoring::ReportSuccess(func, success)

#endif // AI_MONITORING_HPP