#ifndef _CHASSIS_ABSOLUTE_H_
#define _CHASSIS_ABSOLUTE_H_

#include <Arduino.h>
#include <math.h>
#include "chassis.h"

// 位置控制相关常量
#define MAX_COORDINATE 1000000  // 最大坐标值 (1米 = 1000000 单位)
#define MIN_COORDINATE -1000000 // 最小坐标值
#define POSITION_TOLERANCE 1000 // 位置容差 (1mm)
#define DEFAULT_MOVE_SPEED 50   // 默认移动速度

// 位置状态结构体
struct ChassisPosition
{
    long x;                       // X坐标 (单位: 微米或编码器单位)
    long y;                       // Y坐标 (单位: 微米或编码器单位)
    float heading;                // 朝向角度 (度, 0-360)
    bool isCalibrated;            // 是否已校准
    unsigned long lastUpdateTime; // 最后更新时间
};

// 移动计划结构体
struct MovePlan
{
    long deltaX;       // X方向移动距离
    long deltaY;       // Y方向移动距离
    float distance;    // 总移动距离
    float angle;       // 移动角度 (度)
    uint8_t direction; // 主要移动方向
    bool needRotation; // 是否需要旋转
};

// 全局位置状态
extern ChassisPosition currentPosition;

// 声明外部调试函数
extern void debugPrintAll(const String &msg);

/**
 * @brief 初始化绝对位置控制系统
 */
inline void initAbsolutePositionControl()
{
    currentPosition.x = 0;
    currentPosition.y = 0;
    currentPosition.heading = 0.0;
    currentPosition.isCalibrated = false;
    currentPosition.lastUpdateTime = millis();

    debugPrintAll("Absolute position control initialized");
    debugPrintAll("Initial position: (0, 0), heading: 0°");
}

/**
 * @brief 重置/校准当前位置
 * @param x 新的X坐标
 * @param y 新的Y坐标
 * @param heading 新的朝向角度
 */
inline void resetPosition(long x = 0, long y = 0, float heading = 0.0)
{
    currentPosition.x = x;
    currentPosition.y = y;
    currentPosition.heading = heading;
    currentPosition.isCalibrated = true;
    currentPosition.lastUpdateTime = millis();

    debugPrintAll("Position reset to: (" + String(x) + ", " + String(y) + "), heading: " + String(heading) + "°");
}

/**
 * @brief 获取当前位置信息
 * @return 当前位置字符串
 */
inline String getCurrentPositionString()
{
    String status = currentPosition.isCalibrated ? "Calibrated" : "Not Calibrated";
    return "Position: (" + String(currentPosition.x) + ", " + String(currentPosition.y) +
           "), Heading: " + String(currentPosition.heading) + "°, Status: " + status;
}

/**
 * @brief 更新位置 (在相对移动后调用)
 * @param direction 移动方向
 * @param distance 移动距离
 */
inline void updatePosition(uint8_t direction, long distance)
{
    if (!currentPosition.isCalibrated)
    {
        debugPrintAll("Warning: Position not calibrated, update may be inaccurate");
    }

    // 根据方向和距离更新位置
    switch (direction)
    {
    case FORWARD:
        currentPosition.y += distance * cos(currentPosition.heading * PI / 180.0);
        currentPosition.x += distance * sin(currentPosition.heading * PI / 180.0);
        break;
    case BACKWARD:
        currentPosition.y -= distance * cos(currentPosition.heading * PI / 180.0);
        currentPosition.x -= distance * sin(currentPosition.heading * PI / 180.0);
        break;
    case LEFT:
        currentPosition.heading -= 90.0; // 假设左转90度
        if (currentPosition.heading < 0)
            currentPosition.heading += 360.0;
        break;
    case RIGHT:
        currentPosition.heading += 90.0; // 假设右转90度
        if (currentPosition.heading >= 360.0)
            currentPosition.heading -= 360.0;
        break;
    }

    currentPosition.lastUpdateTime = millis();
    debugPrintAll("Position updated: " + getCurrentPositionString());
}

/**
 * @brief 计算移动计划
 * @param targetX 目标X坐标
 * @param targetY 目标Y坐标
 * @return 移动计划
 */
