#ifndef _CHASSIS_SIMPLE_ABSOLUTE_H_
#define _CHASSIS_SIMPLE_ABSOLUTE_H_

#include <Arduino.h>
#include "chassis.h"

// 简化底盘绝对位置控制常量
#define CHASSIS_MAX_POSITION 2000000    // 最大位置值 (2米)
#define CHASSIS_MIN_POSITION -2000000   // 最小位置值 (-2米)
#define CHASSIS_POSITION_TOLERANCE 1000 // 位置容差 (1mm)
#define CHASSIS_DEFAULT_SPEED 50        // 默认速度

// 底盘位置状态结构体 (仅Y轴)
struct ChassisSimplePosition
{
    long yPosition;               // Y轴位置 (前后方向，单位与chassis_adv相同)
    bool isCalibrated;            // 是否已校准
    unsigned long lastUpdateTime; // 最后更新时间
};

// 全局底盘位置状态
extern ChassisSimplePosition chassisPosition;

// 声明外部调试函数
extern void debugPrintAll(const String &msg);

/**
 * @brief 初始化简化底盘绝对位置控制系统
 */
inline void initChassisSimpleAbsolute()
{
    chassisPosition.yPosition = 0;
    chassisPosition.isCalibrated = false;
    chassisPosition.lastUpdateTime = millis();

    debugPrintAll("Chassis simple absolute position control initialized");
    debugPrintAll("Initial Y position: 0");
}

/**
 * @brief 重置/校准当前底盘位置
 * @param yPos 新的Y轴位置
 */
inline void resetChassisPosition(long yPos = 0)
{
    chassisPosition.yPosition = yPos;
    chassisPosition.isCalibrated = true;
    chassisPosition.lastUpdateTime = millis();

    debugPrintAll("Chassis position reset to Y: " + String(yPos));
}

/**
 * @brief 获取当前底盘位置信息
 * @return 当前位置字符串
 */
inline String getChassisPositionString()
{
    String status = chassisPosition.isCalibrated ? "Calibrated" : "Not Calibrated";
    return "Chassis Y Position: " + String(chassisPosition.yPosition) +
           ", Status: " + status;
}

/**
 * @brief 更新底盘位置 (在相对移动后调用)
 * @param direction 移动方向 (CHASSIS_FORWARD 或 CHASSIS_BACKWARD)
 * @param distance 移动距离
 */
inline void updateChassisPosition(uint8_t direction, long distance)
{
    if (!chassisPosition.isCalibrated)
    {
        debugPrintAll("Warning: Chassis position not calibrated, update may be inaccurate");
    }

    // 只处理前后移动
    switch (direction)
    {
    case CHASSIS_FORWARD:
        chassisPosition.yPosition += distance;
        break;
    case CHASSIS_BACKWARD:
        chassisPosition.yPosition -= distance;
        break;
    default:
        // 忽略左右移动和停止命令
        debugPrintAll("Chassis position update: ignoring non-Y-axis movement");
        return;
    }

    chassisPosition.lastUpdateTime = millis();
    debugPrintAll("Chassis position updated: " + getChassisPositionString());
}

/**
 * @brief 移动到绝对Y位置 (支持大距离分段移动)
 * @param targetY 目标Y位置
 * @param speed 移动速度
 * @return 是否成功开始移动
 */
