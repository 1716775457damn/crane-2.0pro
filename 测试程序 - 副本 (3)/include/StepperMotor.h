#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include <Arduino.h>
#include "pins.h" // 引入统一的引脚定义

// 方向定义
#define DIRECTION_CW 1  // 顺时针
#define DIRECTION_CCW 0 // 逆时针

// 电机状态结构体
struct MotorState
{
    bool isRunning;              // 电机是否在运行
    unsigned long startTime;     // 开始运行时间
    unsigned long runDuration;   // 运行持续时间(毫秒)
    unsigned long pulseInterval; // 脉冲间隔(微秒)
    uint32_t remainingSteps;     // 剩余步数
};

// 全局电机状态 - 优化默认脉冲间隔为500微秒（更快响应）
MotorState motor1State = {false, 0, 0, 500, 0};
MotorState motor2State = {false, 0, 0, 500, 0};
// 新增第3和第4个电机的状态
MotorState motor3State = {false, 0, 0, 500, 0};
MotorState motor4State = {false, 0, 0, 500, 0};

/**
 * @brief 初始化步进电机
 */
inline void initSteppers()
{
    // 配置步进电机1引脚
    pinMode(SLEEP1, OUTPUT);
    pinMode(DIR1, OUTPUT);
    pinMode(STEP1, OUTPUT);

    // 配置步进电机2引脚
    pinMode(SLEEP2, OUTPUT);
    pinMode(DIR2, OUTPUT);
    pinMode(STEP2, OUTPUT);

    // 配置步进电机3引脚
    pinMode(SLEEP3, OUTPUT);
    pinMode(DIR3, OUTPUT);
    pinMode(STEP3, OUTPUT);

    // 配置步进电机4引脚
    pinMode(SLEEP4, OUTPUT);
    pinMode(DIR4, OUTPUT);
    pinMode(STEP4, OUTPUT);

    // 初始状态：不休眠（启用）
    digitalWrite(SLEEP1, 1);
    digitalWrite(SLEEP2, 1);
    digitalWrite(SLEEP3, 1);
    digitalWrite(SLEEP4, 1);

    // 设置初始方向为顺时针
    digitalWrite(DIR1, DIRECTION_CW);
    digitalWrite(DIR2, DIRECTION_CW);
    digitalWrite(DIR3, DIRECTION_CW);
    digitalWrite(DIR4, DIRECTION_CW);

    // 初始电平设为低
    digitalWrite(STEP1, LOW);
    digitalWrite(STEP2, LOW);
    digitalWrite(STEP3, LOW);
    digitalWrite(STEP4, LOW);

    // 初始化电机状态
    motor1State.isRunning = false;
    motor2State.isRunning = false;
    motor3State.isRunning = false;
    motor4State.isRunning = false;
}

/**
 * @brief 将速度值转换为脉冲间隔(微秒)
 * @param speed 速度值(1-255),值越大速度越快
 * @return 脉冲间隔(微秒)
 */
inline unsigned long speedToPulseInterval(uint8_t speed)
{
    // 确保速度在有效范围内
    if (speed < 1)
        speed = 1;
    if (speed > 255)
        speed = 255;

    // 映射速度到脉冲间隔(微秒): 255->500us(最快), 1->5000us(最慢)
    return map(speed, 1, 255, 5000, 500);
}

/**
 * @brief 发送一个脉冲到步进电机
 * @param stepPin 步进引脚
 */
inline void sendPulse(uint8_t stepPin)
{
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(10); // 脉冲宽度
    digitalWrite(stepPin, LOW);
}

/**
 * @brief 内部函数 - 更新电机状态
 * 注意: runAllSteppers函数现已包含同步脉冲实现，如果需要四个电机同步运动，建议使用runAllSteppers函数
 */
