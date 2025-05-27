#include <Arduino.h>
#include "laser.h"

/**
 * 激光传感器初始化
 */
void laser_init(void) {
  // 初始化激光传感器串口
  Serial2.begin(115200, SERIAL_8N1, LASER_RX_PIN, LASER_TX_PIN);
  Serial.println("Laser sensor initialized");
}

/**
 * 获取激光传感器测量的实时距离
 * @return 测量的距离，单位mm
 */
float jiguang(void) {
  if (Serial2.available() > 0) {
    String data = Serial2.readStringUntil('\n');
    // 假设数据格式为纯数字字符串
    float distance = data.toFloat();
    return distance;
  }
  
  // 如果没有真实数据，返回模拟数据
  float fake_distance = random(100, 1000) / 10.0; // 模拟10-100cm的距离
  return fake_distance;
} 