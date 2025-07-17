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

// 高级电机控制结构
struct AdvancedMotorState
{
    bool isRunning;
    long currentPosition;
    long targetPosition;
    long remainingSteps;

    // 速度控制
    unsigned long currentInterval;
    unsigned long targetInterval;
    unsigned long minInterval;
    unsigned long maxInterval;

    // 加速度控制
    float acceleration;
    float deceleration;
    float currentSpeed;
    float targetSpeed;
    float maxSpeed;

    // 微分控制
    long lastError;
    long errorSum;
    float kp, ki, kd; // PID参数

    // 时间控制
    unsigned long lastStepTime;
    unsigned long lastUpdateTime;

    // 平滑控制
    bool useSmoothing;
    int smoothingBuffer[5];
    int bufferIndex;
};

// 全局高级电机状态 - 使用默认初始化避免编译错误
AdvancedMotorState advancedMotor1;
AdvancedMotorState advancedMotor2;
AdvancedMotorState advancedMotor3;
AdvancedMotorState advancedMotor4;

// 初始化高级电机状态的函数
inline void initAdvancedMotorState(AdvancedMotorState *motor)
{
    motor->isRunning = false;
    motor->currentPosition = 0;
    motor->targetPosition = 0;
    motor->remainingSteps = 0;

    motor->currentInterval = 1000;
    motor->targetInterval = 1000;
    motor->minInterval = 200;
    motor->maxInterval = 5000;

    motor->acceleration = 2.0;
    motor->deceleration = 2.0;
    motor->currentSpeed = 0;
    motor->targetSpeed = 0;
    motor->maxSpeed = 1000;

    motor->lastError = 0;
    motor->errorSum = 0;
    motor->kp = 0.8;
    motor->ki = 0.1;
    motor->kd = 0.2;

    motor->lastStepTime = 0;
    motor->lastUpdateTime = 0;

    motor->useSmoothing = true;
    for (int i = 0; i < 5; i++)
    {
        motor->smoothingBuffer[i] = 0;
    }
    motor->bufferIndex = 0;
}

// 初始化所有高级电机状态
inline void initAllAdvancedMotors()
{
    initAdvancedMotorState(&advancedMotor1);
    initAdvancedMotorState(&advancedMotor2);
    initAdvancedMotorState(&advancedMotor3);
    initAdvancedMotorState(&advancedMotor4);
}

// 梯形速度控制结构
struct TrapezoidalMotorState
{
    bool isRunning;
    long currentPosition;
    long targetPosition;
    long totalSteps;
    long stepsDone;

    // 梯形速度参数
    float maxSpeed;     // 最大速度 (步/秒)
    float acceleration; // 加速度 (步/秒²)
    float deceleration; // 减速度 (步/秒²)
    float currentSpeed; // 当前速度

    // 梯形速度阶段
    enum Phase
    {
        ACCELERATION,
        CONSTANT_SPEED,
        DECELERATION,
        STOPPED
    } currentPhase;

    // 梯形速度计算参数
    long accelSteps;    // 加速阶段步数
    long decelSteps;    // 减速阶段步数
    long constantSteps; // 匀速阶段步数

    // 时间控制
    unsigned long lastStepTime;
    unsigned long stepInterval; // 当前步进间隔(微秒)

    // 方向
    bool direction; // true=正向, false=反向
};

// 梯形控制电机状态 - 增加默认加速度
TrapezoidalMotorState trapMotor1 = {false, 0, 0, 0, 0, 1500, 1000, 800, 0, TrapezoidalMotorState::STOPPED, 0, 0, 0, 0, 1000, true};
TrapezoidalMotorState trapMotor2 = {false, 0, 0, 0, 0, 1500, 1000, 800, 0, TrapezoidalMotorState::STOPPED, 0, 0, 0, 0, 1000, true};
TrapezoidalMotorState trapMotor3 = {false, 0, 0, 0, 0, 1500, 1000, 800, 0, TrapezoidalMotorState::STOPPED, 0, 0, 0, 0, 1000, true};
TrapezoidalMotorState trapMotor4 = {false, 0, 0, 0, 0, 1500, 1000, 800, 0, TrapezoidalMotorState::STOPPED, 0, 0, 0, 0, 1000, true};

