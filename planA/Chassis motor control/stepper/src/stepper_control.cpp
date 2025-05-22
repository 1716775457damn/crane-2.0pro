#include "stepper_control.h"

// 步进电机结构体定义
typedef struct
{
    // 引脚定义
    uint8_t dir_pin;    // 方向控制引脚
    uint8_t step_pin;   // 步进脉冲引脚
    uint8_t enable_pin; // 使能引脚

    // 控制变量
    int32_t current_position;     // 当前位置
    int32_t target_position;      // 目标位置
    float current_speed;          // 当前速度
    float max_speed;              // 最大速度 (步/秒)
    float acceleration;           // 加速度 (步/秒^2)
    bool is_moving;               // 是否正在移动
    unsigned long last_step_time; // 上次步进时间
    bool is_configured;           // 是否已配置
} stepper_t;

// 步进电机控制变量
static stepper_t steppers[MAX_STEPPER_NUM] = {0};
static unsigned long last_micros = 0;      // 上次微秒时间
static unsigned long last_status_time = 0; // 状态报告时间
static uint8_t current_stepper = 0;        // 当前使用的步进电机

// 默认引脚定义
#define DEFAULT_DIR_PIN 12    // 方向控制引脚
#define DEFAULT_STEP_PIN 14   // 步进脉冲引脚
#define DEFAULT_ENABLE_PIN 13 // 使能引脚

// 初始化步进电机
void stepper_init(void)
{
    // 初始化第一个步进电机
    steppers[0].step_pin = DEFAULT_STEP_PIN;
    steppers[0].dir_pin = DEFAULT_DIR_PIN;
    steppers[0].enable_pin = DEFAULT_ENABLE_PIN;
    steppers[0].current_position = 0;
    steppers[0].target_position = 0;
    steppers[0].current_speed = 0.0;
    steppers[0].max_speed = 1000.0;
    steppers[0].acceleration = 1000.0;
    steppers[0].is_moving = false;
    steppers[0].is_configured = true;

    // 设置引脚模式
    pinMode(steppers[0].dir_pin, OUTPUT);
    pinMode(steppers[0].step_pin, OUTPUT);
    pinMode(steppers[0].enable_pin, OUTPUT);

    // 默认输出状态
    digitalWrite(steppers[0].dir_pin, HIGH);
    digitalWrite(steppers[0].step_pin, LOW);
    digitalWrite(steppers[0].enable_pin, HIGH); // 高电平禁用

    Serial.println("[STEPPER] Initialization complete");
    Serial.printf("[STEPPER] ID:0 Pins - DIR:%d, STEP:%d, ENABLE:%d\n",
                  steppers[0].dir_pin, steppers[0].step_pin, steppers[0].enable_pin);
    Serial.printf("[STEPPER] ID:0 Max Speed: %.2f steps/sec, Acceleration: %.2f steps/sec^2\n",
                  steppers[0].max_speed, steppers[0].acceleration);
}

// 配置步进电机引脚
void stepper_config_pins(uint8_t stepper_id, uint8_t step_pin, uint8_t dir_pin, uint8_t enable_pin)
{
    if (stepper_id >= MAX_STEPPER_NUM)
    {
        Serial.printf("[STEPPER] Error: Invalid stepper ID: %d\n", stepper_id);
        return;
    }

    // 配置引脚
    steppers[stepper_id].step_pin = step_pin;
    steppers[stepper_id].dir_pin = dir_pin;
    steppers[stepper_id].enable_pin = enable_pin;

    // 初始化位置和速度
    steppers[stepper_id].current_position = 0;
    steppers[stepper_id].target_position = 0;
    steppers[stepper_id].current_speed = 0.0;
    steppers[stepper_id].max_speed = 1000.0;
    steppers[stepper_id].acceleration = 1000.0;
    steppers[stepper_id].is_moving = false;
    steppers[stepper_id].is_configured = true;

    // 设置引脚模式
    pinMode(steppers[stepper_id].dir_pin, OUTPUT);
    pinMode(steppers[stepper_id].step_pin, OUTPUT);
    pinMode(steppers[stepper_id].enable_pin, OUTPUT);

    // 默认输出状态
    digitalWrite(steppers[stepper_id].dir_pin, HIGH);
    digitalWrite(steppers[stepper_id].step_pin, LOW);
    digitalWrite(steppers[stepper_id].enable_pin, HIGH); // 高电平禁用

    Serial.printf("[STEPPER] ID:%d configuration complete\n", stepper_id);
    Serial.printf("[STEPPER] ID:%d Pins - DIR:%d, STEP:%d, ENABLE:%d\n",
                  stepper_id, steppers[stepper_id].dir_pin,
                  steppers[stepper_id].step_pin, steppers[stepper_id].enable_pin);
}

