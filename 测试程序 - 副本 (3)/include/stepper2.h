#ifndef STEPPER_CONTROL_H
#define STEPPER_CONTROL_H

/**
 * @file StepperControl.h
 * @brief ESP32-S3步进电机控制库头文件
 * @version 1.0
 * @date 2023-06-15
 *
 * 这个库提供了简单易用的API接口，用于控制两个步进电机。
 * 适用于各种步进电机驱动器，如A4988、DRV8825、TB6600等。
 */

#include <Arduino.h>

/**
 * @brief 步进电机引脚定义
 *
 * 这些引脚定义可以根据实际硬件连接进行修改。
 */
// 电机1
#define STEP1_PIN 6   // 步进控制引脚
#define DIR1_PIN 5    // 方向控制引脚
#define ENABLE1_PIN 4 // 使能控制引脚（低电平有效）

// 电机2
#define STEP2_PIN 16  // 步进控制引脚
#define DIR2_PIN 15   // 方向控制引脚
#define ENABLE2_PIN 7 // 使能控制引脚（低电平有效）

/**
 * @brief 电机方向定义
 *
 * 定义了步进电机的旋转方向常量。
 * 实际旋转方向取决于电机接线方式。
 */
#define DIRECTION_CW true   // 顺时针
#define DIRECTION_CCW false // 逆时针

/**
 * @brief 步进电机状态结构体
 *
 * 用于跟踪每个步进电机的当前状态。
 */
struct StepperState
{
    bool enabled;               // 电机是否启用
    bool direction;             // 电机方向
    uint8_t speed;              // 电机速度
    uint32_t remainingSteps;    // 剩余步数
    uint32_t totalSteps;        // 总步数
    unsigned long lastStepTime; // 上次步进时间
};

// 全局变量，用于跟踪步进电机状态
static StepperState stepper1State = {false, DIRECTION_CW, 0, 0, 0, 0};
static StepperState stepper2State = {false, DIRECTION_CW, 0, 0, 0, 0};

/**
 * @brief 初始化步进电机
 *
 * 配置所有步进电机相关的引脚。
 * 必须在使用其他函数前调用一次。
 */
inline void initSteppers()
{
    // 配置步进电机1引脚
    pinMode(STEP1_PIN, OUTPUT);
    pinMode(DIR1_PIN, OUTPUT);
    pinMode(ENABLE1_PIN, OUTPUT);

    // 配置步进电机2引脚
    pinMode(STEP2_PIN, OUTPUT);
    pinMode(DIR2_PIN, OUTPUT);
    pinMode(ENABLE2_PIN, OUTPUT);

    // 初始状态：禁用电机
    digitalWrite(ENABLE1_PIN, HIGH); // 高电平禁用
    digitalWrite(ENABLE2_PIN, HIGH);

    // 设置初始方向
    digitalWrite(DIR1_PIN, DIRECTION_CW);
    digitalWrite(DIR2_PIN, DIRECTION_CW);
}

/**
 * @brief 设置步进电机方向
 *
 * 设置指定电机的旋转方向。
 *
 * @param motorNumber 电机编号 (1 或 2)
 * @param direction 方向 (true=顺时针, false=逆时针)
 * @return 成功返回true，失败返回false
 */
inline bool setStepperDirection(uint8_t motorNumber, bool direction)
{
    if (motorNumber != 1 && motorNumber != 2)
    {
        return false;
    }

    uint8_t dirPin = (motorNumber == 1) ? DIR1_PIN : DIR2_PIN;

    // 设置方向引脚
    digitalWrite(dirPin, direction ? HIGH : LOW);

    // 更新状态
    if (motorNumber == 1)
    {
        stepper1State.direction = direction;
    }
    else
    {
        stepper2State.direction = direction;
    }

    return true;
}

/**
 * @brief 启用步进电机
 *
 * 启用指定电机，使其准备好接收控制信号。
 *
 * @param motorNumber 电机编号 (1 或 2)
 * @return 成功返回true，失败返回false
 */
inline bool enableStepper(uint8_t motorNumber)
{
    if (motorNumber != 1 && motorNumber != 2)
    {
        return false;
    }

    uint8_t enablePin = (motorNumber == 1) ? ENABLE1_PIN : ENABLE2_PIN;

    // 低电平启用
    digitalWrite(enablePin, LOW);

    // 更新状态
    if (motorNumber == 1)
    {
        stepper1State.enabled = true;
    }
    else
    {
        stepper2State.enabled = true;
    }

    return true;
}

