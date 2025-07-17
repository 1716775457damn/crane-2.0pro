#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "servo.h"
#include "StepperMotor.h"
#include "chassis.h"
#include "wifi_config.h"
#include "html_page.h"

// 包含系统健康监控头文件
#include "system_health.h"

// 声明外部函数和变量
extern void debugPrint(const String &message);
extern void debugPrintAll(const String &message);
extern class GrayTrackerDebug grayTracker;
extern bool continuousGray;
extern bool continuousLaser;
extern int getLaserDistanceOptimized();

// 创建Web服务器，端口80
WebServer server(80);

// 处理根路径请求
void handleRoot()
{
  server.send(200, "text/html", MAIN_page);
}

// 步进电机控制函数
void controlStepper(uint8_t motorNumber, int32_t steps, uint8_t speed)
{
  // 验证参数
  if (motorNumber < 1 || motorNumber > 4)
  {
    debugPrintAll("Error: Invalid motor number");
    return;
  }

  if (speed < 1 || speed > 200)
  {
    speed = constrain(speed, 1, 200);
    debugPrintAll("Warning: Speed adjusted to valid range");
  }

  // 调用StepperMotor.h中的函数控制步进电机
  bool result = stepper(motorNumber, speed, steps);

  if (result)
  {
    debugPrintAll("Stepper " + String(motorNumber) + " moving " + String(steps) + " steps at speed " + String(speed));
  }
  else
  {
    debugPrintAll("Error controlling stepper " + String(motorNumber));
  }
}

// 设置步进电机使能状态
void setStepperEnable(uint8_t motorNumber, bool enable)
{
  // 验证参数
  if (motorNumber < 1 || motorNumber > 4)
  {
    debugPrintAll("Error: Invalid motor number");
    return;
  }

  // 获取相应的休眠引脚
  uint8_t sleepPin;
  switch (motorNumber)
  {
  case 1:
    sleepPin = SLEEP1;
    break;
  case 2:
    sleepPin = SLEEP2;
    break;
  case 3:
    sleepPin = SLEEP3;
    break;
  case 4:
    sleepPin = SLEEP4;
    break;
  }

  // 设置使能状态
  digitalWrite(sleepPin, enable ? HIGH : LOW);
  debugPrintAll("Stepper " + String(motorNumber) + " " + (enable ? "enabled" : "disabled"));
}

