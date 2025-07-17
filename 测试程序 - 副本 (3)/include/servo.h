#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>
#include "pins.h"  // Import pin definitions

// 声明外部函数（在main.cpp中定义）
extern void debugPrint(const String &message);

// Servo type definition
typedef enum {
    SERVO_TYPE_STANDARD = 0,  // Standard servo (now supporting 0-360 degrees)
    SERVO_TYPE_CONTINUOUS = 1 // Continuous rotation servo (0-360 degrees)
} ServoType;

// Servo type configuration (default: standard servo)
ServoType servoTypes[3] = {SERVO_TYPE_STANDARD, SERVO_TYPE_STANDARD, SERVO_TYPE_STANDARD};

// 记录当前舵机角度
int currentServoAngles[3] = {90, 90, 90};

/**
 * @brief 计算两个角度之间的最短路径
 * @param currentAngle 当前角度
 * @param targetAngle 目标角度
 * @param maxAngle 最大角度（360）
 * @return 最短路径的目标角度
 */
int calculateShortestPath(int currentAngle, int targetAngle, int maxAngle)
{
    // 特殊处理360度的情况，将其视为360度而非0度
    bool targetIs360 = (targetAngle == 360);
    
    // 处理超出范围的角度
    currentAngle = currentAngle % 360;
    if (currentAngle < 0) currentAngle += 360;
    
    // 特殊处理360度的情况，确保它不被规范化为0度
    if (!targetIs360) {
        targetAngle = targetAngle % 360;
        if (targetAngle < 0) targetAngle += 360;
    } else {
        targetAngle = 360;
    }
    
    debugPrint("Shortest Path Calculation - Start: Current=" + String(currentAngle) + "°, Target=" + String(targetAngle) + "°");
    
    if (currentAngle == targetAngle) {
        debugPrint("Current angle equals target angle, no rotation needed");
        return targetAngle;
    }
    
    // 计算最短路径
    int clockwiseDist, counterClockwiseDist;
    
    if (targetIs360) {
        // 特殊处理到360度的情况
        clockwiseDist = 360 - currentAngle;
        counterClockwiseDist = currentAngle;
    } else {
        clockwiseDist = (targetAngle - currentAngle + 360) % 360;
        counterClockwiseDist = (currentAngle - targetAngle + 360) % 360;
    }
    
    // 输出调试信息，显示计算的顺时针和逆时针距离
    debugPrint("Clockwise distance: " + String(clockwiseDist) + "°, Counter-clockwise distance: " + String(counterClockwiseDist) + "°");
    
    // 选择最短路径
    int resultAngle;
    if (clockwiseDist < counterClockwiseDist) {
        // 顺时针方向更短
        debugPrint("Choosing clockwise path: " + String(clockwiseDist) + "°");
        resultAngle = targetAngle;
    } else if (counterClockwiseDist < clockwiseDist) {
        // 逆时针方向更短
        debugPrint("Choosing counter-clockwise path: " + String(counterClockwiseDist) + "°");
        resultAngle = targetAngle - 360;  // 使用负角度表示逆时针旋转
    } else {
        // 两个方向距离相等（180度），默认选择顺时针
        debugPrint("Clockwise and counter-clockwise distances equal (180°), defaulting to clockwise");
        resultAngle = targetAngle;
    }
    
    debugPrint("Final calculation result: " + String(resultAngle) + "°");
    return resultAngle;
}

/**
 * @brief Set servo type
 * @param servoIndex Servo index (0-2)
 * @param type Servo type (SERVO_TYPE_STANDARD or SERVO_TYPE_CONTINUOUS)
 */
void setServoType(int servoIndex, ServoType type)
{
    if (servoIndex >= 0 && servoIndex < 3) {
        servoTypes[servoIndex] = type;
        debugPrint("Servo " + String(servoIndex+1) + " type set to " + (type == SERVO_TYPE_STANDARD ? "standard (0-180)" : "continuous (0-360)"));
    }
}

/**
 * @brief Convert angle to pulse width
 * @param angle Angle value
 * @param type Servo type
 * @return Pulse width (microseconds)
 */
