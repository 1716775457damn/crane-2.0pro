#ifndef _CHASSIS_H_
#define _CHASSIS_H_

#include <Arduino.h>
#include "pins.h"  // Import pin definitions

// 底盘控制命令定义
#define CHASSIS_FORWARD  5  // 前进
#define CHASSIS_BACKWARD 6  // 后退
#define CHASSIS_LEFT     8  // 左转
#define CHASSIS_RIGHT    7  // 右转
#define CHASSIS_STOP     0  // 停止

// 方向枚举，使代码更易读
enum ChassisDirection {
    STOP = 0,
    FORWARD = 1,  // 对应命令5
    BACKWARD = 2, // 对应命令6
    LEFT = 3,     // 对应命令8
    RIGHT = 4     // 对应命令7
};

// 底盘控制串口
#define CHASSIS_SERIAL Serial1

/**
 * Initialize chassis control serial port
 */
inline void chassis_serial_init(void)
{
    // 初始化底盘控制串口（只使用TX引脚）
    CHASSIS_SERIAL.begin(CHASSIS_BAUD_RATE, SERIAL_8N1, -1, CHASSIS_SERIAL_TX);
    delay(100);
    Serial.println("Chassis control serial port initialized");
    Serial.println("TX: " + String(CHASSIS_SERIAL_TX) + " (RX disabled)");
}

/**
 * Send chassis control command
 * @param command Command string
 */
inline void send_chassis_command(const char* command)
{
    // 打印十六进制格式的命令
    Serial.print("Chassis command (HEX): ");
    for (int i = 0; i < strlen(command); i++) {
        Serial.print(command[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    
    // 尝试不同的结束符
    Serial.println("Sending with \\n");
    CHASSIS_SERIAL.print(command);
    CHASSIS_SERIAL.print('\n');
    delay(100);
    
    Serial.println("Sending with \\r");
    CHASSIS_SERIAL.print(command);
    CHASSIS_SERIAL.print('\r');
    delay(100);
    
    Serial.println("Sending with \\r\\n");
    CHASSIS_SERIAL.print(command);
    CHASSIS_SERIAL.print('\r');
    CHASSIS_SERIAL.print('\n');
    delay(100);
    
    Serial.println("Chassis command sent: " + String(command));
}

/**
 * Raw chassis control function
 * @param direction Direction: CHASSIS_FORWARD, CHASSIS_BACKWARD, CHASSIS_LEFT, CHASSIS_RIGHT, CHASSIS_STOP
 * @param speed Speed value
 * @param duration Duration
 */
inline void control_chassis_raw(uint8_t direction, uint16_t speed, uint8_t duration)
{
    char direction_str[10] = "UNKNOWN";
    switch(direction) {
        case CHASSIS_FORWARD: strcpy(direction_str, "FORWARD"); break;
        case CHASSIS_BACKWARD: strcpy(direction_str, "BACKWARD"); break;
        case CHASSIS_LEFT: strcpy(direction_str, "LEFT"); break;
        case CHASSIS_RIGHT: strcpy(direction_str, "RIGHT"); break;
        case CHASSIS_STOP: strcpy(direction_str, "STOP"); break;
    }
    
    // 尝试不同的命令格式
    
    // 格式1: CHxyz
    String cmd1 = "CH" + String(direction) + String(speed) + String(duration);
    Serial.println("Trying command format 1: " + cmd1);
    send_chassis_command(cmd1.c_str());
    delay(500);
    
    // 格式2: CH,x,y,z
    String cmd2 = "CH," + String(direction) + "," + String(speed) + "," + String(duration);
    Serial.println("Trying command format 2: " + cmd2);
    send_chassis_command(cmd2.c_str());
    delay(500);
    
    // 格式3: x
    String cmd3 = String(direction);
    Serial.println("Trying command format 3: " + cmd3);
    send_chassis_command(cmd3.c_str());
    delay(500);
    
    // 格式4: x,y,z
    String cmd4 = String(direction) + "," + String(speed) + "," + String(duration);
    Serial.println("Trying command format 4: " + cmd4);
    send_chassis_command(cmd4.c_str());
    delay(500);
    
    Serial.print("Chassis control: ");
    Serial.print(direction_str);
    Serial.print(", Speed: ");
    Serial.print(speed);
    Serial.print(", Duration: ");
    Serial.println(duration);
}

/**
 * Advanced chassis control function
 * @param direction Direction: FORWARD(1), BACKWARD(2), LEFT(3), RIGHT(4), STOP(0)
 * @param distance Distance or angle
 * @param speed Speed level (0-100)
 */
inline void control_chassis(uint8_t direction, uint16_t distance, uint8_t speed)
{
    // 将枚举方向转换为实际命令值
    uint8_t actual_command;
    switch(direction) {
        case FORWARD: actual_command = CHASSIS_FORWARD; break;
        case BACKWARD: actual_command = CHASSIS_BACKWARD; break;
        case LEFT: actual_command = CHASSIS_LEFT; break;
        case RIGHT: actual_command = CHASSIS_RIGHT; break;
        case STOP: 
        default: actual_command = CHASSIS_STOP; break;
    }
    
    // 使用与control_chassis_raw相同的命令格式
    String cmd = "CH" + String(actual_command) + String(distance) + String(speed);
    CHASSIS_SERIAL.println(cmd); // 直接使用println，添加换行符
    
    // 调试输出
    char direction_str[10] = "UNKNOWN";
    switch(actual_command) {
        case CHASSIS_FORWARD: strcpy(direction_str, "FORWARD"); break;
        case CHASSIS_BACKWARD: strcpy(direction_str, "BACKWARD"); break;
        case CHASSIS_LEFT: strcpy(direction_str, "LEFT"); break;
        case CHASSIS_RIGHT: strcpy(direction_str, "RIGHT"); break;
        case CHASSIS_STOP: strcpy(direction_str, "STOP"); break;
    }
    
    Serial.print("Advanced chassis control: ");
    Serial.print("Direction: ");
    Serial.print(direction_str);
    Serial.print("(");
    Serial.print(actual_command);
    Serial.print("), Distance/Speed: ");
    Serial.print(distance);
    Serial.print(", Speed/Duration: ");
    Serial.println(speed);
    Serial.println("Command sent: " + cmd);
}

/**
 * Simple chassis movement commands
 * @param direction Direction: 0-Stop, 1-Forward, 2-Backward, 3-Left, 4-Right
 * @param speed Speed level (0-100)
 */
inline void move_chassis(uint8_t direction, uint8_t speed)
{
    // 将枚举方向转换为实际命令值
    uint8_t actual_command;
    switch(direction) {
        case FORWARD: actual_command = CHASSIS_FORWARD; break;
        case BACKWARD: actual_command = CHASSIS_BACKWARD; break;
        case LEFT: actual_command = CHASSIS_LEFT; break;
        case RIGHT: actual_command = CHASSIS_RIGHT; break;
        case STOP: 
        default: actual_command = CHASSIS_STOP; break;
    }
    
    control_chassis_raw(actual_command, speed, 0);
}

#endif // _CHASSIS_H_ 