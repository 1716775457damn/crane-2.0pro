#include "servo_control.h"
#include <ESP32Servo.h>

// 舵机对象与变量
static Servo myservo;
static uint8_t servo_pin = 0;
static uint8_t current_angle = 90;
static uint8_t target_angle = 90;
static uint8_t sweep_speed = 5;
static bool is_sweeping = false;
static unsigned long last_sweep_time = 0;
static unsigned long last_status_time = 0;

// 初始化舵机 - 优化初始化参数
void servo_init(uint8_t pin)
{
    servo_pin = pin;

    // 更高效地设置定时器
    ESP32PWM::allocateTimer(0);

    // 更精确的脉宽设置
    myservo.setPeriodHertz(50);
    myservo.attach(servo_pin, 500, 2400);

    // 初始位置
    myservo.write(current_angle);

    Serial.printf("[SERVO] Initialization complete, pin: %u\n", servo_pin);
}

// 设置舵机角度
void servo_set_angle(uint8_t angle)
{
    if (angle > 180)
        angle = 180;

    current_angle = angle;
    target_angle = angle;
    is_sweeping = false;

    myservo.write(angle);

    Serial.printf("[SERVO] Angle set to: %u deg\n", angle);
}

// 优化舵机平滑转动
void servo_sweep_to(uint8_t angle, uint8_t speed)
{
    if (angle > 180)
        angle = 180;
    if (speed < 1)
        speed = 1;
    if (speed > 10)
        speed = 10;

    // 如果已经在目标角度，直接返回
    if (angle == current_angle)
        return;

    target_angle = angle;
    sweep_speed = speed;
    is_sweeping = true;
    last_status_time = 0; // 确保初始状态能打印

    Serial.printf("[SERVO] Moving to: %u deg, speed: %u\n", angle, speed);
}

// 优化舵机控制循环
void servo_loop()
{
    if (!is_sweeping)
        return;

    // 周期性输出运动状态
    unsigned long now = millis();
    if (now - last_status_time >= 1000)
    {
        last_status_time = now;
        Serial.printf("[SERVO] Current angle: %u deg, target: %u deg\n", current_angle, target_angle);
    }

    // 动态调整步进间隔，使大范围运动更流畅
    unsigned long sweep_interval = 20 - (sweep_speed * 1.5);

    // 调整角度
    now = millis();
    if ((now - last_sweep_time) >= sweep_interval)
    {
        last_sweep_time = now;

        // 根据目标角度计算最优移动方向与步长
        int step_size = (sweep_speed > 5) ? 2 : 1; // 高速时使用更大步长

        if (current_angle < target_angle)
        {
            current_angle = min((uint8_t)(current_angle + step_size), (uint8_t)target_angle);
            myservo.write(current_angle);
        }
        else if (current_angle > target_angle)
        {
            current_angle = max((uint8_t)(current_angle - step_size), (uint8_t)target_angle);
            myservo.write(current_angle);
        }
        else
        {
            // 到达目标
            is_sweeping = false;
            Serial.printf("[SERVO] Target angle reached: %u deg\n", current_angle);
        }
    }
}

// 获取当前舵机角度
uint8_t servo_get_angle()
{
    return current_angle;
}