inline void updateSteppers()
{
    static unsigned long lastUpdateTime1 = 0;
    static unsigned long lastUpdateTime2 = 0;
    static unsigned long lastUpdateTime3 = 0;
    static unsigned long lastUpdateTime4 = 0;
    unsigned long currentMicros = micros();

    // 更新电机1
    if (motor1State.isRunning)
    {
        if (currentMicros - lastUpdateTime1 >= motor1State.pulseInterval)
        {
            lastUpdateTime1 = currentMicros;

            // 发送一个脉冲
            sendPulse(STEP1);

            // 如果有步数限制，则减少剩余步数
            if (motor1State.remainingSteps > 0)
            {
                motor1State.remainingSteps--;
                if (motor1State.remainingSteps == 0)
                {
                    // 步数完成，停止电机
                    digitalWrite(SLEEP1, 0);
                    motor1State.isRunning = false;
                }
            }
        }
    }

    // 更新电机2
    if (motor2State.isRunning)
    {
        if (currentMicros - lastUpdateTime2 >= motor2State.pulseInterval)
        {
            lastUpdateTime2 = currentMicros;

            // 发送一个脉冲
            sendPulse(STEP2);

            // 如果有步数限制，则减少剩余步数
            if (motor2State.remainingSteps > 0)
            {
                motor2State.remainingSteps--;
                if (motor2State.remainingSteps == 0)
                {
                    // 步数完成，停止电机
                    digitalWrite(SLEEP2, 0);
                    motor2State.isRunning = false;
                }
            }
        }
    }

    // 更新电机3
    if (motor3State.isRunning)
    {
        if (currentMicros - lastUpdateTime3 >= motor3State.pulseInterval)
        {
            lastUpdateTime3 = currentMicros;

            // 发送一个脉冲
            sendPulse(STEP3);

            // 如果有步数限制，则减少剩余步数
            if (motor3State.remainingSteps > 0)
            {
                motor3State.remainingSteps--;
                if (motor3State.remainingSteps == 0)
                {
                    // 步数完成，停止电机
                    digitalWrite(SLEEP3, 0);
                    motor3State.isRunning = false;
                }
            }
        }
    }

    // 更新电机4
    if (motor4State.isRunning)
    {
        if (currentMicros - lastUpdateTime4 >= motor4State.pulseInterval)
        {
            lastUpdateTime4 = currentMicros;

            // 发送一个脉冲
            sendPulse(STEP4);

            // 如果有步数限制，则减少剩余步数
            if (motor4State.remainingSteps > 0)
            {
                motor4State.remainingSteps--;
                if (motor4State.remainingSteps == 0)
                {
                    // 步数完成，停止电机
                    digitalWrite(SLEEP4, 0);
                    motor4State.isRunning = false;
                }
            }
        }
    }
}

/**
 * @brief 简化的三参数步进电机控制函数
 * @param motorNumber 电机号 (1-4)
 * @param speed 速度 (1-255，值越大速度越快)
 * @param steps 步数 (正数顺时针，负数逆时针，0表示停止)
 * @return 成功返回true，失败返回false
 */
inline bool stepper(uint8_t motorNumber, uint8_t speed, int32_t steps)
{
    if (motorNumber < 1 || motorNumber > 4)
    {
        return false;
    }

    // 获取引脚
    uint8_t sleepPin, dirPin;
    MotorState *motorState;

    switch (motorNumber)
    {
    case 1:
        sleepPin = SLEEP1;
        dirPin = DIR1;
        motorState = &motor1State;
        break;
    case 2:
        sleepPin = SLEEP2;
        dirPin = DIR2;
        motorState = &motor2State;
        break;
    case 3:
        sleepPin = SLEEP3;
        dirPin = DIR3;
        motorState = &motor3State;
        break;
    case 4:
        sleepPin = SLEEP4;
        dirPin = DIR4;
        motorState = &motor4State;
        break;
    }

    // 如果速度为0或步数为0，停止电机
    if (speed == 0 || steps == 0)
    {
        digitalWrite(sleepPin, 0);
        motorState->isRunning = false;
        motorState->remainingSteps = 0;
        return true;
    }

    // 唤醒电机
    digitalWrite(sleepPin, 1);

    // 根据步数的正负确定方向
    bool direction = (steps >= 0) ? DIRECTION_CW : DIRECTION_CCW;
    digitalWrite(dirPin, direction);

    // 速度转换为脉冲间隔
    motorState->pulseInterval = speedToPulseInterval(speed);

    // 更新状态
    motorState->isRunning = true;
    motorState->startTime = millis();
    motorState->remainingSteps = abs(steps);

    // 特殊处理持续运行模式
    if (steps == 1 || steps == -1)
    {
        motorState->remainingSteps = 0; // 0表示无限运行
    }

    return true;
}

