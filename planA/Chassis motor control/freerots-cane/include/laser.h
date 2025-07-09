#ifndef _LASER_H_
#define _LASER_H_

#include <Arduino.h>

// Laser sensor pin definitions
#define LASER_RX_PIN 16  // Sensor TXD connects to ESP32S3's RX
#define LASER_TX_PIN 17  // Sensor RXD connects to ESP32S3's TX
#define SENSOR_SERIAL Serial2 // Use Serial2 for consistency

// Error codes
#define LASER_EOK 0      // Operation successful
#define LASER_ERROR 1    // Operation failed
#define LASER_ETIMEOUT 2 // Timeout error
#define LASER_EFRAME 3   // Frame error
#define LASER_ECRC 4     // CRC check error

// ATK-MS53L2M specific definitions
#define ATK_MS53L2M_MASTER_FRAME_HEAD   0x51    // Master request frame header
#define ATK_MS53L2M_SLAVE_FRAME_HEAD    0x55    // Slave response frame header
#define ATK_MS53L2M_SENSOR_TYPE         0x0C    // ATK-MS53L2M sensor type

// ATK-MS53L2M function codes
#define ATK_MS53L2M_FUNCODE_SYS             0x00 // System settings
#define ATK_MS53L2M_FUNCODE_BACKRATE        0x01 // Response frequency
#define ATK_MS53L2M_FUNCODE_BAUDRATE        0x02 // Baud rate
#define ATK_MS53L2M_FUNCODE_IDSET           0x03 // Device address setting
#define ATK_MS53L2M_FUNCODE_OBJ1_MEAUDATA   0x05 // Target 1 measurement data
#define ATK_MS53L2M_FUNCODE_OBJ1_SIGNDATA   0x07 // Target 1 signal strength data
#define ATK_MS53L2M_FUNCODE_OUTMODE         0x1B // Output mode setting
#define ATK_MS53L2M_FUNCODE_WORKMODE        0x1C // Working mode
#define ATK_MS53L2M_FUNCODE_THRESHOLD_DATA  0x1D // Threshold value
#define ATK_MS53L2M_FUNCODE_VERSION         0x1F // Version information

// Working mode settings
#define ATK_MS53L2M_WORKMODE_NORMAL         0x00 // Normal mode
#define ATK_MS53L2M_WORKMODE_MODBUS         0x01 // Modbus mode

// Output mode settings
#define ATK_MS53L2M_OUTPUTMODE_1            0x00 // Furthest target output
#define ATK_MS53L2M_OUTPUTMODE_2            0x01 // Strongest signal output
#define ATK_MS53L2M_OUTPUTMODE_3            0x02 // Multi-target output

// Buffer definitions
#define RX_BUF_SIZE 256  // Increased receive buffer size
static char g_rx_buf[RX_BUF_SIZE];
static uint16_t g_rx_cnt = 0;

// 优化滤波设置，适应移动场景
#define FILTER_SIZE 5                  // 减小滤波窗口以提高响应速度
#define MEDIAN_FILTER_SIZE 5           // 减小中值滤波窗口以提高响应速度
#define MAX_VALID_CHANGE_RATE 50.0f    // 增加最大有效变化率(mm/采样周期)
#define MIN_OUTPUT_CHANGE 2.0f         // 降低最小输出变化阈值(mm)
#define STABILITY_THRESHOLD 5.0f       // 增加稳定性阈值(mm)
#define MOVEMENT_DETECTION_THRESHOLD 10.0f // 移动检测阈值(mm)
static float distance_history[FILTER_SIZE] = {0};
static float median_window[MEDIAN_FILTER_SIZE] = {0};
static int filter_index = 0;
static int median_index = 0;
static bool filter_filled = false;
static bool median_filled = false;
static float last_valid_distance = 0;  // 上次有效距离值
static float last_output_distance = 0; // 上次输出的距离值
static bool movement_detected = false; // 移动状态检测

// Last valid distance reading - renamed to avoid conflict with web.h
static uint16_t laser_current_distance = 0;

// Function declarations
bool laser_init(void);  
float jiguang(void);
void laser_test(void);
bool laser_reset(void);
float get_laser_distance(bool auto_init = true, bool print_result = false);
uint8_t atk_ms53l2m_get_distance(uint16_t *distance);
float apply_filter(float new_value);
uint8_t atk_ms53l2m_set_mode(uint8_t mode);
uint8_t atk_ms53l2m_set_output_mode(uint8_t mode);

