#include <Arduino.h>
#include "pins.h"
#include "servo.h"
#include "StepperMotor.h"
#include "GrayTrackerDebug.h"
#include "chassis.h"
#include "task.h"       // 包含NeoPixel定义
#include "web_server.h" // 使用同步Web服务器
#include "wifi_config.h"
#include "laser.h"

GrayTrackerDebug grayTracker;

// 串口相关
String serialBuffer = "";
String serialBuffer2 = "";

// 控制连续输出的变量
bool continuousGray = false;
bool continuousLaser = false;
unsigned long lastOutputTime = 0;
const unsigned long outputInterval = 50; // 输出间隔（毫秒）- 优化为50ms

// 声明命令处理函数
void handleCommand(String cmd);

// 声明辅助函数
void showHelp();
void debugPrintAll(const String &message);
void debugPrint(const String &message);
int getLaserDistanceOptimized();

// 错误处理和状态监控
void handleSystemError(const String &errorMsg);
void printSystemStatus();
void watchdogReset();

// 位置管理函数
void resetAllPositions();
void printAllPositions();
void initializePositions();

// 在setup中已有定义
void setup()
{
  // 初始化串口
  Serial.begin(115200);
  Serial1.begin(9600);
  // 初始化Serial2，指定RX=21, TX=20引脚
  Serial2.begin(HOST_BAUD_RATE, SERIAL_8N1, HOST_SERIAL_RX, HOST_SERIAL_TX);

  debugPrintAll("Serial ports initialized:");
  debugPrintAll("  USB Serial: 115200 baud");
  debugPrintAll("  Serial1 (Chassis): 9600 baud, TX=" + String(CHASSIS_SERIAL_TX));
  debugPrintAll("  Serial2 (Host): " + String(HOST_BAUD_RATE) + " baud, RX=" + String(HOST_SERIAL_RX) + ", TX=" + String(HOST_SERIAL_TX));

  // 初始化PinMux
  delay(500);

  // 配置灰度传感器引脚
  grayTracker.begin();

  // 初始化舵机，默认位置为90度，不移动
  initServos(90, 90, 90, false);

  // 初始化步进电机
  initSteppers();
  debugPrintAll("Stepper motors initialized (all disabled)");

  // 初始化底盘控制串口
  chassis_serial_init();
  debugPrintAll("Chassis control initialized");

  // 初始化激光传感器
  if (laser_init())
  {
    debugPrintAll("Laser sensor initialized successfully");
  }
  else
  {
    debugPrintAll("Warning: Laser sensor initialization failed");
  }

  // 初始化WiFi
  initWiFi();
  debugPrintAll("WiFi AP initialized");

  // 初始化Web服务器
  initWebServer();
  debugPrintAll("Web server initialized");

  // 初始化位置跟踪
  initializePositions();

  debugPrintAll("Ready. Type 'help' for command list.");
  debugPrintAll("Serial ports: USB(Serial), Host(Serial2 RX=21 TX=20), Laser(Serial0 RX=36 TX=35), Chassis(Serial1 TX=19)");

  // 打印初始系统状态
  printSystemStatus();

  // 设置看门狗（如果需要）
  // esp_task_wdt_init(30, true); // 30秒看门狗
}