/**
 * @brief 停止步进电机
 *
 * 停止指定电机的运行。
 *
 * @param motorNumber 电机编号 (1 或 2)
 * @return 成功返回true，失败返回false
 */
inline bool stopStepper(uint8_t motorNumber)
{
    if (motorNumber != 1 && motorNumber != 2)
    {
        return false;
    }

    // 更新状态
    if (motorNumber == 1)
    {
        stepper1State.speed = 0;
        stepper1State.remainingSteps = 0;
    }
    else
    {
        stepper2State.speed = 0;
        stepper2State.remainingSteps = 0;
    }

    return true;
}

/**
 * @brief 控制步进电机运行
 *
 * 这是主要的控制函数，用于设置电机的速度、方向和步数。
 * 如果steps为0，电机将持续运行，直到调用stopStepper()。
 *
 * @param motorNumber 电机编号 (1 或 2)
 * @param speed 速度 (1-255，0表示停止)
 * @param steps 总步数 (如果为0则一直运行)
 * @param direction 方向 (true=顺时针, false=逆时针)
 * @return 成功返回true，失败返回false
 */
inline bool stepper(uint8_t motorNumber, uint8_t speed, uint32_t steps, bool direction)
{
    // 参数检查
    if (motorNumber != 1 && motorNumber != 2)
    {
        return false; // 无效的电机编号
    }

    if (speed == 0)
    {
        // 速度为0表示停止
        return stopStepper(motorNumber);
    }

    // 设置方向
    setStepperDirection(motorNumber, direction);

    // 启用电机
    enableStepper(motorNumber);

    // 如果 steps 为 0，则持续运行
    if (steps == 0)
    {
        // 在这里实现持续运行的逻辑，可能需要在一个任务或循环中调用
        while (true)
        {
            // 生成脉冲信号
            uint8_t stepPin = (motorNumber == 1) ? STEP1_PIN : STEP2_PIN;
            digitalWrite(stepPin, HIGH);
            delayMicroseconds(100); // 脉冲宽度
            digitalWrite(stepPin, LOW);
            delayMicroseconds(1000000 / speed); // 根据速度计算延迟
        }
    }
    else
    {
        // 运行指定步数
        uint8_t stepPin = (motorNumber == 1) ? STEP1_PIN : STEP2_PIN;
        for (uint32_t i = 0; i < steps; i++)
        {
            digitalWrite(stepPin, HIGH);
            delayMicroseconds(100); // 脉冲宽度
            digitalWrite(stepPin, LOW);
            delayMicroseconds(1000000 / speed); // 根据速度计算延迟
        }
    }

    return true;
}

/**
 * @brief 控制步进电机运行指定步数
 *
 * 这是stepper()的简化版本，用于运行指定步数。
 * 方向默认为顺时针(DIRECTION_CW)。
 *
 * @param motorNumber 电机编号 (1 或 2)
 * @param speed 速度 (1-255)
 * @param steps 总步数
 * @return 成功返回true，失败返回false
 */
inline bool stepperSteps(uint8_t motorNumber, uint8_t speed, uint32_t steps)
{
    // 调用主函数，使用默认方向
    return stepper(motorNumber, speed, steps, DIRECTION_CW);
}

/**
 * @brief 禁用步进电机
 *
 * 禁用指定电机，使其进入休眠状态，减少功耗。
 *
 * @param motorNumber 电机编号 (1 或 2)
 * @return 成功返回true，失败返回false
 */
inline bool disableStepper(uint8_t motorNumber)
{
    if (motorNumber != 1 && motorNumber != 2)
    {
        return false;
    }

    uint8_t enablePin = (motorNumber == 1) ? ENABLE1_PIN : ENABLE2_PIN;

    // 高电平禁用
    digitalWrite(enablePin, HIGH);

    // 更新状态
    if (motorNumber == 1)
    {
        stepper1State.enabled = false;
    }
    else
    {
        stepper2State.enabled = false;
    }

    return true;
}

#endif // STEPPER_CONTROL_H