int angleToPulseWidth(int angle, ServoType type)
{
    // 保存原始角度用于调试输出
    int originalAngle = angle;
    
    // 处理负角度（表示逆时针方向）
    if (angle < 0) {
        debugPrint("Handling negative angle: " + String(angle) + "° (representing counter-clockwise rotation)");
        angle = 360 + angle; // 转换为等效的正角度
    }
    
    angle = angle % 360; // 确保在0-359范围内
    
    debugPrint("Original angle: " + String(originalAngle) + "°, Converted angle: " + String(angle) + "°");
    
    int pulsewidth = 0;
    
    if (type == SERVO_TYPE_STANDARD) {
        // 将0-360度映射到标准舵机的500-2480微秒范围
        pulsewidth = 500 + (angle * 1980 / 360);
    } else {
        // 连续旋转舵机 (0-360度)
        if (angle <= 180) {
            // 0-180 degrees maps to 500-1500 microseconds
            pulsewidth = 500 + (angle * 1000 / 180);
        } else {
            // 180-360 degrees maps to 1500-2500 microseconds
            pulsewidth = 1500 + ((angle - 180) * 1000 / 180);
        }
    }
    
    debugPrint("Calculating pulse width: " + String(pulsewidth) + " microseconds");
    return pulsewidth;
}

/**
 * @brief Control Servo 1 to rotate to specified angle
 * @param angle Angle value (0-360 for both standard and continuous servo)
 */
void servo1(int angle)
{ 
    ServoType type = servoTypes[0];
    
    // 特殊处理360度的情况
    bool targetIs360 = (angle == 360);
    
    debugPrint("Servo 1: Start rotation - Current position: " + String(currentServoAngles[0]) + "°, Target position: " + String(angle) + "°");
    
    // 计算最短路径
    int targetAngle = calculateShortestPath(currentServoAngles[0], angle, 360);
    
    // 更新当前角度记录（使用正规化后的角度）
    int actualAngle;
    if (targetIs360) {
        actualAngle = 360;
    } else {
        actualAngle = angle % 360;
        if (actualAngle < 0) actualAngle += 360;
    }
    
    // 更新当前角度
    currentServoAngles[0] = actualAngle;
    
    debugPrint("Servo 1 moving to angle: " + String(actualAngle) + " (type: " + 
              (type == SERVO_TYPE_STANDARD ? "standard" : "continuous") + ")");
    
    // 确定是否为逆时针旋转（负角度）
    bool isCounterClockwise = (targetAngle < 0);
    if (isCounterClockwise) {
        // 负角度表示逆时针旋转，转换为正角度用于计算脉冲宽度
        targetAngle = 360 + targetAngle;  // 这会将负角度转换为等效的正角度
        debugPrint("Servo 1: Counter-clockwise rotation, converting negative angle " + String(targetAngle - 360) + "° to " + String(targetAngle) + "°");
    }
    
    // 计算脉冲宽度
    int pulsewidth;
    if (targetIs360) {
        // 360度应该映射到最大脉冲宽度（2480微秒），而不是0度的脉冲宽度
        if (type == SERVO_TYPE_STANDARD) {
            // 标准舵机：360度 = 2480微秒（最大值）
            pulsewidth = 500 + (360 * 1980 / 360); // 等于2480
        } else {
            // 连续旋转舵机：360度 = 2500微秒（最大值）
            pulsewidth = 2500;
        }
        debugPrint("Servo 1: Handling 360 degree special case, setting maximum pulse width");
    } else if (type == SERVO_TYPE_STANDARD) {
        // 将0-360度映射到标准舵机的500-2480微秒范围
        pulsewidth = 500 + (targetAngle * 1980 / 360);
    } else {
        // 连续旋转舵机 (0-360度)
        if (targetAngle <= 180) {
            // 0-180 degrees maps to 500-1500 microseconds
            pulsewidth = 500 + (targetAngle * 1000 / 180);
        } else {
            // 180-360 degrees maps to 1500-2500 microseconds
            pulsewidth = 1500 + ((targetAngle - 180) * 1000 / 180);
        }
    }
    
    // 显示脉冲宽度信息
    debugPrint("Servo 1: Pulse width set to " + String(pulsewidth) + " microseconds");
    
    // Send 50 pulses
    pinMode(SERVO_PIN_1, OUTPUT);
    for (int i = 0; i < 50; i++)
    {
        digitalWrite(SERVO_PIN_1, HIGH);     // Set servo pin to HIGH
        delayMicroseconds(pulsewidth);       // Delay for pulse width microseconds
        digitalWrite(SERVO_PIN_1, LOW);      // Set servo pin to LOW
        delayMicroseconds(20000 - pulsewidth);
    }
    delay(100);
    
    debugPrint("Servo 1: Rotation complete, current position recorded as " + String(currentServoAngles[0]) + "°");
}

