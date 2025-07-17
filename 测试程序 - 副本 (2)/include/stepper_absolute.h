#ifndef _STEPPER_ABSOLUTE_H_
#define _STEPPER_ABSOLUTE_H_

#include <Arduino.h>
#include "pins.h"

// 步进电机绝对位置控制常量
#define STEPPER_MAX_POSITION 1000000    // 最大位置值
#define STEPPER_MIN_POSITION -1000000   // 最小位置值
#define STEPPER_POSITION_TOLERANCE 1    // 位置容差 (1步)
#define STEPPER_DEFAULT_SPEED 100       // 默认速度
#define STEPPER_DEFAULT_ACCELERATION 50 // 默认加速度

// 步进电机绝对位置状态结构体
struct StepperAbsoluteState
{
    long currentPosition;         // 当前绝对位置 (步数)
    bool isCalibrated;            // 是否已校准
    bool isEnabled;               // 是否使能 (与现有系统同步)
    unsigned long lastUpdateTime; // 最后更新时间
    int lastSpeed;                // 最后使用的速度
    int lastAcceleration;         // 最后使用的加速度
};

// 全局步进电机绝对位置状态 (4个电机)
extern StepperAbsoluteState stepperAbsStates[4];

// 声明外部函数
extern void debugPrintAll(const String &msg);
extern void controlStepper(int motor, int steps, int speed);
extern void setStepperEnable(int motor, bool enable);
extern bool stepperEnabled[4]; // 现有的使能状态数组

/**
 * @brief 初始化步进电机绝对位置控制系统
 */
inline void initStepperAbsolute()
{
    for (int i = 0; i < 4; i++)
    {
        stepperAbsStates[i].currentPosition = 0;
        stepperAbsStates[i].isCalibrated = false;
        stepperAbsStates[i].isEnabled = false;
        stepperAbsStates[i].lastUpdateTime = millis();
        stepperAbsStates[i].lastSpeed = STEPPER_DEFAULT_SPEED;
        stepperAbsStates[i].lastAcceleration = STEPPER_DEFAULT_ACCELERATION;
    }

    debugPrintAll("Stepper absolute position control initialized");
    debugPrintAll("All 4 motors set to position 0 (not calibrated)");
}

/**
 * @brief 重置/校准指定步进电机的位置
 * @param motor 电机编号 (1-4)
 * @param position 新的绝对位置
 */
inline void resetStepperPosition(int motor, long position = 0)
{
    if (motor < 1 || motor > 4)
    {
        debugPrintAll("Error: Invalid motor number (1-4)");
        return;
    }

    int index = motor - 1;
    stepperAbsStates[index].currentPosition = position;
    stepperAbsStates[index].isCalibrated = true;
    stepperAbsStates[index].lastUpdateTime = millis();

    debugPrintAll("Motor " + String(motor) + " position reset to: " + String(position));
}

/**
 * @brief 获取指定步进电机的位置信息
 * @param motor 电机编号 (1-4)
 * @return 位置信息字符串
 */
inline String getStepperPositionString(int motor)
{
    if (motor < 1 || motor > 4)
    {
        return "Error: Invalid motor number";
    }

    int index = motor - 1;
    String status = stepperAbsStates[index].isCalibrated ? "Calibrated" : "Not Calibrated";
    String enabled = stepperAbsStates[index].isEnabled ? "Enabled" : "Disabled";

    return "Motor " + String(motor) + " - Position: " + String(stepperAbsStates[index].currentPosition) +
           ", Status: " + status + ", State: " + enabled;
}

/**
 * @brief 获取所有步进电机的位置信息
 * @return 所有电机位置信息
 */
inline String getAllStepperPositionsString()
{
    String result = "=== All Stepper Positions ===\n";
    for (int motor = 1; motor <= 4; motor++)
    {
        result += getStepperPositionString(motor) + "\n";
    }
    return result;
}

/**
 * @brief 更新步进电机位置 (在相对移动后调用)
 * @param motor 电机编号 (1-4)
 * @param steps 移动的步数 (正数或负数)
 */
inline void updateStepperPosition(int motor, int steps)
{
    if (motor < 1 || motor > 4)
    {
        debugPrintAll("Error: Invalid motor number (1-4)");
        return;
    }

    int index = motor - 1;

    if (!stepperAbsStates[index].isCalibrated)
    {
        debugPrintAll("Warning: Motor " + String(motor) + " position not calibrated, update may be inaccurate");
    }

    stepperAbsStates[index].currentPosition += steps;
    stepperAbsStates[index].lastUpdateTime = millis();

    debugPrintAll("Motor " + String(motor) + " position updated by " + String(steps) +
                  " steps to: " + String(stepperAbsStates[index].currentPosition));
}

/**
 * @brief 同步使能状态 (与现有系统同步)
 * @param motor 电机编号 (1-4)
 */
inline void syncStepperEnableState(int motor)
{
    if (motor < 1 || motor > 4)
    {
        return;
    }

    int index = motor - 1;
    stepperAbsStates[index].isEnabled = stepperEnabled[index];
}

/**
 * @brief 移动步进电机到绝对位置
 * @param motor 电机编号 (1-4)
 * @param targetPosition 目标绝对位置
 * @param maxSpeed 最大速度 (1-200)
 * @param acceleration 加速度 (可选，暂未实现)
 * @return 是否成功开始移动
 */
