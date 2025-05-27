#include <Arduino.h>
#include "servo_control.h"

// 定义舵机对象
Servo myservo;

/**
 * 舵机初始化
 */
void servo_init(void) {
  myservo.attach(SERVO_PIN);
  myservo.write(90); // 初始化到中间位置
  Serial.println("Servo initialized");
}

/**
 * 舵机角度控制函数
 * @param angle 目标角度 (0-180)
 */
void servo(int angle) {
  // 确保角度在有效范围内
  if (angle < 0) angle = 0;
  if (angle > 180) angle = 180;
  
  // 控制舵机旋转到指定角度
  myservo.write(angle);
}