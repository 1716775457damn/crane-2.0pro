#include <Arduino.h>
#include "chassis.h"
#include "line_tracking.h"

void setup() {
  // 初始化串口
  Serial.begin(115200);
  Serial.println("循迹小车初始化...");

  // 初始化底盘控制串口
  chassis_serial_init();

  // 初始化循迹传感器
  line_sensor_init();

  // 设置个别传感器阈值（如果需要）
  set_sensor_threshold(0, 150); // R2传感器
  set_sensor_threshold(1, 150); // R1传感器
  set_sensor_threshold(2, 150); // M传感器
  set_sensor_threshold(3, 150); // L1传感器
  set_sensor_threshold(4, 150); // L2传感器

  delay(1000);
  Serial.println("初始化完成，开始循迹");
}

void loop() {
  // 运行循迹控制逻辑
  line_tracking_control(30); // 速度设为30
  
  // 适当延迟
  delay(100);
}