// 原有loop保持不变，只添加Web服务器处理
void loop()
{
  // Process serial commands
  if (Serial.available())
  {
    String cmd = Serial.readStringUntil('\n');
    Serial.println("USB Serial received: " + cmd);
    handleCommand(cmd);
  }
  if (Serial2.available())
  {
    String cmd = Serial2.readStringUntil('\n');
    cmd.trim(); // 移除换行符和空格
    Serial.println("Serial2 (RX=" + String(HOST_SERIAL_RX) + ", TX=" + String(HOST_SERIAL_TX) + ") received: '" + cmd + "'");
    debugPrintAll("Host command: " + cmd);
    handleCommand(cmd);
  }

  // Continuous output mode
  unsigned long currentTime = millis();
  if (currentTime - lastOutputTime >= outputInterval)
  {
    lastOutputTime = currentTime;

    if (continuousGray)
    {
      int values[5];
      grayTracker.update(); // Update sensor data first
      grayTracker.getSensorValues(values);
      debugPrintAll("GRAY: " + String(values[0]) + "," + String(values[1]) + "," + String(values[2]) + "," + String(values[3]) + "," + String(values[4]));
    }

    if (continuousLaser)
    {
      int dist = getLaserDistanceOptimized();
      if (dist > 0)
      {
        debugPrintAll("LASER: " + String(dist) + " mm (filtered)");
      }
      else
      {
        debugPrintAll("LASER: Out of range (filtered)");
      }
    }
  }

  // Update stepper motors (CRITICAL for web control)
  updateSteppers();

  // 处理Web服务器请求
  handleWebServer();

  // 内存监控（每30秒检查一次）
  static unsigned long lastMemoryCheck = 0;
  if (millis() - lastMemoryCheck > 30000)
  {
    lastMemoryCheck = millis();
    if (ESP.getFreeHeap() < 10000)
    { // 如果可用内存少于10KB
      handleSystemError("Low memory warning: " + String(ESP.getFreeHeap()) + " bytes free");
    }
  }
}

