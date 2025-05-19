#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// OLED显示屏设置
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C  // 或尝试 0x3D
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 舵机设置
#define SERVO_X_PIN 25  // ESP32S3的GPIO25引脚连接X轴舵机
#define SERVO_Y_PIN 26  // ESP32S3的GPIO26引脚连接Y轴舵机

// 摄像头视野中心点
#define CENTER_X 112    // 224/2
#define CENTER_Y 112    // 224/2

// PID控制参数
#define KP 0.1          // 比例系数
#define KI 0.01         // 积分系数
#define KD 0.05         // 微分系数

// 串口通信设置
#define SERIAL_BAUD 115200
#define K210_SERIAL Serial1
// 注意：K210代码实际使用的引脚是10(TX)和11(RX)
#define K210_RX_PIN 18  // ESP32S3的GPIO引脚，连接到K210的10号引脚(TX)
#define K210_TX_PIN 17  // ESP32S3的GPIO引脚，连接到K210的11号引脚(RX)

Servo servoX;
Servo servoY;

// PID控制变量
float errorX = 0, errorY = 0;
float prevErrorX = 0, prevErrorY = 0;
float integralX = 0, integralY = 0;
float derivativeX = 0, derivativeY = 0;

// 目标位置
int targetX = CENTER_X;
int targetY = CENTER_Y;

// 舵机当前角度
int servoXPos = 90;
int servoYPos = 90;

// 识别结果
int detectedNumber = -1;
bool objectDetected = false;
unsigned long lastDetectionTime = 0;

void setup() {
  // 初始化串口通信
  Serial.begin(SERIAL_BAUD);
  Serial.println("\n\n=== ESP32S3 Number Tracking System ===");
  
  // 初始化与K210的串口
  Serial.println("Configuring K210 UART...");
  Serial.printf("ESP32S3 RX pin: %d (connects to K210 TX pin 10)\n", K210_RX_PIN);
  Serial.printf("ESP32S3 TX pin: %d (connects to K210 RX pin 11)\n", K210_TX_PIN);
  Serial.printf("Baud: %d\n", SERIAL_BAUD);
  
  K210_SERIAL.begin(SERIAL_BAUD, SERIAL_8N1, K210_RX_PIN, K210_TX_PIN);
  Serial.println("K210 UART initialized");
  
  // 初始化I2C
  Wire.begin();
  Wire.setClock(400000); // 设置I2C时钟频率
  Serial.println("I2C initialized");
  
  // 等待OLED稳定
  delay(100);
  
  // 初始化OLED显示屏
  Serial.println("Initializing OLED display...");
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 init failed! Check connection and address."));
    // 尝试另一个常用地址
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      Serial.println(F("SSD1306 init failed at alternative address too!"));
      for(;;);
    } else {
      Serial.println(F("SSD1306 initialized at address 0x3D"));
    }
  } else {
    Serial.println(F("SSD1306 initialized at address 0x3C"));
  }
  
  // 清屏并设置初始显示
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Number Tracking System"));
  display.println(F("Starting..."));
  display.display();
  Serial.println("OLED display initialized");
  delay(1000);
  
  // 初始化舵机
  Serial.println("Initializing servos...");
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  servoX.setPeriodHertz(50);
  servoY.setPeriodHertz(50);
  servoX.attach(SERVO_X_PIN, 500, 2500);
  servoY.attach(SERVO_Y_PIN, 500, 2500);
  
  // 舵机归中
  servoX.write(servoXPos);
  servoY.write(servoYPos);
  Serial.println("Servos initialized and centered");
  
  Serial.println("System started, waiting for K210 data...");
  
  // 在OLED上显示准备就绪
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Number Tracking"));
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println(F("System ready"));
  display.setCursor(0, 32);
  display.println(F("Waiting for K210..."));
  display.display();
}