/**
 * @brief Control Servo 2 to rotate to specified angle
 * @param angle Angle value (0-360 for both standard and continuous servo)
 */
void servo2(int angle)
{ 
    ServoType type = servoTypes[1];
    
    // 特殊处理360度的情况
    bool targetIs360 = (angle == 360);
    
    debugPrint("Servo 2: Start rotation - Current position: " + String(currentServoAngles[1]) + "°, Target position: " + String(angle) + "°");
    
    // 计算最短路径
    int targetAngle = calculateShortestPath(currentServoAngles[1], angle, 360);
    
    // 更新当前角度记录（使用正规化后的角度）
    int actualAngle;
    if (targetIs360) {
        actualAngle = 360;
    } else {
        actualAngle = angle % 360;
        if (actualAngle < 0) actualAngle += 360;
    }
    
    // 显示计算结果
    debugPrint("Servo 2: Calculation result - Target angle: " + String(targetAngle) + "°, Actual recorded angle: " + String(actualAngle) + "°");
    
    // 更新当前角度
    currentServoAngles[1] = actualAngle;
    
    debugPrint("Servo 2 moving to angle: " + String(actualAngle) + " (type: " + 
              (type == SERVO_TYPE_STANDARD ? "standard" : "continuous") + ")");
    
    // 确定是否为逆时针旋转（负角度）
    bool isCounterClockwise = (targetAngle < 0);
    if (isCounterClockwise) {
        // 负角度表示逆时针旋转，转换为正角度用于计算脉冲宽度
        targetAngle = 360 + targetAngle;  // 这会将负角度转换为等效的正角度
        debugPrint("Servo 2: Counter-clockwise rotation, converting negative angle " + String(targetAngle - 360) + "° to " + String(targetAngle) + "°");
    }
    
    // 计算脉冲宽度
    int pulsewidth;
    if (targetIs360) {
        // 360度应该映射到最大脉冲宽度（2480微秒），而不是0度的脉冲宽度
        if (type == SERVO_TYPE_STANDARD) {
            // 标准舵机：360度 = 2480微秒（最大值）
            pulsewidth = 500 + (360 * 1980 / 360); // 等于2480
        } else {
            // 连续旋转舵机：360度 = 2500微秒（最大值）
            pulsewidth = 2500;
        }
        debugPrint("Servo 2: Handling 360 degree special case, setting maximum pulse width");
    } else if (type == SERVO_TYPE_STANDARD) {
        // 将0-360度映射到标准舵机的500-2480微秒范围
        pulsewidth = 500 + (targetAngle * 1980 / 360);
    } else {
        // 连续旋转舵机 (0-360度)
        if (targetAngle <= 180) {
            // 0-180 degrees maps to 500-1500 microseconds
            pulsewidth = 500 + (targetAngle * 1000 / 180);
        } else {
            // 180-360 degrees maps to 1500-2500 microseconds
            pulsewidth = 1500 + ((targetAngle - 180) * 1000 / 180);
        }
    }
    
    // 显示脉冲宽度信息
    debugPrint("Servo 2: Pulse width set to " + String(pulsewidth) + " microseconds");
    
    // Send 50 pulses
    pinMode(SERVO_PIN_2, OUTPUT);
    for (int i = 0; i < 50; i++)
    {
        digitalWrite(SERVO_PIN_2, HIGH);     // Set servo pin to HIGH
        delayMicroseconds(pulsewidth);       // Delay for pulse width microseconds
        digitalWrite(SERVO_PIN_2, LOW);      // Set servo pin to LOW
        delayMicroseconds(20000 - pulsewidth);
    }
    delay(100);
    
    debugPrint("Servo 2: Rotation complete, current position recorded as " + String(currentServoAngles[1]) + "°");
}