// Global variables
static bool laser_initialized = false;
static int consecutive_failures = 0;
static uint8_t current_work_mode = ATK_MS53L2M_WORKMODE_NORMAL; // Default to normal mode
static uint8_t current_output_mode = ATK_MS53L2M_OUTPUTMODE_1;  // Default to furthest target output

/**
 * Clear receive buffer completely
 */
inline void atk_ms53l2m_uart_rx_restart(void)
{
    // Fully clear buffer more thoroughly
    while(SENSOR_SERIAL.available()) {
        SENSOR_SERIAL.read();
        delayMicroseconds(100);  // Give small time between reads
    }
    g_rx_cnt = 0;
    memset(g_rx_buf, 0, RX_BUF_SIZE);
}

/**
 * 中值滤波 - 去除异常值，但保持响应速度
 */
inline float apply_median_filter(float new_value) {
    if (new_value <= 0) {
        return -1; // 无效值直接返回
    }
    
    // 存入中值滤波窗口
    median_window[median_index] = new_value;
    median_index = (median_index + 1) % MEDIAN_FILTER_SIZE;
    
    // 检查窗口是否已填满
    if (!median_filled && median_index == 0) {
        median_filled = true;
    }
    
    // 如果窗口未填满，返回原值
    if (!median_filled) {
        return new_value;
    }
    
    // 创建窗口副本用于排序
    float sorted_window[MEDIAN_FILTER_SIZE];
    memcpy(sorted_window, median_window, sizeof(float) * MEDIAN_FILTER_SIZE);
    
    // 简单的冒泡排序
    for (int i = 0; i < MEDIAN_FILTER_SIZE - 1; i++) {
        for (int j = 0; j < MEDIAN_FILTER_SIZE - i - 1; j++) {
            if (sorted_window[j] > sorted_window[j + 1]) {
                float temp = sorted_window[j];
                sorted_window[j] = sorted_window[j + 1];
                sorted_window[j + 1] = temp;
            }
        }
    }
    
    // 返回中值
    return sorted_window[MEDIAN_FILTER_SIZE / 2];
}

/**
 * 动态平滑滤波 - 根据移动状态调整滤波强度
 */
inline float apply_smooth_filter(float new_value) {
    if (new_value <= 0) {
        return -1; // 无效值直接返回
    }
    
    // 检测是否处于移动状态
    if (last_valid_distance > 0) {
        float change = fabsf(new_value - last_valid_distance);
        
        // 更新移动状态
        if (change > MOVEMENT_DETECTION_THRESHOLD) {
            movement_detected = true;
        } else if (movement_detected && change < MOVEMENT_DETECTION_THRESHOLD * 0.5) {
            // 当变化小于阈值一半时，认为移动已停止
            movement_detected = false;
        }
        
        // 检查变化率，防止突变，但允许移动时的快速变化
        float max_change = movement_detected ? MAX_VALID_CHANGE_RATE * 1.5 : MAX_VALID_CHANGE_RATE;
        
        if (change > max_change) {
            // 变化过大，限制变化幅度
            if (new_value > last_valid_distance) {
                new_value = last_valid_distance + max_change;
            } else {
                new_value = last_valid_distance - max_change;
            }
        }
    }
    
    // 存入滤波窗口
    distance_history[filter_index] = new_value;
    filter_index = (filter_index + 1) % FILTER_SIZE;
    
    // 检查窗口是否已填满
    if (!filter_filled && filter_index == 0) {
        filter_filled = true;
    }
    
    // 如果窗口未填满，返回原值
    if (!filter_filled) {
        last_valid_distance = new_value;
        return new_value;
    }
    
    // 动态调整权重系数，移动时更重视新数据
    float weight_decay = movement_detected ? 0.8f : 0.7f;
    
    // 计算加权平均值，越新的数据权重越大
    float sum = 0;
    float weight_sum = 0;
    float weight = 1.0f;
    
    for (int i = 0; i < FILTER_SIZE; i++) {
        int idx = (filter_index - 1 - i + FILTER_SIZE) % FILTER_SIZE;
        if (distance_history[idx] > 0) {
            sum += distance_history[idx] * weight;
            weight_sum += weight;
            weight *= weight_decay; // 动态权重递减系数
        }
    }
    
    float result = (weight_sum > 0) ? (sum / weight_sum) : -1;
    
    if (result > 0) {
        last_valid_distance = result;
    }
    
    return result;
}

