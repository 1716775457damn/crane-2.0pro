#include <Arduino.h>
#include "pins.h"
#include "servo.h"
#include "laser.h"
#include "GrayTrackerDebug.h"
#include "chassis.h"
#include "chassis_simple_absolute.h"
#include "stepper_absolute.h"

GrayTrackerDebug grayTracker;

// 全局位置状态变量
ChassisSimplePosition chassisPosition;
StepperAbsoluteState stepperAbsStates[4];

// 持续输出模式标志
bool continuousGray = false;
bool continuousLaser = false;
unsigned long lastOutputTime = 0;
const unsigned long outputInterval = 100; // 激光测距间隔改为100ms，提高响应速度

// 激光测距优化参数
const int LASER_SAMPLE_COUNT = 5; // 每次测量采样5次
const int LASER_FILTER_SIZE = 10; // 滤波窗口大小
int laserHistory[LASER_FILTER_SIZE] = {0};
int laserHistoryIndex = 0;
bool laserHistoryFilled = false;

// 步进电机控制参数
bool stepperEnabled[4] = {false, false, false, false}; // 四个步进电机的使能状态

// Optimize stepper motor parameters to avoid missing steps
int stepperMaxSpeed = 200;     // Keep max speed limit
int stepperAcceleration = 50;  // Reduce acceleration for smoother ramp-up
int stepperPulseWidthMin = 2;  // Try shorter pulse width
int stepperPulseWidthMax = 10; // Adjust max pulse width

// 同时输出到两个串口
void debugPrintAll(const String &msg)
{
  Serial.println(msg);
  if (Serial2)
  {
    Serial2.println(msg);
  }
  else
  {
    Serial.println("Warning: Serial2 not available");
  }
}

// 为servo.h提供的debugPrint函数
void debugPrint(const String &msg)
{
  debugPrintAll(msg);
}

// 步进电机初始化
void initSteppers()
{
  // 初始化所有步进电机引脚
  pinMode(SLEEP1, OUTPUT);
  pinMode(DIR1, OUTPUT);
  pinMode(STEP1, OUTPUT);

  pinMode(SLEEP2, OUTPUT);
  pinMode(DIR2, OUTPUT);
  pinMode(STEP2, OUTPUT);

  pinMode(SLEEP3, OUTPUT);
  pinMode(DIR3, OUTPUT);
  pinMode(STEP3, OUTPUT);

  pinMode(SLEEP4, OUTPUT);
  pinMode(DIR4, OUTPUT);
  pinMode(STEP4, OUTPUT);

  // 默认禁用所有步进电机
  digitalWrite(SLEEP1, LOW);
  digitalWrite(SLEEP2, LOW);
  digitalWrite(SLEEP3, LOW);
  digitalWrite(SLEEP4, LOW);

  debugPrintAll("Stepper motors initialized (all disabled)");
}

// 控制单个步进电机
void controlStepper(int motor, int steps, int speed)
{
  if (motor < 1 || motor > 4)
  {
    debugPrintAll("Error: Invalid motor number (1-4)");
    return;
  }

  if (!stepperEnabled[motor - 1])
  {
    debugPrintAll("Error: Motor " + String(motor) + " is disabled. Use 'stepper_enable,N' first");
    return;
  }

  // 检查速度范围 - 限制在安全范围内
  if (speed < 0 || speed > 200)
  {
    debugPrintAll("Error: Speed must be between 0-200 (safe range)");
    return;
  }

  // 确定方向和步数
  bool direction = (steps >= 0); // 正数为正向，负数为反向
  int absSteps = abs(steps);     // 取绝对值作为步数

  // 选择对应的引脚
  int sleepPin, dirPin, stepPin;
  switch (motor)
  {
  case 1:
    sleepPin = SLEEP1;
    dirPin = DIR1;
    stepPin = STEP1;
    break;
  case 2:
    sleepPin = SLEEP2;
    dirPin = DIR2;
    stepPin = STEP2;
    break;
  case 3:
    sleepPin = SLEEP3;
    dirPin = DIR3;
    stepPin = STEP3;
    break;
  case 4:
    sleepPin = SLEEP4;
    dirPin = DIR4;
    stepPin = STEP4;
    break;
  default:
    return;
  }

  // 设置方向
  digitalWrite(dirPin, direction ? HIGH : LOW);

  // 执行步进
  debugPrintAll("Motor " + String(motor) + " moving " + String(direction ? "forward" : "backward") + " " + String(absSteps) + " steps at speed " + String(speed));

  // 改进的速度计算 - 更保守但稳定的速度
  int stepDelay;
  if (speed == 0)
  {
    stepDelay = 10000; // 最慢速度
  }
  else if (speed <= 50)
  {
    // 低速范围: 安全启动
    stepDelay = 1000000 / (speed * 5);
  }
  else if (speed <= 100)
  {
    // 中速范围: 稳定运行
    stepDelay = 1000000 / (speed * 8);
  }
  else
  {
    // 高速范围: 谨慎运行
    stepDelay = 1000000 / (speed * 10);
  }

  // 确保最小延时 - 防止失步
  if (stepDelay < 200)
    stepDelay = 200;

  // 固定的脉冲宽度 - 确保驱动稳定
  int pulseWidth = 100; // 固定100微秒脉冲宽度

  // 添加加速度控制 - 防止失步
  int currentDelay = stepDelay * 2; // 从较慢速度开始
  int targetDelay = stepDelay;
  int accelerationSteps = min(50, absSteps / 4); // 加速步数

  for (int i = 0; i < absSteps; i++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(pulseWidth);
    digitalWrite(stepPin, LOW);

    // 加速度控制
    if (i < accelerationSteps)
    {
      // 加速阶段
      currentDelay = stepDelay * 2 - (stepDelay * i / accelerationSteps);
    }
    else if (i >= absSteps - accelerationSteps)
    {
      // 减速阶段
      currentDelay = stepDelay + (stepDelay * (i - (absSteps - accelerationSteps)) / accelerationSteps);
    }
    else
    {
      // 匀速阶段
      currentDelay = targetDelay;
    }

    delayMicroseconds(currentDelay);
  }

  debugPrintAll("Motor " + String(motor) + " movement completed");
}