// 保持原有的简单状态结构以兼容现有代码
MotorState motor1State = {false, 0, 0, 500, 0};
MotorState motor2State = {false, 0, 0, 500, 0};
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

    // 映射速度到脉冲间隔(微秒): 255->200us(最快), 1->5000us(最慢) - 优化更快响应
    return map(speed, 1, 255, 5000, 200);
}

//=========================
// 高级控制算法
//=========================

/**
 * PID控制器计算
 */
inline float calculatePID(AdvancedMotorState *motor, long error)
{
    unsigned long currentTime = micros();
    float deltaTime = (currentTime - motor->lastUpdateTime) / 1000000.0; // 转换为秒

    if (deltaTime <= 0)
        return 0;

    // 比例项
    float proportional = motor->kp * error;

    // 积分项
    motor->errorSum += error * deltaTime;
    // 积分限幅
    if (motor->errorSum > 1000)
        motor->errorSum = 1000;
    if (motor->errorSum < -1000)
        motor->errorSum = -1000;
    float integral = motor->ki * motor->errorSum;

    // 微分项
    float derivative = motor->kd * (error - motor->lastError) / deltaTime;

    // PID输出
    float output = proportional + integral + derivative;

    // 更新历史值
    motor->lastError = error;
    motor->lastUpdateTime = currentTime;

    return output;
}

/**
 * 平滑滤波器
 */
inline long smoothFilter(AdvancedMotorState *motor, long newValue)
{
    if (!motor->useSmoothing)
        return newValue;

    motor->smoothingBuffer[motor->bufferIndex] = newValue;
    motor->bufferIndex = (motor->bufferIndex + 1) % 5;

    long sum = 0;
    for (int i = 0; i < 5; i++)
    {
        sum += motor->smoothingBuffer[i];
    }

    return sum / 5;
}

/**
 * S曲线加速度控制
 */
inline void updateSCurveAcceleration(AdvancedMotorState *motor)
{
    unsigned long currentTime = micros();
    float deltaTime = (currentTime - motor->lastUpdateTime) / 1000000.0;

    if (deltaTime <= 0)
        return;

    // 计算到目标位置的距离
    long error = motor->targetPosition - motor->currentPosition;
    long absError = abs(error);

    // S曲线速度规划
    float targetSpeed = 0;

    if (absError > 100)
    { // 距离较远时，使用最大速度
        targetSpeed = motor->maxSpeed;
    }
    else if (absError > 10)
    { // 中等距离，线性减速
        targetSpeed = motor->maxSpeed * (absError / 100.0);
    }
    else
    { // 接近目标，慢速精确定位
        targetSpeed = motor->maxSpeed * 0.1;
    }

    // 平滑加速/减速
    float speedDiff = targetSpeed - motor->currentSpeed;
    float maxChange = motor->acceleration * deltaTime;

    if (abs(speedDiff) <= maxChange)
    {
        motor->currentSpeed = targetSpeed;
    }
    else if (speedDiff > 0)
    {
        motor->currentSpeed += maxChange;
    }
    else
    {
        motor->currentSpeed -= maxChange;
    }

    // 确保速度不为负
    if (motor->currentSpeed < 0)
        motor->currentSpeed = 0;

    // 转换速度为脉冲间隔
    if (motor->currentSpeed > 0)
    {
        motor->currentInterval = (unsigned long)(1000000.0 / motor->currentSpeed);
        if (motor->currentInterval < motor->minInterval)
        {
            motor->currentInterval = motor->minInterval;
        }
        if (motor->currentInterval > motor->maxInterval)
        {
            motor->currentInterval = motor->maxInterval;
        }
    }
    else
    {
        motor->currentInterval = motor->maxInterval;
    }

    motor->lastUpdateTime = currentTime;
}

//=========================
// 梯形速度控制算法
//=========================

/**
 * 获取梯形控制电机状态指针
 */
inline TrapezoidalMotorState *getTrapezoidalMotor(uint8_t motorNumber)
{
    switch (motorNumber)
    {
    case 1:
        return &trapMotor1;
    case 2:
        return &trapMotor2;
    case 3:
        return &trapMotor3;
    case 4:
        return &trapMotor4;
    default:
        return nullptr;
    }
}

/**
 * 计算梯形速度参数
 */