/**
 * @brief 封装好的单行调用函数 - 控制电机并等待停止
 * @param motorNumber 电机号 (1-4)
 * @param speed 速度 (1-255，值越大速度越快)
 * @param steps 步数 (正数顺时针，负数逆时针，0表示停止)
 * @param waitUntilStop 是否等待电机停止(默认为true)
 * @return 成功返回true，失败返回false
 */
inline bool runStepper(uint8_t motorNumber, uint8_t speed, int32_t steps, bool waitUntilStop = true)
{
    // 启动电机
    if (!stepper(motorNumber, speed, steps))
    {
        return false;
    }

    // 如果是持续运行模式(steps=±1)或不需要等待，则直接返回
    if (abs(steps) == 1 || !waitUntilStop)
    {
        return true;
    }

    // 获取对应电机状态和引脚
    MotorState *motorState;
    uint8_t stepPin;

    switch (motorNumber)
    {
    case 1:
        motorState = &motor1State;
        stepPin = STEP1;
        break;
    case 2:
        motorState = &motor2State;
        stepPin = STEP2;
        break;
    case 3:
        motorState = &motor3State;
        stepPin = STEP3;
        break;
    case 4:
        motorState = &motor4State;
        stepPin = STEP4;
        break;
    default:
        return false;
    }

    // 等待电机停止，使用直接脉冲发送方式
    static unsigned long lastUpdateTime = 0;

    while (motorState->isRunning)
    {
        unsigned long currentMicros = micros();

        if (currentMicros - lastUpdateTime >= motorState->pulseInterval)
        {
            lastUpdateTime = currentMicros;

            // 发送一个脉冲
            sendPulse(stepPin);

            // 如果有步数限制，则减少剩余步数
            if (motorState->remainingSteps > 0)
            {
                motorState->remainingSteps--;
                if (motorState->remainingSteps == 0)
                {
                    // 步数完成，停止电机
                    uint8_t sleepPin;
                    switch (motorNumber)
                    {
                    case 1:
                        sleepPin = SLEEP1;
                        break;
                    case 2:
                        sleepPin = SLEEP2;
                        break;
                    case 3:
                        sleepPin = SLEEP3;
                        break;
                    case 4:
                        sleepPin = SLEEP4;
                        break;
                    }
                    digitalWrite(sleepPin, 0);
                    motorState->isRunning = false;
                }
            }
        }

        // 短暂延时减少CPU占用 - 优化为更小延时
        delayMicroseconds(5);
    }

    return true;
}

/**
 * @brief 同时控制两个电机并等待它们都停止
 * @param speed1 电机1速度 (1-255)
 * @param steps1 电机1步数
 * @param speed2 电机2速度 (1-255)
 * @param steps2 电机2步数
 * @return 成功返回true
 */
