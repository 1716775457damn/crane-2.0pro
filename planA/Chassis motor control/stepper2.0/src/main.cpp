/**
 * ESP32多传感器控制系统主程序
 * - 步进电机控制
 * - 激光测距传感器
 * - 舵机控制
 */

#include <Arduino.h>
#include "stepper.h"
#include "laser.h"
#include "servo_control.h"

void setup()
{
  // 初始化串口，波特率115200
  Serial.begin(115200);
  Serial.println("Multi-sensor control system initializing");

  // 初始化所有模块
  stepper_init();
  laser_init();
  servo_init();
  
  // 步进电机控制 - 速度1500, 加速度1800, 步数2000
  controlStepper(stepper1, 1500, 1800, 2000);
  
  // 获取激光传感器距离
  float distance = jiguang();
  
  // 舵机控制到90度
  servo(90);
}

void loop()
{
  // 步进电机运动控制需要持续调用
  stepper_loop();
}