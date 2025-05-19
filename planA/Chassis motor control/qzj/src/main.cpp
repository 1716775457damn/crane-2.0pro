#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// 定义连接到L298N的引脚
const int motorAin1 = 17; // 电机A的输入1
const int motorAin2 = 16; // 电机A的输入2
const int motorBin1 = 5;  // 电机B的输入1
const int motorBin2 = 18; // 电机B的输入2
const int enA = 19;       // 电机A的使能引脚
const int enB = 21;       // 电机B的使能引脚

// 初始化电机控制函数
void setupMotorPins()
{
  pinMode(motorAin1, OUTPUT);
  pinMode(motorAin2, OUTPUT);
  pinMode(motorBin1, OUTPUT);
  pinMode(motorBin2, OUTPUT);
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);

  digitalWrite(enA, LOW); // 默认使ENA处于启用状态
  digitalWrite(enB, LOW); // 默认使ENB处于启用状态
}

// 函数声明
void setMotorSpeed(int motorPin1, int motorPin2, int speed);
void stopMotor(int motorPin1, int motorPin2);
void enableMotor(int enPin);
void disableMotor(int enPin);
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();

// setup 和 loop 函数保持不变

// 启用电机
void enableMotor(int enPin)
{
  digitalWrite(enPin, HIGH);
}

// 禁用电机
void disableMotor(int enPin)
{
  digitalWrite(enPin, LOW);
}

// 设置电机速度（正数向前，负数向后）
void setMotorSpeed(int motorPin1, int motorPin2, int speed)
{
  if (speed > 0)
  {
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
  }
  else if (speed < 0)
  {
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
  }
  else
  {
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
  }
  analogWrite(enA, abs(speed));
  analogWrite(enB, abs(speed));
}

// 停止电机
void stopMotor(int motorPin1, int motorPin2)
{
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  analogWrite(enA, 0);
  analogWrite(enB, 0);
}

// 小车前进
void moveForward()
{
  setMotorSpeed(motorAin1, motorAin2, 150); // 电机A向前
  setMotorSpeed(motorBin1, motorBin2, 150); // 电机B向前
}

// 小车后退
void moveBackward()
{
  setMotorSpeed(motorAin1, motorAin2, -150); // 电机A向后
  setMotorSpeed(motorBin1, motorBin2, -150); // 电机B向后
}

// 小车左转
void turnLeft()
{
  setMotorSpeed(motorAin1, motorAin2, -150); // 电机A向后
  setMotorSpeed(motorBin1, motorBin2, 150);  // 电机B向前
}

// 小车右转
void turnRight()
{
  setMotorSpeed(motorAin1, motorAin2, 150);  // 电机A向前
  setMotorSpeed(motorBin1, motorBin2, -150); // 电机B向后
}


// 接收信息的web server 监听80端口
WebServer server(80);

unsigned long timeNow = 0;
unsigned long lastDataTickTime = 0;

int LED_BUILTIN = 2;
bool ledShow = false;
int ledLoopTick = -1;

void handleRoot()
{
  String c = server.arg("c");
  // Serial.println(c.c_str());
  float speed, turn;
  sscanf(c.c_str(), "c:%f,%f", &speed, &turn);
  Serial.println("speed: " + String(speed) + " turn: " + String(turn));
  if(speed > turn)
  {
    if (speed > 10)
    {
      moveForward();
    }
    else if (speed < -10)
    {
      moveBackward();
    }
    else
    {
      stopMotor(motorAin1, motorAin2);
      stopMotor(motorBin1, motorBin2);
    }
  }
  else if(speed < turn)
  {
    if (turn > 10)
    {
      turnRight();
    }
    else if (turn < -10)
    {
      turnLeft();
    }
    else
    {
      stopMotor(motorAin1, motorAin2);
      stopMotor(motorBin1, motorBin2);
    }
  }
  else
  {
    stopMotor(motorAin1, motorAin2);
    stopMotor(motorBin1, motorBin2);
  }
  lastDataTickTime = millis();
  server.send(200, "text/plain", "success");
}

void registerEvent()
{
  server.on("/", handleRoot);

  server.enableCORS();
  server.begin();
  Serial.println("HTTP server started");
}

void setup()
{
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LOW);

  setupMotorPins();

  Serial.println();
  Serial.print("Configuring access point...");
  WiFi.softAP("txw", "twx20051");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  if (MDNS.begin("esp32"))
  {
    Serial.println("MDNS responder started");
  }

  registerEvent();
}

void loop()
{
  server.handleClient();
  timeNow = millis();

  if (timeNow > lastDataTickTime && timeNow - lastDataTickTime > 1000)
  {
    // 超过1秒未收到数据，自动停止，开始闪灯
    stopMotor(motorAin1, motorAin2);
    stopMotor(motorBin1, motorBin2);

    ledLoopTick += 1;
    if (ledLoopTick >= 50)
    { // 闪太快看不清，隔50帧闪一次
      ledLoopTick = 0;
    }
    if (ledLoopTick == 0)
    {
      if (ledShow)
      {
        digitalWrite(LED_BUILTIN, LOW);
        ledShow = false;
      }
      else
      {
        digitalWrite(LED_BUILTIN, HIGH);
        ledShow = true;
      }
    }
  }
  else
  {
    // 有数据，就常亮
    digitalWrite(LED_BUILTIN, HIGH);
    ledShow = true;
  }
}