// 命令处理函数实现
void handleCommand(String cmd)
{
  cmd.trim();

  if (cmd == "help")
  {
    showHelp();
    return;
  }

  if (cmd == "debug")
  {
    debugPrintAll("=== Debug Information ===");
    debugPrintAll("Current servo positions: " + String(currentServoAngles[0]) + ", " + String(currentServoAngles[1]) + ", " + String(currentServoAngles[2]));
    debugPrintAll("Web server running: Yes");
    debugPrintAll("WiFi AP: " + String(ssid) + " (Password: " + String(password) + ")");
    debugPrintAll("=== End of Debug Info ===");
    return;
  }

  if (cmd == "status")
  {
    printSystemStatus();
    return;
  }

  if (cmd == "restart")
  {
    debugPrintAll("Restarting system...");
    watchdogReset();
    return;
  }

  if (cmd == "test_laser")
  {
    debugPrintAll("Testing laser sensor...");
    for (int i = 0; i < 5; i++)
    {
      int distance = getLaserDistanceOptimized();
      if (distance > 0)
      {
        debugPrintAll("Test " + String(i + 1) + ": " + String(distance) + " mm");
      }
      else
      {
        debugPrintAll("Test " + String(i + 1) + ": FAILED");
      }
      delay(200); // 优化延时从500ms到200ms
    }
    return;
  }

  if (cmd == "test_serial")
  {
    debugPrintAll("=== Serial Port Test ===");
    debugPrintAll("USB Serial (Serial): Available");
    debugPrintAll("Serial1 (Chassis, TX=" + String(CHASSIS_SERIAL_TX) + "): " + (Serial1 ? "Available" : "Not Available"));
    debugPrintAll("Serial2 (Host, RX=" + String(HOST_SERIAL_RX) + ", TX=" + String(HOST_SERIAL_TX) + "): " + (Serial2 ? "Available" : "Not Available"));

    // 发送测试消息到Serial2
    Serial2.println("Serial2 test message from ESP32");
    debugPrintAll("Test message sent to Serial2");
    debugPrintAll("Please check if host receives this message");
    debugPrintAll("========================");
    return;
  }

  // 位置管理命令
  if (cmd == "positions")
  {
    printAllPositions();
    return;
  }

  if (cmd == "reset_positions")
  {
    resetAllPositions();
    return;
  }

  // 底盘绝对位置控制命令（支持速度参数）
  if (cmd.startsWith("chassis_to_x,"))
  {
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);

    if (firstComma > 0)
    {
      int target_x = cmd.substring(firstComma + 1, secondComma > 0 ? secondComma : cmd.length()).toInt();
      uint8_t speed = 0; // 默认速度

      if (secondComma > 0)
      {
        speed = cmd.substring(secondComma + 1).toInt();
      }

      chassis_move_to_x(target_x, speed);
      debugPrintAll("Chassis moved to X=" + String(target_x) + "mm" + (speed > 0 ? " at speed " + String(speed) : ""));
    }
    else
    {
      debugPrintAll("Format error, should be: chassis_to_x,X[,SPEED]");
    }
    return;
  }

  if (cmd.startsWith("chassis_to_y,"))
  {
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);

    if (firstComma > 0)
    {
      int target_y = cmd.substring(firstComma + 1, secondComma > 0 ? secondComma : cmd.length()).toInt();
      uint8_t speed = 0; // 默认速度

      if (secondComma > 0)
      {
        speed = cmd.substring(secondComma + 1).toInt();
      }

      chassis_move_to_y(target_y, speed);
      debugPrintAll("Chassis moved to Y=" + String(target_y) + "mm" + (speed > 0 ? " at speed " + String(speed) : ""));
    }
    else
    {
      debugPrintAll("Format error, should be: chassis_to_y,Y[,SPEED]");
    }
    return;
  }

  if (cmd.startsWith("chassis_to_angle,"))
  {
    int commaIndex = cmd.indexOf(',');
    if (commaIndex > 0)
    {
      int target_angle = cmd.substring(commaIndex + 1).toInt();
      chassis_rotate_to_angle(target_angle);
      debugPrintAll("Chassis rotated to " + String(target_angle) + "°");
    }
    else
    {
      debugPrintAll("Format error, should be: chassis_to_angle,ANGLE");
    }
    return;
  }

  if (cmd.startsWith("chassis_to_pos,"))
  {
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);
    if (firstComma > 0 && secondComma > firstComma)
    {
      int target_x = cmd.substring(firstComma + 1, secondComma).toInt();
      int target_y = cmd.substring(secondComma + 1).toInt();
      chassis_move_to_position(target_x, target_y);
      debugPrintAll("Chassis moved to position (" + String(target_x) + ", " + String(target_y) + ")");
    }
    else
    {
      debugPrintAll("Format error, should be: chassis_to_pos,X,Y");
    }
    return;
  }

  // 直接底盘移动控制命令
  if (cmd.startsWith("chassis_move,"))
  {
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);

    if (firstComma > 0 && secondComma > firstComma)
    {
      String direction = cmd.substring(firstComma + 1, secondComma);
      uint8_t speed = cmd.substring(secondComma + 1).toInt();

      if (speed < 1)
        speed = 50; // 默认速度
      if (speed > 100)
        speed = 100; // 限制最大速度

      uint8_t chassisDirection = CHASSIS_STOP;

      if (direction == "forward" || direction == "f")
      {
        chassisDirection = CHASSIS_FORWARD;
      }
      else if (direction == "backward" || direction == "b")
      {
        chassisDirection = CHASSIS_BACKWARD;
      }
      else if (direction == "left" || direction == "l")
      {
        chassisDirection = CHASSIS_LEFT;
      }
      else if (direction == "right" || direction == "r")
      {
        chassisDirection = CHASSIS_RIGHT;
      }
      else if (direction == "stop" || direction == "s")
      {
        chassisDirection = CHASSIS_STOP;
      }

      control_chassis_raw(chassisDirection, speed, 0);
      debugPrintAll("Chassis " + direction + " at speed " + String(speed));
    }
    else
    {
      debugPrintAll("Format error, should be: chassis_move,DIRECTION,SPEED");
      debugPrintAll("DIRECTION: forward/f, backward/b, left/l, right/r, stop/s");
      debugPrintAll("SPEED: 1-100");
    }
    return;
  }

  // 步进电机绝对位置控制命令（支持速度参数）
  if (cmd.startsWith("stepper_to,"))
  {
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);
    int thirdComma = cmd.indexOf(',', secondComma + 1);

    if (firstComma > 0 && secondComma > firstComma)
    {
      int motorNumber = cmd.substring(firstComma + 1, secondComma).toInt();
      long target_position = cmd.substring(secondComma + 1, thirdComma > 0 ? thirdComma : cmd.length()).toInt();
      uint8_t speed = 0; // 默认速度

      if (thirdComma > 0)
      {
        speed = cmd.substring(thirdComma + 1).toInt();
      }

      if (stepper_move_to_position(motorNumber, target_position, speed))
      {
        debugPrintAll("Stepper motor " + String(motorNumber) + " moved to position " + String(target_position) + " steps" + (speed > 0 ? " at speed " + String(speed) : ""));
      }
      else
      {
        debugPrintAll("Failed to move stepper motor " + String(motorNumber));
      }
    }
    else
    {
      debugPrintAll("Format error, should be: stepper_to,MOTOR,POSITION[,SPEED]");
      debugPrintAll("Example: stepper_to,1,200,100 (move motor 1 to position 200 steps at speed 100)");
    }
    return;
  }

  if (cmd.startsWith("stepper_rel,"))
  {
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);
    int thirdComma = cmd.indexOf(',', secondComma + 1);

    if (firstComma > 0 && secondComma > firstComma)
    {
      int motorNumber = cmd.substring(firstComma + 1, secondComma).toInt();
      long relative_steps = cmd.substring(secondComma + 1, thirdComma > 0 ? thirdComma : cmd.length()).toInt();
      uint8_t speed = 0; // 默认速度

      if (thirdComma > 0)
      {
        speed = cmd.substring(thirdComma + 1).toInt();
      }

      if (stepper_move_relative(motorNumber, relative_steps, speed))
      {
        debugPrintAll("Stepper motor " + String(motorNumber) + " moved " + String(relative_steps) + " steps relative" + (speed > 0 ? " at speed " + String(speed) : ""));
      }
      else
      {
        debugPrintAll("Failed to move stepper motor " + String(motorNumber));
      }
    }
    else
    {
      debugPrintAll("Format error, should be: stepper_rel,MOTOR,STEPS[,SPEED]");
      debugPrintAll("Example: stepper_rel,1,100,80 (move motor 1 by 100 steps at speed 80)");
    }
    return;
  }

  if (cmd.startsWith("stepper_pos,"))
  {
    int commaIndex = cmd.indexOf(',');
    if (commaIndex > 0)
    {
      int motorNumber = cmd.substring(commaIndex + 1).toInt();
      stepper_print_position(motorNumber);
    }
    else
    {
      debugPrintAll("Format error, should be: stepper_pos,MOTOR");
      debugPrintAll("Example: stepper_pos,1 (show position of motor 1)");
    }
    return;
  }

  if (cmd == "stepper_positions")
  {
    stepper_print_all_positions();
    return;
  }

  if (cmd == "fast_mode")
  {
    // 启用快速响应模式
    motor1State.pulseInterval = 200;
    motor2State.pulseInterval = 200;
    motor3State.pulseInterval = 200;
    motor4State.pulseInterval = 200;
    debugPrintAll("Fast response mode enabled (200us pulse interval)");
    return;
  }

  if (cmd == "normal_mode")
  {
    // 恢复正常响应模式
    motor1State.pulseInterval = 500;
    motor2State.pulseInterval = 500;
    motor3State.pulseInterval = 500;
    motor4State.pulseInterval = 500;
    debugPrintAll("Normal response mode enabled (500us pulse interval)");
    return;
  }

  if (cmd == "test_chassis")
  {
    debugPrintAll("=== Chassis Communication Test ===");
    debugPrintAll("Testing chassis serial communication...");

    // 测试不同的命令格式
    debugPrintAll("Test 1: Forward command");
    control_chassis_raw(CHASSIS_FORWARD, 50, 0);
    delay(1000);

    debugPrintAll("Test 2: Stop command");
    control_chassis_raw(CHASSIS_STOP, 0, 0);
    delay(500);

    debugPrintAll("Test 3: Backward command");
    control_chassis_raw(CHASSIS_BACKWARD, 50, 0);
    delay(1000);

    debugPrintAll("Test 4: Stop command");
    control_chassis_raw(CHASSIS_STOP, 0, 0);

    debugPrintAll("Chassis test completed");
    debugPrintAll("===============================");
    return;
  }

  if (cmd == "gray")
  {
    continuousGray = !continuousGray;
    debugPrintAll("Continuous grayscale output " + String(continuousGray ? "enabled" : "disabled"));
    return;
  }

  if (cmd == "gray_raw")
  {
    grayTracker.update();
    int values[5];
    grayTracker.getSensorValues(values);
    debugPrintAll("GRAY_RAW: " + String(values[0]) + "," + String(values[1]) + "," + String(values[2]) + "," + String(values[3]) + "," + String(values[4]));
    return;
  }

  if (cmd == "laser")
  {
    continuousLaser = !continuousLaser;
    debugPrintAll("Continuous laser output " + String(continuousLaser ? "enabled" : "disabled"));
    return;
  }

  if (cmd.startsWith("servo,"))
  {
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);
    if (firstComma > 0 && secondComma > firstComma)
    {
      int n = cmd.substring(firstComma + 1, secondComma).toInt();
      int a = cmd.substring(secondComma + 1).toInt();

      // Get servo type and handle angle
      ServoType type = servoTypes[n - 1];
      int currentAngle = currentServoAngles[n - 1];

      debugPrintAll("Control Servo " + String(n) + ": Current position=" + String(currentAngle) + "°, Target position=" + String(a) + "°");

      // 规范化角度值，但特殊处理360度
      int actualAngle;
      if (a == 360)
      {
        actualAngle = 360;
      }
      else
      {
        actualAngle = a % 360;
        if (actualAngle < 0)
          actualAngle += 360;
      }

      // 计算两个角度之间的角度差（最短路径）
      int clockwiseDist, counterClockwiseDist;
      if (a == 360)
      {
        clockwiseDist = 360 - currentAngle;
        counterClockwiseDist = currentAngle;
      }
      else
      {
        clockwiseDist = (actualAngle - currentAngle + 360) % 360;
        counterClockwiseDist = (currentAngle - actualAngle + 360) % 360;
      }

      debugPrintAll("Angle difference: Clockwise=" + String(clockwiseDist) + "°, Counter-clockwise=" + String(counterClockwiseDist) + "°");

      // Control servo
      if (n == 1)
        servo1(a);
      else if (n == 2)
        servo2(a);
      else if (n == 3)
        servo3(a);

      String typeStr = (type == SERVO_TYPE_STANDARD) ? "Standard (0-360)" : "Continuous (0-360)";

      debugPrintAll("SERVO" + String(n) + ": " + String(actualAngle) + " (Type: " + typeStr + ")");
    }
    else
    {
      debugPrintAll("Format error, should be: servo,N,A");
    }
    return;
  }

  // Set servo type command
  if (cmd.startsWith("servo_type,"))
  {
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);
    if (firstComma > 0 && secondComma > firstComma)
    {
      int servoIndex = cmd.substring(firstComma + 1, secondComma).toInt() - 1; // Convert to 0-2 index
      int typeValue = cmd.substring(secondComma + 1).toInt();

      if (servoIndex >= 0 && servoIndex < 3)
      {
        ServoType type = (typeValue == 0) ? SERVO_TYPE_STANDARD : SERVO_TYPE_CONTINUOUS;
        setServoType(servoIndex, type);
        debugPrintAll("Servo " + String(servoIndex + 1) + " type set to: " +
                      (type == SERVO_TYPE_STANDARD ? "Standard (0-360)" : "Continuous (0-360)"));
      }
      else
      {
        debugPrintAll("Error: Servo index must be 1-3");
      }
    }
    else
    {
      debugPrintAll("Format error, should be: servo_type,N,T");
      debugPrintAll("N: Servo number (1-3), T: Type (0=Standard 0-360, 1=Continuous 0-360)");
    }
    return;
  }

  // Set all servos to standard mode
  if (cmd == "servo_all_std")
  {
    setAllServosStandard();
    debugPrintAll("All servos set to standard mode (0-360 degrees)");
    return;
  }

  // Set all servos to continuous rotation mode
  if (cmd == "servo_all_cont")
  {
    setAllServosContinuous();
    debugPrintAll("All servos set to continuous mode (0-360 degrees)");
    return;
  }

  // Show servo status
  if (cmd == "servo_status")
  {
    showServoStatus();
    return;
  }

  if (cmd.startsWith("stepper,"))
  {
    // 处理步进电机命令
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);
    int thirdComma = cmd.indexOf(',', secondComma + 1);

    if (firstComma > 0 && secondComma > firstComma && thirdComma > secondComma)
    {
      int motorNumber = cmd.substring(firstComma + 1, secondComma).toInt();
      int steps = cmd.substring(secondComma + 1, thirdComma).toInt();
      int speed = cmd.substring(thirdComma + 1).toInt();

      // 验证参数
      if (motorNumber < 1 || motorNumber > 4)
      {
        debugPrintAll("Error: Motor number must be 1-4");
        return;
      }

      if (speed < 1 || speed > 200)
      {
        debugPrintAll("Warning: Speed should be 1-200, adjusting to valid range");
        speed = constrain(speed, 1, 200);
      }

      // 控制步进电机
      debugPrintAll("Controlling stepper " + String(motorNumber) + ": " + String(steps) + " steps at speed " + String(speed));
      bool result = stepper(motorNumber, speed, steps);

      if (result)
      {
        debugPrintAll("Stepper motor command sent successfully");
      }
      else
      {
        debugPrintAll("Error controlling stepper motor");
      }
    }
    else
    {
      debugPrintAll("Format error, should be: stepper,N,S,V");
      debugPrintAll("N: Motor number (1-4), S: Steps (+ or -), V: Speed (1-200)");
    }
    return;
  }

  // 步进电机使能/禁用命令
  if (cmd.startsWith("stepper_enable,"))
  {
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);

    if (firstComma > 0 && secondComma > firstComma)
    {
      int motorNumber = cmd.substring(firstComma + 1, secondComma).toInt();
      int enable = cmd.substring(secondComma + 1).toInt();

      // 验证参数
      if (motorNumber < 1 || motorNumber > 4)
      {
        debugPrintAll("Error: Motor number must be 1-4");
        return;
      }

      // 获取引脚
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
    else
    {
      debugPrintAll("Format error, should be: stepper_enable,N,E");
      debugPrintAll("N: Motor number (1-4), E: Enable (1) or Disable (0)");
    }
    return;
  }

  // 显示步进电机状态
  if (cmd == "stepper_status")
  {
    debugPrintAll("=== Stepper Motors Status ===");
    for (int i = 1; i <= 4; i++)
    {
      bool isRunning = false;
      uint8_t sleepPin;

      // 获取引脚和运行状态
      switch (i)
      {
      case 1:
        sleepPin = SLEEP1;
        isRunning = motor1State.isRunning;
        break;
      case 2:
        sleepPin = SLEEP2;
        isRunning = motor2State.isRunning;
        break;
      case 3:
        sleepPin = SLEEP3;
        isRunning = motor3State.isRunning;
        break;
      case 4:
        sleepPin = SLEEP4;
        isRunning = motor4State.isRunning;
        break;
      }

      bool isEnabled = (digitalRead(sleepPin) == HIGH);

      debugPrintAll("Stepper " + String(i) + ": " +
                    (isEnabled ? "Enabled" : "Disabled") + ", " +
                    (isRunning ? "Running" : "Stopped"));
    }
    debugPrintAll("=========================");
    return;
  }

  // 如果没有匹配的命令，显示未知命令消息
  debugPrintAll("Unknown command, type 'help' for command list.");
}

