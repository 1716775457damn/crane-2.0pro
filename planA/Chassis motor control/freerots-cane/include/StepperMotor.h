#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include <Arduino.h>

// 电机引脚定义
#define SLEEP1  4
#define DIR1    5
#define STEP1   6

#define SLEEP2  7
#define DIR2    15
#define STEP2   16

// 方向定义
#define DIRECTION_CW  1   // 顺时针
#define DIRECTION_CCW 0   // 逆时针

// 电机状态结构体
struct MotorState {
  bool isRunning;             // 电机是否在运行
  unsigned long startTime;    // 开始运行时间
  unsigned long runDuration;  // 运行持续时间(毫秒)
};

// 全局电机状态
MotorState motor1State = {false, 0, 0};
MotorState motor2State = {false, 0, 0};

/**
 * @brief 初始化步进电机
 */
inline void initSteppers() {
    // 配置步进电机1引脚
    pinMode(SLEEP1, OUTPUT);
    pinMode(DIR1, OUTPUT);
    pinMode(STEP1, OUTPUT);
    
    // 配置步进电机2引脚
    pinMode(SLEEP2, OUTPUT);
    pinMode(DIR2, OUTPUT);
    pinMode(STEP2, OUTPUT);
    
    // 初始状态：不休眠（启用）
    digitalWrite(SLEEP1, 1);
    digitalWrite(SLEEP2, 1);
    
    // 设置初始方向为顺时针
    digitalWrite(DIR1, DIRECTION_CW);
    digitalWrite(DIR2, DIRECTION_CW);
    
    // 初始速度设为0（停止）
    analogWrite(STEP1, 0);
    analogWrite(STEP2, 0);
    
    // 初始化电机状态
    motor1State.isRunning = false;
    motor2State.isRunning = false;
}

/**
 * @brief 内部函数 - 更新电机状态
 */
inline void updateSteppers() {
    unsigned long currentTime = millis();
    
    // 检查电机1是否需要停止
    if (motor1State.isRunning && motor1State.runDuration != 0xFFFFFFFF && 
        (currentTime - motor1State.startTime >= motor1State.runDuration)) {
        // 停止电机1
        analogWrite(STEP1, 0);
        digitalWrite(SLEEP1, 0);
        motor1State.isRunning = false;
    }
    
    // 检查电机2是否需要停止
    if (motor2State.isRunning && motor2State.runDuration != 0xFFFFFFFF && 
        (currentTime - motor2State.startTime >= motor2State.runDuration)) {
        // 停止电机2
        analogWrite(STEP2, 0);
        digitalWrite(SLEEP2, 0);
        motor2State.isRunning = false;
    }
}

/**
 * @brief 简化的三参数步进电机控制函数
 * @param motorNumber 电机号 (1或2)
 * @param speed 速度 (0-255)
 * @param steps 步数 (正数顺时针，负数逆时针，0表示停止，|steps|=运行时间/10ms)
 * @return 成功返回true，失败返回false
 */
inline bool stepper(uint8_t motorNumber, uint8_t speed, int32_t steps) {
    if (motorNumber != 1 && motorNumber != 2) {
        return false;
    }
    
    // 获取引脚
    uint8_t sleepPin = (motorNumber == 1) ? SLEEP1 : SLEEP2;
    uint8_t dirPin = (motorNumber == 1) ? DIR1 : DIR2;
    uint8_t stepPin = (motorNumber == 1) ? STEP1 : STEP2;
    
    // 获取状态引用
    MotorState& motorState = (motorNumber == 1) ? motor1State : motor2State;
    
    // 如果速度为0或步数为0，停止电机
    if (speed == 0 || steps == 0) {
        analogWrite(stepPin, 0);
        digitalWrite(sleepPin, 0);
        motorState.isRunning = false;
        return true;
    }
    
    // 唤醒电机
    digitalWrite(sleepPin, 1);
    
    // 根据步数的正负确定方向
    bool direction = (steps > 0) ? DIRECTION_CW : DIRECTION_CCW;
    digitalWrite(dirPin, direction);
    
    // 速度限制
    if (speed > 255) speed = 255;
    if (speed < 1) speed = 1;
    
    // 设置速度
    analogWrite(stepPin, speed);
    
    // 更新状态
    motorState.isRunning = true;
    motorState.startTime = millis();
    
    // 特殊处理：对于特定值，设置为持续运行
    if (steps == 1 && speed > 0) {
        // 持续运行（直到手动停止）
        motorState.runDuration = 0xFFFFFFFF;
    } else {
        // 正常定时运行 - 步数决定时间
        motorState.runDuration = abs(steps) * 10; // 每步10毫秒
    }
    
    // 启动后立即执行一次更新，以处理特殊情况
    updateSteppers();
    
    return true;
}

/**
 * @brief 停止电机
 */
inline bool stopStepper(uint8_t motorNumber) {
    return stepper(motorNumber, 0, 0);
}

/**
 * @brief 停止所有电机
 */
inline void stopAllSteppers() {
    stopStepper(1);
    stopStepper(2);
}

/**
 * @brief 检查电机是否在运行
 */
inline bool isStepperRunning(uint8_t motorNumber) {
    if (motorNumber == 1) {
        return motor1State.isRunning;
    } else if (motorNumber == 2) {
        return motor2State.isRunning;
    }
    return false;
}

#endif // STEPPER_MOTOR_H 