inline MovePlan calculateMovePlan(long targetX, long targetY)
{
    MovePlan plan;

    // 计算位移
    plan.deltaX = targetX - currentPosition.x;
    plan.deltaY = targetY - currentPosition.y;

    // 计算距离和角度
    plan.distance = sqrt(plan.deltaX * plan.deltaX + plan.deltaY * plan.deltaY);
    plan.angle = atan2(plan.deltaX, plan.deltaY) * 180.0 / PI;
    if (plan.angle < 0)
        plan.angle += 360.0;

    // 确定主要移动方向
    if (abs(plan.deltaX) > abs(plan.deltaY))
    {
        // X方向移动为主
        plan.direction = (plan.deltaX > 0) ? RIGHT : LEFT;
    }
    else
    {
        // Y方向移动为主
        plan.direction = (plan.deltaY > 0) ? FORWARD : BACKWARD;
    }

    // 判断是否需要旋转 (简化版本，实际可能需要更复杂的逻辑)
    float headingDiff = abs(plan.angle - currentPosition.heading);
    if (headingDiff > 180.0)
        headingDiff = 360.0 - headingDiff;
    plan.needRotation = headingDiff > 45.0; // 如果角度差超过45度则需要旋转

    return plan;
}

/**
 * @brief 执行绝对位置移动 (分步移动版本)
 * @param targetX 目标X坐标
 * @param targetY 目标Y坐标
 * @param speed 移动速度
 * @return 是否成功开始移动
 */
inline bool moveToAbsolutePosition(long targetX, long targetY, uint8_t speed = DEFAULT_MOVE_SPEED)
{
    // 检查坐标范围
    if (targetX < MIN_COORDINATE || targetX > MAX_COORDINATE ||
        targetY < MIN_COORDINATE || targetY > MAX_COORDINATE)
    {
        debugPrintAll("Error: Target coordinates out of range");
        return false;
    }

    // 计算移动计划
    MovePlan plan = calculateMovePlan(targetX, targetY);

    debugPrintAll("Move plan calculated:");
    debugPrintAll("  From: (" + String(currentPosition.x) + ", " + String(currentPosition.y) + ")");
    debugPrintAll("  To: (" + String(targetX) + ", " + String(targetY) + ")");
    debugPrintAll("  Delta X: " + String(plan.deltaX) + ", Delta Y: " + String(plan.deltaY));
    debugPrintAll("  Distance: " + String(plan.distance));
    debugPrintAll("  Angle: " + String(plan.angle) + "°");

    // 检查是否已经在目标位置
    if (plan.distance < POSITION_TOLERANCE)
    {
        debugPrintAll("Already at target position (within tolerance)");
        return true;
    }

    // 分步移动：先X方向，再Y方向
    debugPrintAll("Executing step-by-step movement...");

    // 第一步：X方向移动
    if (abs(plan.deltaX) > POSITION_TOLERANCE)
    {
        uint8_t xDirection = (plan.deltaX > 0) ? CHASSIS_RIGHT : CHASSIS_LEFT;
        long xDistance = abs(plan.deltaX);

        debugPrintAll("Step 1: Moving in X direction, distance: " + String(xDistance));
        control_chassis_raw(xDirection, (uint16_t)min(xDistance, 65535), speed);

        // 更新X位置
        currentPosition.x = targetX;
        debugPrintAll("X position updated to: " + String(currentPosition.x));
    }

    // 第二步：Y方向移动
    if (abs(plan.deltaY) > POSITION_TOLERANCE)
    {
        uint8_t yDirection = (plan.deltaY > 0) ? CHASSIS_FORWARD : CHASSIS_BACKWARD;
        long yDistance = abs(plan.deltaY);

        debugPrintAll("Step 2: Moving in Y direction, distance: " + String(yDistance));
        control_chassis_raw(yDirection, (uint16_t)min(yDistance, 65535), speed);

        // 更新Y位置
        currentPosition.y = targetY;
        debugPrintAll("Y position updated to: " + String(currentPosition.y));
    }

    currentPosition.lastUpdateTime = millis();
    debugPrintAll("Absolute movement completed. Final position: " + getCurrentPositionString());

    return true;
}

/**
 * @brief 检查是否在目标位置
 * @param targetX 目标X坐标
 * @param targetY 目标Y坐标
 * @return 是否在目标位置
 */
inline bool isAtPosition(long targetX, long targetY)
{
    long deltaX = targetX - currentPosition.x;
    long deltaY = targetY - currentPosition.y;
    float distance = sqrt(deltaX * deltaX + deltaY * deltaY);
    return distance < POSITION_TOLERANCE;
}

#endif // _CHASSIS_ABSOLUTE_H_
