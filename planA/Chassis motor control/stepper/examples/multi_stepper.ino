/**
 * ESP32 步进电机控制库 - 多电机控制示例
 * 演示如何同时控制多个步进电机
 */

#include "stepper_control.h"

// 定义第二个步进电机的引脚
#define STEPPER2_STEP_PIN 26
#define STEPPER2_DIR_PIN 25
#define STEPPER2_ENABLE_PIN 27

void setup()
{
    // 初始化串口
    Serial.begin(115200);
    Serial.println("ESP32 多步进电机控制示例");

    // 初始化步进电机控制库 (默认配置电机0)
    stepper_init();

    // 配置第二个步进电机 (电机1)
    stepper_config_pins(1, STEPPER2_STEP_PIN, STEPPER2_DIR_PIN, STEPPER2_ENABLE_PIN);
    Serial.println("步进电机配置完成");

    // 启动电机运动
    Serial.println("开始电机运动...");

    // 电机0: 慢速移动到1000步位置
    stepper_run(0, 800, 1500, 1000);

    // 电机1: 快速移动到-800步位置
    stepper_run(1, 1200, 2500, -800);
}

void loop()
{
    // 必须在主循环中调用此函数来处理所有电机运动
    stepper_loop();

    // 检测电机运动状态并执行往返运动
    static bool motors_moving = true;
    static bool motors_returned = false;
    static unsigned long last_change = 0;

    // 保存当前时间
    unsigned long now = millis();

    // 第一阶段：两个电机都到达目标位置
    if (motors_moving)
    {
        // 选择电机0检查状态
        select_stepper(0);
        bool motor0_moving = stepper_is_moving();

        // 选择电机1检查状态
        select_stepper(1);
        bool motor1_moving = stepper_is_moving();

        // 如果两个电机都停止了
        if (!motor0_moving && !motor1_moving)
        {
            motors_moving = false;
            last_change = now;
            Serial.println("两个电机都到达目标位置");
        }
    }
    // 第二阶段：等待1秒后返回原点
    else if (!motors_moving && !motors_returned && (now - last_change > 1000))
    {
        Serial.println("两个电机开始返回原点...");

        // 电机0返回原点
        stepper_run(0, 1000, 2000, 0);

        // 电机1返回原点
        stepper_run(1, 1500, 3000, 0);

        motors_returned = true;
        last_change = now;
    }
    // 第三阶段：两个电机都返回原点后，等待2秒再重新开始
    else if (motors_returned && (now - last_change > 2000))
    {
        // 选择电机0检查状态
        select_stepper(0);
        bool motor0_moving = stepper_is_moving();

        // 选择电机1检查状态
        select_stepper(1);
        bool motor1_moving = stepper_is_moving();

        // 如果两个电机都停止了
        if (!motor0_moving && !motor1_moving)
        {
            // 重新开始新的循环
            Serial.println("重新开始新的运动循环...");

            // 电机0: 慢速移动到1000步位置
            stepper_run(0, 800, 1500, 1000);

            // 电机1: 快速移动到-800步位置
            stepper_run(1, 1200, 2500, -800);

            motors_moving = true;
            motors_returned = false;
            last_change = now;
        }
    }
}