#ifndef _CHASSIS_H_
#define _CHASSIS_H_

#include <Arduino.h>
#include "pins.h" // Import pin definitions

// 底盘控制命令定义
#define CHASSIS_FORWARD 5  // 前进
#define CHASSIS_BACKWARD 6 // 后退
#define CHASSIS_LEFT 8     // 左转
#define CHASSIS_RIGHT 7    // 右转
#define CHASSIS_STOP 0     // 停止

// 方向枚举，使代码更易读
enum ChassisDirection
{
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
inline void send_chassis_command(const char *command)
{
    // 打印十六进制格式的命令
    Serial.print("Chassis command (HEX): ");
    for (int i = 0; i < strlen(command); i++)
    {
        Serial.print(command[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    // 尝试不同的结束符 - 优化延时
    Serial.println("Sending with \\n");
    CHASSIS_SERIAL.print(command);
    CHASSIS_SERIAL.print('\n');
    delay(20); // 减少延时到20ms

    Serial.println("Sending with \\r");
    CHASSIS_SERIAL.print(command);
    CHASSIS_SERIAL.print('\r');
    delay(20); // 减少延时到20ms

    Serial.println("Sending with \\r\\n");
    CHASSIS_SERIAL.print(command);
    CHASSIS_SERIAL.print('\r');
    CHASSIS_SERIAL.print('\n');
    delay(20); // 减少延时到20ms

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
    switch (direction)
    {
    case CHASSIS_FORWARD:
        strcpy(direction_str, "FORWARD");
        break;
    case CHASSIS_BACKWARD:
        strcpy(direction_str, "BACKWARD");
        break;
    case CHASSIS_LEFT:
        strcpy(direction_str, "LEFT");
        break;
    case CHASSIS_RIGHT:
        strcpy(direction_str, "RIGHT");
        break;
    case CHASSIS_STOP:
        strcpy(direction_str, "STOP");
        break;
    }

    // 尝试不同的命令格式

    // 格式1: CHxyz - 优化延时
    String cmd1 = "CH" + String(direction) + String(speed) + String(duration);
    Serial.println("Trying command format 1: " + cmd1);
    send_chassis_command(cmd1.c_str());
    delay(100); // 减少延时到100ms

    // 格式2: CH,x,y,z
    String cmd2 = "CH," + String(direction) + "," + String(speed) + "," + String(duration);
    Serial.println("Trying command format 2: " + cmd2);
    send_chassis_command(cmd2.c_str());
    delay(100); // 减少延时到100ms

    // 格式3: x
    String cmd3 = String(direction);
    Serial.println("Trying command format 3: " + cmd3);
    send_chassis_command(cmd3.c_str());
    delay(100); // 减少延时到100ms

    // 格式4: x,y,z
    String cmd4 = String(direction) + "," + String(speed) + "," + String(duration);
    Serial.println("Trying command format 4: " + cmd4);
    send_chassis_command(cmd4.c_str());
    delay(100); // 减少延时到100ms

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
    switch (direction)
    {
    case FORWARD:
        actual_command = CHASSIS_FORWARD;
        break;
    case BACKWARD:
        actual_command = CHASSIS_BACKWARD;
        break;
    case LEFT:
        actual_command = CHASSIS_LEFT;
        break;
    case RIGHT:
        actual_command = CHASSIS_RIGHT;
        break;
    case STOP:
    default:
        actual_command = CHASSIS_STOP;
        break;
    }

    // 使用与control_chassis_raw相同的命令格式
    String cmd = "CH" + String(actual_command) + String(distance) + String(speed);
    CHASSIS_SERIAL.println(cmd); // 直接使用println，添加换行符

    // 调试输出
    char direction_str[10] = "UNKNOWN";
    switch (actual_command)
    {
    case CHASSIS_FORWARD:
        strcpy(direction_str, "FORWARD");
        break;
    case CHASSIS_BACKWARD:
        strcpy(direction_str, "BACKWARD");
        break;
    case CHASSIS_LEFT:
        strcpy(direction_str, "LEFT");
        break;
    case CHASSIS_RIGHT:
        strcpy(direction_str, "RIGHT");
        break;
    case CHASSIS_STOP:
        strcpy(direction_str, "STOP");
        break;
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
    switch (direction)
    {
    case FORWARD:
        actual_command = CHASSIS_FORWARD;
        break;
    case BACKWARD:
        actual_command = CHASSIS_BACKWARD;
        break;
    case LEFT:
        actual_command = CHASSIS_LEFT;
        break;
    case RIGHT:
        actual_command = CHASSIS_RIGHT;
        break;
    case STOP:
    default:
        actual_command = CHASSIS_STOP;
        break;
    }

    control_chassis_raw(actual_command, speed, 0);
}

//=========================
// 绝对位置控制功能
//=========================

// 底盘当前位置记录 (单位: mm)
static int chassis_current_x = 0;     // X轴位置 (前后方向，正值为前进)
static int chassis_current_y = 0;     // Y轴位置 (左右方向，正值为右移)
static int chassis_current_angle = 0; // 当前角度 (度，正值为顺时针)

// 底盘移动参数
static uint8_t chassis_default_speed = 50;   // 默认速度 (0-100)
static uint16_t chassis_move_precision = 10; // 移动精度 (mm)

/**
 * 重置底盘位置为原点
 */
inline void chassis_reset_position()
{
    chassis_current_x = 0;
    chassis_current_y = 0;
    chassis_current_angle = 0;
    Serial.println("Chassis position reset to origin (0, 0, 0°)");
}

/**
 * 设置底盘当前位置
 * @param x X轴位置 (mm)
 * @param y Y轴位置 (mm)
 * @param angle 角度 (度)
 */
inline void chassis_set_position(int x, int y, int angle)
{
    chassis_current_x = x;
    chassis_current_y = y;
    chassis_current_angle = angle % 360;
    if (chassis_current_angle < 0)
        chassis_current_angle += 360;

    Serial.println("Chassis position set to (" + String(x) + ", " + String(y) + ", " + String(chassis_current_angle) + "°)");
}

/**
 * 获取底盘当前位置
 * @param x 返回X轴位置
 * @param y 返回Y轴位置
 * @param angle 返回角度
 */
inline void chassis_get_position(int &x, int &y, int &angle)
{
    x = chassis_current_x;
    y = chassis_current_y;
    angle = chassis_current_angle;
}

/**
 * 打印底盘当前位置
 */
inline void chassis_print_position()
{
    Serial.println("Chassis Position: X=" + String(chassis_current_x) + "mm, Y=" + String(chassis_current_y) + "mm, Angle=" + String(chassis_current_angle) + "°");
}

/**
 * 设置底盘默认速度
 * @param speed 速度 (0-100)
 */
inline void chassis_set_default_speed(uint8_t speed)
{
    if (speed <= 100)
    {
        chassis_default_speed = speed;
        Serial.println("Chassis default speed set to: " + String(speed));
    }
    else
    {
        Serial.println("Error: Speed must be 0-100");
    }
}

/**
 * 底盘绝对位置移动 - 移动到指定X坐标
 * @param target_x 目标X坐标 (mm)
 * @param speed 移动速度 (0-100)，默认使用设定的默认速度
 */
inline void chassis_move_to_x(int target_x, uint8_t speed = 0)
{
    if (speed == 0)
        speed = chassis_default_speed;

    int distance = target_x - chassis_current_x;

    if (distance == 0)
    {
        Serial.println("Already at target X position: " + String(target_x) + "mm");
        return;
    }

    Serial.println("Moving from X=" + String(chassis_current_x) + "mm to X=" + String(target_x) + "mm (distance=" + String(distance) + "mm)");

    if (distance > 0)
    {
        // 前进
        control_chassis(FORWARD, abs(distance), speed);
    }
    else
    {
        // 后退
        control_chassis(BACKWARD, abs(distance), speed);
    }

    // 更新位置记录
    chassis_current_x = target_x;
    Serial.println("Chassis moved to X=" + String(chassis_current_x) + "mm");
}

/**
 * 底盘绝对位置移动 - 移动到指定Y坐标
 * @param target_y 目标Y坐标 (mm)
 * @param speed 移动速度 (0-100)，默认使用设定的默认速度
 */
inline void chassis_move_to_y(int target_y, uint8_t speed = 0)
{
    if (speed == 0)
        speed = chassis_default_speed;

    int distance = target_y - chassis_current_y;

    if (distance == 0)
    {
        Serial.println("Already at target Y position: " + String(target_y) + "mm");
        return;
    }

    Serial.println("Moving from Y=" + String(chassis_current_y) + "mm to Y=" + String(target_y) + "mm (distance=" + String(distance) + "mm)");

    if (distance > 0)
    {
        // 右移
        control_chassis(RIGHT, abs(distance), speed);
    }
    else
    {
        // 左移
        control_chassis(LEFT, abs(distance), speed);
    }

    // 更新位置记录
    chassis_current_y = target_y;
    Serial.println("Chassis moved to Y=" + String(chassis_current_y) + "mm");
}

/**
 * 底盘绝对位置移动 - 旋转到指定角度
 * @param target_angle 目标角度 (度)
 * @param speed 旋转速度 (0-100)，默认使用设定的默认速度
 */
inline void chassis_rotate_to_angle(int target_angle, uint8_t speed = 0)
{
    if (speed == 0)
        speed = chassis_default_speed;

    // 规范化目标角度
    target_angle = target_angle % 360;
    if (target_angle < 0)
        target_angle += 360;

    // 计算最短旋转路径
    int angle_diff = target_angle - chassis_current_angle;
    if (angle_diff > 180)
    {
        angle_diff -= 360;
    }
    else if (angle_diff < -180)
    {
        angle_diff += 360;
    }

    if (angle_diff == 0)
    {
        Serial.println("Already at target angle: " + String(target_angle) + "°");
        return;
    }

    Serial.println("Rotating from " + String(chassis_current_angle) + "° to " + String(target_angle) + "° (rotation=" + String(angle_diff) + "°)");

    if (angle_diff > 0)
    {
        // 顺时针旋转
        control_chassis(RIGHT, abs(angle_diff), speed);
    }
    else
    {
        // 逆时针旋转
        control_chassis(LEFT, abs(angle_diff), speed);
    }

    // 更新角度记录
    chassis_current_angle = target_angle;
    Serial.println("Chassis rotated to " + String(chassis_current_angle) + "°");
}

/**
 * 底盘绝对位置移动 - 移动到指定坐标
 * @param target_x 目标X坐标 (mm)
 * @param target_y 目标Y坐标 (mm)
 * @param speed 移动速度 (0-100)，默认使用设定的默认速度
 */
inline void chassis_move_to_position(int target_x, int target_y, uint8_t speed = 0)
{
    Serial.println("Moving to position (" + String(target_x) + ", " + String(target_y) + ")");

    // 先移动X轴，再移动Y轴 - 优化延时
    chassis_move_to_x(target_x, speed);
    delay(100); // 减少等待时间到100ms
    chassis_move_to_y(target_y, speed);

    Serial.println("Arrived at position (" + String(chassis_current_x) + ", " + String(chassis_current_y) + ")");
}

/**
 * 底盘绝对位置移动 - 移动到指定坐标和角度
 * @param target_x 目标X坐标 (mm)
 * @param target_y 目标Y坐标 (mm)
 * @param target_angle 目标角度 (度)
 * @param speed 移动速度 (0-100)，默认使用设定的默认速度
 */
inline void chassis_move_to_pose(int target_x, int target_y, int target_angle, uint8_t speed = 0)
{
    Serial.println("Moving to pose (" + String(target_x) + ", " + String(target_y) + ", " + String(target_angle) + "°)");

    // 先移动到位置，再旋转到角度 - 优化延时
    chassis_move_to_position(target_x, target_y, speed);
    delay(100); // 减少等待时间到100ms
    chassis_rotate_to_angle(target_angle, speed);

    Serial.println("Arrived at pose (" + String(chassis_current_x) + ", " + String(chassis_current_y) + ", " + String(chassis_current_angle) + "°)");
}

#endif // _CHASSIS_H_