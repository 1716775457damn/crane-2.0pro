#ifndef _LASER_H_
#define _LASER_H_

#include <Arduino.h>
#include "pins.h"

// 激光传感器串口定义 - 使用Serial0避免与20、21串口冲突
#define SENSOR_SERIAL Serial0

// 激光传感器命令定义
#define ATK_MS53L2M_WORKMODE_NORMAL 0x00
#define ATK_MS53L2M_WORKMODE_FAST 0x01
#define ATK_MS53L2M_OUTPUTMODE_1 0x01
#define ATK_MS53L2M_OUTPUTMODE_2 0x02

// 调试模式定义
#define LASER_DEBUG_BASIC 0
#define LASER_DEBUG_DETAILED 1
#define LASER_DEBUG_RAW 2

// 滤波参数
#define FILTER_SIZE 5
#define MEDIAN_FILTER_SIZE 5
#define MOVEMENT_DETECTION_THRESHOLD 50
#define MAX_VALID_CHANGE_RATE 100
#define MIN_OUTPUT_CHANGE 5

// 缓冲区定义
#define RX_BUF_SIZE 256
static char g_rx_buf[RX_BUF_SIZE];
static uint16_t g_rx_cnt = 0;

// 滤波数据存储
static int distance_history[FILTER_SIZE];
static int filter_index = 0;
static bool filter_filled = false;

// 中值滤波相关定义
static int median_window[MEDIAN_FILTER_SIZE] = {0};
static int median_index = 0;
static bool median_filled = false;

// 全局变量
static bool laser_initialized = false;
static uint16_t laser_current_distance = 0;
static uint8_t consecutive_failures = 0;
static uint8_t current_output_mode = ATK_MS53L2M_OUTPUTMODE_1;
static uint8_t current_work_mode = ATK_MS53L2M_WORKMODE_NORMAL;

// 调试模式变量
static uint8_t laser_debug_mode = LASER_DEBUG_BASIC;
static bool laser_continuous_mode = false;
static unsigned long last_debug_print = 0;

// 输出滤波值
static int last_output_distance = 0;
static int last_valid_distance = 0;

// 运动检测
static bool movement_detected = false;

// 函数声明
inline bool laser_reset(void);
inline bool laser_init(void);
inline int apply_filter(int new_value);
inline int apply_median_filter(int new_value);
inline int apply_smooth_filter(int new_value);
inline bool is_stable(int current_value);
inline uint8_t atk_ms53l2m_get_distance(uint16_t *distance);
inline bool send_laser_commands(void);
inline void atk_ms53l2m_uart_rx_restart(void);
inline int get_laser_distance(bool auto_init = true, bool print_result = false);

/**
 * Get laser distance with auto-initialization
 */
inline int get_laser_distance(bool auto_init, bool print_result)
{
    uint16_t distance;
    uint8_t result = atk_ms53l2m_get_distance(&distance);

    if (result == 0)
    {
        if (print_result)
        {
            Serial.print("Laser distance: ");
            Serial.print(distance);
            Serial.println(" mm");
        }
        return (int)distance;
    }
    else
    {
        if (print_result)
        {
            Serial.print("Laser read failed, error code: ");
            Serial.println(result);
        }
        return -1;
    }
}

/**
 * Reset receive buffer
 */
inline void atk_ms53l2m_uart_rx_restart(void)
{
    // Clear serial buffer
    while (SENSOR_SERIAL.available())
    {
        SENSOR_SERIAL.read();
    }

    // Reset global counter
    g_rx_cnt = 0;
    memset(g_rx_buf, 0, sizeof(g_rx_buf));
}

/**
 * Median filter - removes outliers while maintaining responsiveness
 */
inline int apply_median_filter(int new_value)
{
    if (new_value <= 0)
    {
        return -1; // Invalid value, return directly
    }

    // Store in median filter window
    median_window[median_index] = new_value;
    median_index = (median_index + 1) % MEDIAN_FILTER_SIZE;

    // Check if window is filled
    if (!median_filled && median_index == 0)
    {
        median_filled = true;
    }

    // If window is not filled, return original value
    if (!median_filled)
    {
        return new_value;
    }

    // Create a copy of the window for sorting
    int sorted_window[MEDIAN_FILTER_SIZE];
    memcpy(sorted_window, median_window, sizeof(int) * MEDIAN_FILTER_SIZE);

    // Simple bubble sort
    for (int i = 0; i < MEDIAN_FILTER_SIZE - 1; i++)
    {
        for (int j = 0; j < MEDIAN_FILTER_SIZE - i - 1; j++)
        {
            if (sorted_window[j] > sorted_window[j + 1])
            {
                int temp = sorted_window[j];
                sorted_window[j] = sorted_window[j + 1];
                sorted_window[j + 1] = temp;
            }
        }
    }

    // Return median
    return sorted_window[MEDIAN_FILTER_SIZE / 2];
}

/**
 * Dynamic smoothing filter - adjusts filter strength based on movement status
 */
