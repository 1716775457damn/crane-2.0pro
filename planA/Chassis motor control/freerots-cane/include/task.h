// #include<sensor.h>
// #include<stepper.h>
#include<servo.h>
#include "StepperMotor.h"
// #include<relay.h>
#include<laser.h>  // 添加激光传感器头文件
#include<web.h>    // 添加Web服务器头文件
#include<chassis.h>
#include <Adafruit_NeoPixel.h> // 添加NeoPixel库


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
    float distance = get_laser_distance();
    
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