inline bool moveChassisToAbsoluteY(long targetY, uint8_t speed = CHASSIS_DEFAULT_SPEED)
{
    // 检查位置范围
    if (targetY < CHASSIS_MIN_POSITION || targetY > CHASSIS_MAX_POSITION)
    {
        debugPrintAll("Error: Target Y position out of range (" +
                      String(CHASSIS_MIN_POSITION) + " to " + String(CHASSIS_MAX_POSITION) + ")");
        return false;
    }

    // 计算移动距离和方向
    long deltaY = targetY - chassisPosition.yPosition;

    debugPrintAll("Chassis absolute Y movement:");
    debugPrintAll("  From Y: " + String(chassisPosition.yPosition));
    debugPrintAll("  To Y: " + String(targetY));
    debugPrintAll("  Delta Y: " + String(deltaY));
    debugPrintAll("  Speed: " + String(speed));

    // 检查是否已经在目标位置
    if (abs(deltaY) < CHASSIS_POSITION_TOLERANCE)
    {
        debugPrintAll("Already at target Y position (within tolerance)");
        return true;
    }

    // 确定移动方向和总距离
    uint8_t direction;
    long totalDistance = abs(deltaY);

    if (deltaY > 0)
    {
        direction = CHASSIS_FORWARD;
        debugPrintAll("Moving FORWARD total " + String(totalDistance) + " units");
    }
    else
    {
        direction = CHASSIS_BACKWARD;
        debugPrintAll("Moving BACKWARD total " + String(totalDistance) + " units");
    }

    // 分段移动以突破uint16_t限制
    const long MAX_SEGMENT = 60000; // 安全的分段大小，小于65535
    long remainingDistance = totalDistance;
    int segmentCount = 0;

    debugPrintAll("Starting segmented movement...");

    while (remainingDistance > 0)
    {
        segmentCount++;
        long segmentDistance = min(remainingDistance, MAX_SEGMENT);

        debugPrintAll("Segment " + String(segmentCount) + ": Moving " + String(segmentDistance) + " units");
        debugPrintAll("  Remaining after this segment: " + String(remainingDistance - segmentDistance));

        // 使用现有的control_chassis_raw函数
        // 实际调用约定是 control_chassis_raw(direction, distance, speed)
        // 尽管函数签名显示为 (direction, speed, duration)，但实际使用中第二个参数是距离
        control_chassis_raw(direction, (uint16_t)segmentDistance, speed);

        // 更新剩余距离
        remainingDistance -= segmentDistance;

        // 更新当前位置
        if (direction == CHASSIS_FORWARD)
        {
            chassisPosition.yPosition += segmentDistance;
        }
        else
        {
            chassisPosition.yPosition -= segmentDistance;
        }

        debugPrintAll("Segment completed. Current Y position: " + String(chassisPosition.yPosition));

        // 如果还有剩余距离，添加短暂延迟
        if (remainingDistance > 0)
        {
            delay(100); // 100ms延迟，让底盘完成当前段移动
        }
    }

    // 最终位置更新
    chassisPosition.yPosition = targetY;
    chassisPosition.lastUpdateTime = millis();

    debugPrintAll("Chassis absolute Y movement completed in " + String(segmentCount) + " segments");
    debugPrintAll("Final position: " + getChassisPositionString());

    return true;
}

/**
 * @brief 检查底盘是否在目标Y位置
 * @param targetY 目标Y位置
 * @return 是否在目标位置
 */
inline bool isChassisAtY(long targetY)
{
    long deltaY = targetY - chassisPosition.yPosition;
    return abs(deltaY) < CHASSIS_POSITION_TOLERANCE;
}

/**
 * @brief 相对移动底盘并更新位置跟踪
 * @param direction 移动方向
 * @param distance 移动距离
 * @param speed 移动速度
 * @return 是否成功
 */
inline bool moveChassisRelativeY(uint8_t direction, uint16_t distance, uint8_t speed)
{
    // 只允许前后移动
    if (direction != CHASSIS_FORWARD && direction != CHASSIS_BACKWARD)
    {
        debugPrintAll("Error: Simple absolute chassis only supports FORWARD/BACKWARD movement");
        return false;
    }

    debugPrintAll("Chassis relative Y movement:");
    debugPrintAll("  Direction: " + String(direction == CHASSIS_FORWARD ? "FORWARD" : "BACKWARD"));
    debugPrintAll("  Distance: " + String(distance));
    debugPrintAll("  Speed: " + String(speed));

    // 使用现有的control_chassis_raw函数
    control_chassis_raw(direction, distance, speed);

    // 更新位置跟踪
    updateChassisPosition(direction, distance);

    return true;
}

/**
 * @brief 获取当前Y位置
 * @return 当前Y位置
 */
inline long getCurrentChassisY()
{
    return chassisPosition.yPosition;
}

/**
 * @brief 检查底盘位置是否已校准
 * @return 是否已校准
 */
inline bool isChassisCalibrated()
{
    return chassisPosition.isCalibrated;
}

/**
 * @brief 停止底盘移动
 */
inline void stopChassis()
{
    debugPrintAll("Stopping chassis...");
    control_chassis_raw(CHASSIS_STOP, 0, 0);
}

#endif // _CHASSIS_SIMPLE_ABSOLUTE_H_