// 选择当前操作的步进电机 - 现在是公共函数
bool select_stepper(uint8_t stepper_id)
{
    if (stepper_id >= MAX_STEPPER_NUM)
    {
        Serial.printf("[STEPPER] Error: Invalid stepper ID: %d\n", stepper_id);
        return false;
    }

    if (!steppers[stepper_id].is_configured)
    {
        Serial.printf("[STEPPER] Error: Stepper ID:%d not configured\n", stepper_id);
        return false;
    }

    current_stepper = stepper_id;
    return true;
}

// 设置步进电机最大速度
void stepper_set_speed(uint16_t steps_per_second)
{
    if (steps_per_second > 0 && steps_per_second <= 5000)
    {
        steppers[current_stepper].max_speed = (float)steps_per_second;
        Serial.printf("[STEPPER] ID:%d Max speed set to: %.2f steps/sec\n",
                      current_stepper, steppers[current_stepper].max_speed);
    }
}

// 设置步进电机加速度
void stepper_set_acceleration(float accel)
{
    if (accel > 0 && accel <= 10000.0)
    {
        steppers[current_stepper].acceleration = accel;
        Serial.printf("[STEPPER] ID:%d Acceleration set to: %.2f steps/sec^2\n",
                      current_stepper, steppers[current_stepper].acceleration);
    }
}

// 移动到指定位置
void stepper_move_to(int32_t position)
{
    steppers[current_stepper].target_position = position;
    steppers[current_stepper].is_moving = true;

    // 使能电机
    digitalWrite(steppers[current_stepper].enable_pin, LOW);

    Serial.printf("[STEPPER] ID:%d Moving to position: %d (current: %d)\n",
                  current_stepper, steppers[current_stepper].target_position,
                  steppers[current_stepper].current_position);
}

// 相对移动指定步数
void stepper_move_steps(int32_t steps)
{
    if (steps == 0)
        return;

    stepper_move_to(steppers[current_stepper].current_position + steps);
}

// 停止步进电机
void stepper_stop(void)
{
    if (steppers[current_stepper].is_moving)
    {
        steppers[current_stepper].is_moving = false;
        steppers[current_stepper].current_speed = 0;
        digitalWrite(steppers[current_stepper].enable_pin, HIGH); // 禁用电机
        Serial.printf("[STEPPER] ID:%d Stopped\n", current_stepper);
    }
}

// 获取步进电机是否正在运动
bool stepper_is_moving(void)
{
    return steppers[current_stepper].is_moving;
}

// 获取当前位置
int32_t stepper_get_position(void)
{
    return steppers[current_stepper].current_position;
}

// 获取到目标位置的距离
int32_t stepper_distance_to_go(void)
{
    return steppers[current_stepper].target_position - steppers[current_stepper].current_position;
}

// 执行一步
static void step_motor(uint8_t id)
{
    // 设置方向
    int8_t direction = (steppers[id].target_position > steppers[id].current_position) ? 1 : -1;
    digitalWrite(steppers[id].dir_pin, direction > 0 ? HIGH : LOW);

    // 产生一个步进脉冲
    digitalWrite(steppers[id].step_pin, HIGH);
    delayMicroseconds(20); // 20微秒脉冲宽度
    digitalWrite(steppers[id].step_pin, LOW);

    // 更新位置
    steppers[id].current_position += direction;
}