inline bool runSteppers(uint8_t speed1, int32_t steps1, uint8_t speed2, int32_t steps2)
{
    // 启动两个电机
    stepper(1, speed1, steps1);
    stepper(2, speed2, steps2);

    // 如果两个电机都是持续运行模式，则直接返回
    if ((abs(steps1) == 1 || steps1 == 0) && (abs(steps2) == 1 || steps2 == 0))
    {
        return true;
    }

    // 等待两个电机都停止，使用同步脉冲方式
    while (motor1State.isRunning || motor2State.isRunning)
    {
        // 获取当前微秒数
        unsigned long currentMicros = micros();

        // 定义静态变量记录上次更新时间
        static unsigned long lastUpdateTime1 = 0;
        static unsigned long lastUpdateTime2 = 0;

        // 同步更新电机1
        if (motor1State.isRunning)
        {
            if (currentMicros - lastUpdateTime1 >= motor1State.pulseInterval)
            {
                lastUpdateTime1 = currentMicros;

                // 发送一个脉冲
                sendPulse(STEP1);

                // 如果有步数限制，则减少剩余步数
                if (motor1State.remainingSteps > 0)
                {
                    motor1State.remainingSteps--;
                    if (motor1State.remainingSteps == 0)
                    {
                        // 步数完成，停止电机
                        digitalWrite(SLEEP1, 0);
                        motor1State.isRunning = false;
                    }
                }
            }
        }

        // 同步更新电机2
        if (motor2State.isRunning)
        {
            if (currentMicros - lastUpdateTime2 >= motor2State.pulseInterval)
            {
                lastUpdateTime2 = currentMicros;

                // 发送一个脉冲
                sendPulse(STEP2);

                // 如果有步数限制，则减少剩余步数
                if (motor2State.remainingSteps > 0)
                {
                    motor2State.remainingSteps--;
                    if (motor2State.remainingSteps == 0)
                    {
                        // 步数完成，停止电机
                        digitalWrite(SLEEP2, 0);
                        motor2State.isRunning = false;
                    }
                }
            }
        }

        // 短暂延时减少CPU占用 - 优化为更小延时
        delayMicroseconds(5);
    }

    return true;
}

/**
 * @brief 同时控制四个电机并等待它们都停止
 * @param speed1 电机1速度 (1-255)
 * @param steps1 电机1步数
 * @param speed2 电机2速度 (1-255)
 * @param steps2 电机2步数
 * @param speed3 电机3速度 (1-255)
 * @param steps3 电机3步数
 * @param speed4 电机4速度 (1-255)
 * @param steps4 电机4步数
 * @return 成功返回true
 */