inline void calculateTrapezoidalParams(TrapezoidalMotorState *motor)
{
    motor->totalSteps = abs(motor->targetPosition - motor->currentPosition);
    motor->direction = (motor->targetPosition > motor->currentPosition);

    if (motor->totalSteps == 0)
    {
        motor->currentPhase = TrapezoidalMotorState::STOPPED;
        return;
    }

    // 计算加速和减速所需的步数
    float accelTime = motor->maxSpeed / motor->acceleration;
    float decelTime = motor->maxSpeed / motor->deceleration;

    motor->accelSteps = (long)(0.5 * motor->acceleration * accelTime * accelTime);
    motor->decelSteps = (long)(0.5 * motor->deceleration * decelTime * decelTime);

    // 检查是否能达到最大速度
    if (motor->accelSteps + motor->decelSteps >= motor->totalSteps)
    {
        // 三角形速度曲线（无匀速段）
        float peakSpeed = sqrt(motor->totalSteps * motor->acceleration * motor->deceleration / (motor->acceleration + motor->deceleration));
        motor->maxSpeed = peakSpeed;

        accelTime = peakSpeed / motor->acceleration;
        decelTime = peakSpeed / motor->deceleration;

        motor->accelSteps = (long)(0.5 * motor->acceleration * accelTime * accelTime);
        motor->decelSteps = (long)(0.5 * motor->deceleration * decelTime * decelTime);
        motor->constantSteps = 0;
    }
    else
    {
        // 梯形速度曲线（有匀速段）
        motor->constantSteps = motor->totalSteps - motor->accelSteps - motor->decelSteps;
    }

    motor->stepsDone = 0;
    motor->currentSpeed = 0;
    motor->currentPhase = TrapezoidalMotorState::ACCELERATION;
    motor->stepInterval = 1000000; // 开始时很慢

    Serial.println("Trapezoidal motion calculated:");
    Serial.println("  Total steps: " + String(motor->totalSteps));
    Serial.println("  Accel steps: " + String(motor->accelSteps));
    Serial.println("  Constant steps: " + String(motor->constantSteps));
    Serial.println("  Decel steps: " + String(motor->decelSteps));
    Serial.println("  Max speed: " + String(motor->maxSpeed) + " steps/sec");
}

/**
 * 更新梯形速度控制
 */
inline void updateTrapezoidalSpeed(TrapezoidalMotorState *motor)
{
    if (!motor->isRunning)
        return;

    unsigned long currentTime = micros();

    // 检查是否到了发送脉冲的时间
    if (currentTime - motor->lastStepTime < motor->stepInterval)
    {
        return;
    }

    // 根据当前阶段更新速度
    switch (motor->currentPhase)
    {
    case TrapezoidalMotorState::ACCELERATION:
        if (motor->stepsDone < motor->accelSteps)
        {
            // 加速阶段：线性增加速度
            float progress = (float)motor->stepsDone / motor->accelSteps;
            motor->currentSpeed = motor->maxSpeed * progress;
        }
        else
        {
            // 进入匀速阶段
            motor->currentSpeed = motor->maxSpeed;
            motor->currentPhase = TrapezoidalMotorState::CONSTANT_SPEED;
        }
        break;

    case TrapezoidalMotorState::CONSTANT_SPEED:
        if (motor->stepsDone >= motor->accelSteps + motor->constantSteps)
        {
            // 进入减速阶段
            motor->currentPhase = TrapezoidalMotorState::DECELERATION;
        }
        // 保持最大速度
        motor->currentSpeed = motor->maxSpeed;
        break;

    case TrapezoidalMotorState::DECELERATION:
        if (motor->stepsDone < motor->totalSteps)
        {
            // 减速阶段：线性减少速度
            long decelStepsDone = motor->stepsDone - (motor->accelSteps + motor->constantSteps);
            float progress = 1.0 - (float)decelStepsDone / motor->decelSteps;
            motor->currentSpeed = motor->maxSpeed * progress;

            // 确保速度不会太低
            if (motor->currentSpeed < 50)
                motor->currentSpeed = 50;
        }
        else
        {
            // 运动完成
            motor->isRunning = false;
            motor->currentPhase = TrapezoidalMotorState::STOPPED;
            motor->currentSpeed = 0;
            return;
        }
        break;

    case TrapezoidalMotorState::STOPPED:
        return;
    }

    // 计算步进间隔
    if (motor->currentSpeed > 0)
    {
        motor->stepInterval = (unsigned long)(1000000.0 / motor->currentSpeed);
        if (motor->stepInterval < 200)
            motor->stepInterval = 200; // 最小间隔200μs
        if (motor->stepInterval > 50000)
            motor->stepInterval = 50000; // 最大间隔50ms
    }

    motor->lastStepTime = currentTime;
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

//=========================
// 高级步进电机控制函数
//=========================

/**
 * 获取高级电机状态指针
 */
inline AdvancedMotorState *getAdvancedMotor(uint8_t motorNumber)
{
    switch (motorNumber)
    {
    case 1:
        return &advancedMotor1;
    case 2:
        return &advancedMotor2;
    case 3:
        return &advancedMotor3;
    case 4:
        return &advancedMotor4;
    default:
        return nullptr;
    }
}

/**
 * 高级步进电机移动到绝对位置（带PID控制和S曲线加速）
 */
inline bool stepper_move_to_position_advanced(uint8_t motorNumber, long target_position, uint8_t speed = 0)
{
    if (motorNumber < 1 || motorNumber > 4)
        return false;

    AdvancedMotorState *motor = getAdvancedMotor(motorNumber);
    if (!motor)
        return false;

    // 启用步进电机（设置SLEEP引脚为HIGH）
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
    default:
        return false;
    }
    digitalWrite(sleepPin, HIGH);

    // 设置目标位置
    motor->targetPosition = target_position;
    motor->currentPosition = stepper_current_position[motorNumber - 1]; // 同步当前位置
    motor->isRunning = true;

    // 设置速度
    if (speed > 0)
    {
        motor->maxSpeed = map(speed, 1, 255, 100, 2000); // 转换为步/秒
        motor->targetSpeed = motor->maxSpeed;
    }

    // 重置PID参数
    motor->errorSum = 0;
    motor->lastError = motor->targetPosition - motor->currentPosition;
    motor->lastUpdateTime = micros();

    Serial.println("Advanced motor " + String(motorNumber) + " moving to position " + String(target_position) + " with PID control");

    return true;
}