inline int apply_smooth_filter(int new_value)
{
    if (new_value <= 0)
    {
        return -1; // Invalid value, return directly
    }

    // Detect if in movement state
    if (last_valid_distance > 0)
    {
        int change = abs(new_value - last_valid_distance);

        // Update movement status
        if (change > MOVEMENT_DETECTION_THRESHOLD)
        {
            movement_detected = true;
        }
        else if (movement_detected && change < MOVEMENT_DETECTION_THRESHOLD / 2)
        {
            // When change is less than half of threshold, consider movement stopped
            movement_detected = false;
        }

        // Check change rate, prevent sudden changes, but allow fast changes during movement
        int max_change = movement_detected ? MAX_VALID_CHANGE_RATE * 3 / 2 : MAX_VALID_CHANGE_RATE;

        if (change > max_change)
        {
            // Change too large, limit change magnitude
            if (new_value > last_valid_distance)
            {
                new_value = last_valid_distance + max_change;
            }
            else
            {
                new_value = last_valid_distance - max_change;
            }
        }
    }

    // Store in filter window
    distance_history[filter_index] = new_value;
    filter_index = (filter_index + 1) % FILTER_SIZE;

    // Check if window is filled
    if (!filter_filled && filter_index == 0)
    {
        filter_filled = true;
    }

    // If window is not filled, return original value
    if (!filter_filled)
    {
        last_valid_distance = new_value;
        return new_value;
    }

    // Dynamically adjust weight coefficient, give more weight to new data during movement
    int weight_decay_percent = movement_detected ? 80 : 70;

    // Calculate weighted average, newer data has more weight
    long sum = 0;
    long weight_sum = 0;
    long weight = 100; // Base weight * 100

    for (int i = 0; i < FILTER_SIZE; i++)
    {
        int idx = (filter_index - 1 - i + FILTER_SIZE) % FILTER_SIZE;
        if (distance_history[idx] > 0)
        {
            sum += (long)distance_history[idx] * weight;
            weight_sum += weight;
            weight = (weight * weight_decay_percent) / 100; // Dynamic weight decay coefficient
        }
    }

    int result = (weight_sum > 0) ? (int)(sum / weight_sum) : -1;

    if (result > 0)
    {
        last_valid_distance = result;
    }

    return result;
}

/**
 * Send commands to laser sensor
 */
inline bool send_laser_commands()
{
    static int command_index = 0;
    const char *commands[] = {
        "F\r\n",       // Common single measurement command
        "C\r\n",       // Continuous measurement command
        "D\r\n",       // Distance request
        "GETDATA\r\n", // Get data command
        "GETDIST\r\n", // Get distance command
        "MEASU\r\n",   // Measurement command
    };

    // Send different commands each time, try all commands in turn
    const int commands_count = sizeof(commands) / sizeof(commands[0]);

    // Send the command corresponding to the current index
    SENSOR_SERIAL.write(commands[command_index]);

    // Update index for next time
    command_index = (command_index + 1) % commands_count;

    return true;
}

/**
 * Stability check - checks if data is stable, reduces requirements during movement
 */
inline bool is_stable(int current_value)
{
    if (!filter_filled)
        return false;

    // Calculate variance in filter window
    int max_val = distance_history[0];
    int min_val = distance_history[0];

    for (int i = 1; i < FILTER_SIZE; i++)
    {
        if (distance_history[i] > 0)
        {
            if (distance_history[i] > max_val)
                max_val = distance_history[i];
            if (distance_history[i] < min_val)
                min_val = distance_history[i];
        }
    }

    int variance = max_val - min_val;

    // During movement, allow larger variance
    int max_variance = movement_detected ? 30 : 15;

    return variance <= max_variance;
}

/**
 * Apply comprehensive filter
 */
inline int apply_filter(int new_value)
{
    if (new_value <= 0)
    {
        return -1; // Invalid value, return directly
    }

    // First apply median filter to remove outliers
    int median_filtered = apply_median_filter(new_value);
    if (median_filtered <= 0)
        return -1;

    // Then apply smoothing filter
    int smoothed = apply_smooth_filter(median_filtered);
    if (smoothed <= 0)
        return -1;

    // Check stability
    if (!is_stable(smoothed))
    {
        return -1; // If not stable, return error
    }

    return smoothed;
}

/**
 * Get distance from laser sensor
 */