// 获取舵机状态
void handleGetServoStatus()
{
  StaticJsonDocument<200> doc;

  // 填充舵机信息
  doc["servo1Angle"] = currentServoAngles[0];
  doc["servo2Angle"] = currentServoAngles[1];
  doc["servo3Angle"] = currentServoAngles[2];

  doc["servo1Type"] = (int)servoTypes[0];
  doc["servo2Type"] = (int)servoTypes[1];
  doc["servo3Type"] = (int)servoTypes[2];

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// 移动单个舵机
void handleMoveServo()
{
  int servoNum = server.arg("servo").toInt();
  int angle = server.arg("angle").toInt();

  if (servoNum >= 1 && servoNum <= 3)
  {
    if (servoNum == 1)
      servo1(angle);
    else if (servoNum == 2)
      servo2(angle);
    else if (servoNum == 3)
      servo3(angle);

    StaticJsonDocument<100> doc;
    doc["success"] = true;
    doc["message"] = "Servo " + String(servoNum) + " moved to " + String(angle) + " degrees";

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  }
  else
  {
    server.send(400, "application/json", "{\"success\": false, \"message\": \"Invalid servo number\"}");
  }
}

// 设置舵机类型
void handleSetServoType()
{
  int servoNum = server.arg("servo").toInt();
  int type = server.arg("type").toInt();

  if (servoNum >= 1 && servoNum <= 3 && (type == 0 || type == 1))
  {
    setServoType(servoNum - 1, (ServoType)type);

    StaticJsonDocument<100> doc;
    doc["success"] = true;
    doc["message"] = "Servo " + String(servoNum) + " type set to " + (type == 0 ? "Standard" : "Continuous");

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  }
  else
  {
    server.send(400, "application/json", "{\"success\": false, \"message\": \"Invalid parameters\"}");
  }
}

// 设置所有舵机类型
void handleSetAllServoType()
{
  int type = server.arg("type").toInt();

  if (type == 0)
  {
    setAllServosStandard();
  }
  else if (type == 1)
  {
    setAllServosContinuous();
  }
  else
  {
    server.send(400, "application/json", "{\"success\": false, \"message\": \"Invalid type parameter\"}");
    return;
  }

  StaticJsonDocument<100> doc;
  doc["success"] = true;
  doc["message"] = "All servos set to " + String(type == 0 ? "Standard" : "Continuous");

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// 移动所有舵机
void handleMoveAllServos()
{
  int angle1 = server.arg("angle1").toInt();
  int angle2 = server.arg("angle2").toInt();
  int angle3 = server.arg("angle3").toInt();

  servoAll(angle1, angle2, angle3);

  StaticJsonDocument<100> doc;
  doc["success"] = true;
  doc["message"] = "All servos moved";

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// 处理404错误
void handleNotFound()
{
  server.send(404, "text/plain", "Not found");
}

// 获取灰度传感器数据
void handleGetGraySensorData()
{
  DynamicJsonDocument doc(256);

  // 更新传感器读数
  grayTracker.update();

  // 获取传感器值
  int values[5];
  grayTracker.getSensorValues(values);

  // 填充JSON响应
  JsonArray valuesArray = doc.createNestedArray("values");
  for (int i = 0; i < 5; i++)
  {
    valuesArray.add(values[i]);
  }

  String jsonResponse;
  serializeJson(doc, jsonResponse);
  server.send(200, "application/json", jsonResponse);
}

// 切换灰度传感器连续模式
void handleToggleGraySensor()
{
  // 切换连续模式
  continuousGray = !continuousGray;

  // 创建响应
  DynamicJsonDocument doc(128);
  doc["enabled"] = continuousGray;
  doc["message"] = continuousGray ? "Continuous grayscale output enabled" : "Continuous grayscale output disabled";

  String jsonResponse;
  serializeJson(doc, jsonResponse);
  server.send(200, "application/json", jsonResponse);
}

// 获取激光距离
void handleGetLaserDistance()
{
  DynamicJsonDocument doc(128);

  // 获取激光测距值
  int distance = getLaserDistanceOptimized();

  // 填充JSON响应
  doc["distance"] = distance;
  doc["unit"] = "mm";

  String jsonResponse;
  serializeJson(doc, jsonResponse);
  server.send(200, "application/json", jsonResponse);
}

// 切换激光测距连续模式
void handleToggleLaserDistance()
{
  // 切换连续模式
  continuousLaser = !continuousLaser;

  // 创建响应
  DynamicJsonDocument doc(128);
  doc["enabled"] = continuousLaser;
  doc["message"] = continuousLaser ? "Continuous laser distance output enabled" : "Continuous laser distance output disabled";

  String jsonResponse;
  serializeJson(doc, jsonResponse);
  server.send(200, "application/json", jsonResponse);
}

// 控制步进电机
void handleControlStepper()
{
  int motor = server.arg("motor").toInt();
  int steps = server.arg("steps").toInt();
  int speed = server.arg("speed").toInt();

  // 快速参数验证和响应
  if (motor >= 1 && motor <= 4 && abs(steps) <= 10000 && speed >= 0 && speed <= 255)
  {
    // 立即发送简单响应，提高响应速度
    server.send(200, "text/plain", "OK");

    // 然后执行电机控制（异步）
    controlStepper(motor, steps, speed);
  }
  else
  {
    server.send(400, "text/plain", "ERROR");
  }
}

// 处理底盘控制请求 - 增强版本
void handleControlChassis()
{
  String direction = server.arg("direction");
  int speed = server.arg("speed").toInt();

  // 限制速度范围
  speed = constrain(speed, 0, 100);

  uint8_t chassisDirection = CHASSIS_STOP;

  if (direction == "forward")
  {
    chassisDirection = CHASSIS_FORWARD;
  }
  else if (direction == "backward")
  {
    chassisDirection = CHASSIS_BACKWARD;
  }
  else if (direction == "left")
  {
    chassisDirection = CHASSIS_LEFT;
  }
  else if (direction == "right")
  {
    chassisDirection = CHASSIS_RIGHT;
  }
  else if (direction == "stop")
  {
    chassisDirection = CHASSIS_STOP;
  }

  // 立即发送响应，提高响应速度
  server.send(200, "text/plain", "OK");

  // 使用增强的底盘控制（带平滑加速和安全功能）
  control_chassis_enhanced(chassisDirection, speed, 0);

  debugPrintAll("Enhanced web chassis control: " + direction + " target_speed=" + String(speed));
}

// 处理紧急停止请求
void handleEmergencyStop()
{
  chassis_emergency_stop();
  server.send(200, "application/json", "{\"status\":\"emergency_stop_activated\"}");
  debugPrintAll("Web emergency stop activated");
}

// 处理清除紧急停止请求
void handleClearEmergencyStop()
{
  chassis_clear_emergency_stop();
  server.send(200, "application/json", "{\"status\":\"emergency_stop_cleared\"}");
  debugPrintAll("Web emergency stop cleared");
}

// 获取底盘状态
void handleGetChassisStatus()
{
  DynamicJsonDocument doc(512);

  bool isMoving, emergencyStop;
  uint8_t currentSpeed, targetSpeed, direction;
  chassis_get_status(isMoving, emergencyStop, currentSpeed, targetSpeed, direction);

  doc["isMoving"] = isMoving;
  doc["emergencyStop"] = emergencyStop;
  doc["currentSpeed"] = currentSpeed;
  doc["targetSpeed"] = targetSpeed;
  doc["direction"] = direction;
  doc["position"]["x"] = chassis_current_x;
  doc["position"]["y"] = chassis_current_y;
  doc["position"]["angle"] = chassis_current_angle;

  // 添加安全状态信息
  doc["safety"]["enabled"] = chassisState.safetyEnabled;
  doc["safety"]["acceleration"] = chassisState.acceleration;
  doc["safety"]["deceleration"] = chassisState.deceleration;
  doc["safety"]["minSpeed"] = chassisState.minSpeed;
  doc["safety"]["maxSpeed"] = chassisState.maxSpeed;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// 获取系统健康状态
void handleGetSystemHealth()
{
  DynamicJsonDocument doc(512);

  doc["systemHealthy"] = checkSystemHealth();
  doc["timestamp"] = millis();

  // 这里需要访问systemHealth结构，但它在main.cpp中是静态的
  // 为了简化，我们提供基本的健康检查
  doc["memory"]["free"] = ESP.getFreeHeap();
  doc["memory"]["total"] = ESP.getHeapSize();
  doc["memory"]["healthy"] = (ESP.getFreeHeap() > 15000);

  doc["temperature"]["value"] = temperatureRead();
  doc["temperature"]["healthy"] = (temperatureRead() < 80.0);

  doc["wifi"]["clients"] = WiFi.softAPgetStationNum();
  doc["wifi"]["healthy"] = true;

  doc["uptime"] = millis() / 1000;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);

  logSystemEvent("System health requested via web interface");
}

// 设置步进电机使能状态
void handleSetStepperEnable()
{
  int motor = server.arg("motor").toInt();
  bool enable = server.arg("enable") == "true";

  if (motor >= 1 && motor <= 4)
  {
    setStepperEnable(motor, enable);

    StaticJsonDocument<100> doc;
    doc["success"] = true;
    doc["message"] = "Stepper motor " + String(motor) + " " + (enable ? "enabled" : "disabled");

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  }
  else
  {
    server.send(400, "application/json", "{\"success\": false, \"message\": \"Invalid motor number\"}");
  }
}

// 获取步进电机状态
void handleGetStepperStatus()
{
  DynamicJsonDocument doc(512);
  doc["success"] = true;

  JsonArray motors = doc.createNestedArray("motors");
  for (int i = 1; i <= 4; i++)
  {
    JsonObject motor = motors.createNestedObject();
    motor["id"] = i;

    // 获取电机状态
    bool isRunning = false;
    uint8_t sleepPin;

    switch (i)
    {
    case 1:
      sleepPin = SLEEP1;
      isRunning = motor1State.isRunning;
      motor["steps_remaining"] = motor1State.remainingSteps;
      break;
    case 2:
      sleepPin = SLEEP2;
      isRunning = motor2State.isRunning;
      motor["steps_remaining"] = motor2State.remainingSteps;
      break;
    case 3:
      sleepPin = SLEEP3;
      isRunning = motor3State.isRunning;
      motor["steps_remaining"] = motor3State.remainingSteps;
      break;
    case 4:
      sleepPin = SLEEP4;
      isRunning = motor4State.isRunning;
      motor["steps_remaining"] = motor4State.remainingSteps;
      break;
    }

    bool isEnabled = (digitalRead(sleepPin) == HIGH);

    motor["enabled"] = isEnabled;
    motor["running"] = isRunning;
  }

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// 获取系统状态
void handleGetSystemStatus()
{
  DynamicJsonDocument doc(1024);

  // 基本系统信息
  doc["uptime"] = millis() / 1000;                                  // 运行时间（秒）
  doc["free_memory"] = ESP.getFreeHeap() * 100 / ESP.getHeapSize(); // 可用内存百分比
  doc["total_memory"] = ESP.getHeapSize();
  doc["cpu_temp"] = temperatureRead(); // ESP32 内部温度

  // WiFi信息
  doc["wifi_ssid"] = ssid;
  doc["wifi_connected"] = true; // 作为AP模式，始终为true
  doc["wifi_ip"] = WiFi.softAPIP().toString();
  doc["clients_connected"] = WiFi.softAPgetStationNum();

  // 舵机信息
  JsonArray servos = doc.createNestedArray("servos");
  for (int i = 0; i < 3; i++)
  {
    JsonObject servo = servos.createNestedObject();
    servo["id"] = i + 1;
    servo["angle"] = currentServoAngles[i];
    servo["type"] = servoTypes[i];
  }

  // 灰度传感器信息
  JsonObject graySensor = doc.createNestedObject("graySensor");
  graySensor["continuousEnabled"] = continuousGray;
  JsonArray grayValues = graySensor.createNestedArray("values");
  int values[5];
  grayTracker.getSensorValues(values);
  for (int i = 0; i < 5; i++)
  {
    grayValues.add(values[i]);
  }

  // 激光测距信息
  JsonObject laser = doc.createNestedObject("laser");
  laser["continuousEnabled"] = continuousLaser;
  laser["distance"] = getLaserDistanceOptimized();
  laser["unit"] = "mm";

  String jsonResponse;
  serializeJson(doc, jsonResponse);
  server.send(200, "application/json", jsonResponse);
}

// 保存设置API
void handleSaveSettings()
{
  String data = server.arg("plain");
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, data);

  if (error)
  {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
    return;
  }

  bool restartRequired = false;
  String newSSID = doc["ap_name"].as<String>();
  String newPassword = doc["ap_password"].as<String>();
  int newRefreshRate = doc["refresh_rate"].as<int>();

  // 验证输入
  if (newSSID.length() < 1 || newSSID.length() > 32)
  {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"AP name must be 1-32 characters\"}");
    return;
  }

  if (newPassword.length() < 8 || newPassword.length() > 64)
  {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Password must be 8-64 characters\"}");
    return;
  }

  if (newRefreshRate < 100 || newRefreshRate > 5000)
  {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Refresh rate must be 100-5000 ms\"}");
    return;
  }

  // 如果WiFi名称或密码改变，则需要重启
  if (newSSID != String(ssid) || newPassword != String(password))
  {
    restartRequired = true;
  }

  // 保存设置到EEPROM
  saveSettingsToEEPROM(newSSID, newPassword, newRefreshRate);

  // 构建响应
  DynamicJsonDocument response(256);
  response["status"] = "success";
  response["message"] = "Settings saved successfully";
  response["restart_required"] = restartRequired;

  String jsonResponse;
  serializeJson(response, jsonResponse);
  server.send(200, "application/json", jsonResponse);
}