/**
 * @brief Control Servo 3 to rotate to specified angle
 * @param angle Angle value (0-360 for both standard and continuous servo)
 */
void servo3(int angle)
{ 
    ServoType type = servoTypes[2];
    
    // 特殊处理360度的情况
    bool targetIs360 = (angle == 360);
    
    debugPrint("Servo 3: Start rotation - Current position: " + String(currentServoAngles[2]) + "°, Target position: " + String(angle) + "°");
    
    // 计算最短路径
    int targetAngle = calculateShortestPath(currentServoAngles[2], angle, 360);
    
    // 更新当前角度记录（使用正规化后的角度）
    int actualAngle;
    if (targetIs360) {
        actualAngle = 360;
    } else {
        actualAngle = angle % 360;
        if (actualAngle < 0) actualAngle += 360;
    }
    
    // 更新当前角度
    currentServoAngles[2] = actualAngle;
    
    debugPrint("Servo 3 moving to angle: " + String(actualAngle) + " (type: " + 
              (type == SERVO_TYPE_STANDARD ? "standard" : "continuous") + ")");
    
    // 确定是否为逆时针旋转（负角度）
    bool isCounterClockwise = (targetAngle < 0);
    if (isCounterClockwise) {
        // 负角度表示逆时针旋转，转换为正角度用于计算脉冲宽度
        targetAngle = 360 + targetAngle;  // 这会将负角度转换为等效的正角度
        debugPrint("Servo 3: Counter-clockwise rotation, converting negative angle " + String(targetAngle - 360) + "° to " + String(targetAngle) + "°");
    }
    
    // 计算脉冲宽度
    int pulsewidth;
    if (targetIs360) {
        // 360度应该映射到最大脉冲宽度（2480微秒），而不是0度的脉冲宽度
        if (type == SERVO_TYPE_STANDARD) {
            // 标准舵机：360度 = 2480微秒（最大值）
            pulsewidth = 500 + (360 * 1980 / 360); // 等于2480
        } else {
            // 连续旋转舵机：360度 = 2500微秒（最大值）
            pulsewidth = 2500;
        }
        debugPrint("Servo 3: Handling 360 degree special case, setting maximum pulse width");
    } else if (type == SERVO_TYPE_STANDARD) {
        // 将0-360度映射到标准舵机的500-2480微秒范围
        pulsewidth = 500 + (targetAngle * 1980 / 360);
    } else {
        // 连续旋转舵机 (0-360度)
        if (targetAngle <= 180) {
            // 0-180 degrees maps to 500-1500 microseconds
            pulsewidth = 500 + (targetAngle * 1000 / 180);
        } else {
            // 180-360 degrees maps to 1500-2500 microseconds
            pulsewidth = 1500 + ((targetAngle - 180) * 1000 / 180);
        }
    }
    
    // 显示脉冲宽度信息
    debugPrint("Servo 3: Pulse width set to " + String(pulsewidth) + " microseconds");
    
    // Send 50 pulses
    pinMode(SERVO_PIN_3, OUTPUT);
    for (int i = 0; i < 50; i++)
    {
        digitalWrite(SERVO_PIN_3, HIGH);     // Set servo pin to HIGH
        delayMicroseconds(pulsewidth);       // Delay for pulse width microseconds
        digitalWrite(SERVO_PIN_3, LOW);      // Set servo pin to LOW
        delayMicroseconds(20000 - pulsewidth);
    }
    delay(100);
    
    debugPrint("Servo 3: Rotation complete, current position recorded as " + String(currentServoAngles[2]) + "°");
}

/**
 * @brief Control all three servos to rotate to specified angles simultaneously
 * @param angle1 Servo 1 angle value (0-360 regardless of type)
 * @param angle2 Servo 2 angle value (0-360 regardless of type)
 * @param angle3 Servo 3 angle value (0-360 regardless of type)
 */