// 使能/禁用步进电机
void setStepperEnable(int motor, bool enable)
{
  if (motor < 1 || motor > 4)
  {
    debugPrintAll("Error: Invalid motor number (1-4)");
    return;
  }

  int sleepPin;
  switch (motor)
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
  default:
    return;
  }

  digitalWrite(sleepPin, enable ? HIGH : LOW);
  stepperEnabled[motor - 1] = enable;

  debugPrintAll("Motor " + String(motor) + " " + (enable ? "enabled" : "disabled"));
}

// 显示步进电机状态
void showStepperStatus()
{
  debugPrintAll("=== Stepper Motor Status ===");
  for (int i = 1; i <= 4; i++)
  {
    debugPrintAll("Motor " + String(i) + ": " + (stepperEnabled[i - 1] ? "ENABLED" : "DISABLED"));
  }
  debugPrintAll("============================");
}

// 优化的激光测距函数
int getLaserDistanceOptimized()
{
  int samples[LASER_SAMPLE_COUNT];
  int validCount = 0;

  // 快速采样多次
  for (int i = 0; i < LASER_SAMPLE_COUNT; i++)
  {
    int dist = get_laser_distance(true, false);
    if (dist > 0 && dist < 8000)
    { // 有效范围检查
      samples[validCount++] = dist;
    }
    delay(10); // 短暂延时
  }

  if (validCount == 0)
  {
    return -1; // 没有有效数据
  }

  // 计算中值（去除异常值）
  int median = samples[0];
  if (validCount > 1)
  {
    // 简单排序找中值
    for (int i = 0; i < validCount - 1; i++)
    {
      for (int j = i + 1; j < validCount; j++)
      {
        if (samples[i] > samples[j])
        {
          int temp = samples[i];
          samples[i] = samples[j];
          samples[j] = temp;
        }
      }
    }
    median = samples[validCount / 2];
  }

  // 添加到历史记录
  laserHistory[laserHistoryIndex] = median;
  laserHistoryIndex = (laserHistoryIndex + 1) % LASER_FILTER_SIZE;
  if (laserHistoryIndex == 0)
  {
    laserHistoryFilled = true;
  }

  // 计算移动平均
  int sum = 0;
  int count = laserHistoryFilled ? LASER_FILTER_SIZE : laserHistoryIndex;
  for (int i = 0; i < count; i++)
  {
    sum += laserHistory[i];
  }

  return sum / count;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("USB Serial initialized");

  // 等待USB串口稳定
  delay(1000);

  // 初始化Serial2专门用于20、21引脚通信
  Serial2.setPins(HOST_SERIAL_RX, HOST_SERIAL_TX); // 先设置引脚 RX=21, TX=20
  Serial2.begin(115200);                           // 然后初始化
  Serial.println("Serial2 initialized on pins RX=" + String(HOST_SERIAL_RX) + ", TX=" + String(HOST_SERIAL_TX));

  // 初始化Serial0用于激光传感器通信
  Serial.println("Initializing Serial0 for laser sensor on pins RX=" + String(LASER_RX_PIN) + ", TX=" + String(LASER_TX_PIN));
  Serial0.setPins(LASER_RX_PIN, LASER_TX_PIN); // 明确设置Serial0引脚
  Serial0.begin(115200, SERIAL_8N1, LASER_RX_PIN, LASER_TX_PIN, false);
  delay(200); // 等待Serial0初始化
  Serial.println("Serial0 initialized for laser sensor");
  Serial.println("Note: Laser sensor uses Serial0 (RX=36, TX=35), Serial2 is for host communication");

  // 初始化底盘控制串口Serial1
  Serial1.begin(CHASSIS_BAUD_RATE, SERIAL_8N1, -1, CHASSIS_SERIAL_TX);
  Serial.println("Chassis control serial initialized on TX pin " + String(CHASSIS_SERIAL_TX));
  delay(100);

  // 测试Serial2输出
  Serial.println("Sending test message to Serial2...");
  Serial2.println("=== Serial2 Test ===");
  Serial2.println("If you see this message, Serial2 is working!");
  Serial2.println("===================");
  Serial.println("Serial2 test message sent");

  // 等待一下让消息发送完成
  delay(100);

  grayTracker.begin();
  Serial.println("Grayscale sensors initialized on pins: " + String(GRAY_SENSOR_R2) + "," + String(GRAY_SENSOR_R1) + "," + String(GRAY_SENSOR_M) + "," + String(GRAY_SENSOR_L1) + "," + String(GRAY_SENSOR_L2));

  // 激光传感器初始化
  Serial.println("Initializing laser sensor on pins RX=" + String(LASER_RX_PIN) + ", TX=" + String(LASER_TX_PIN));
  if (laser_init())
  {
    Serial.println("Laser sensor initialized successfully");
  }
  else
  {
    Serial.println("Laser sensor initialization failed!");
    // 尝试直接通过Serial0初始化
    Serial0.end();
    delay(100);
    Serial0.begin(115200, SERIAL_8N1, LASER_RX_PIN, LASER_TX_PIN, false);
    delay(200);
    Serial.println("Attempted direct Serial0 initialization");
  }

  // laser2_init(); // 移除第二个激光传感器初始化
  initServos(90, 90, 90, false);
  initSteppers();              // 初始化步进电机
  chassis_serial_init();       // 初始化底盘控制
  initChassisSimpleAbsolute(); // 初始化简化底盘绝对位置控制系统
  initStepperAbsolute();       // 初始化步进电机绝对位置控制系统
  debugPrintAll("Ready. Type 'help' for command list.");
  debugPrintAll("Serial ports: USB(Serial), Host(Serial2 RX=21 TX=20), Laser(Serial0 RX=36 TX=35), Chassis(Serial1 TX=19)");
}

