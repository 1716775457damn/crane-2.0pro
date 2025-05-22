/**
 * ESP32 步进电机控制库 - 简单示例
 * 演示如何使用新增的一键调用函数控制步进电机
 */

#include "stepper_control.h"

void setup()
{
    // 初始化串口
    Serial.begin(115200);
    Serial.println("ESP32 步进电机控制示例");

    // 初始化步进电机控制库
    stepper_init();

    // 初始化成功后，让电机开始运动
    Serial.println("开始电机运动 - 前进1000步");

    // 一键调用函数 - 电机ID, 最大速度, 加速度, 目标位置
    stepper_run(0, 1000, 2000, 1000);
}

void loop()
{
    // 必须在主循环中调用此函数来处理电机运动
    stepper_loop();

    // 检查电机是否完成运动
    static bool moving_forward = true;
    static bool moving_backward = false;
    static unsigned long last_change = 0;

    // 如果电机完成向前运动并且还没开始向后运动
    if (!stepper_is_moving() && moving_forward && !moving_backward)
    {
        // 等待1秒后开始向后运动
        unsigned long now = millis();
        if (now - last_change > 1000)
        {
            Serial.println("开始电机运动 - 回到原点");
            stepper_run(0, 1500, 3000, 0); // 使用不同的速度和加速度回到原点
            moving_forward = false;
            moving_backward = true;
            last_change = now;
        }
    }

    // 如果电机完成向后运动
    if (!stepper_is_moving() && !moving_forward && moving_backward)
    {
        // 等待1秒后再次开始向前运动
        unsigned long now = millis();
        if (now - last_change > 1000)
        {
            Serial.println("开始电机运动 - 再次前进");
            stepper_run(0, 1000, 2000, 1000);
            moving_forward = true;
            moving_backward = false;
            last_change = now;
        }
    }
}