/**
 * 高级步进电机更新函数（在loop中调用）
 */
inline void updateAdvancedSteppers()
{
    AdvancedMotorState *motors[] = {&advancedMotor1, &advancedMotor2, &advancedMotor3, &advancedMotor4};
    uint8_t stepPins[] = {STEP1, STEP2, STEP3, STEP4};
    uint8_t dirPins[] = {DIR1, DIR2, DIR3, DIR4};

    for (int i = 0; i < 4; i++)
    {
        AdvancedMotorState *motor = motors[i];

        if (!motor->isRunning)
            continue;

        unsigned long currentTime = micros();

        // 计算位置误差
        long error = motor->targetPosition - motor->currentPosition;

        // 如果到达目标位置，停止电机
        if (abs(error) <= 1)
        {
            motor->isRunning = false;
            motor->currentSpeed = 0;
            continue;
        }

        // 更新S曲线加速度控制
        updateSCurveAcceleration(motor);

        // 使用PID控制微调速度
        float pidOutput = calculatePID(motor, error);

        // 应用PID输出到速度控制
        motor->currentInterval = motor->currentInterval - (long)(pidOutput * 10);
        if (motor->currentInterval < motor->minInterval)
        {
            motor->currentInterval = motor->minInterval;
        }
        if (motor->currentInterval > motor->maxInterval)
        {
            motor->currentInterval = motor->maxInterval;
        }

        // 检查是否到了发送脉冲的时间
        if (currentTime - motor->lastStepTime >= motor->currentInterval)
        {
            // 设置方向
            digitalWrite(dirPins[i], error > 0 ? HIGH : LOW);

            // 发送脉冲
            digitalWrite(stepPins[i], HIGH);
            delayMicroseconds(5);
            digitalWrite(stepPins[i], LOW);

            // 更新位置
            motor->currentPosition += (error > 0) ? 1 : -1;
            stepper_current_position[i] = motor->currentPosition;

            motor->lastStepTime = currentTime;
        }
    }
}

/**
 * 设置PID参数
 */
inline void stepper_set_pid_params(uint8_t motorNumber, float kp, float ki, float kd)
{
    AdvancedMotorState *motor = getAdvancedMotor(motorNumber);
    if (motor)
    {
        motor->kp = kp;
        motor->ki = ki;
        motor->kd = kd;
        Serial.println("Motor " + String(motorNumber) + " PID params: Kp=" + String(kp) + " Ki=" + String(ki) + " Kd=" + String(kd));
    }
}