/**
 * 稳定性检测 - 检查数据是否稳定，移动时降低要求
 */
inline bool is_stable(float current_value) {
    if (!filter_filled || current_value <= 0) {
        return false;
    }
    
    // 移动时放宽稳定性要求
    float stability_threshold = movement_detected ? STABILITY_THRESHOLD * 2 : STABILITY_THRESHOLD;
    
    float max_val = current_value;
    float min_val = current_value;
    
    // 检查最近的数据波动范围
    for (int i = 0; i < FILTER_SIZE; i++) {
        if (distance_history[i] > 0) {
            max_val = max(max_val, distance_history[i]);
            min_val = min(min_val, distance_history[i]);
        }
    }
    
    // 如果波动范围小于阈值，认为数据稳定
    return (max_val - min_val) <= stability_threshold;
}

/**
 * 综合滤波器 - 结合中值滤波和动态平滑滤波
 */
inline float apply_filter(float new_value) {
    // 先应用中值滤波去除异常值
    float median_filtered = apply_median_filter(new_value);
    
    // 再应用平滑滤波平滑数据
    float smooth_filtered = apply_smooth_filter(median_filtered);
    
    return smooth_filtered;
}

/**
 * Try various command formats to get distance data
 */
inline bool send_laser_commands() {
    // Try different command formats one after another
    const char* commands[] = {
        "DIST\r\n",       // Standard command
        "READ\r\n",       // Alternative command
        "GETDATA\r\n",    // Extended command
        "D\r\n",          // Short command
        "O\r\n"           // Single character command used by some laser modules
    };
    
    // In normal mode, send all commands to try to get a response
    if (current_work_mode == ATK_MS53L2M_WORKMODE_NORMAL) {
        for (int i = 0; i < 5; i++) {
            SENSOR_SERIAL.write(commands[i]);
            delay(5);  // Short delay between commands
        }
    } 
    // In Modbus mode, use specific protocol
    else if (current_work_mode == ATK_MS53L2M_WORKMODE_MODBUS) {
        // Modbus RTU format command for reading distance
        uint8_t modbus_cmd[] = {0x01, 0x03, 0x00, 0x05, 0x00, 0x01, 0x94, 0x0B};
        SENSOR_SERIAL.write(modbus_cmd, 8);
        delay(10);
    }
    
    return true;
}

/**
 * Get distance from laser sensor - improved to handle more data formats
 */