inline bool runAllSteppers(uint8_t speed1, int32_t steps1, uint8_t speed2, int32_t steps2,
                           uint8_t speed3, int32_t steps3, uint8_t speed4, int32_t steps4)
{
    // 启动四个电机
    stepper(1, speed1, steps1);
    stepper(2, speed2, steps2);
    stepper(3, speed3, steps3);
    stepper(4, speed4, steps4);

    // 如果所有电机都是持续运行模式，则直接返回
    if ((abs(steps1) == 1 || steps1 == 0) &&
        (abs(steps2) == 1 || steps2 == 0) &&
        (abs(steps3) == 1 || steps3 == 0) &&
        (abs(steps4) == 1 || steps4 == 0))
    {
        return true;
    }

    // 等待所有电机都停止，使用同步脉冲方式
    while (motor1State.isRunning || motor2State.isRunning ||
           motor3State.isRunning || motor4State.isRunning)
    {
        // 获取当前微秒数
        unsigned long currentMicros = micros();

        // 定义静态变量记录上次更新时间
        static unsigned long lastUpdateTime1 = 0;
        static unsigned long lastUpdateTime2 = 0;
        static unsigned long lastUpdateTime3 = 0;
        static unsigned long lastUpdateTime4 = 0;

        // 同步更新电机1
        if (motor1State.isRunning)
        {
            if (currentMicros - lastUpdateTime1 >= motor1State.pulseInterval)
            {
                lastUpdateTime1 = currentMicros;

                // 发送一个脉冲
                sendPulse(STEP1);

                // 如果有步数限制，则减少剩余步数
                if (motor1State.remainingSteps > 0)
                {
                    motor1State.remainingSteps--;
                    if (motor1State.remainingSteps == 0)
                    {
                        // 步数完成，停止电机
                        digitalWrite(SLEEP1, 0);
                        motor1State.isRunning = false;
                    }
                }
            }
        }

        // 同步更新电机2
        if (motor2State.isRunning)
        {
            if (currentMicros - lastUpdateTime2 >= motor2State.pulseInterval)
            {
                lastUpdateTime2 = currentMicros;

                // 发送一个脉冲
                sendPulse(STEP2);

                // 如果有步数限制，则减少剩余步数
                if (motor2State.remainingSteps > 0)
                {
                    motor2State.remainingSteps--;
                    if (motor2State.remainingSteps == 0)
                    {
                        // 步数完成，停止电机
                        digitalWrite(SLEEP2, 0);
                        motor2State.isRunning = false;
                    }
                }
            }
        }

        // 同步更新电机3
        if (motor3State.isRunning)
        {
            if (currentMicros - lastUpdateTime3 >= motor3State.pulseInterval)
            {
                lastUpdateTime3 = currentMicros;

                // 发送一个脉冲
                sendPulse(STEP3);

                // 如果有步数限制，则减少剩余步数
                if (motor3State.remainingSteps > 0)
                {
                    motor3State.remainingSteps--;
                    if (motor3State.remainingSteps == 0)
                    {
                        // 步数完成，停止电机
                        digitalWrite(SLEEP3, 0);
                        motor3State.isRunning = false;
                    }
                }
            }
        }

        // 同步更新电机4
        if (motor4State.isRunning)
        {
            if (currentMicros - lastUpdateTime4 >= motor4State.pulseInterval)
            {
                lastUpdateTime4 = currentMicros;

                // 发送一个脉冲
                sendPulse(STEP4);

                // 如果有步数限制，则减少剩余步数
                if (motor4State.remainingSteps > 0)
                {
                    motor4State.remainingSteps--;
                    if (motor4State.remainingSteps == 0)
                    {
                        // 步数完成，停止电机
                        digitalWrite(SLEEP4, 0);
                        motor4State.isRunning = false;
                    }
                }
            }
        }

        // 短暂延时减少CPU占用 - 优化为更小延时
        delayMicroseconds(5);
    }

    return true;
}

/**
 * @brief 停止电机
 */
inline bool stopStepper(uint8_t motorNumber)
{
    if (motorNumber < 1 || motorNumber > 4)
    {
        return false;
    }
    return stepper(motorNumber, 0, 0);
}

/**
 * @brief 停止所有电机
 */
inline void stopAllSteppers()
{
    stopStepper(1);
    stopStepper(2);
    stopStepper(3);
    stopStepper(4);
}

/**
 * @brief 检查电机是否在运行
 */
inline bool isStepperRunning(uint8_t motorNumber)
{
    switch (motorNumber)
    {
    case 1:
        return motor1State.isRunning;
    case 2:
        return motor2State.isRunning;
    case 3:
        return motor3State.isRunning;
    case 4:
        return motor4State.isRunning;
    default:
        return false;
    }
}

//=========================
// 绝对位置控制功能
//=========================

// 步进电机当前位置记录 (单位: 步数)
static long stepper_current_position[4] = {0, 0, 0, 0}; // 4个电机的当前位置

// 步进电机默认速度
static uint8_t stepper_default_speed = 100; // 默认速度 (1-255)

/**
 * @brief 重置所有步进电机位置为零点
 */
inline void stepper_reset_all_positions()
{
    for (int i = 0; i < 4; i++)
    {
        stepper_current_position[i] = 0;
    }
    Serial.println("All stepper motor positions reset to zero");
}

/**
 * @brief 重置指定步进电机位置为零点
 * @param motorNumber 电机号 (1-4)
 */
inline void stepper_reset_position(uint8_t motorNumber)
{
    if (motorNumber >= 1 && motorNumber <= 4)
    {
        stepper_current_position[motorNumber - 1] = 0;
        Serial.println("Stepper motor " + String(motorNumber) + " position reset to zero");
    }
    else
    {
        Serial.println("Error: Motor number must be 1-4");
    }
}

