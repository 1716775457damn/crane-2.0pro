// #include<sensor.h>
// #include<stepper.h>
#include<servo.h>
#include "StepperMotor.h"
// #include<relay.h>
#include<laser.h>  // 添加激光传感器头文件
#include<web.h>    // 添加Web服务器头文件
#include<chassis.h>
#include <Adafruit_NeoPixel.h> // 添加NeoPixel库
#include "GrayTracker.h" // 引入灰度循迹库
#include "GrayTrackerDebug.h" // 引入灰度循迹调试库
#include "comm.h" // 添加上位机通信头文件


#define BOARD_LED_PIN 48  // ESP32-S3 开发板的板载 RGB LED 引脚号
#define NUMPIXELS 1       // 板载 RGB LED 的数量

Adafruit_NeoPixel pixels(NUMPIXELS, BOARD_LED_PIN, NEO_GRB + NEO_KHZ800);

#define COLOR_RED pixels.Color(255, 0, 0)
#define COLOR_GREEN pixels.Color(0, 255, 0)
#define COLOR_BLUE pixels.Color(0, 0, 255)
#define COLOR_OFF pixels.Color(0, 0, 0)

void task_00(void *pvParameters);
void task_chassis(void *pvParameters);
void task_one_line(void *pvParameters);
void task_four_motors(void *pvParameters); 
void task_servos(void *pvParameters);      
void task_laser_test(void *pvParameters);  
void task_laser_stepper(void *pvParameters); 
void task_1(void *pvParameters); // 添加task_1函数声明
void task01(void *pvParameters); // 添加task01函数声明 - 灰度循迹任务

// 添加task_laser_stepper函数的实现，重定向到task_1
void task_laser_stepper(void *pvParameters)
{
    // 重定向到task_1函数
    task_1(pvParameters);
}

void task_00(void *pvParameters)
{
    initSteppers();
    
    chassis_serial_init();
    
    // 初始化激光传感器
    laser_init();
    
    send_chassis_command("7,11000,30"); 
    delay(5000);               
    servo1(95); // across
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    // 获取激光测距数据
    float distance = get_laser_distance(true, false);
    
    // 根据测距结果决定上升或下降
    if (distance > 70) {
        // 距离大于70mm，执行下降操作
        runAllSteppers(0, 0, 200, 200, 0, 0, 0, -0); // down
    } else {
        // 距离小于等于70mm，执行上升操作
        runAllSteppers(0, 0, 200, -200, 0, 0, 0, -0); // up
    }
    
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    servo2(100);  // clamp
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    runAllSteppers(0, 0, 200, -600, 0, 0, 0, -0);//up   
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    runAllSteppers(200, 2000, 0, 0, 0, 0, 0, -0); // back
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    runAllSteppers(0, 0, 200, -5400, 0, 0, 0, -0);//back,down       
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    runAllSteppers(200, 2000, 200, 6000, 0, 0, 0, -0);//down
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    servo2(100); // clamp
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    send_chassis_command("8,11000,30");
    delay(5000);
    runAllSteppers(200, -2000, 0, 0, 0, 0, 0, -0);//go
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    servo2(0);    // servo2-0
    vTaskDelay(1000 / portTICK_PERIOD_MS);
        
    servo3(90);   // servo3-90
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    xTaskCreatePinnedToCore(task_1, "Task_1", 4096, NULL, 1, NULL, 0);
    vTaskDelete(NULL);
}