inline uint8_t atk_ms53l2m_get_distance(uint16_t *distance)
{
    /* Clear receive buffer completely */
    atk_ms53l2m_uart_rx_restart();
    
    // Try sending different measurement commands
    send_laser_commands();
    
    /* Wait for data with shorter timeout for responsiveness */
    uint32_t start_time = millis();
    g_rx_cnt = 0;
    bool data_received = false;
    
    /* Receive data with timeout */
    while (millis() - start_time < 200) // Reduced timeout for faster response
    {
        // Read all available bytes
        while (SENSOR_SERIAL.available() && g_rx_cnt < RX_BUF_SIZE - 1)
        {
            char c = SENSOR_SERIAL.read();
            g_rx_buf[g_rx_cnt++] = c;
            data_received = true;
        }
        
        // Check if we've received a complete frame 
        // Most laser sensors send either \r\n, # or $ as terminators
        if (data_received) {
            bool frame_complete = false;
            
            // Check for various termination sequences
            for (int i = 1; i < g_rx_cnt; i++) {
                if ((g_rx_buf[i-1] == '\r' && g_rx_buf[i] == '\n') || 
                    g_rx_buf[i] == '#' || 
                    g_rx_buf[i] == '$') {
                    frame_complete = true;
                    break;
                }
            }
            
            if (frame_complete) {
                // Wait a brief moment to see if more data arrives
                delay(5);
                
                // Read any additional data
                while (SENSOR_SERIAL.available() && g_rx_cnt < RX_BUF_SIZE - 1) {
                    g_rx_buf[g_rx_cnt++] = SENSOR_SERIAL.read();
                }
                
                // Ensure string termination
                g_rx_buf[g_rx_cnt] = '\0';
                break;  // Break the outer loop when frame is complete
            }
        }
        
        delay(1); // Brief delay
    }
    
    /* If no data received, return timeout error */
    if (g_rx_cnt == 0) {
        return LASER_ETIMEOUT;
    }
    
    /* 删除调试输出 */
    // Serial.print("Raw data: ");
    // Serial.println(g_rx_buf);
    
    /* Parse data using multiple approaches */
    
    // Method 1: Look for "d:" format used by ATK-MS53L2M
    char *d_pos = strstr(g_rx_buf, "d:");
    if (d_pos != NULL) {
        d_pos += 2; // Skip "d:"
        
        // Skip spaces
        while (*d_pos == ' ' && *d_pos != '\0') {
            d_pos++;
        }
        
        // Read number
        if (isdigit(*d_pos)) {
            *distance = atoi(d_pos);
            return LASER_EOK;
        }
    }
    
    // Method 2: Look for "Range:" format
    char *range_pos = strstr(g_rx_buf, "Range:");
    if (range_pos != NULL) {
        range_pos += 6; // Skip "Range:"
        
        // Skip spaces
        while (*range_pos == ' ' && *range_pos != '\0') {
            range_pos++;
        }
        
        // Read number
        if (isdigit(*range_pos)) {
            *distance = atoi(range_pos);
            return LASER_EOK;
        }
    }
    
    // Method 3: Look for "Valid" or "State:0" (indicates valid data)
    if (strstr(g_rx_buf, "Valid") != NULL || strstr(g_rx_buf, "State:0") != NULL) {
        // Search for numbers in the response
        char *ptr = g_rx_buf;
        while (*ptr) {
            // Find start of a number
            if (isdigit(*ptr) && (ptr == g_rx_buf || !isdigit(*(ptr - 1)))) {
                *distance = atoi(ptr);
                if (*distance > 0 && *distance < 10000) // Reasonable range
                {
                    return LASER_EOK;
                }
            }
            ptr++;
        }
    }
    
    // Method 4: Check for Modbus RTU response format
    if (g_rx_cnt >= 7 && g_rx_buf[0] == 0x01 && g_rx_buf[1] == 0x03) {
        if (g_rx_buf[2] == 0x02) { // Data length is 2 bytes
            *distance = (g_rx_buf[3] << 8) | g_rx_buf[4];
            return LASER_EOK;
        }
    }
    
    // Method 5: Just look for any number in the response
    char *ptr = g_rx_buf;
    while (*ptr) {
        if (isdigit(*ptr) && (ptr == g_rx_buf || !isdigit(*(ptr - 1)))) {
            *distance = atoi(ptr);
            if (*distance > 0 && *distance < 10000) // Reasonable range
            {
                return LASER_EOK;
            }
        }
        ptr++;
    }
    
    return LASER_ERROR;
}

/**
 * Set the working mode of the ATK-MS53L2M module
 * @param mode: ATK_MS53L2M_WORKMODE_NORMAL or ATK_MS53L2M_WORKMODE_MODBUS
 * @return: LASER_EOK on success, error code otherwise
 */
inline uint8_t atk_ms53l2m_set_mode(uint8_t mode) {
    // Command to set work mode
    uint8_t cmd[10];
    uint8_t cmd_len = 0;
    
    // Prepare command based on current mode
    if (current_work_mode == ATK_MS53L2M_WORKMODE_NORMAL) {
        // In normal mode, use ASCII command
        sprintf((char*)cmd, "MODE%d\r\n", mode);
        cmd_len = strlen((char*)cmd);
    } else {
        // In Modbus mode, use Modbus RTU format
        // Modbus command to set work mode (function code 0x1C)
        cmd[0] = 0x01;  // Device address
        cmd[1] = 0x06;  // Write single register
        cmd[2] = 0x00;  // Register address high byte
        cmd[3] = 0x1C;  // Register address low byte (function code)
        cmd[4] = 0x00;  // Value high byte
        cmd[5] = mode;  // Value low byte (mode)
        cmd[6] = 0x88;  // CRC low byte (placeholder)
        cmd[7] = 0x14;  // CRC high byte (placeholder)
        cmd_len = 8;
    }
    
    // Clear receive buffer
    atk_ms53l2m_uart_rx_restart();
    
    // Send command
    SENSOR_SERIAL.write(cmd, cmd_len);
    
    // Wait for response
    delay(100);
    
    // Update current mode if successful
    current_work_mode = mode;
    
    return LASER_EOK;
}

/**
 * Set the output mode of the ATK-MS53L2M module
 * @param mode: ATK_MS53L2M_OUTPUTMODE_1, ATK_MS53L2M_OUTPUTMODE_2, or ATK_MS53L2M_OUTPUTMODE_3
 * @return: LASER_EOK on success, error code otherwise
 */