// 计算下一个速度
static float calculate_speed(uint8_t id)
{
    int32_t remaining = steppers[id].target_position - steppers[id].current_position;
    float target_speed;

    if (remaining == 0)
    {
        return 0.0;
    }

    // 计算理想速度 (v² = 2as)
    float required_speed = sqrt(2.0 * steppers[id].acceleration * abs(remaining));
    if (required_speed > steppers[id].max_speed)
    {
        required_speed = steppers[id].max_speed;
    }

    // 根据方向确定速度正负
    target_speed = (remaining > 0) ? required_speed : -required_speed;

    return target_speed;
}

// 简化的一键调用函数
bool stepper_run(uint8_t stepper_id, uint16_t max_speed, float accel, int32_t position)
{
    // 检查参数
    if (stepper_id >= MAX_STEPPER_NUM)
    {
        Serial.printf("[STEPPER] Error: Invalid stepper ID: %d\n", stepper_id);
        return false;
    }

    if (!steppers[stepper_id].is_configured)
    {
        Serial.printf("[STEPPER] Error: Stepper ID:%d not configured\n", stepper_id);
        return false;
    }

    if (max_speed <= 0 || max_speed > 5000)
    {
        Serial.printf("[STEPPER] Error: Invalid speed: %d\n", max_speed);
        return false;
    }

    if (accel <= 0 || accel > 10000.0)
    {
        Serial.printf("[STEPPER] Error: Invalid acceleration: %.2f\n", accel);
        return false;
    }

    // 更新参数
    steppers[stepper_id].max_speed = max_speed;
    steppers[stepper_id].acceleration = accel;
    steppers[stepper_id].target_position = position;
    steppers[stepper_id].is_moving = true;

    // 使能电机
    digitalWrite(steppers[stepper_id].enable_pin, LOW);

    Serial.printf("[STEPPER] ID:%d Running to position %d at speed %.2f with accel %.2f\n",
                  stepper_id, position, (float)max_speed, accel);

    return true;
}

// 步进电机控制循环
void stepper_loop(void)
{
    unsigned long current_micros = micros();
    float dt = (current_micros - last_micros) / 1000000.0; // 转换为秒

    // 更新时间
    if (last_micros == 0)
    {
        last_micros = current_micros;
        return;
    }
    last_micros = current_micros;

    // 周期性输出状态
    unsigned long now = millis();
    if (now - last_status_time >= 1000)
    {
        last_status_time = now;

        // 输出每个电机的状态
        for (uint8_t i = 0; i < MAX_STEPPER_NUM; i++)
        {
            if (steppers[i].is_configured && steppers[i].is_moving)
            {
                Serial.printf("[STEPPER] ID:%d Position: %d/%d, Speed: %.2f steps/sec\n",
                              i, steppers[i].current_position, steppers[i].target_position,
                              steppers[i].current_speed);
            }
        }
    }

    // 处理每个电机
    for (uint8_t i = 0; i < MAX_STEPPER_NUM; i++)
    {
        if (!steppers[i].is_configured || !steppers[i].is_moving)
        {
            continue;
        }

        // 检查是否到达目标位置
        if (steppers[i].current_position == steppers[i].target_position)
        {
            steppers[i].is_moving = false;
            steppers[i].current_speed = 0.0;
            digitalWrite(steppers[i].enable_pin, HIGH); // 禁用电机
            Serial.printf("[STEPPER] ID:%d Target position reached\n", i);
            continue;
        }

        // 计算目标速度
        float target_speed = calculate_speed(i);

        // 应用加速度限制
        if (target_speed > steppers[i].current_speed)
        {
            steppers[i].current_speed = min(target_speed, steppers[i].current_speed + steppers[i].acceleration * dt);
        }
        else if (target_speed < steppers[i].current_speed)
        {
            steppers[i].current_speed = max(target_speed, steppers[i].current_speed - steppers[i].acceleration * dt);
        }

        // 处理接近零的速度
        if (abs(steppers[i].current_speed) < 0.01)
        {
            steppers[i].current_speed = 0.0;
        }

        // 计算步进间隔
        if (steppers[i].current_speed != 0.0)
        {
            float step_interval_micros = 1000000.0 / abs(steppers[i].current_speed);

            // 判断是否应该执行步进
            if (current_micros - steppers[i].last_step_time >= step_interval_micros)
            {
                steppers[i].last_step_time = current_micros;
                step_motor(i);
            }
        }
    }
}