/**
 * 设置加速度参数
 */
inline void stepper_set_acceleration(uint8_t motorNumber, float acceleration, float deceleration)
{
    AdvancedMotorState *motor = getAdvancedMotor(motorNumber);
    if (motor)
    {
        motor->acceleration = acceleration;
        motor->deceleration = deceleration;
        Serial.println("Motor " + String(motorNumber) + " acceleration: " + String(acceleration) + " deceleration: " + String(deceleration));
    }
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

//=========================
// 梯形速度控制函数
//=========================

/**
 * 梯形速度控制移动到绝对位置
 */
inline bool stepper_move_to_position_trapezoidal(uint8_t motorNumber, long target_position, uint8_t speed = 100)
{
    if (motorNumber < 1 || motorNumber > 4)
        return false;

    TrapezoidalMotorState *motor = getTrapezoidalMotor(motorNumber);
    if (!motor)
        return false;

    // 启用步进电机（设置SLEEP引脚为HIGH）
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
    default:
        return false;
    }
    digitalWrite(sleepPin, HIGH);

    // 设置目标位置
    motor->targetPosition = target_position;
    motor->currentPosition = stepper_current_position[motorNumber - 1];

    // 根据速度设置最大速度
    motor->maxSpeed = map(speed, 1, 255, 100, 2000); // 100-2000 步/秒

    // 计算梯形速度参数
    calculateTrapezoidalParams(motor);

    // 开始运动
    motor->isRunning = true;
    motor->lastStepTime = micros();

    Serial.println("Trapezoidal motor " + String(motorNumber) + " moving from " + String(motor->currentPosition) + " to " + String(target_position));
    Serial.println("Motor state: isRunning=" + String(motor->isRunning) + ", phase=" + String(motor->currentPhase) + ", maxSpeed=" + String(motor->maxSpeed));

    return true;
}

/**
 * 更新所有梯形控制电机
 */
inline void updateTrapezoidalSteppers()
{
    TrapezoidalMotorState *motors[] = {&trapMotor1, &trapMotor2, &trapMotor3, &trapMotor4};
    uint8_t stepPins[] = {STEP1, STEP2, STEP3, STEP4};
    uint8_t dirPins[] = {DIR1, DIR2, DIR3, DIR4};

    for (int i = 0; i < 4; i++)
    {
        TrapezoidalMotorState *motor = motors[i];

        if (!motor->isRunning)
            continue;

        // 更新速度
        updateTrapezoidalSpeed(motor);

        if (!motor->isRunning)
            continue; // 可能在updateTrapezoidalSpeed中停止了

        unsigned long currentTime = micros();

        // 检查是否到了发送脉冲的时间
        if (currentTime - motor->lastStepTime >= motor->stepInterval)
        {
            // 设置方向
            digitalWrite(dirPins[i], motor->direction ? HIGH : LOW);

            // 发送脉冲
            digitalWrite(stepPins[i], HIGH);
            delayMicroseconds(5);
            digitalWrite(stepPins[i], LOW);

            // 更新位置
            if (motor->direction)
            {
                motor->currentPosition++;
            }
            else
            {
                motor->currentPosition--;
            }

            stepper_current_position[i] = motor->currentPosition;
            motor->stepsDone++;
            motor->lastStepTime = currentTime;

            // 检查是否完成
            if (motor->stepsDone >= motor->totalSteps)
            {
                motor->isRunning = false;
                motor->currentPhase = TrapezoidalMotorState::STOPPED;
                Serial.println("Trapezoidal motor " + String(i + 1) + " reached target position " + String(motor->targetPosition));
            }
        }
    }
}

/**
 * 设置梯形控制参数
 */
inline void stepper_set_trapezoidal_params(uint8_t motorNumber, float maxSpeed, float acceleration, float deceleration)
{
    TrapezoidalMotorState *motor = getTrapezoidalMotor(motorNumber);
    if (motor)
    {
        motor->maxSpeed = maxSpeed;
        motor->acceleration = acceleration;
        motor->deceleration = deceleration;
        Serial.println("Motor " + String(motorNumber) + " trapezoidal params: MaxSpeed=" + String(maxSpeed) + " Accel=" + String(acceleration) + " Decel=" + String(deceleration));
    }
}

#endif // STEPPER_MOTOR_H