void servoAll(int angle1, int angle2, int angle3)
{
    // 特殊处理360度的情况
    bool target1Is360 = (angle1 == 360);
    bool target2Is360 = (angle2 == 360);
    bool target3Is360 = (angle3 == 360);
    
    // 计算最短路径
    int targetAngle1 = calculateShortestPath(currentServoAngles[0], angle1, 360);
    int targetAngle2 = calculateShortestPath(currentServoAngles[1], angle2, 360);
    int targetAngle3 = calculateShortestPath(currentServoAngles[2], angle3, 360);
    
    // 更新当前角度记录（使用正规化后的角度）
    int actualAngle1, actualAngle2, actualAngle3;
    
    // 处理舵机1的角度
    if (target1Is360) {
        actualAngle1 = 360;
    } else {
        actualAngle1 = angle1 % 360;
        if (actualAngle1 < 0) actualAngle1 += 360;
    }
    
    // 处理舵机2的角度
    if (target2Is360) {
        actualAngle2 = 360;
    } else {
        actualAngle2 = angle2 % 360;
        if (actualAngle2 < 0) actualAngle2 += 360;
    }
    
    // 处理舵机3的角度
    if (target3Is360) {
        actualAngle3 = 360;
    } else {
        actualAngle3 = angle3 % 360;
        if (actualAngle3 < 0) actualAngle3 += 360;
    }
    
    // 更新当前角度记录
    currentServoAngles[0] = actualAngle1;
    currentServoAngles[1] = actualAngle2;
    currentServoAngles[2] = actualAngle3;
    
    debugPrint("Moving all servos to angles: " + String(actualAngle1) + ", " + String(actualAngle2) + ", " + String(actualAngle3));
    
    // 处理可能的逆时针旋转（负角度）
    int processedAngle1 = targetAngle1;
    int processedAngle2 = targetAngle2;
    int processedAngle3 = targetAngle3;
    
    if (processedAngle1 < 0) processedAngle1 = 360 + processedAngle1;
    if (processedAngle2 < 0) processedAngle2 = 360 + processedAngle2;
    if (processedAngle3 < 0) processedAngle3 = 360 + processedAngle3;
    
    // 计算脉冲宽度
    int pulsewidth1, pulsewidth2, pulsewidth3;
    
    // 舵机1脉冲宽度计算
    if (target1Is360) {
        if (servoTypes[0] == SERVO_TYPE_STANDARD) {
            pulsewidth1 = 2480;
        } else {
            pulsewidth1 = 2500;
        }
    } else if (servoTypes[0] == SERVO_TYPE_STANDARD) {
        pulsewidth1 = 500 + (processedAngle1 * 1980 / 360);
    } else {
        if (processedAngle1 <= 180) {
            pulsewidth1 = 500 + (processedAngle1 * 1000 / 180);
        } else {
            pulsewidth1 = 1500 + ((processedAngle1 - 180) * 1000 / 180);
        }
    }
    
    // 舵机2脉冲宽度计算
    if (target2Is360) {
        if (servoTypes[1] == SERVO_TYPE_STANDARD) {
            pulsewidth2 = 2480;
        } else {
            pulsewidth2 = 2500;
        }
    } else if (servoTypes[1] == SERVO_TYPE_STANDARD) {
        pulsewidth2 = 500 + (processedAngle2 * 1980 / 360);
    } else {
        if (processedAngle2 <= 180) {
            pulsewidth2 = 500 + (processedAngle2 * 1000 / 180);
        } else {
            pulsewidth2 = 1500 + ((processedAngle2 - 180) * 1000 / 180);
        }
    }
    
    // 舵机3脉冲宽度计算
    if (target3Is360) {
        if (servoTypes[2] == SERVO_TYPE_STANDARD) {
            pulsewidth3 = 2480;
        } else {
            pulsewidth3 = 2500;
        }
    } else if (servoTypes[2] == SERVO_TYPE_STANDARD) {
        pulsewidth3 = 500 + (processedAngle3 * 1980 / 360);
    } else {
        if (processedAngle3 <= 180) {
            pulsewidth3 = 500 + (processedAngle3 * 1000 / 180);
        } else {
            pulsewidth3 = 1500 + ((processedAngle3 - 180) * 1000 / 180);
        }
    }
    
    debugPrint("Pulse widths set: " + String(pulsewidth1) + ", " + String(pulsewidth2) + ", " + String(pulsewidth3) + " microseconds");
    
    // Set pin modes
    pinMode(SERVO_PIN_1, OUTPUT);
    pinMode(SERVO_PIN_2, OUTPUT);
    pinMode(SERVO_PIN_3, OUTPUT);
    
    // Send 50 pulses
    for (int i = 0; i < 50; i++)
    {
        // Control servo 1
        digitalWrite(SERVO_PIN_1, HIGH);
        delayMicroseconds(pulsewidth1);
        digitalWrite(SERVO_PIN_1, LOW);
        
        // Control servo 2
        digitalWrite(SERVO_PIN_2, HIGH);
        delayMicroseconds(pulsewidth2);
        digitalWrite(SERVO_PIN_2, LOW);
        
        // Control servo 3
        digitalWrite(SERVO_PIN_3, HIGH);
        delayMicroseconds(pulsewidth3);
        digitalWrite(SERVO_PIN_3, LOW);
        
        // Wait for next cycle
        delayMicroseconds(20000 - max(pulsewidth1, max(pulsewidth2, pulsewidth3)));
    }
    delay(100);
    
    debugPrint("All servos rotation complete, current positions: " + String(currentServoAngles[0]) + "°, " + 
              String(currentServoAngles[1]) + "°, " + String(currentServoAngles[2]) + "°");
}