// 显示帮助信息
void showHelp()
{
  debugPrintAll("=== Command List ===");
  debugPrintAll("help                  Show this help");
  debugPrintAll("debug                 Show debug information");
  debugPrintAll("status                Show detailed system status");
  debugPrintAll("restart               Restart the system");
  debugPrintAll("test_laser            Test laser sensor (5 readings)");
  debugPrintAll("test_serial           Test serial port connections");
  debugPrintAll("test_chassis          Test chassis communication");
  debugPrintAll("");
  debugPrintAll("=== Position Management ===");
  debugPrintAll("positions             Show all current positions");
  debugPrintAll("reset_positions       Reset all positions to origin");
  debugPrintAll("");
  debugPrintAll("=== Chassis Control ===");
  debugPrintAll("chassis_to_x,X[,S]    Move chassis to X position (mm) [at speed S]");
  debugPrintAll("chassis_to_y,Y[,S]    Move chassis to Y position (mm) [at speed S]");
  debugPrintAll("chassis_to_angle,A    Rotate chassis to angle A (degrees)");
  debugPrintAll("chassis_to_pos,X,Y    Move chassis to position (X,Y)");
  debugPrintAll("chassis_move,DIR,S    Direct chassis control (DIR: f/b/l/r/s, S: 1-100)");
  debugPrintAll("");
  debugPrintAll("=== Stepper Motor Control ===");
  debugPrintAll("stepper_to,M,P[,S]    Move motor M to absolute position P [at speed S]");
  debugPrintAll("stepper_rel,M,ST[,S]  Move motor M by ST steps relative [at speed S]");
  debugPrintAll("stepper_pos,M         Show position of motor M");
  debugPrintAll("stepper_positions     Show all stepper motor positions");
  debugPrintAll("");
  debugPrintAll("=== Performance Control ===");
  debugPrintAll("fast_mode             Enable fast response mode (200us pulse)");
  debugPrintAll("normal_mode           Enable normal response mode (500us pulse)");
  debugPrintAll("");
  debugPrintAll("=== Sensor Control ===");
  debugPrintAll("gray                  Toggle continuous grayscale sensor output");
  debugPrintAll("gray_raw              Show raw grayscale sensor values");
  debugPrintAll("laser                 Toggle continuous laser distance output");
  debugPrintAll("servo,N,A             Control servo N (1-3) to angle A (0-360 regardless of type)");
  debugPrintAll("servo_type,N,T        Set servo N type: T=0 for standard (0-360), T=1 for continuous (0-360)");
  debugPrintAll("servo_all_std         Set all servos to standard mode (0-360 degrees)");
  debugPrintAll("servo_all_cont        Set all servos to continuous mode (0-360 degrees)");
  debugPrintAll("servo_status          Show current servo positions and types");
  debugPrintAll("stepper,N,S,V         Control stepper N with S steps at speed V");
  debugPrintAll("stepper_enable,N,E    Enable (E=1) or disable (E=0) stepper motor N");
  debugPrintAll("stepper_status        Show stepper motors status");

  debugPrintAll("\n=== Web Interface ===");
  debugPrintAll("A web interface is available at http://" + WiFi.softAPIP().toString());
  debugPrintAll("WiFi SSID: " + String(ssid) + ", Password: " + String(password));
  debugPrintAll("\nWeb interface features:");
  debugPrintAll("- Servo control with preset positions and custom angle settings");
  debugPrintAll("- Real-time grayscale sensor data with visual graphs");
  debugPrintAll("- Laser distance measurement with visual display");
  debugPrintAll("- Stepper motor control with step and speed adjustments");
  debugPrintAll("- System status monitoring and configuration options");
  debugPrintAll("- Data logging and download capabilities");
  debugPrintAll("- Settings management for WiFi and refresh rates");

  debugPrintAll("\n=== ESP32 Status ===");
  debugPrintAll("Free Memory: " + String(ESP.getFreeHeap()) + " bytes of " + String(ESP.getHeapSize()) + " bytes");
  debugPrintAll("CPU Temperature: " + String(temperatureRead()) + "°C");
  debugPrintAll("Uptime: " + String(millis() / 1000) + " seconds");
}