/**
 * @brief 设置指定步进电机的当前位置
 * @param motorNumber 电机号 (1-4)
 * @param position 位置值 (步数)
 */
inline void stepper_set_position(uint8_t motorNumber, long position)
{
    if (motorNumber >= 1 && motorNumber <= 4)
    {
        stepper_current_position[motorNumber - 1] = position;
        Serial.println("Stepper motor " + String(motorNumber) + " position set to " + String(position) + " steps");
    }
    else
    {
        Serial.println("Error: Motor number must be 1-4");
    }
}

/**
 * @brief 获取指定步进电机的当前位置
 * @param motorNumber 电机号 (1-4)
 * @return 当前位置 (步数)
 */
inline long stepper_get_position(uint8_t motorNumber)
{
    if (motorNumber >= 1 && motorNumber <= 4)
    {
        return stepper_current_position[motorNumber - 1];
    }
    else
    {
        Serial.println("Error: Motor number must be 1-4");
        return 0;
    }
}

/**
 * @brief 打印所有步进电机的当前位置
 */
inline void stepper_print_all_positions()
{
    Serial.println("=== Stepper Motor Positions ===");
    for (int i = 0; i < 4; i++)
    {
        Serial.println("Motor " + String(i + 1) + ": " + String(stepper_current_position[i]) + " steps");
    }
    Serial.println("===============================");
}

/**
 * @brief 打印指定步进电机的当前位置
 * @param motorNumber 电机号 (1-4)
 */
inline void stepper_print_position(uint8_t motorNumber)
{
    if (motorNumber >= 1 && motorNumber <= 4)
    {
        Serial.println("Stepper motor " + String(motorNumber) + " position: " + String(stepper_current_position[motorNumber - 1]) + " steps");
    }
    else
    {
        Serial.println("Error: Motor number must be 1-4");
    }
}

/**
 * @brief 设置步进电机默认速度
 * @param speed 速度 (1-255)
 */
inline void stepper_set_default_speed(uint8_t speed)
{
    if (speed >= 1 && speed <= 255)
    {
        stepper_default_speed = speed;
        Serial.println("Stepper default speed set to: " + String(speed));
    }
    else
    {
        Serial.println("Error: Speed must be 1-255");
    }
}

/**
 * @brief 步进电机绝对位置移动 - 移动到指定位置
 * @param motorNumber 电机号 (1-4)
 * @param target_position 目标位置 (步数)
 * @param speed 移动速度 (1-255)，默认使用设定的默认速度
 * @return 成功返回true，失败返回false
 */
inline bool stepper_move_to_position(uint8_t motorNumber, long target_position, uint8_t speed = 0)
{
    if (motorNumber < 1 || motorNumber > 4)
    {
        Serial.println("Error: Motor number must be 1-4");
        return false;
    }

    if (speed == 0)
        speed = stepper_default_speed;

    long current_position = stepper_current_position[motorNumber - 1];
    long steps_to_move = target_position - current_position;

    if (steps_to_move == 0)
    {
        Serial.println("Motor " + String(motorNumber) + " already at target position: " + String(target_position) + " steps");
        return true;
    }

    Serial.println("Motor " + String(motorNumber) + " moving from " + String(current_position) + " to " + String(target_position) + " steps (distance=" + String(steps_to_move) + " steps)");

    // 使用现有的stepper函数进行移动
    bool result = stepper(motorNumber, speed, steps_to_move);

    if (result)
    {
        // 更新位置记录
        stepper_current_position[motorNumber - 1] = target_position;
        Serial.println("Motor " + String(motorNumber) + " moved to position " + String(target_position) + " steps");
    }
    else
    {
        Serial.println("Error: Failed to move motor " + String(motorNumber));
    }

    return result;
}