void loop() {
  // 持续处理K210发来的数据
  if (readDataFromK210()) {
    // 有数据时更新OLED和控制舵机
    updateDisplay();
    
    if (objectDetected) {
      trackObject();
      lastDetectionTime = millis();
    }
  }
  
  // 如果一段时间没有检测到物体，重置状态
  if (objectDetected && millis() - lastDetectionTime > 2000) {
    objectDetected = false;
    detectedNumber = -1;
    updateDisplay();
  }
  
  delay(10);  // 短暂延时
}

// 读取K210的数据，支持多种格式
bool readDataFromK210() {
  if (!K210_SERIAL.available()) {
    return false;  // 没有数据
  }
  
  String data = K210_SERIAL.readStringUntil('\n');
  // 清理字符串
  data.trim();
  
  // 打印原始数据
  Serial.print("K210 data: ");
  Serial.println(data);
  
  // 过滤无效数据
  if (data.length() < 3) {
    return false;
  }
  
  // 解析数据格式：NUMBER,X,Y,WIDTH,HEIGHT
  int commaIndex1 = data.indexOf(',');
  if (commaIndex1 > 0) {
    // 尝试解析数字
    String numberStr = data.substring(0, commaIndex1);
    detectedNumber = numberStr.toInt();
    
    int commaIndex2 = data.indexOf(',', commaIndex1 + 1);
    int commaIndex3 = data.indexOf(',', commaIndex2 + 1);
    int commaIndex4 = data.indexOf(',', commaIndex3 + 1);
    
    if (commaIndex2 > 0 && commaIndex3 > 0 && commaIndex4 > 0) {
      int x = data.substring(commaIndex1 + 1, commaIndex2).toInt();
      int y = data.substring(commaIndex2 + 1, commaIndex3).toInt();
      int width = data.substring(commaIndex3 + 1, commaIndex4).toInt();
      int height = data.substring(commaIndex4 + 1).toInt();
      
      // 计算目标中心点
      targetX = x + width / 2;
      targetY = y + height / 2;
      objectDetected = true;
      
      return true;
    }
  }
  
  return false;
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("Number Tracking"));
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  if (objectDetected) {
    display.setCursor(0, 16);
    display.print(F("Number: "));
    display.println(detectedNumber);
    
    display.setCursor(0, 28);
    display.print(F("Pos: X="));
    display.print(targetX);
    display.print(F(" Y="));
    display.println(targetY);
    
    display.setCursor(0, 40);
    display.print(F("Servo: X="));
    display.print(servoXPos);
    display.print(F(" Y="));
    display.println(servoYPos);
    
    // 绘制简单的位置指示图
    display.drawRect(90, 16, 38, 38, SSD1306_WHITE);
    int indicatorX = 90 + map(targetX, 0, 224, 0, 38);
    int indicatorY = 16 + map(targetY, 0, 224, 0, 38);
    display.fillCircle(constrain(indicatorX, 92, 126), constrain(indicatorY, 18, 52), 2, SSD1306_WHITE);
  } else {
    display.setCursor(0, 24);
    display.println(F("Waiting for detection..."));
  }
  
  display.display();
}

void trackObject() {
  // 计算误差
  errorX = targetX - CENTER_X;
  errorY = targetY - CENTER_Y;
  
  // 计算PID控制量
  integralX += errorX;
  integralY += errorY;
  derivativeX = errorX - prevErrorX;
  derivativeY = errorY - prevErrorY;
  
  // 防止积分过大
  integralX = constrain(integralX, -300, 300);
  integralY = constrain(integralY, -300, 300);
  
  // 计算舵机调整量
  float adjustX = KP * errorX + KI * integralX + KD * derivativeX;
  float adjustY = KP * errorY + KI * integralY + KD * derivativeY;
  
  // 更新舵机位置 (注意X轴和Y轴可能需要反向)
  servoXPos = constrain(servoXPos - adjustX, 0, 180);
  servoYPos = constrain(servoYPos + adjustY, 0, 180);
  
  // 控制舵机
  servoX.write(servoXPos);
  servoY.write(servoYPos);
  
  // 保存当前误差
  prevErrorX = errorX;
  prevErrorY = errorY;
} 