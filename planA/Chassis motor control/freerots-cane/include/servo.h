#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>

// 舵机接口引脚定义
const int servoPin1 = 9;  // 舵机1接口引脚，接橙色信号线
const int servoPin2 = 12; // 舵机2接口引脚，接橙色信号线
const int servoPin3 = 21; // 舵机3接口引脚，接橙色信号线

/**
 * @brief 控制舵机1转动到指定角度
 * @param angle 角度值(0-180)
 */
void servo1(int angle)
{ 
    // 确保角度在有效范围内
    if(angle < 0) angle = 0;
    if(angle > 180) angle = 180;
    
    // 发送50个脉冲
    pinMode(servoPin1, OUTPUT);
    for (int i = 0; i < 50; i++)
    {
        int pulsewidth = (angle * 11) + 500; // 将角度转化为500-2480的脉宽值
        digitalWrite(servoPin1, HIGH);       // 将舵机接口电平至高
        delayMicroseconds(pulsewidth);       // 延时脉宽值的微秒数
        digitalWrite(servoPin1, LOW);        // 将舵机接口电平至低
        delayMicroseconds(20000 - pulsewidth);
    }
    delay(100);
}

/**
 * @brief 控制舵机2转动到指定角度
 * @param angle 角度值(0-180)
 */
void servo2(int angle)
{ 
    // 确保角度在有效范围内
    if(angle < 0) angle = 0;
    if(angle > 180) angle = 180;
    
    // 发送50个脉冲
    pinMode(servoPin2, OUTPUT);
    for (int i = 0; i < 50; i++)
    {
        int pulsewidth = (angle * 11) + 500; // 将角度转化为500-2480的脉宽值
        digitalWrite(servoPin2, HIGH);       // 将舵机接口电平至高
        delayMicroseconds(pulsewidth);       // 延时脉宽值的微秒数
        digitalWrite(servoPin2, LOW);        // 将舵机接口电平至低
        delayMicroseconds(20000 - pulsewidth);
    }
    delay(100);
}

/**
 * @brief 控制舵机3转动到指定角度
 * @param angle 角度值(0-180)
 */
void servo3(int angle)
{ 
    // 确保角度在有效范围内
    if(angle < 0) angle = 0;
    if(angle > 180) angle = 180;
    
    // 发送50个脉冲
    pinMode(servoPin3, OUTPUT);
    for (int i = 0; i < 50; i++)
    {
        int pulsewidth = (angle * 11) + 500; // 将角度转化为500-2480的脉宽值
        digitalWrite(servoPin3, HIGH);       // 将舵机接口电平至高
        delayMicroseconds(pulsewidth);       // 延时脉宽值的微秒数
        digitalWrite(servoPin3, LOW);        // 将舵机接口电平至低
        delayMicroseconds(20000 - pulsewidth);
    }
    delay(100);
}

/**
 * @brief 同时控制三个舵机转动到指定角度
 * @param angle1 舵机1角度值(0-180)
 * @param angle2 舵机2角度值(0-180)
 * @param angle3 舵机3角度值(0-180)
 */
void servoAll(int angle1, int angle2, int angle3)
{
    // 确保角度在有效范围内
    if(angle1 < 0) angle1 = 0;
    if(angle1 > 180) angle1 = 180;
    if(angle2 < 0) angle2 = 0;
    if(angle2 > 180) angle2 = 180;
    if(angle3 < 0) angle3 = 0;
    if(angle3 > 180) angle3 = 180;
    
    // 转换为脉宽
    int pulsewidth1 = (angle1 * 11) + 500;
    int pulsewidth2 = (angle2 * 11) + 500;
    int pulsewidth3 = (angle3 * 11) + 500;
    
    // 设置引脚模式
    pinMode(servoPin1, OUTPUT);
    pinMode(servoPin2, OUTPUT);
    pinMode(servoPin3, OUTPUT);
    
    // 发送50个脉冲
    for (int i = 0; i < 50; i++)
    {
        // 控制舵机1
        digitalWrite(servoPin1, HIGH);
        delayMicroseconds(pulsewidth1);
        digitalWrite(servoPin1, LOW);
        
        // 控制舵机2
        digitalWrite(servoPin2, HIGH);
        delayMicroseconds(pulsewidth2);
        digitalWrite(servoPin2, LOW);
        
        // 控制舵机3
        digitalWrite(servoPin3, HIGH);
        delayMicroseconds(pulsewidth3);
        digitalWrite(servoPin3, LOW);
        
        // 等待下一个周期
        delayMicroseconds(20000 - max(pulsewidth1, max(pulsewidth2, pulsewidth3)));
    }
    delay(100);
}

/**
 * @brief 初始化所有舵机到指定位置
 * @param angle1 舵机1初始角度(0-180)，默认为90度(中间位置)
 * @param angle2 舵机2初始角度(0-180)，默认为90度(中间位置)
 * @param angle3 舵机3初始角度(0-180)，默认为90度(中间位置)
 */
void initServos(int angle1 = 90, int angle2 = 90, int angle3 = 90)
{
    // 设置引脚模式
    pinMode(servoPin1, OUTPUT);
    pinMode(servoPin2, OUTPUT);
    pinMode(servoPin3, OUTPUT);
    
    // 同时设置所有舵机到初始位置
    servoAll(angle1, angle2, angle3);
    
    Serial.println("舵机初始化完成!");
}

#endif