// 调试打印函数，发送到所有串口
void debugPrintAll(const String &message)
{
  Serial.println(message);
  Serial2.println(message);
}

// 调试打印函数，仅发送到USB串口
void debugPrint(const String &message)
{
  Serial.println(message);
}

// 获取优化后的激光距离（带滤波）
int getLaserDistanceOptimized()
{
  // 初始化激光传感器（如果尚未初始化）
  if (!laser_initialized)
  {
    if (!laser_init())
    {
      return -1; // 初始化失败
    }
  }

  // 获取原始距离值
  int rawDistance = get_laser_distance(true, false);

  // 如果原始值无效，返回-1
  if (rawDistance <= 0)
  {
    return -1;
  }

  // 应用滤波器
  int filteredDistance = apply_filter(rawDistance);

  return filteredDistance;
}

// 错误处理函数
void handleSystemError(const String &errorMsg)
{
  debugPrintAll("SYSTEM ERROR: " + errorMsg);
  debugPrintAll("Free heap: " + String(ESP.getFreeHeap()) + " bytes");
  debugPrintAll("CPU temperature: " + String(temperatureRead()) + "°C");
  debugPrintAll("Uptime: " + String(millis() / 1000) + " seconds");

  // 可以在这里添加更多的错误恢复逻辑
  // 例如重启某些模块或记录错误到EEPROM
}