// 重置设置API
void handleResetSettings()
{
  // 构建默认设置响应
  DynamicJsonDocument response(256);
  response["status"] = "success";
  response["ap_name"] = "ServoController";
  response["ap_password"] = "12345678";
  response["refresh_rate"] = 500;

  String jsonResponse;
  serializeJson(response, jsonResponse);
  server.send(200, "application/json", jsonResponse);
}

// 设备重启API
void handleRestart()
{
  server.send(200, "text/plain", "Restarting device...");
  delay(1000);
  ESP.restart();
}

// 初始化Web服务器
void initWebServer()
{
  // 设置Web服务器路由
  server.on("/", handleRoot);

  // 舵机控制API
  server.on("/servo_status", HTTP_GET, handleGetServoStatus);
  server.on("/move_servo", HTTP_POST, handleMoveServo);
  server.on("/set_servo_type", HTTP_POST, handleSetServoType);
  server.on("/set_all_servo_type", HTTP_POST, handleSetAllServoType);
  server.on("/move_all_servos", HTTP_POST, handleMoveAllServos);

  // 传感器API
  server.on("/gray_data", HTTP_GET, handleGetGraySensorData);
  server.on("/toggle_gray", HTTP_POST, handleToggleGraySensor);
  server.on("/laser_data", HTTP_GET, handleGetLaserDistance);
  server.on("/toggle_laser", HTTP_POST, handleToggleLaserDistance);

  // 步进电机API
  server.on("/control_stepper", HTTP_POST, handleControlStepper);
  server.on("/set_stepper_enable", HTTP_POST, handleSetStepperEnable);
  server.on("/stepper_status", HTTP_GET, handleGetStepperStatus);

  // 底盘控制API
  server.on("/control_chassis", HTTP_POST, handleControlChassis);
  server.on("/emergency_stop", HTTP_POST, handleEmergencyStop);
  server.on("/clear_emergency_stop", HTTP_POST, handleClearEmergencyStop);
  server.on("/chassis_status", HTTP_GET, handleGetChassisStatus);

  // 系统API
  server.on("/system_status", HTTP_GET, handleGetSystemStatus);
  server.on("/system_health", HTTP_GET, handleGetSystemHealth);

  // 新增设置API
  server.on("/save_settings", HTTP_POST, handleSaveSettings);
  server.on("/reset_settings", HTTP_GET, handleResetSettings);
  server.on("/restart", HTTP_GET, handleRestart);

  // 处理404
  server.onNotFound(handleNotFound);

  // 启动Web服务器
  server.begin();
  debugPrintAll("Web server started at http://" + WiFi.softAPIP().toString());
}

// 处理Web服务器请求
void handleWebServer()
{
  server.handleClient();
}

#endif