inline bool moveStepperToAbsolutePosition(int motor, long targetPosition, int maxSpeed = STEPPER_DEFAULT_SPEED, int acceleration = STEPPER_DEFAULT_ACCELERATION)
{
    if (motor < 1 || motor > 4)
    {
        debugPrintAll("Error: Invalid motor number (1-4)");
        return false;
    }

    // 同步使能状态
    syncStepperEnableState(motor);

    int index = motor - 1;

    // 自动使能电机（如果未使能）
    if (!stepperAbsStates[index].isEnabled)
    {
        debugPrintAll("Motor " + String(motor) + " is disabled. Auto-enabling...");
        setStepperEnable(motor, true);
        syncStepperEnableState(motor);
        debugPrintAll("Motor " + String(motor) + " auto-enabled successfully");
    }

    // 检查位置范围
    if (targetPosition < STEPPER_MIN_POSITION || targetPosition > STEPPER_MAX_POSITION)
    {
        debugPrintAll("Error: Target position out of range (" +
                      String(STEPPER_MIN_POSITION) + " to " + String(STEPPER_MAX_POSITION) + ")");
        return false;
    }

    // 检查速度范围
    if (maxSpeed < 1 || maxSpeed > 200)
    {
        debugPrintAll("Error: Speed must be between 1-200");
        return false;
    }

    // 计算移动步数
    long deltaSteps = targetPosition - stepperAbsStates[index].currentPosition;

    debugPrintAll("Stepper absolute movement - Motor " + String(motor) + ":");
    debugPrintAll("  From position: " + String(stepperAbsStates[index].currentPosition));
    debugPrintAll("  To position: " + String(targetPosition));
    debugPrintAll("  Delta steps: " + String(deltaSteps));
    debugPrintAll("  Max speed: " + String(maxSpeed));
    debugPrintAll("  Acceleration: " + String(acceleration));

    // 检查是否已经在目标位置
    if (abs(deltaSteps) <= STEPPER_POSITION_TOLERANCE)
    {
        debugPrintAll("Motor " + String(motor) + " already at target position (within tolerance)");
        return true;
    }

    // 保存参数
    stepperAbsStates[index].lastSpeed = maxSpeed;
    stepperAbsStates[index].lastAcceleration = acceleration;

    // 使用现有的controlStepper函数执行移动
    debugPrintAll("Executing stepper movement using existing controlStepper...");
    controlStepper(motor, (int)deltaSteps, maxSpeed);

    // 更新位置 (预估位置，实际位置可能有误差)
    stepperAbsStates[index].currentPosition = targetPosition;
    stepperAbsStates[index].lastUpdateTime = millis();

    debugPrintAll("Stepper absolute movement completed for Motor " + String(motor));
    debugPrintAll("Updated position: " + getStepperPositionString(motor));

    return true;
}

/**
 * @brief 检查步进电机是否在目标位置
 * @param motor 电机编号 (1-4)
 * @param targetPosition 目标位置
 * @return 是否在目标位置
 */
inline bool isStepperAtPosition(int motor, long targetPosition)
{
    if (motor < 1 || motor > 4)
    {
        return false;
    }

    int index = motor - 1;
    long deltaSteps = targetPosition - stepperAbsStates[index].currentPosition;
    return abs(deltaSteps) <= STEPPER_POSITION_TOLERANCE;
}

/**
 * @brief 获取步进电机当前位置
 * @param motor 电机编号 (1-4)
 * @return 当前位置，如果电机编号无效返回0
 */
inline long getStepperCurrentPosition(int motor)
{
    if (motor < 1 || motor > 4)
    {
        return 0;
    }

    return stepperAbsStates[motor - 1].currentPosition;
}

/**
 * @brief 检查步进电机位置是否已校准
 * @param motor 电机编号 (1-4)
 * @return 是否已校准
 */
inline bool isStepperCalibrated(int motor)
{
    if (motor < 1 || motor > 4)
    {
        return false;
    }

    return stepperAbsStates[motor - 1].isCalibrated;
}

/**
 * @brief 相对移动步进电机并更新位置跟踪
 * @param motor 电机编号 (1-4)
 * @param steps 移动步数
 * @param speed 移动速度
 * @return 是否成功
 */
inline bool moveStepperRelative(int motor, int steps, int speed)
{
    if (motor < 1 || motor > 4)
    {
        debugPrintAll("Error: Invalid motor number (1-4)");
        return false;
    }

    // 同步使能状态
    syncStepperEnableState(motor);

    int index = motor - 1;

    // 自动使能电机（如果未使能）
    if (!stepperAbsStates[index].isEnabled)
    {
        debugPrintAll("Motor " + String(motor) + " is disabled. Auto-enabling...");
        setStepperEnable(motor, true);
        syncStepperEnableState(motor);
        debugPrintAll("Motor " + String(motor) + " auto-enabled successfully");
    }

    debugPrintAll("Stepper relative movement - Motor " + String(motor) + ":");
    debugPrintAll("  Steps: " + String(steps));
    debugPrintAll("  Speed: " + String(speed));

    // 使用现有的controlStepper函数
    controlStepper(motor, steps, speed);

    // 更新位置跟踪
    updateStepperPosition(motor, steps);

    return true;
}

#endif // _STEPPER_ABSOLUTE_H_