inline uint8_t atk_ms53l2m_get_distance(uint16_t *distance)
{
    // Reset receive buffer
    atk_ms53l2m_uart_rx_restart();

    // Clear global variables
    g_rx_cnt = 0;

    // Send measurement command
    send_laser_commands();

    // Wait for response
    unsigned long start_time = millis();
    while (millis() - start_time < 100)
    {
        // Read serial data
        while (SENSOR_SERIAL.available() && g_rx_cnt < RX_BUF_SIZE - 1)
        {
            char c = SENSOR_SERIAL.read();
            g_rx_buf[g_rx_cnt++] = c;
        }

        // Check if we have a complete line
        if (g_rx_cnt > 0)
        {
            for (int i = 1; i < g_rx_cnt; i++)
            {
                if ((g_rx_buf[i - 1] == '\r' && g_rx_buf[i] == '\n') ||
                    (g_rx_buf[i - 1] == '\n' && g_rx_buf[i] == '\r'))
                {

                    // Continue reading to get complete data
                    while (SENSOR_SERIAL.available() && g_rx_cnt < RX_BUF_SIZE - 1)
                    {
                        g_rx_buf[g_rx_cnt++] = SENSOR_SERIAL.read();
                    }

                    // Null terminate
                    g_rx_buf[g_rx_cnt] = '\0';

                    // Parse distance value
                    char *d_pos = strstr(g_rx_buf, "d:");
                    if (d_pos)
                    {
                        d_pos += 2; // Skip "d:"
                        *distance = atoi(d_pos);
                        return 0; // Success
                    }

                    // Try other formats
                    char *num_pos = g_rx_buf;
                    while (*num_pos && !isdigit(*num_pos))
                        num_pos++;
                    if (*num_pos)
                    {
                        *distance = atoi(num_pos);
                        return 0; // Success
                    }
                }
            }
        }

        delay(1);
    }

    return 1; // Timeout
}

/**
 * Initialize laser sensor
 */
inline bool laser_init(void)
{
    // End existing serial connection
    SENSOR_SERIAL.end();
    delay(100);

    // Initialize serial port
    SENSOR_SERIAL.begin(115200, SERIAL_8N1, LASER_RX_PIN, LASER_TX_PIN, false);
    delay(100);

    // Reset receive buffer
    atk_ms53l2m_uart_rx_restart();

    // Initialize filter data
    for (int i = 0; i < FILTER_SIZE; i++)
    {
        distance_history[i] = 0;
    }
    filter_index = 0;
    filter_filled = false;

    // Initialize median filter
    for (int i = 0; i < MEDIAN_FILTER_SIZE; i++)
    {
        median_window[i] = 0;
    }
    median_index = 0;
    median_filled = false;

    // Reset global variables
    last_output_distance = 0;
    last_valid_distance = 0;
    movement_detected = false;
    consecutive_failures = 0;

    // Test sensor communication
    uint16_t test_distance;
    uint8_t result = atk_ms53l2m_get_distance(&test_distance);

    if (result != 0)
    {
        // Try reset
        if (!laser_reset())
        {
            laser_initialized = false;
            Serial.println("Laser sensor initialization failed");
            return false;
        }

        // Test again
        result = atk_ms53l2m_get_distance(&test_distance);
        if (result != 0)
        {
            laser_initialized = false;
            Serial.println("Laser sensor communication failed after reset");
            return false;
        }
    }

    // Set initial distance
    laser_current_distance = test_distance;
    laser_initialized = true;
    consecutive_failures = 0;

    Serial.println("Laser sensor initialized successfully");
    Serial.print("Initial distance: ");
    Serial.print(test_distance);
    Serial.println(" mm");

    return true;
}

/**
 * Reset laser sensor
 */
inline bool laser_reset(void)
{
    // End serial connection
    SENSOR_SERIAL.end();
    delay(100);

    // Try different baud rates
    uint32_t baudrates[] = {115200, 9600, 19200, 38400, 57600};
    const int baud_count = sizeof(baudrates) / sizeof(baudrates[0]);

    for (int i = 0; i < baud_count; i++)
    {
        SENSOR_SERIAL.begin(baudrates[i], SERIAL_8N1, LASER_RX_PIN, LASER_TX_PIN, false);
        delay(100);

        atk_ms53l2m_uart_rx_restart();

        // Send reset command
        SENSOR_SERIAL.print("RESET\r\n");
        delay(200);

        // Test communication
        uint16_t test_distance;
        if (atk_ms53l2m_get_distance(&test_distance) == 0)
        {
            Serial.print("Laser sensor reset successful at ");
            Serial.print(baudrates[i]);
            Serial.println(" baud");
            return true;
        }

        SENSOR_SERIAL.end();
        delay(50);
    }

    Serial.println("Laser sensor reset failed");
    return false;
}

/**
 * Get laser distance with Chinese function name (compatibility function)
 * 激光测距函数 - 兼容性函数
 */
inline float jiguang()
{
    int distance = get_laser_distance(true, false);
    return (distance > 0) ? (float)distance : -1.0f;
}

/**
 * Test laser sensor functionality
 */
inline void laser_test()
{
    Serial.println("=== Laser Sensor Test ===");

    // Test initialization
    if (laser_init())
    {
        Serial.println("Laser sensor initialization: SUCCESS");
    }
    else
    {
        Serial.println("Laser sensor initialization: FAILED");
        return;
    }

    // Test multiple readings
    Serial.println("Taking 10 test readings...");
    for (int i = 0; i < 10; i++)
    {
        int distance = get_laser_distance(true, false);
        if (distance > 0)
        {
            Serial.println("Reading " + String(i + 1) + ": " + String(distance) + " mm");
        }
        else
        {
            Serial.println("Reading " + String(i + 1) + ": FAILED");
        }
        delay(200);
    }

    Serial.println("=== Test Complete ===");
}

#endif // _LASER_H_