void task_1(void *pvParameters)
{
    // 初始化激光传感器
    laser_init();
    
    // 初始化RGB LED
    pixels.begin();
    pixels.setBrightness(50);  // 设置亮度，范围为 0~255
    pixels.clear();            // 清除所有像素的颜色
    pixels.show();             // 更新LED状态
    
    // 主循环
    while(1) {
        // 获取激光测距数据
        float distance = jiguang();
        
        // 只在有效测距值时进行操作
        if (distance >= 0) {
            // 根据距离设置不同颜色
            if (distance < 300) {
                // 近距离(<30cm)显示红色
                pixels.setPixelColor(0, COLOR_RED);
                pixels.show();
            }
            else if (distance < 500) {
                // 中距离(<50cm)显示蓝色
                pixels.setPixelColor(0, COLOR_BLUE);
                pixels.show();
            }
            else {
                // 远距离(>=50cm)显示绿色
                pixels.setPixelColor(0, COLOR_GREEN);
                pixels.show();
            }
        } 
        else {
            // 测距失败时LED闪烁一次表示错误
            pixels.setPixelColor(0, COLOR_RED);
            pixels.show();
            vTaskDelay(50 / portTICK_PERIOD_MS);
            pixels.setPixelColor(0, COLOR_OFF);
            pixels.show();
        }
        
        // 适当延时
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// 灰度循迹任务实现
void task01(void *pvParameters)
{
    Serial.println("开始初始化灰度循迹任务...");
    
    // 初始化灰度传感器(使用调试版本)
    GrayTrackerDebug tracker;
    
    Serial.println("开始初始化GrayTrackerDebug...");
    tracker.begin();
    Serial.println("GrayTrackerDebug初始化完成");
    
    // 设置传感器阈值 - 可以根据实际情况调整
    tracker.setThreshold(1200);
    Serial.println("传感器阈值设置为1200");
    
    // 设置为基本循迹模式
    tracker.setRouteMode(GrayTracker::BASIC_MODE);
    Serial.println("设置为基本循迹模式");
    
    // 初始化底盘控制
    chassis_serial_init();
    Serial.println("底盘控制初始化完成");
    
    // 暂停一下，确保所有设备初始化完成
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    Serial.println("灰度循迹任务已启动");
    Serial.println("自动运行模式已开启");
    
    // 传感器值数组
    int sensorValues[5];
    int sensorStates[5];
    
    // 主循环
    while(1) {
        // 更新灰度传感器状态和处理调试命令
        tracker.update();
        
        // 获取当前循迹状态
        int tracingState = tracker.getTracingState();
        
        // 获取传感器值和状态（用于调试）
        tracker.getSensorValues(sensorValues);
        tracker.getSensorStates(sensorStates);
        
        // 每秒打印一次传感器状态（调试用）
        static unsigned long lastPrintTime = 0;
        if (millis() - lastPrintTime > 1000) {
            lastPrintTime = millis();
            Serial.print("传感器状态: ");
            for (int i = 0; i < 5; i++) {
                Serial.print(sensorStates[i]);
                Serial.print(" ");
            }
            Serial.print(" 循迹状态: ");
            Serial.print(tracingState);
            Serial.print(" 自动模式: ");
            Serial.println(tracker.isAutoRunEnabled() ? "开启" : "关闭");
        }
        
        // 检测十字路口
        bool isCrossroad = tracker.detectCrossroad();
        
        // 根据循迹状态控制底盘运动（只在自动运行模式下）
        if (tracker.isAutoRunEnabled() && !isCrossroad) {
            switch(tracingState) {
                case GrayTracker::STRAIGHT:
                    // 直行状态 - 前进
                    control_chassis_raw(CHASSIS_FORWARD, 1000, 20);
                    break;
                    
                case GrayTracker::RIGHT_TURN:
                    // 右转状态 - 左转（修正方向）
                    control_chassis_raw(CHASSIS_LEFT, 1000, 30);
                    break;
                    
                case GrayTracker::LEFT_TURN:
                    // 左转状态 - 右转（修正方向）
                    control_chassis_raw(CHASSIS_RIGHT, 1000, 30);
                    break;
                    
                case GrayTracker::SHARP_RIGHT_TURN:
                    // 大幅右转状态 - 大角度左转（修正方向）
                    control_chassis_raw(CHASSIS_LEFT, 2000, 15);
                    break;
                    
                case GrayTracker::SHARP_LEFT_TURN:
                    // 大幅左转状态 - 大角度右转（修正方向）
                    control_chassis_raw(CHASSIS_RIGHT, 2000, 15);
                    break;
                    
                case GrayTracker::ALL_BLACK:
                    // 全黑状态 - 停止
                    control_chassis_raw(CHASSIS_STOP, 0, 0);
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    break;
                    
                default:
                    // 默认状态 - 停止
                    control_chassis_raw(CHASSIS_STOP, 0, 0);
                    break;
            }
        } else if (isCrossroad) {
            // 十字路口处理 - 暂停一下，让其他功能检测到十字路口
            vTaskDelay(200 / portTICK_PERIOD_MS);
        }
        
        // 适当延时，避免命令发送太频繁
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