/**
 * @brief 多个步进电机同时移动到指定位置
 * @param motor1_pos 电机1目标位置 (步数)，设为LONG_MAX表示不移动
 * @param motor2_pos 电机2目标位置 (步数)，设为LONG_MAX表示不移动
 * @param motor3_pos 电机3目标位置 (步数)，设为LONG_MAX表示不移动
 * @param motor4_pos 电机4目标位置 (步数)，设为LONG_MAX表示不移动
 * @param speed 移动速度 (1-255)，默认使用设定的默认速度
 * @return 成功返回true，失败返回false
 */
inline bool stepper_move_all_to_positions(long motor1_pos = LONG_MAX, long motor2_pos = LONG_MAX,
                                          long motor3_pos = LONG_MAX, long motor4_pos = LONG_MAX,
                                          uint8_t speed = 0)
{
    if (speed == 0)
        speed = stepper_default_speed;

    Serial.println("Moving multiple motors to positions:");

    bool success = true;

    // 计算每个电机需要移动的步数
    long steps[4];
    bool move_motor[4] = {false, false, false, false};

    if (motor1_pos != LONG_MAX)
    {
        steps[0] = motor1_pos - stepper_current_position[0];
        move_motor[0] = true;
        Serial.println("  Motor 1: " + String(stepper_current_position[0]) + " -> " + String(motor1_pos) + " (" + String(steps[0]) + " steps)");
    }

    if (motor2_pos != LONG_MAX)
    {
        steps[1] = motor2_pos - stepper_current_position[1];
        move_motor[1] = true;
        Serial.println("  Motor 2: " + String(stepper_current_position[1]) + " -> " + String(motor2_pos) + " (" + String(steps[1]) + " steps)");
    }

    if (motor3_pos != LONG_MAX)
    {
        steps[2] = motor3_pos - stepper_current_position[2];
        move_motor[2] = true;
        Serial.println("  Motor 3: " + String(stepper_current_position[2]) + " -> " + String(motor3_pos) + " (" + String(steps[2]) + " steps)");
    }

    if (motor4_pos != LONG_MAX)
    {
        steps[3] = motor4_pos - stepper_current_position[3];
        move_motor[3] = true;
        Serial.println("  Motor 4: " + String(stepper_current_position[3]) + " -> " + String(motor4_pos) + " (" + String(steps[3]) + " steps)");
    }

    // 执行移动
    for (int i = 0; i < 4; i++)
    {
        if (move_motor[i] && steps[i] != 0)
        {
            bool result = stepper(i + 1, speed, steps[i]);
            if (result)
            {
                // 更新位置记录
                if (i == 0 && motor1_pos != LONG_MAX)
                    stepper_current_position[0] = motor1_pos;
                if (i == 1 && motor2_pos != LONG_MAX)
                    stepper_current_position[1] = motor2_pos;
                if (i == 2 && motor3_pos != LONG_MAX)
                    stepper_current_position[2] = motor3_pos;
                if (i == 3 && motor4_pos != LONG_MAX)
                    stepper_current_position[3] = motor4_pos;
            }
            else
            {
                success = false;
                Serial.println("Error: Failed to move motor " + String(i + 1));
            }
        }
    }

    if (success)
    {
        Serial.println("All specified motors moved to target positions");
    }

    return success;
}

/**
 * @brief 步进电机相对位置移动 - 在当前位置基础上移动指定步数
 * @param motorNumber 电机号 (1-4)
 * @param relative_steps 相对步数 (正数顺时针，负数逆时针)
 * @param speed 移动速度 (1-255)，默认使用设定的默认速度
 * @return 成功返回true，失败返回false
 */
inline bool stepper_move_relative(uint8_t motorNumber, long relative_steps, uint8_t speed = 0)
{
    if (motorNumber < 1 || motorNumber > 4)
    {
        Serial.println("Error: Motor number must be 1-4");
        return false;
    }

    long current_position = stepper_current_position[motorNumber - 1];
    long target_position = current_position + relative_steps;

    return stepper_move_to_position(motorNumber, target_position, speed);
}

#endif // STEPPER_MOTOR_H