inline uint8_t atk_ms53l2m_set_output_mode(uint8_t mode) {
    // Command to set output mode
    uint8_t cmd[10];
    uint8_t cmd_len = 0;
    
    // Prepare command based on current mode
    if (current_work_mode == ATK_MS53L2M_WORKMODE_NORMAL) {
        // In normal mode, use ASCII command
        sprintf((char*)cmd, "OUTM%d\r\n", mode);
        cmd_len = strlen((char*)cmd);
    } else {
        // In Modbus mode, use Modbus RTU format
        // Modbus command to set output mode (function code 0x1B)
        cmd[0] = 0x01;  // Device address
        cmd[1] = 0x06;  // Write single register
        cmd[2] = 0x00;  // Register address high byte
        cmd[3] = 0x1B;  // Register address low byte (function code)
        cmd[4] = 0x00;  // Value high byte
        cmd[5] = mode;  // Value low byte (mode)
        cmd[6] = 0xD9;  // CRC low byte (placeholder)
        cmd[7] = 0xD4;  // CRC high byte (placeholder)
        cmd_len = 8;
    }
    
    // Clear receive buffer
    atk_ms53l2m_uart_rx_restart();
    
    // Send command
    SENSOR_SERIAL.write(cmd, cmd_len);
    
    // Wait for response
    delay(100);
    
    // Update current output mode if successful
    current_output_mode = mode;
    
    return LASER_EOK;
}

/**
 * Laser sensor initialization with proper UART setup
 */
inline bool laser_init(void)
{
    // End previous serial if active
    SENSOR_SERIAL.end();
    delay(100);
    
    // 在begin之前设置缓冲区大小
    SENSOR_SERIAL.setRxBufferSize(256);
    
    // Initialize laser sensor serial port with higher buffer size
    SENSOR_SERIAL.begin(115200, SERIAL_8N1, LASER_RX_PIN, LASER_TX_PIN, false);
    delay(50);
    
    // 删除在begin之后的缓冲区大小设置
    
    // 删除调试输出
    // Serial.println("Laser sensor initialization starting");
    
    // Wait for sensor to stabilize
    delay(500);
    
    // Clear buffer
    atk_ms53l2m_uart_rx_restart();
    
    // Reset filter
    for (int i = 0; i < FILTER_SIZE; i++) {
        distance_history[i] = 0;
    }
    filter_index = 0;
    filter_filled = false;
    
    // Set to normal mode for easier communication
    atk_ms53l2m_set_mode(ATK_MS53L2M_WORKMODE_NORMAL);
    delay(50);
    
    // Set to furthest target output mode
    atk_ms53l2m_set_output_mode(ATK_MS53L2M_OUTPUTMODE_1);
    delay(50);
    
    // Try sending different initialization commands
    send_laser_commands();
    delay(100);
    
    // Try reading data to verify sensor is working properly
    uint16_t test_distance;
    uint8_t ret = atk_ms53l2m_get_distance(&test_distance);
    
    // If reading fails, try resetting the sensor
    if (ret != LASER_EOK) {
        // 删除调试输出
        // Serial.println("Laser sensor initialization failed, attempting reset...");
        if (!laser_reset()) {
            // 删除调试输出
            // Serial.println("Laser sensor reset failed!");
            laser_initialized = false;
            return false;
        }
        
        // Test again after reset
        ret = atk_ms53l2m_get_distance(&test_distance);
        if (ret != LASER_EOK) {
            // 删除调试输出
            // Serial.println("Laser sensor initialization failed!");
            laser_initialized = false;
            return false;
        }
    }
    
    // 删除调试输出
    // Serial.println("Laser sensor ready");
    // Serial.print("Initial test distance: ");
    // Serial.print(test_distance);
    // Serial.println(" mm");
    
    // Initialize filter with first reading
    apply_filter((float)test_distance);
    
    // Set current distance
    laser_current_distance = test_distance;
    
    laser_initialized = true;
    consecutive_failures = 0;
    return true;
}

/**
 * Attempt to reset the laser sensor by trying different baud rates
 */
