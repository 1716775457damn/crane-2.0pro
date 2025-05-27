#ifndef _SERVO_CONTROL_H_
#define _SERVO_CONTROL_H_

#include <ESP32Servo.h>

// 舵机引脚定义
#define SERVO_PIN 13

// 舵机对象声明
extern Servo myservo;

// 函数声明
void servo_init(void);
void servo(int angle);

#endif // _SERVO_CONTROL_H_