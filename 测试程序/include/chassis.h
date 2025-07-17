#ifndef _CHASSIS_H_
#define _CHASSIS_H_

#include <Arduino.h>
#include "pins.h" // Import pin definitions

// 底盘控制命令定义
#define CHASSIS_FORWARD 5  // 前进
#define CHASSIS_BACKWARD 6 // 后退
#define CHASSIS_LEFT 8     // 左转（禁用）
#define CHASSIS_RIGHT 7    // 右转（禁用）
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
    // 静默模式：简洁的调试信息
    Serial.println("Chassis TX: " + String(command));

    // 19号串口静默发送
    CHASSIS_SERIAL.println(command);
    CHASSIS_SERIAL.flush(); // 确保数据完全发送

    // 适当延时，确保底盘控制器接收完成
    delay(50);
}

/**
 * Enhanced chassis control function with smooth acceleration and safety
 * @param direction Direction: CHASSIS_FORWARD, CHASSIS_BACKWARD, CHASSIS_LEFT, CHASSIS_RIGHT, CHASSIS_STOP
 * @param targetSpeed Target speed value (0-100)
 * @param duration Duration (unused, kept for compatibility)
 */
inline void control_chassis_enhanced(uint8_t direction, uint8_t targetSpeed, uint8_t duration = 0)
{
    // 安全检查
    if (!chassis_safety_check())
    {
        return;
    }

    // 更新命令时间
    chassisState.lastCommandTime = millis();

    // 处理停止命令
    if (direction == CHASSIS_STOP || targetSpeed == 0)
    {
        chassisState.targetSpeed = 0;
        // 如果当前不在移动，立即停止
        if (!chassisState.isMoving)
        {
            chassisState.currentSpeed = 0;
            chassisState.currentDirection = CHASSIS_STOP;
            String cmd = "0,0,30";
            CHASSIS_SERIAL.println(cmd);
            CHASSIS_SERIAL.flush();
            Serial.println("Chassis TX: " + cmd + " (STOP)");
        }
        return;
    }

    // 限制速度范围
    targetSpeed = constrain(targetSpeed, chassisState.minSpeed, chassisState.maxSpeed);

    // 设置目标参数
    chassisState.targetSpeed = targetSpeed;
    chassisState.currentDirection = direction;

    // 如果不在移动，开始移动
    if (!chassisState.isMoving)
    {
        chassisState.isMoving = true;
        chassisState.movementStartTime = millis();
        chassisState.currentSpeed = chassisState.minSpeed; // 从最小速度开始
    }

    // 立即发送当前速度的命令（平滑更新将在后台进行）
    uint8_t actual_command = direction;
    uint16_t steps = chassisState.currentSpeed * 10;
    uint8_t chassis_speed = 30; // 固定底盘速度参数

    // 停止命令特殊处理
    if (actual_command == 0)
    {
        steps = 0;
    }

    String cmd = String(actual_command) + "," + String(steps) + "," + String(chassis_speed);
    CHASSIS_SERIAL.println(cmd);
    CHASSIS_SERIAL.flush();

    // 调试输出
    char direction_str[20] = "UNKNOWN";
    switch (direction)
    {
    case CHASSIS_FORWARD:
        strcpy(direction_str, "FORWARD");
        break;
    case CHASSIS_BACKWARD:
        strcpy(direction_str, "BACKWARD");
        break;
    case CHASSIS_LEFT:
        strcpy(direction_str, "LEFT_DISABLED");
        break;
    case CHASSIS_RIGHT:
        strcpy(direction_str, "RIGHT_DISABLED");
        break;
    case CHASSIS_STOP:
        strcpy(direction_str, "STOP");
        break;
    }

    Serial.println("Enhanced Chassis TX: " + cmd + " (" + String(direction_str) +
                   ", Target:" + String(targetSpeed) + ", Current:" + String(chassisState.currentSpeed) + ")");
}

