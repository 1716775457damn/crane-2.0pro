#ifndef SYSTEM_HEALTH_H
#define SYSTEM_HEALTH_H

#include <Arduino.h>

//=========================
// System Health Monitoring
//=========================

/**
 * System health status structure
 */
struct SystemHealth {
    bool memoryOK;
    bool temperatureOK;
    bool communicationOK;
    bool motorsOK;
    bool sensorsOK;
    unsigned long lastHealthCheck;
    int errorCount;
    String lastError;
};

// External variable declaration
extern SystemHealth systemHealth;

//=========================
// Function Declarations
//=========================

/**
 * Check overall system health status
 * @return true if all systems are healthy
 */
bool checkSystemHealth();

/**
 * Log system events with timestamp
 * @param event Event description to log
 */
void logSystemEvent(const String &event);

/**
 * Perform comprehensive safety checks
 * Called periodically from main loop
 */
void performSafetyChecks();

/**
 * Handle critical system errors
 * @param errorMsg Error message description
 */
void handleCriticalError(const String &errorMsg);

/**
 * Print detailed system health report
 * Outputs comprehensive health status to serial
 */
void printSystemHealthReport();

/**
 * Reset error counters and clear error history
 */
void resetSystemErrors();

/**
 * Get current system uptime in seconds
 * @return System uptime in seconds
 */
unsigned long getSystemUptime();

/**
 * Get free heap memory in bytes
 * @return Available heap memory
 */
uint32_t getFreeHeapMemory();

/**
 * Get CPU temperature in Celsius
 * @return CPU temperature
 */
float getCPUTemperature();

/**
 * Check if system is in emergency state
 * @return true if emergency stop is active
 */
bool isEmergencyState();

//=========================
// Performance Monitoring
//=========================

/**
 * Performance metrics structure
 */
struct PerformanceMetrics {
    unsigned long loopTime;           // Main loop execution time (microseconds)
    unsigned long webResponseTime;    // Web interface response time (milliseconds)
    unsigned long chassisResponseTime; // Chassis command response time (milliseconds)
    unsigned long safetyCheckTime;    // Safety check execution time (microseconds)
    unsigned long maxLoopTime;        // Maximum recorded loop time
    unsigned long avgLoopTime;        // Average loop time
    uint32_t loopCount;               // Total loop iterations
};

// External performance metrics
extern PerformanceMetrics performanceMetrics;

/**
 * Update performance metrics
 * @param loopStartTime Start time of current loop iteration
 */
void updatePerformanceMetrics(unsigned long loopStartTime);

/**
 * Print performance report
 */
void printPerformanceReport();

/**
 * Reset performance counters
 */
void resetPerformanceMetrics();

//=========================
// Memory Management
//=========================

/**
 * Memory usage structure
 */
struct MemoryUsage {
    uint32_t totalHeap;      // Total heap size
    uint32_t freeHeap;       // Current free heap
    uint32_t minFreeHeap;    // Minimum free heap recorded
    uint32_t maxAllocHeap;   // Maximum allocated heap
    uint32_t psramTotal;     // Total PSRAM (if available)
    uint32_t psramFree;      // Free PSRAM (if available)
};

/**
 * Get current memory usage statistics
 * @return MemoryUsage structure with current stats
 */
MemoryUsage getMemoryUsage();

/**
 * Check if memory usage is within safe limits
 * @return true if memory usage is safe
 */
bool isMemoryUsageSafe();

/**
 * Print detailed memory report
 */
void printMemoryReport();

//=========================
// System Configuration
//=========================

/**
 * System configuration structure
 */
struct SystemConfig {
    bool safetyEnabled;           // Safety monitoring enabled
    bool performanceMonitoring;  // Performance monitoring enabled
    bool debugMode;               // Debug mode enabled
    uint32_t memoryThreshold;     // Memory warning threshold (bytes)
    float temperatureThreshold;   // Temperature warning threshold (°C)
    unsigned long safetyInterval; // Safety check interval (ms)
};

// External system configuration
extern SystemConfig systemConfig;

/**
 * Initialize system health monitoring
 */
void initSystemHealth();

/**
 * Configure system health parameters
 * @param memThreshold Memory warning threshold in bytes
 * @param tempThreshold Temperature warning threshold in Celsius
 * @param safetyInt Safety check interval in milliseconds
 */
void configureSystemHealth(uint32_t memThreshold, float tempThreshold, unsigned long safetyInt);

/**
 * Enable or disable safety monitoring
 * @param enabled true to enable, false to disable
 */
void setSafetyMonitoring(bool enabled);

/**
 * Enable or disable performance monitoring
 * @param enabled true to enable, false to disable
 */
void setPerformanceMonitoring(bool enabled);

/**
 * Enable or disable debug mode
 * @param enabled true to enable, false to disable
 */
void setDebugMode(bool enabled);

//=========================
// Utility Functions
//=========================

/**
 * Format uptime as human-readable string
 * @param uptimeSeconds Uptime in seconds
 * @return Formatted string (e.g., "1d 2h 30m 45s")
 */
String formatUptime(unsigned long uptimeSeconds);

/**
 * Format memory size as human-readable string
 * @param bytes Memory size in bytes
 * @return Formatted string (e.g., "1.5MB", "512KB")
 */
String formatMemorySize(uint32_t bytes);

/**
 * Get system health status as JSON string
 * @return JSON formatted health status
 */
String getHealthStatusJSON();

/**
 * Get performance metrics as JSON string
 * @return JSON formatted performance metrics
 */
String getPerformanceMetricsJSON();

//=========================
// Constants
//=========================

// Default thresholds
#define DEFAULT_MEMORY_THRESHOLD    20000    // 20KB minimum free memory
#define DEFAULT_TEMP_THRESHOLD      80.0     // 80°C maximum temperature
#define DEFAULT_SAFETY_INTERVAL     1000     // 1 second safety check interval

// Performance limits
#define MAX_LOOP_TIME_WARNING       10000    // 10ms loop time warning
#define MAX_WEB_RESPONSE_WARNING    100      // 100ms web response warning
#define MAX_CHASSIS_RESPONSE_WARNING 50      // 50ms chassis response warning

// Memory limits
#define CRITICAL_MEMORY_THRESHOLD   10000    // 10KB critical memory level
#define PSRAM_AVAILABLE_THRESHOLD   1000000  // 1MB PSRAM availability check

#endif // SYSTEM_HEALTH_H