void handleCommand(String cmd)
{
  cmd.trim();
  cmd.toLowerCase();
  if (cmd == "help")
  {
    debugPrintAll("=== Command List ===");
    debugPrintAll("help                  Show this help");
    debugPrintAll("debug                 Show debug information");
    debugPrintAll("gray                  Toggle continuous grayscale sensor output");
    debugPrintAll("gray_raw              Show raw grayscale sensor values");
    debugPrintAll("laser                 Toggle continuous laser distance output");
    debugPrintAll("servo,N,A             Control servo N (1-3) to angle A (0-360)");
    debugPrintAll("stepper,N,S,V         Control stepper motor N, S steps at speed V");
    debugPrintAll("stepper_enable,N      Enable stepper motor N");
    debugPrintAll("stepper_disable,N     Disable stepper motor N");
    debugPrintAll("stepper_status        Show stepper motors status");
    debugPrintAll("stepper_test          Test stepper motor speeds");
    debugPrintAll("chassis,D,S           Simple chassis control, D=direction, S=speed");
    debugPrintAll("chassis_adv,D,L,S     Advanced chassis control, D=direction, L=distance, S=speed");
    debugPrintAll("chassis_abs,Y,S       Move to absolute Y position at speed S");
    debugPrintAll("chassis_pos           Show current chassis Y position");
    debugPrintAll("chassis_reset,Y       Reset chassis Y position");
    debugPrintAll("stepper_abs,M,P,S,A   Move stepper M to absolute position P at speed S (auto-enable)");
    debugPrintAll("stepper_pos,M         Show stepper M position (or 'all' for all)");
    debugPrintAll("stepper_reset,M,P     Reset stepper M position to P");
    debugPrintAll("chassis_test          Test basic chassis movement");
    debugPrintAll("chassis_raw,CMD       Send raw command to chassis");
    debugPrintAll("chassis_baud,RATE     Change chassis serial baud rate");
    debugPrintAll("chassis_simple        Test basic chassis communication");
    debugPrintAll("chassis_pin_test      Test chassis control pin connection");
    debugPrintAll("chassis_uart,波特率,配置,信号反转 重新配置底盘串口");
    debugPrintAll("chassis_dir_test      Test chassis movement in all directions");
    debugPrintAll("=== End of List ===");
    return;
  }
  if (cmd == "exit")
  {
    continuousGray = false;
    continuousLaser = false;
    debugPrintAll("Exited continuous output mode");
    return;
  }
  if (cmd == "gray")
  {
    int values[5];
    grayTracker.update(); // 先更新传感器数据
    grayTracker.getSensorValues(values);
    debugPrintAll("GRAY: " + String(values[0]) + "," + String(values[1]) + "," + String(values[2]) + "," + String(values[3]) + "," + String(values[4]));
    // 开启持续输出模式
    continuousGray = true;
    continuousLaser = false;
    debugPrintAll("Grayscale sensor continuous mode enabled, type 'exit' to stop");
    return;
  }
  if (cmd == "laser")
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
    // 开启持续输出模式
    continuousLaser = true;
    continuousGray = false;
    debugPrintAll("Laser sensor continuous mode enabled (optimized for motion), type 'exit' to stop");
    return;
  }
  if (cmd == "laser_raw")
  {
    int dist = get_laser_distance(true, false);
    if (dist > 0)
    {
      debugPrintAll("LASER_RAW: " + String(dist) + " mm");
    }
    else
    {
      debugPrintAll("LASER_RAW: Out of range");
    }
    return;
  }
  if (cmd == "laser_info")
  {
    debugPrintAll("=== Laser Sensor Information ===");
    debugPrintAll("Model: ATK-MS53L2M Laser Distance Sensor");
    debugPrintAll("Measurement range: 20mm - 400mm (typical)");
    debugPrintAll("Resolution: 1mm");
    debugPrintAll("Accuracy: ±1% (typical)");
    debugPrintAll("Update rate: 10Hz (optimized)");
    debugPrintAll("Communication: Serial0 (RX=36, TX=35) at 115200 baud");
    debugPrintAll("Note: Readings may become unreliable beyond 350-400mm");
    debugPrintAll("Note: Readings are affected by target surface color and texture");
    debugPrintAll("===============================");
    return;
  }
  if (cmd == "laser_test")
  {
    // 测试激光传感器初始化
    debugPrintAll("Testing laser sensor...");
    debugPrintAll("Laser pins: RX=" + String(LASER_RX_PIN) + ", TX=" + String(LASER_TX_PIN));

    // 测试Serial0是否可用
    if (Serial0)
    {
      debugPrintAll("Serial0 is available");
    }
    else
    {
      debugPrintAll("Serial0 is not available");
      // 尝试重新初始化Serial0
      Serial0.end();
      delay(100);
      Serial0.begin(115200, SERIAL_8N1, LASER_RX_PIN, LASER_TX_PIN, false);
      delay(200);
      debugPrintAll("Attempted to reinitialize Serial0");

      if (Serial0)
      {
        debugPrintAll("Serial0 is now available after reinitialization");
      }
      else
      {
        debugPrintAll("Serial0 is still not available after reinitialization");
      }
    }

    // 尝试重新初始化激光传感器
    if (laser_init())
    {
      debugPrintAll("Laser sensor re-initialized successfully");
    }
    else
    {
      debugPrintAll("Laser sensor re-initialization failed");
    }

    // 测试读取
    int dist = get_laser_distance(true, true);
    debugPrintAll("Test read result: " + String(dist) + " mm");
    return;
  }
  if (cmd == "laser_debug")
  {
    // 详细调试激光传感器
    debugPrintAll("=== Laser Debug Mode ===");

    // 确保Serial0初始化
    if (!Serial0)
    {
      debugPrintAll("Serial0 not initialized, attempting to initialize...");
      Serial0.end();
      delay(100);
      Serial0.begin(115200, SERIAL_8N1, LASER_RX_PIN, LASER_TX_PIN, false);
      delay(200);
    }

    // 直接测试Serial0通信
    debugPrintAll("Sending test commands to Serial0...");
    Serial0.flush();      // 确保之前的数据已发送
    Serial0.println("F"); // 单次测量命令
    delay(100);

    // 读取响应
    String response = "";
    unsigned long startTime = millis();
    while (millis() - startTime < 500)
    {
      if (Serial0.available())
      {
        char c = Serial0.read();
        response += c;
        if (c == '\n' || c == '\r')
          break;
      }
    }

    debugPrintAll("Serial0 response: [" + response + "]");
    debugPrintAll("Response length: " + String(response.length()));

    // 测试不同的命令
    const char *commands[] = {"F\r\n", "C\r\n", "D\r\n", "GETDATA\r\n"};
    for (int i = 0; i < 4; i++)
    {
      debugPrintAll("Testing command: [" + String(commands[i]) + "]");
      Serial0.print(commands[i]);
      Serial0.flush(); // 确保命令发送完成
      delay(200);

      response = "";
      startTime = millis();
      while (millis() - startTime < 300)
      {
        if (Serial0.available())
        {
          char c = Serial0.read();
          response += c;
        }
      }
      debugPrintAll("Response: [" + response + "]");
    }

    // 尝试不同波特率
    debugPrintAll("Testing different baud rates...");
    uint32_t baudrates[] = {9600, 19200, 38400, 57600, 115200};
    for (int i = 0; i < 5; i++)
    {
      debugPrintAll("Testing baud rate: " + String(baudrates[i]));
      Serial0.end();
      delay(100);
      Serial0.begin(baudrates[i], SERIAL_8N1, LASER_RX_PIN, LASER_TX_PIN, false);
      delay(200);
      Serial0.println("F");
      delay(200);

      response = "";
      startTime = millis();
      while (millis() - startTime < 300)
      {
        if (Serial0.available())
        {
          char c = Serial0.read();
          response += c;
        }
      }
      debugPrintAll("Response at " + String(baudrates[i]) + " baud: [" + response + "]");
    }

    // 恢复默认波特率
    Serial0.end();
    delay(100);
    Serial0.begin(115200, SERIAL_8N1, LASER_RX_PIN, LASER_TX_PIN, false);
    delay(200);

    debugPrintAll("=== Debug Complete ===");
    return;
  }
  if (cmd == "gray_raw")
  {
    // 直接读取原始模拟值
    int r2 = analogRead(GRAY_SENSOR_R2);
    int r1 = analogRead(GRAY_SENSOR_R1);
    int m = analogRead(GRAY_SENSOR_M);
    int l1 = analogRead(GRAY_SENSOR_L1);
    int l2 = analogRead(GRAY_SENSOR_L2);
    debugPrintAll("GRAY_RAW: " + String(r2) + "," + String(r1) + "," + String(m) + "," + String(l1) + "," + String(l2));
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
      if (n == 1)
        servo1(a);
      else if (n == 2)
        servo2(a);
      else if (n == 3)
        servo3(a);
      debugPrintAll("SERVO" + String(n) + ": " + String(a));
    }
    else
    {
      debugPrintAll("Format error, should be: servo,N,A");
    }
    return;
  }
  if (cmd.startsWith("stepper_enable,"))
  {
    int motor = cmd.substring(15).toInt();
    setStepperEnable(motor, true);
    return;
  }
  if (cmd.startsWith("stepper_disable,"))
  {
    int motor = cmd.substring(16).toInt();
    setStepperEnable(motor, false);
    return;
  }
  if (cmd == "stepper_status")
  {
    showStepperStatus();
    return;
  }
  if (cmd == "stepper_test")
  {
    // 步进电机速度测试
    debugPrintAll("Testing stepper motor speeds...");

    // 测试不同速度
    int testSpeeds[] = {50, 100, 150, 180, 200};
    int testSteps = 200;

    for (int i = 0; i < 5; i++)
    {
      debugPrintAll("Testing speed " + String(testSpeeds[i]));
      controlStepper(1, testSteps, testSpeeds[i]);
      delay(500); // 等待电机停止
    }

    debugPrintAll("Speed test completed");
    return;
  }
  if (cmd.startsWith("stepper_speed_test"))
  {
    Serial.println("Starting stepper speed test...");
    Serial2.println("Starting stepper speed test...");
    for (int speed = 50; speed <= 300; speed += 50)
    {
      Serial.printf("Testing speed: %d\n", speed);
      Serial2.printf("Testing speed: %d\n", speed);
      controlStepper(0, 100, speed); // Test motor 0
      delay(1000);                   // Wait 1 second to observe if steps are missed
      // stepperStop(0); // This function doesn't exist, so we'll just delay
      Serial.printf("Speed %d test complete. Check for missed steps.\n", speed);
      Serial2.printf("Speed %d test complete. Check for missed steps.\n", speed);
      delay(2000); // Wait 2 seconds before next speed test
    }
    Serial.println("Stepper speed test completed.");
    Serial2.println("Stepper speed test completed.");
  }
  if (cmd.startsWith("stepper,"))
  {
    // 解析 stepper,N,S,V 格式
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);
    int thirdComma = cmd.indexOf(',', secondComma + 1);

    if (firstComma > 0 && secondComma > firstComma && thirdComma > secondComma)
    {
      int motor = cmd.substring(firstComma + 1, secondComma).toInt();
      int steps = cmd.substring(secondComma + 1, thirdComma).toInt();
      int speed = cmd.substring(thirdComma + 1).toInt();

      // 添加调试信息
      debugPrintAll("Parsed: motor=" + String(motor) + ", steps=" + String(steps) + ", speed=" + String(speed));

      controlStepper(motor, steps, speed);

      // 更新步进电机位置跟踪
      updateStepperPosition(motor, steps);
    }
    else
    {
      debugPrintAll("Format error, should be: stepper,N,S,V");
      debugPrintAll("Example: stepper,2,100,100");
    }
    return;
  }
  if (cmd.startsWith("chassis,"))
  {
    int firstComma = cmd.indexOf(',');
    if (firstComma > 0)
    {
      int direction = cmd.substring(8, firstComma).toInt();
      int speed = cmd.substring(firstComma + 1).toInt();
      move_chassis(direction, speed);
    }
    else
    {
      debugPrintAll("Format error, should be: chassis,D,S");
    }
    return;
  }
  if (cmd.startsWith("chassis_adv,"))
  {
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);
    int thirdComma = cmd.indexOf(',', secondComma + 1);
    if (firstComma > 0 && secondComma > firstComma)
    {
      int direction = cmd.substring(firstComma + 1, secondComma).toInt();
      int distance = cmd.substring(secondComma + 1, thirdComma > 0 ? thirdComma : cmd.length()).toInt();
      int speed = thirdComma > 0 ? cmd.substring(thirdComma + 1).toInt() : 50; // 默认速度50

      // 添加调试信息
      debugPrintAll("Chassis ADV command parsed: Direction=" + String(direction) +
                    ", Distance=" + String(distance) +
                    ", Speed=" + String(speed));

      // 直接使用方向值，不再映射
      uint8_t mappedDirection;
      switch (direction)
      {
      case 1: // 前进
      case 5:
        mappedDirection = FORWARD;
        debugPrintAll("Direction mapped to FORWARD (" + String(FORWARD) + ")");
        break;
      case 2: // 后退
      case 6:
        mappedDirection = BACKWARD;
        debugPrintAll("Direction mapped to BACKWARD (" + String(BACKWARD) + ")");
        break;
      case 3: // 左转
      case 8:
        mappedDirection = LEFT;
        debugPrintAll("Direction mapped to LEFT (" + String(LEFT) + ")");
        break;
      case 4: // 右转
      case 7:
        mappedDirection = RIGHT;
        debugPrintAll("Direction mapped to RIGHT (" + String(RIGHT) + ")");
        break;
      case 0: // 停止
      default:
        mappedDirection = STOP;
        debugPrintAll("Direction mapped to STOP (" + String(STOP) + ")");
        break;
      }

      // 输出枚举值和对应的命令值
      debugPrintAll("FORWARD=" + String(FORWARD) + ", BACKWARD=" + String(BACKWARD) +
                    ", LEFT=" + String(LEFT) + ", RIGHT=" + String(RIGHT) + ", STOP=" + String(STOP));
      debugPrintAll("CHASSIS_FORWARD=" + String(CHASSIS_FORWARD) + ", CHASSIS_BACKWARD=" + String(CHASSIS_BACKWARD) +
                    ", CHASSIS_LEFT=" + String(CHASSIS_LEFT) + ", CHASSIS_RIGHT=" + String(CHASSIS_RIGHT) +
                    ", CHASSIS_STOP=" + String(CHASSIS_STOP));

      // 添加一个直接使用control_chassis_raw的调试信息
      debugPrintAll("使用control_chassis_raw直接发送命令");

      // 将枚举方向转换为实际命令值
      uint8_t actual_command;
      switch (mappedDirection)
      {
      case FORWARD:
        actual_command = CHASSIS_FORWARD;
        break;
      case BACKWARD:
        actual_command = CHASSIS_BACKWARD;
        break;
      case LEFT:
        actual_command = CHASSIS_LEFT;
        break;
      case RIGHT:
        actual_command = CHASSIS_RIGHT;
        break;
      case STOP:
      default:
        actual_command = CHASSIS_STOP;
        break;
      }

      // 直接使用control_chassis_raw
      control_chassis_raw(actual_command, distance, speed);

      // 更新简化底盘位置跟踪
      updateChassisPosition(actual_command, distance);

      // 添加确认信息
      debugPrintAll("Chassis ADV command sent using raw format");
      debugPrintAll("Position updated: " + getChassisPositionString());
    }
    else
    {
      debugPrintAll("Format error, should be: chassis_adv,D,L,S");
      debugPrintAll("Example: chassis_adv,1,50,50 (Forward 50cm at speed 50)");
      debugPrintAll("Directions: 1/5=Forward, 2/6=Backward, 3/8=Left, 4/7=Right, 0=Stop");
    }
    return;
  }

  // 简化底盘绝对位置移动命令 (仅Y轴)
  if (cmd.startsWith("chassis_abs,"))
  {
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);

    if (firstComma > 0)
    {
      long targetY = cmd.substring(firstComma + 1, secondComma > 0 ? secondComma : cmd.length()).toInt();
      uint8_t speed = secondComma > 0 ? cmd.substring(secondComma + 1).toInt() : CHASSIS_DEFAULT_SPEED;

      debugPrintAll("Chassis absolute Y position command parsed:");
      debugPrintAll("  Target Y: " + String(targetY));
      debugPrintAll("  Speed: " + String(speed));

      if (moveChassisToAbsoluteY(targetY, speed))
      {
        debugPrintAll("Chassis absolute Y movement initiated successfully");
      }
      else
      {
        debugPrintAll("Failed to initiate chassis absolute Y movement");
      }
    }
    else
    {
      debugPrintAll("Format error, should be: chassis_abs,Y,S");
      debugPrintAll("Example: chassis_abs,100000,50 (Move to Y=100000 at speed 50)");
      debugPrintAll("Or: chassis_abs,100000 (Move to Y=100000 at default speed)");
    }
    return;
  }

  // 显示当前底盘位置命令
  if (cmd == "chassis_pos")
  {
    debugPrintAll("=== Current Chassis Position ===");
    debugPrintAll(getChassisPositionString());
    debugPrintAll("Last update: " + String(millis() - chassisPosition.lastUpdateTime) + "ms ago");
    return;
  }

  // 重置底盘位置命令
  if (cmd.startsWith("chassis_reset"))
  {
    if (cmd == "chassis_reset")
    {
      // 重置到原点
      resetChassisPosition(0);
      debugPrintAll("Chassis Y position reset to origin (0)");
    }
    else
    {
      // 解析参数
      int firstComma = cmd.indexOf(',');
      if (firstComma > 0)
      {
        long yPos = cmd.substring(firstComma + 1).toInt();
        resetChassisPosition(yPos);
        debugPrintAll("Chassis Y position reset completed");
      }
      else
      {
        debugPrintAll("Format error, should be: chassis_reset,Y");
        debugPrintAll("Example: chassis_reset,100000 (Reset to Y=100000)");
        debugPrintAll("Or simply: chassis_reset (Reset to Y=0)");
      }
    }
    return;
  }

  // 步进电机绝对位置移动命令
  if (cmd.startsWith("stepper_abs,"))
  {
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);
    int thirdComma = cmd.indexOf(',', secondComma + 1);
    int fourthComma = cmd.indexOf(',', thirdComma + 1);

    if (firstComma > 0 && secondComma > firstComma && thirdComma > secondComma)
    {
      int motor = cmd.substring(firstComma + 1, secondComma).toInt();
      long targetPosition = cmd.substring(secondComma + 1, thirdComma).toInt();
      int maxSpeed = cmd.substring(thirdComma + 1, fourthComma > 0 ? fourthComma : cmd.length()).toInt();
      int acceleration = fourthComma > 0 ? cmd.substring(fourthComma + 1).toInt() : STEPPER_DEFAULT_ACCELERATION;

      debugPrintAll("Stepper absolute position command parsed:");
      debugPrintAll("  Motor: " + String(motor));
      debugPrintAll("  Target Position: " + String(targetPosition));
      debugPrintAll("  Max Speed: " + String(maxSpeed));
      debugPrintAll("  Acceleration: " + String(acceleration));

      if (moveStepperToAbsolutePosition(motor, targetPosition, maxSpeed, acceleration))
      {
        debugPrintAll("Stepper absolute movement initiated successfully");
      }
      else
      {
        debugPrintAll("Failed to initiate stepper absolute movement");
      }
    }
    else
    {
      debugPrintAll("Format error, should be: stepper_abs,M,P,S,A");
      debugPrintAll("Example: stepper_abs,1,1000,100,50 (Motor 1 to position 1000 at speed 100)");
      debugPrintAll("M=Motor(1-4), P=Position, S=Speed(1-200), A=Acceleration(optional)");
    }
    return;
  }

  // 显示步进电机位置命令
  if (cmd.startsWith("stepper_pos"))
  {
    if (cmd == "stepper_pos,all" || cmd == "stepper_pos all")
    {
      debugPrintAll(getAllStepperPositionsString());
    }
    else
    {
      int firstComma = cmd.indexOf(',');
      if (firstComma > 0)
      {
        int motor = cmd.substring(firstComma + 1).toInt();
        debugPrintAll("=== Stepper Position ===");
        debugPrintAll(getStepperPositionString(motor));
        debugPrintAll("Last update: " + String(millis() - stepperAbsStates[motor - 1].lastUpdateTime) + "ms ago");
      }
      else
      {
        debugPrintAll("Format error, should be: stepper_pos,M or stepper_pos,all");
        debugPrintAll("Example: stepper_pos,1 (Show motor 1 position)");
        debugPrintAll("Example: stepper_pos,all (Show all motor positions)");
      }
    }
    return;
  }

  // 重置步进电机位置命令
  if (cmd.startsWith("stepper_reset,"))
  {
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);

    if (firstComma > 0)
    {
      int motor = cmd.substring(firstComma + 1, secondComma > 0 ? secondComma : cmd.length()).toInt();
      long position = secondComma > 0 ? cmd.substring(secondComma + 1).toInt() : 0;

      resetStepperPosition(motor, position);
      debugPrintAll("Stepper position reset completed");
    }
    else
    {
      debugPrintAll("Format error, should be: stepper_reset,M,P");
      debugPrintAll("Example: stepper_reset,1,0 (Reset motor 1 to position 0)");
      debugPrintAll("Example: stepper_reset,1 (Reset motor 1 to position 0)");
    }
    return;
  }

  if (cmd == "chassis_test")
  {
    debugPrintAll("Testing basic chassis movement...");

    // 测试前进
    debugPrintAll("Testing FORWARD movement...");
    control_chassis_raw(CHASSIS_FORWARD, 50, 10);
    delay(2000);

    // 测试后退
    debugPrintAll("Testing BACKWARD movement...");
    control_chassis_raw(CHASSIS_BACKWARD, 50, 10);
    delay(2000);

    // 测试左转
    debugPrintAll("Testing LEFT turn...");
    control_chassis_raw(CHASSIS_LEFT, 50, 10);
    delay(2000);

    // 测试右转
    debugPrintAll("Testing RIGHT turn...");
    control_chassis_raw(CHASSIS_RIGHT, 50, 10);
    delay(2000);

    // 测试停止
    debugPrintAll("Testing STOP...");
    control_chassis_raw(CHASSIS_STOP, 0, 0);

    debugPrintAll("Chassis test completed");
    return;
  }
  if (cmd.startsWith("chassis_raw,"))
  {
    int firstComma = cmd.indexOf(',');
    if (firstComma > 0)
    {
      String rawCommand = cmd.substring(firstComma + 1);
      debugPrintAll("Sending raw command to chassis: " + rawCommand);

      // 直接发送原始命令到底盘串口
      Serial1.println(rawCommand);

      debugPrintAll("Raw command sent");
    }
    else
    {
      debugPrintAll("Format error, should be: chassis_raw,COMMAND");
      debugPrintAll("Example: chassis_raw,CH5100 (Forward at speed 100)");
    }
    return;
  }
  if (cmd.startsWith("chassis_baud,"))
  {
    int firstComma = cmd.indexOf(',');
    if (firstComma > 0)
    {
      int baudRate = cmd.substring(firstComma + 1).toInt();
      debugPrintAll("Changing chassis serial baud rate to: " + String(baudRate));

      // 重新初始化底盘串口
      Serial1.end();
      delay(100);
      Serial1.begin(baudRate, SERIAL_8N1, -1, CHASSIS_SERIAL_TX);
      delay(100);

      debugPrintAll("Chassis serial baud rate changed, sending test command...");

      // 发送测试命令
      Serial1.println("CH5100");
      delay(500);
      Serial1.println("5");

      debugPrintAll("Test commands sent with new baud rate: " + String(baudRate));
    }
    else
    {
      debugPrintAll("Format error, should be: chassis_baud,RATE");
      debugPrintAll("Example: chassis_baud,9600");
      debugPrintAll("Common baud rates: 9600, 19200, 38400, 57600, 115200");
    }
    return;
  }
  if (cmd == "chassis_simple")
  {
    debugPrintAll("发送简单字符测试底盘通信...");

    // 发送单个数字
    debugPrintAll("发送: 5");
    Serial1.write('5');
    delay(500);

    // 发送字符串
    debugPrintAll("发送: FORWARD");
    Serial1.print("FORWARD");
    delay(500);

    // 发送带结束符的字符串
    debugPrintAll("发送: 5\\r\\n");
    Serial1.print("5\r\n");
    delay(500);

    // 发送二进制数据
    debugPrintAll("发送二进制: 0x05 0x64 0x00");
    uint8_t data[] = {0x05, 0x64, 0x00};
    Serial1.write(data, 3);

    debugPrintAll("简单通信测试完成");
    return;
  }

  if (cmd == "chassis_pin_test")
  {
    debugPrintAll("开始测试底盘控制引脚...");

    // 获取当前引脚模式
    int pinState = digitalRead(CHASSIS_SERIAL_TX);
    debugPrintAll("引脚" + String(CHASSIS_SERIAL_TX) + "当前状态: " + String(pinState));

    // 临时将引脚设置为输出模式
    debugPrintAll("将引脚" + String(CHASSIS_SERIAL_TX) + "设置为输出模式");
    pinMode(CHASSIS_SERIAL_TX, OUTPUT);

    // 设置引脚为高电平
    debugPrintAll("将引脚" + String(CHASSIS_SERIAL_TX) + "设置为高电平");
    digitalWrite(CHASSIS_SERIAL_TX, HIGH);
    delay(1000);

    // 设置引脚为低电平
    debugPrintAll("将引脚" + String(CHASSIS_SERIAL_TX) + "设置为低电平");
    digitalWrite(CHASSIS_SERIAL_TX, LOW);
    delay(1000);

    // 设置引脚为高电平
    debugPrintAll("将引脚" + String(CHASSIS_SERIAL_TX) + "设置为高电平");
    digitalWrite(CHASSIS_SERIAL_TX, HIGH);
    delay(1000);

    // 恢复串口功能
    debugPrintAll("恢复串口功能");
    Serial1.end();
    delay(100);
    Serial1.begin(CHASSIS_BAUD_RATE, SERIAL_8N1, -1, CHASSIS_SERIAL_TX);

    debugPrintAll("引脚测试完成，串口已恢复");
    return;
  }
  if (cmd.startsWith("chassis_uart,"))
  {
    int firstComma = cmd.indexOf(',');
    int secondComma = cmd.indexOf(',', firstComma + 1);
    int thirdComma = cmd.indexOf(',', secondComma + 1);

    if (firstComma > 0 && secondComma > firstComma && thirdComma > secondComma)
    {
      int baudRate = cmd.substring(firstComma + 1, secondComma).toInt();
      int config = cmd.substring(secondComma + 1, thirdComma).toInt();
      int inverted = cmd.substring(thirdComma + 1).toInt();

      debugPrintAll("重新配置底盘串口...");
      debugPrintAll("波特率: " + String(baudRate));
      debugPrintAll("配置: " + String(config) + " (0=SERIAL_8N1, 1=SERIAL_8E1, 2=SERIAL_8O1)");
      debugPrintAll("信号反转: " + String(inverted) + " (0=正常, 1=反转)");

      // 关闭当前串口
      Serial1.end();
      delay(100);

      // 重新配置串口
      switch (config)
      {
      case 1:
        if (inverted)
        {
          Serial1.begin(baudRate, SERIAL_8E1, -1, CHASSIS_SERIAL_TX, true);
        }
        else
        {
          Serial1.begin(baudRate, SERIAL_8E1, -1, CHASSIS_SERIAL_TX, false);
        }
        break;
      case 2:
        if (inverted)
        {
          Serial1.begin(baudRate, SERIAL_8O1, -1, CHASSIS_SERIAL_TX, true);
        }
        else
        {
          Serial1.begin(baudRate, SERIAL_8O1, -1, CHASSIS_SERIAL_TX, false);
        }
        break;
      default:
        if (inverted)
        {
          Serial1.begin(baudRate, SERIAL_8N1, -1, CHASSIS_SERIAL_TX, true);
        }
        else
        {
          Serial1.begin(baudRate, SERIAL_8N1, -1, CHASSIS_SERIAL_TX, false);
        }
        break;
      }

      delay(100);

      // 发送测试命令
      debugPrintAll("发送测试命令: CH5100");
      Serial1.println("CH5100");

      debugPrintAll("串口重新配置完成");
    }
    else
    {
      debugPrintAll("格式错误，应为: chassis_uart,波特率,配置,信号反转");
      debugPrintAll("例如: chassis_uart,115200,0,0");
      debugPrintAll("配置: 0=SERIAL_8N1, 1=SERIAL_8E1, 2=SERIAL_8O1");
      debugPrintAll("信号反转: 0=正常, 1=反转");
    }
    return;
  }
  if (cmd == "chassis_dir_test")
  {
    debugPrintAll("测试各个方向的底盘移动...");

    // 测试前进
    debugPrintAll("测试前进 (CHASSIS_FORWARD=5)");
    Serial1.println("CH5500");
    delay(1000);
    Serial1.println("CH00");
    delay(500);

    // 测试后退
    debugPrintAll("测试后退 (CHASSIS_BACKWARD=6)");
    Serial1.println("CH6500");
    delay(1000);
    Serial1.println("CH00");
    delay(500);

    // 测试左转
    debugPrintAll("测试左转 (CHASSIS_LEFT=8)");
    Serial1.println("CH8500");
    delay(1000);
    Serial1.println("CH00");
    delay(500);

    // 测试右转
    debugPrintAll("测试右转 (CHASSIS_RIGHT=7)");
    Serial1.println("CH7500");
    delay(1000);
    Serial1.println("CH00");
    delay(500);

    debugPrintAll("方向测试完成");
    return;
  }
  debugPrintAll("Unknown command, type 'help' for command list.");
}

void loop()
{
  // 处理串口命令
  if (Serial.available())
  {
    String cmd = Serial.readStringUntil('\n');
    Serial.println("USB Serial received: " + cmd);
    handleCommand(cmd);
  }
  if (Serial2.available())
  {
    String cmd = Serial2.readStringUntil('\n');
    Serial.println("Serial2 received: " + cmd); // 调试信息
    handleCommand(cmd);
  }

  // 持续输出模式
  unsigned long currentTime = millis();
  if (currentTime - lastOutputTime >= outputInterval)
  {
    lastOutputTime = currentTime;

    if (continuousGray)
    {
      int values[5];
      grayTracker.update(); // 先更新传感器数据
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
}