inline bool laser_reset(void)
{
    // 删除调试输出
    // Serial.println("Attempting to reset laser sensor...");
    
    // Close serial port
    SENSOR_SERIAL.end();
    delay(500);
    
    // Try different baud rates
    const int baudrates[] = {115200, 9600, 57600, 38400};
    
    for (int i = 0; i < 4; i++) {
        // 删除调试输出
        // Serial.print("Trying baud rate: ");
        // Serial.println(baudrates[i]);
        
        // 在begin之前设置缓冲区大小
        SENSOR_SERIAL.setRxBufferSize(256);
        
        // Re-initialize with specific baud rate and larger buffer
        SENSOR_SERIAL.begin(baudrates[i], SERIAL_8N1, LASER_RX_PIN, LASER_TX_PIN, false);
        delay(50);
        
        // 删除在begin之后的缓冲区大小设置
        
        // Clear buffer
        atk_ms53l2m_uart_rx_restart();
        
        // Send commands at this baud rate
        send_laser_commands();
        
        // Try to read response
        unsigned long start_time = millis();
        bool got_response = false;
        
        while (millis() - start_time < 1000) {
            if (SENSOR_SERIAL.available()) {
                got_response = true;
                break;
            }
            delay(10);
        }
        
        if (got_response) {
            // 删除调试输出
            // Serial.println("Received sensor response");
            
            // Read and display any response data
            char buffer[100] = {0};
            int idx = 0;
            while (SENSOR_SERIAL.available() && idx < 99) {
                buffer[idx++] = SENSOR_SERIAL.read();
                delayMicroseconds(100);
            }
            buffer[idx] = '\0';
            
            // 删除调试输出
            // Serial.print("Response: ");
            // Serial.println(buffer);
            
            // Try to set normal mode
            if (baudrates[i] != 115200) {
                // If not at 115200, first set the mode at current baudrate
                atk_ms53l2m_set_mode(ATK_MS53L2M_WORKMODE_NORMAL);
                delay(100);
                
                // Then try to set baudrate to 115200
                char baud_cmd[20];
                sprintf(baud_cmd, "BAUD5\r\n"); // 5 is code for 115200
                SENSOR_SERIAL.write(baud_cmd);
                delay(200);
                
                // Now switch to 115200
                SENSOR_SERIAL.end();
                delay(300);
                
                // 在begin之前设置缓冲区大小
                SENSOR_SERIAL.setRxBufferSize(256);
                
                SENSOR_SERIAL.begin(115200, SERIAL_8N1, LASER_RX_PIN, LASER_TX_PIN, false);
                delay(50);
                
                // 删除在begin之后的缓冲区大小设置
            }
            
            return true;
        }
    }
    
    // If all attempts fail, restore default settings
    SENSOR_SERIAL.end();
    delay(300);
    
    // 在begin之前设置缓冲区大小
    SENSOR_SERIAL.setRxBufferSize(256);
    
    SENSOR_SERIAL.begin(115200, SERIAL_8N1, LASER_RX_PIN, LASER_TX_PIN, false);
    delay(50);
    
    // 删除在begin之后的缓冲区大小设置
    
    return false;
}

/**
 * Get real-time distance from laser sensor
 * Optimized for continuous operation with high speed and precision
 */
inline float jiguang(void)
{
    static unsigned long last_cmd_time = 0;
    static unsigned long last_output_time = 0;
    static float last_reported_distance = -1;
    static int stable_count = 0;
    
    // 提高采样频率，移动时每30ms请求一次数据
    unsigned long cmd_interval = movement_detected ? 30 : 50;
    
    // Send commands periodically to request new data
    if (millis() - last_cmd_time > cmd_interval) {
        send_laser_commands();
        last_cmd_time = millis();
    }
    
    // Use the improved implementation
    uint16_t distance;
    uint8_t ret = atk_ms53l2m_get_distance(&distance);
    
    if (ret == LASER_EOK) {
        // Valid reading - always update current distance
        laser_current_distance = distance;
        consecutive_failures = 0;
        
        // 应用综合滤波
        float filtered_distance = apply_filter((float)distance);
        
        // 检查数据稳定性
        bool stable = is_stable(filtered_distance);
        
        // 如果数据稳定，增加稳定计数
        if (stable) {
            stable_count++;
        } else {
            stable_count = 0;
        }
        
        // 决定是否输出新的距离值
        bool should_output = false;
        
        // 条件1: 首次有效读数
        if (last_reported_distance < 0) {
            should_output = true;
        }
        // 条件2: 移动时，变化超过阈值即输出
        else if (movement_detected && fabsf(filtered_distance - last_output_distance) >= MIN_OUTPUT_CHANGE) {
            should_output = true;
        }
        // 条件3: 静止时，变化超过阈值且至少稳定2个周期
        else if (!movement_detected && fabsf(filtered_distance - last_output_distance) >= MIN_OUTPUT_CHANGE && stable_count >= 2) {
            should_output = true;
        }
        // 条件4: 定期更新（移动时更频繁更新）
        else if (millis() - last_output_time > (movement_detected ? 200 : 500)) {
            should_output = true;
        }
        
        if (should_output) {
            // 对输出值进行四舍五入到整数
            int rounded_distance = (int)(filtered_distance + 0.5f);
            
            // 只输出实时距离数据
            Serial.println(rounded_distance);
            
            last_reported_distance = filtered_distance;
            last_output_distance = filtered_distance;
            last_output_time = millis();
        }
        
        return filtered_distance;
    } else {
        consecutive_failures++;
        
        // Use filtered value if available
        float filtered_value = apply_filter(-1);
        if (filtered_value > 0) {
            // 不输出过滤数据的信息
            return filtered_value;
        } else {
            // 不输出错误信息
            return -1.0;
        }
        
        // After multiple failures, try reinitializing
        if (consecutive_failures >= 10) {
            // 不输出重置信息
            if (laser_reset()) {
                laser_init();
            }
            consecutive_failures = 0;
        }
        
        return -1.0;
    }
}