/**
 * Raw chassis control function (legacy compatibility)
 * @param direction Direction: CHASSIS_FORWARD, CHASSIS_BACKWARD, CHASSIS_LEFT, CHASSIS_RIGHT, CHASSIS_STOP
 * @param speed Speed value
 * @param duration Duration
 */
inline void control_chassis_raw(uint8_t direction, uint16_t speed, uint8_t duration)
{
    char direction_str[10] = "UNKNOWN";
    uint8_t actual_command = direction;

    switch (direction)
    {
    case CHASSIS_FORWARD:
        strcpy(direction_str, "FORWARD");
        actual_command = 5; // 前进命令
        break;
    case CHASSIS_BACKWARD:
        strcpy(direction_str, "BACKWARD");
        actual_command = 6; // 后退命令
        break;
    case CHASSIS_LEFT:
        strcpy(direction_str, "LEFT_DISABLED");
        actual_command = 0; // 左转禁用，设为停止
        break;
    case CHASSIS_RIGHT:
        strcpy(direction_str, "RIGHT_DISABLED");
        actual_command = 0; // 右转禁用，设为停止
        break;
    case CHASSIS_STOP:
        strcpy(direction_str, "STOP");
        actual_command = 0; // 停止命令
        break;
    }

    // 限制速度范围并转换为底盘期望的范围
    if (speed > 100)
        speed = 100;
    if (speed < 0)
        speed = 0;

    // 新的命令格式：command,steps,speed
    // 使用正确的命令号：5=前进，6=后退，0=停止
    uint16_t steps = speed * 10;                          // 将速度转换为步数（可调整）
    uint8_t chassis_speed = duration > 0 ? duration : 30; // 使用duration作为速度参数

    // 停止命令特殊处理
    if (actual_command == 0)
    {
        steps = 0; // 停止时步数为0
    }

    // 限制参数范围
    if (steps > 10000)
        steps = 10000;
    if (chassis_speed > 100)
        chassis_speed = 100;
    if (chassis_speed < 1)
        chassis_speed = 1;

    String cmd = String(actual_command) + "," + String(steps) + "," + String(chassis_speed);

    // 静默模式：19号串口只发送纯净的底盘控制数据
    // 调试信息只在USB串口显示，不影响19号串口
    Serial.println("Chassis TX: " + cmd + " (" + String(direction_str) + ")");

    // 19号串口静默发送：只发送纯净的底盘控制数据
    CHASSIS_SERIAL.println(cmd);
    CHASSIS_SERIAL.flush(); // 确保数据完全发送

    // 适当延时确保底盘控制器接收完成
    delay(50);
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

//=========================
// 增强的底盘控制功能
//=========================

// 底盘状态结构体
struct ChassisState
{
    bool isMoving;                   // 是否正在移动
    bool emergencyStop;              // 紧急停止状态
    uint8_t currentDirection;        // 当前移动方向
    uint8_t currentSpeed;            // 当前速度 (0-100)
    uint8_t targetSpeed;             // 目标速度 (0-100)
    unsigned long lastCommandTime;   // 最后命令时间
    unsigned long movementStartTime; // 移动开始时间
    unsigned long lastSpeedUpdate;   // 最后速度更新时间

    // 平滑加速参数
    float acceleration; // 加速度 (单位/秒²)
    float deceleration; // 减速度 (单位/秒²)
    uint8_t minSpeed;   // 最小速度
    uint8_t maxSpeed;   // 最大速度

    // 安全参数
    unsigned long maxMovementTime; // 最大移动时间 (ms)
    unsigned long commandTimeout;  // 命令超时时间 (ms)
    bool safetyEnabled;            // 安全功能启用
};

// 全局底盘状态
static ChassisState chassisState = {
    .isMoving = false,
    .emergencyStop = false,
    .currentDirection = CHASSIS_STOP,
    .currentSpeed = 0,
    .targetSpeed = 0,
    .lastCommandTime = 0,
    .movementStartTime = 0,
    .lastSpeedUpdate = 0,
    .acceleration = 50.0,     // 50 单位/秒²
    .deceleration = 80.0,     // 80 单位/秒²
    .minSpeed = 5,            // 最小速度 5%
    .maxSpeed = 100,          // 最大速度 100%
    .maxMovementTime = 30000, // 30秒最大移动时间
    .commandTimeout = 2000,   // 2秒命令超时
    .safetyEnabled = true};

/**
 * 初始化底盘状态
 */
inline void chassis_init_state()
{
    chassisState.isMoving = false;
    chassisState.emergencyStop = false;
    chassisState.currentDirection = CHASSIS_STOP;
    chassisState.currentSpeed = 0;
    chassisState.targetSpeed = 0;
    chassisState.lastCommandTime = millis();
    chassisState.movementStartTime = 0;
    chassisState.lastSpeedUpdate = millis();
    Serial.println("Chassis state initialized");
}

/**
 * 紧急停止功能
 */
inline void chassis_emergency_stop()
{
    chassisState.emergencyStop = true;
    chassisState.isMoving = false;
    chassisState.currentSpeed = 0;
    chassisState.targetSpeed = 0;
    chassisState.currentDirection = CHASSIS_STOP;

    // 立即发送停止命令
    String cmd = "0,0,30";
    CHASSIS_SERIAL.println(cmd);
    CHASSIS_SERIAL.flush();

    Serial.println("EMERGENCY STOP ACTIVATED!");
    Serial.println("Chassis TX: " + cmd + " (EMERGENCY_STOP)");
}

/**
 * 清除紧急停止状态
 */
inline void chassis_clear_emergency_stop()
{
    chassisState.emergencyStop = false;
    Serial.println("Emergency stop cleared");
}

/**
 * 检查安全条件
 */
inline bool chassis_safety_check()
{
    if (!chassisState.safetyEnabled)
        return true;

    unsigned long currentTime = millis();

    // 检查紧急停止状态
    if (chassisState.emergencyStop)
    {
        Serial.println("Safety check failed: Emergency stop active");
        return false;
    }

    // 检查命令超时
    if (chassisState.isMoving &&
        (currentTime - chassisState.lastCommandTime) > chassisState.commandTimeout)
    {
        Serial.println("Safety check failed: Command timeout");
        chassis_emergency_stop();
        return false;
    }

    // 检查最大移动时间
    if (chassisState.isMoving &&
        (currentTime - chassisState.movementStartTime) > chassisState.maxMovementTime)
    {
        Serial.println("Safety check failed: Maximum movement time exceeded");
        chassis_emergency_stop();
        return false;
    }

    return true;
}

/**
 * 平滑速度控制更新
 */
inline void chassis_update_smooth_speed()
{
    if (!chassisState.isMoving || chassisState.emergencyStop)
        return;

    unsigned long currentTime = millis();
    float deltaTime = (currentTime - chassisState.lastSpeedUpdate) / 1000.0; // 转换为秒

    if (deltaTime < 0.05)
        return; // 最小更新间隔50ms

    chassisState.lastSpeedUpdate = currentTime;

    // 计算速度变化
    float speedDiff = chassisState.targetSpeed - chassisState.currentSpeed;
    float maxSpeedChange;

    if (speedDiff > 0)
    {
        // 加速
        maxSpeedChange = chassisState.acceleration * deltaTime;
    }
    else
    {
        // 减速
        maxSpeedChange = chassisState.deceleration * deltaTime;
    }

    // 限制速度变化量
    if (abs(speedDiff) <= maxSpeedChange)
    {
        chassisState.currentSpeed = chassisState.targetSpeed;
    }
    else
    {
        if (speedDiff > 0)
        {
            chassisState.currentSpeed += maxSpeedChange;
        }
        else
        {
            chassisState.currentSpeed -= maxSpeedChange;
        }
    }

    // 限制速度范围
    chassisState.currentSpeed = constrain(chassisState.currentSpeed,
                                          chassisState.minSpeed,
                                          chassisState.maxSpeed);

    // 如果到达目标速度且为0，停止移动
    if (chassisState.currentSpeed == 0 && chassisState.targetSpeed == 0)
    {
        chassisState.isMoving = false;
        chassisState.currentDirection = CHASSIS_STOP;
    }
}

/**
 * 重置底盘位置为原点
 */
inline void chassis_reset_position()
{
    chassis_current_x = 0;
    chassis_current_y = 0;
    chassis_current_angle = 0;
    chassis_init_state();
    Serial.println("Chassis position reset to origin (0, 0, 0°)");
}

/**
 * 获取底盘状态信息
 */
inline void chassis_get_status(bool &isMoving, bool &emergencyStop, uint8_t &currentSpeed,
                               uint8_t &targetSpeed, uint8_t &direction)
{
    isMoving = chassisState.isMoving;
    emergencyStop = chassisState.emergencyStop;
    currentSpeed = chassisState.currentSpeed;
    targetSpeed = chassisState.targetSpeed;
    direction = chassisState.currentDirection;
}

/**
 * 打印底盘详细状态
 */
inline void chassis_print_detailed_status()
{
    Serial.println("=== Chassis Detailed Status ===");
    Serial.println("Position: X=" + String(chassis_current_x) + "mm, Y=" + String(chassis_current_y) + "mm, Angle=" + String(chassis_current_angle) + "°");
    Serial.println("Movement: " + String(chassisState.isMoving ? "Moving" : "Stopped"));
    Serial.println("Emergency Stop: " + String(chassisState.emergencyStop ? "ACTIVE" : "Clear"));
    Serial.println("Direction: " + String(chassisState.currentDirection));
    Serial.println("Speed: Current=" + String(chassisState.currentSpeed) + "%, Target=" + String(chassisState.targetSpeed) + "%");
    Serial.println("Safety: " + String(chassisState.safetyEnabled ? "Enabled" : "Disabled"));
    Serial.println("Acceleration: " + String(chassisState.acceleration) + " units/s²");
    Serial.println("Deceleration: " + String(chassisState.deceleration) + " units/s²");
    Serial.println("Speed Range: " + String(chassisState.minSpeed) + "% - " + String(chassisState.maxSpeed) + "%");

    unsigned long currentTime = millis();
    if (chassisState.isMoving)
    {
        Serial.println("Movement Time: " + String((currentTime - chassisState.movementStartTime) / 1000.0) + "s");
        Serial.println("Last Command: " + String((currentTime - chassisState.lastCommandTime) / 1000.0) + "s ago");
    }
    Serial.println("==============================");
}

/**
 * 配置底盘安全参数
 */
inline void chassis_configure_safety(float acceleration, float deceleration,
                                     uint8_t minSpeed, uint8_t maxSpeed,
                                     unsigned long maxMovementTime, unsigned long commandTimeout)
{
    chassisState.acceleration = constrain(acceleration, 10.0, 200.0);
    chassisState.deceleration = constrain(deceleration, 10.0, 200.0);
    chassisState.minSpeed = constrain(minSpeed, 1, 50);
    chassisState.maxSpeed = constrain(maxSpeed, 50, 100);
    chassisState.maxMovementTime = constrain(maxMovementTime, 5000UL, 60000UL);
    chassisState.commandTimeout = constrain(commandTimeout, 500UL, 10000UL);

    Serial.println("Chassis safety parameters updated:");
    Serial.println("  Acceleration: " + String(chassisState.acceleration) + " units/s²");
    Serial.println("  Deceleration: " + String(chassisState.deceleration) + " units/s²");
    Serial.println("  Speed range: " + String(chassisState.minSpeed) + "% - " + String(chassisState.maxSpeed) + "%");
    Serial.println("  Max movement time: " + String(chassisState.maxMovementTime / 1000.0) + "s");
    Serial.println("  Command timeout: " + String(chassisState.commandTimeout / 1000.0) + "s");
}

/**
 * 启用/禁用安全功能
 */
inline void chassis_set_safety_enabled(bool enabled)
{
    chassisState.safetyEnabled = enabled;
    Serial.println("Chassis safety " + String(enabled ? "enabled" : "disabled"));
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