// 打印系统状态
void printSystemStatus()
{
  debugPrintAll("=== System Status ===");
  debugPrintAll("ESP32 Chip Model: " + String(ESP.getChipModel()));
  debugPrintAll("Chip Revision: " + String(ESP.getChipRevision()));
  debugPrintAll("CPU Frequency: " + String(ESP.getCpuFreqMHz()) + " MHz");
  debugPrintAll("Flash Size: " + String(ESP.getFlashChipSize() / 1024 / 1024) + " MB");
  debugPrintAll("Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
  debugPrintAll("Total Heap: " + String(ESP.getHeapSize()) + " bytes");
  debugPrintAll("CPU Temperature: " + String(temperatureRead()) + "°C");
  debugPrintAll("Uptime: " + String(millis() / 1000) + " seconds");
  debugPrintAll("WiFi Status: " + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected"));
  debugPrintAll("WiFi SSID: " + String(ssid));
  debugPrintAll("WiFi IP: " + WiFi.softAPIP().toString());
  debugPrintAll("===================");
}

// 看门狗重置（软重启）
void watchdogReset()
{
  debugPrintAll("Performing watchdog reset...");
  delay(1000);
  ESP.restart();
}

// 位置管理函数实现

/**
 * 重置所有设备的位置为零点
 */
void resetAllPositions()
{
  debugPrintAll("=== Resetting All Positions ===");

  // Reset chassis position
  chassis_reset_position();

  // Reset all stepper motor positions
  stepper_reset_all_positions();

  // Reset servo position records to 90 degrees
  for (int i = 0; i < 3; i++)
  {
    currentServoAngles[i] = 90;
  }
  debugPrintAll("Servo positions reset to 90 degrees");

  debugPrintAll("All positions reset to origin");
  debugPrintAll("===============================");
}

/**
 * 打印所有设备的当前位置
 */
void printAllPositions()
{
  debugPrintAll("=== Current Positions ===");

  // Print chassis position
  chassis_print_position();

  // Print stepper motor positions
  stepper_print_all_positions();

  // Print servo positions
  debugPrintAll("=== Servo Positions ===");
  for (int i = 0; i < 3; i++)
  {
    debugPrintAll("Servo " + String(i + 1) + ": " + String(currentServoAngles[i]) + "°");
  }
  debugPrintAll("=======================");

  debugPrintAll("========================");
}

/**
 * 初始化所有位置记录
 */
void initializePositions()
{
  debugPrintAll("Initializing position tracking...");

  // Initialize chassis position to origin
  chassis_reset_position();

  // Initialize stepper motor positions to zero
  stepper_reset_all_positions();

  // Servo positions already initialized to 90 degrees in servo.h

  debugPrintAll("Position tracking initialized");
}