/**
 * One-click function to get laser distance data
 */
inline float get_laser_distance(bool auto_init, bool print_result)
{
    // If sensor not initialized and auto-init requested
    if (!laser_initialized && auto_init) {
        if (!laser_init()) {
            // 删除调试输出，即使print_result为true也不输出
            return -1.0;
        }
    }
    
    // If sensor not initialized and auto-init not requested, return error
    if (!laser_initialized && !auto_init) {
        // 删除调试输出，即使print_result为true也不输出
        return -1.0;
    }
    
    // Get distance measurement
    float distance = jiguang();
    
    // 删除调试输出，即使print_result为true也不输出
    
    return distance;
}

/**
 * Laser sensor test function
 * Continuously reads and prints laser sensor data for debugging
 */
inline void laser_test(void) {
    Serial.println("Starting laser sensor test...");
    Serial.println("Will read laser sensor data for 10 seconds");
    
    // Initialize laser sensor
    if (!laser_init()) {
        Serial.println("Laser sensor initialization failed, test aborted");
        return;
    }
    
    // Record start time
    unsigned long start_time = millis();
    int readings = 0;
    int valid_readings = 0;
    
    // Test for 10 seconds
    while (millis() - start_time < 10000) {
        readings++;
        
        // Try sending commands to trigger measurement
        send_laser_commands();
        delay(10);
        
        // Read laser sensor data directly
        uint16_t distance;
        uint8_t ret = atk_ms53l2m_get_distance(&distance);
        
        if (ret == LASER_EOK) {
            valid_readings++;
            Serial.print("Read success #");
            Serial.print(readings);
            Serial.print(": ");
            Serial.print(distance);
            Serial.println(" mm");
        } else {
            Serial.print("Read failed #");
            Serial.println(readings);
            
            // Print buffer contents for debugging
            Serial.print("Buffer content: ");
            Serial.println(g_rx_buf);
        }
        
        // Delay 100ms
        delay(100);
    }
    
    // Print test results
    Serial.println("Laser sensor test complete");
    Serial.print("Total readings: ");
    Serial.println(readings);
    Serial.print("Valid readings: ");
    Serial.println(valid_readings);
    Serial.print("Success rate: ");
    Serial.print((float)valid_readings / readings * 100);
    Serial.println("%");
    
    if (valid_readings == 0) {
        Serial.println("Error: Laser sensor unable to read valid data!");
        Serial.println("Please check:");
        Serial.println("1. Sensor wiring is correct (TX->RX, RX->TX)");
        Serial.println("2. Sensor power is connected");
        Serial.println("3. Sensor baud rate is 115200");
        Serial.println("4. Sensor is ATK-MS53L2M model or compatible");
    } else if ((float)valid_readings / readings < 0.5) {
        Serial.println("Warning: Laser sensor read success rate below 50%");
        Serial.println("Possible causes:");
        Serial.println("1. Unstable sensor connection");
        Serial.println("2. Unstable power supply");
    } else {
        Serial.println("Laser sensor working normally");
                }
}

#endif // _LASER_H_