/**
 * @brief Initialize all servos to specified positions
 * @param angle1 Servo 1 initial angle, default is 90 degrees (center position)
 * @param angle2 Servo 2 initial angle, default is 90 degrees (center position)
 * @param angle3 Servo 3 initial angle, default is 90 degrees (center position)
 * @param moveToPosition Whether to move servos to specified positions during initialization, default is false
 */
void initServos(int angle1 = 90, int angle2 = 90, int angle3 = 90, bool moveToPosition = false)
{
    debugPrint("Initializing servos...");
    
    // Set pin modes
    pinMode(SERVO_PIN_1, OUTPUT);
    pinMode(SERVO_PIN_2, OUTPUT);
    pinMode(SERVO_PIN_3, OUTPUT);
    
    // 初始化当前角度记录
    currentServoAngles[0] = angle1 % 360;
    if (currentServoAngles[0] < 0) currentServoAngles[0] += 360;
    currentServoAngles[1] = angle2 % 360;
    if (currentServoAngles[1] < 0) currentServoAngles[1] += 360;
    currentServoAngles[2] = angle3 % 360;
    if (currentServoAngles[2] < 0) currentServoAngles[2] += 360;
    
    // Only move servos when moveToPosition is true
    if (moveToPosition) {
        // Set all servos to initial positions simultaneously
        servoAll(angle1, angle2, angle3);
        debugPrint("Servos moved to initial positions");
    } else {
        debugPrint("Servo pins initialized, but not moved (safety feature)");
    }
    
    debugPrint("Servo initialization complete!");
}

/**
 * @brief 设置所有舵机为连续旋转模式 (0-360度)
 */
void setAllServosContinuous()
{
    for (int i = 0; i < 3; i++) {
        servoTypes[i] = SERVO_TYPE_CONTINUOUS;
    }
    debugPrint("All servos set to continuous rotation mode (0-360 degrees)");
}

/**
 * @brief 设置所有舵机为标准模式 (0-180度)
 */
void setAllServosStandard()
{
    for (int i = 0; i < 3; i++) {
        servoTypes[i] = SERVO_TYPE_STANDARD;
    }
    debugPrint("All servos set to standard mode (0-180 degrees)");
}

/**
 * @brief 显示当前舵机角度和类型
 */
void showServoStatus()
{
    debugPrint("=== Servo Status ===");
    for (int i = 0; i < 3; i++) {
        String typeStr = (servoTypes[i] == SERVO_TYPE_STANDARD) ? "Standard (0-180)" : "Continuous (0-360)";
        debugPrint("Servo " + String(i+1) + ": " + String(currentServoAngles[i]) + "° (Type: " + typeStr + ")");
    }
    debugPrint("===================");
}

#endif
