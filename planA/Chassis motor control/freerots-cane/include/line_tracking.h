#ifndef _LINE_TRACKING_H_
#define _LINE_TRACKING_H_

#include <Arduino.h>
#include "chassis.h"

// 定义传感器引脚
#define SENSOR_R2 48  // 最右侧传感器
#define SENSOR_R1 47  // 右侧传感器
#define SENSOR_M  21  // 中间传感器
#define SENSOR_L1 20   // 左侧传感器
#define SENSOR_L2 19   // 最左侧传感器

// 传感器阈值，可根据实际情况调整
#define DEFAULT_THRESHOLD 150

// 传感器状态
typedef struct {
    uint16_t raw_values[5];   // 原始模拟值
    uint8_t digital_values[5]; // 数字值(0或1)
    uint16_t thresholds[5];    // 阈值
} LineSensor;

// 全局传感器对象
extern LineSensor line_sensor;

// 函数声明
void line_sensor_init();
void line_sensor_read();
void line_sensor_print();
void line_tracking_control(int speed = DEFAULT_SPEED);
void set_sensor_threshold(int sensor_index, uint16_t threshold);
void set_all_thresholds(uint16_t threshold);
bool is_on_line();

/**
 * 初始化循迹传感器
 */
inline void line_sensor_init() {
    // 设置传感器引脚为输入模式
    pinMode(SENSOR_R2, INPUT);
    pinMode(SENSOR_R1, INPUT);
    pinMode(SENSOR_M, INPUT);
    pinMode(SENSOR_L1, INPUT);
    pinMode(SENSOR_L2, INPUT);
    
    // 设置默认阈值
    for (int i = 0; i < 5; i++) {
        line_sensor.thresholds[i] = DEFAULT_THRESHOLD;
    }
    
    Serial.println("循迹传感器初始化完成");
}

/**
 * 读取所有传感器的值
 */
inline void line_sensor_read() {
    // 读取原始模拟值
    line_sensor.raw_values[0] = analogRead(SENSOR_R2);
    line_sensor.raw_values[1] = analogRead(SENSOR_R1);
    line_sensor.raw_values[2] = analogRead(SENSOR_M);
    line_sensor.raw_values[3] = analogRead(SENSOR_L1);
    line_sensor.raw_values[4] = analogRead(SENSOR_L2);
    
    // 转换为数字值
    for (int i = 0; i < 5; i++) {
        line_sensor.digital_values[i] = (line_sensor.raw_values[i] > line_sensor.thresholds[i]) ? 1 : 0;
    }
}

/**
 * 打印传感器状态
 */
inline void line_sensor_print() {
    Serial.println("传感器状态:");
    for (int i = 0; i < 5; i++) {
        Serial.print("传感器");
        Serial.print(i);
        Serial.print(": 模拟值=");
        Serial.print(line_sensor.raw_values[i]);
        Serial.print(", 数字值=");
        Serial.println(line_sensor.digital_values[i]);
    }
    Serial.println("----");
}

/**
 * 设置单个传感器的阈值
 */
inline void set_sensor_threshold(int sensor_index, uint16_t threshold) {
    if (sensor_index >= 0 && sensor_index < 5) {
        line_sensor.thresholds[sensor_index] = threshold;
    }
}

/**
 * 设置所有传感器的阈值
 */
inline void set_all_thresholds(uint16_t threshold) {
    for (int i = 0; i < 5; i++) {
        line_sensor.thresholds[i] = threshold;
    }
}

/**
 * 检查是否检测到线
 */
inline bool is_on_line() {
    for (int i = 0; i < 5; i++) {
        if (line_sensor.digital_values[i] == 1) {
            return true;
        }
    }
    return false;
}

/**
 * 根据传感器值控制底盘运动
 * @param speed 底盘运动速度
 */
inline void line_tracking_control(int speed) {
    // 首先读取传感器值
    line_sensor_read();
    
    // 获取传感器状态
    uint8_t r2 = line_sensor.digital_values[0];
    uint8_t r1 = line_sensor.digital_values[1];
    uint8_t m  = line_sensor.digital_values[2];
    uint8_t l1 = line_sensor.digital_values[3];
    uint8_t l2 = line_sensor.digital_values[4];
    
    // 根据传感器状态控制底盘
    if (m == 1 && r1 == 0 && l1 == 0) {
        // 中间传感器检测到线，直行
        control_chassis_raw(CHASSIS_FORWARD, DEFAULT_DISTANCE, speed);
        Serial.println("循迹：直行");
    }
    else if ((r1 == 1 || r2 == 1) && l1 == 0 && l2 == 0) {
        // 右侧传感器检测到线，向右修正
        control_chassis_raw(CHASSIS_RIGHT, DEFAULT_DISTANCE/4, speed);
        Serial.println("循迹：右转");
    }
    else if ((l1 == 1 || l2 == 1) && r1 == 0 && r2 == 0) {
        // 左侧传感器检测到线，向左修正
        control_chassis_raw(CHASSIS_LEFT, DEFAULT_DISTANCE/4, speed);
        Serial.println("循迹：左转");
    }
    else if (r2 == 1 && r1 == 1 && l1 == 0 && l2 == 0) {
        // 右侧两个传感器都检测到，大幅度右转
        control_chassis_raw(CHASSIS_RIGHT, DEFAULT_DISTANCE/2, speed);
        Serial.println("循迹：大幅右转");
    }
    else if (l2 == 1 && l1 == 1 && r1 == 0 && r2 == 0) {
        // 左侧两个传感器都检测到，大幅度左转
        control_chassis_raw(CHASSIS_LEFT, DEFAULT_DISTANCE/2, speed);
        Serial.println("循迹：大幅左转");
    }
    else if (r1 == 0 && r2 == 0 && m == 0 && l1 == 0 && l2 == 0) {
        // 所有传感器都没检测到，停止
        control_chassis_raw(CHASSIS_STOP, 0, 0);
        Serial.println("循迹：未检测到线，停止");
    }
    else {
        // 其他情况，默认直行
        control_chassis_raw(CHASSIS_FORWARD, DEFAULT_DISTANCE/2, speed);
        Serial.println("循迹：默认直行");
    }
    
    // 打印当前传感器状态
    line_sensor_print();
}

// 在.cpp文件中实现全局传感器对象
#ifndef LINE_TRACKING_CPP
LineSensor line_sensor = {
    .raw_values = {0, 0, 0, 0, 0},
    .digital_values = {0, 0, 0, 0, 0},
    .thresholds = {DEFAULT_THRESHOLD, DEFAULT_THRESHOLD, DEFAULT_THRESHOLD, DEFAULT_THRESHOLD, DEFAULT_THRESHOLD}
};
#endif

#endif // _LINE_TRACKING_H_ 