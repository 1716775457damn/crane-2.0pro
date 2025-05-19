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
  
  // 发送测试数据到K210
  K210_SERIAL.println("ESP32S3 init test");
  Serial.println("Sent test message to K210");
  
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
  // 检查K210串口可用性并测试通信
  static unsigned long lastPingTime = 0;
  static unsigned long lastSerialCheckTime = 0;
  static int testCounter = 0;
  static int receivedCount = 0;
  
  // 每5秒测试一次串口通信
  if (millis() - lastPingTime > 5000) {
    testCounter++;
    Serial.printf("Testing K210 UART connection (#%d)...\n", testCounter);
    K210_SERIAL.print("ESP32S3_PING_");
    K210_SERIAL.println(testCounter);
    lastPingTime = millis();
  }
  
  // 每500ms检查一次串口数据
  if (millis() - lastSerialCheckTime > 500) {
    checkK210Serial(true);  // 强制检查并报告状态
    lastSerialCheckTime = millis();
  }
  
  // 持续处理数据
  if (readDataFromK210()) {
    // 有数据时更新OLED和控制舵机
    receivedCount++;
    Serial.printf("Successfully parsed data #%d\n", receivedCount);
    updateDisplay();
    
    if (objectDetected) {
      trackObject();
    }
  }
  
  delay(10);  // 短暂延时
}

// 检查K210串口数据（包括测试消息和其他信息）
bool checkK210Serial(bool reportStatus) {
  if (K210_SERIAL.available()) {
    String data = K210_SERIAL.readStringUntil('\n');
    // 清理字符串
    data.trim();
    
    // 打印原始数据
    Serial.print("K210 Serial Data: '");
    Serial.print(data);
    Serial.println("'");
    
    // 特殊消息处理
    if (data.startsWith("K210") || data.startsWith("UART test") || 
        data.startsWith("Camera") || data.startsWith("LCD") || 
        data.startsWith("Model") || data.startsWith("Detection") ||
        data.startsWith("No object")) {
      // 这些是状态消息，不是检测数据
      return true;
    }
    
    return true;  // 有数据返回true
  }
  
  if (reportStatus) {
    Serial.println("No data from K210");
  }
  
  return false;
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
  Serial.print("Raw data: '");
  Serial.print(data);
  Serial.println("'");
  
  // 记录接收到的每个字节（十六进制）
  Serial.print("Hex: ");
  for (int i = 0; i < data.length(); i++) {
    Serial.printf("%02X ", (unsigned char)data.charAt(i));
  }
  Serial.println();
  
  // 如果是状态或测试消息，忽略处理
  if (data.startsWith("K210") || data.startsWith("UART test") || 
      data.startsWith("Camera") || data.startsWith("LCD") || 
      data.startsWith("Model") || data.startsWith("Detection") ||
      data.startsWith("No object")) {
    Serial.println("Received status message, ignoring");
    return false;
  }
  
  // 尝试多种格式解析
  
  // 格式1: 使用逗号分隔的简单格式 (number,x,y,width,height)
  if (data.indexOf(',') > 0) {
    return parseCommaFormat(data);
  }
  
  // 格式2: 使用冒号分隔的格式 (x:y:w:h:classid:confidence:label)
  if (data.indexOf(':') > 0) {
    return parseColonFormat(data);
  }
  
  Serial.println("Unknown data format");
  return false;
}

// 解析逗号分隔格式: number,x,y,width,height
bool parseCommaFormat(String data) {
  Serial.println("Parsing comma format");
  
  // 检查数据格式
  if (data.indexOf(',') < 0) {
    Serial.println("Invalid data format (no commas)");
    return false;
  }
  
  // 解析数据格式：NUMBER,X,Y,WIDTH,HEIGHT
  int commaIndex1 = data.indexOf(',');
  if (commaIndex1 > 0) {
    // 尝试解析数字
    String numberStr = data.substring(0, commaIndex1);
    Serial.println("Detected number: " + numberStr);
    detectedNumber = numberStr.toInt();
    
    int commaIndex2 = data.indexOf(',', commaIndex1 + 1);
    int commaIndex3 = data.indexOf(',', commaIndex2 + 1);
    int commaIndex4 = data.indexOf(',', commaIndex3 + 1);
    
    if (commaIndex2 > 0 && commaIndex3 > 0 && commaIndex4 > 0) {
      int x = data.substring(commaIndex1 + 1, commaIndex2).toInt();
      int y = data.substring(commaIndex2 + 1, commaIndex3).toInt();
      int width = data.substring(commaIndex3 + 1, commaIndex4).toInt();
      int height = data.substring(commaIndex4 + 1).toInt();
      
      // 打印解析后的数据
      Serial.printf("Parsed data: Number=%d, X=%d, Y=%d, W=%d, H=%d\n", 
                    detectedNumber, x, y, width, height);
      
      // 计算目标中心点
      targetX = x + width / 2;
      targetY = y + height / 2;
      objectDetected = true;
      
      return true;
    } else {
      Serial.println("Invalid data format (missing commas)");
    }
  }
  
  return false;
}

// 解析冒号分隔格式: x:y:w:h:classid:confidence:label
bool parseColonFormat(String data) {
  Serial.println("Parsing colon format");
  
  // 检查数据格式
  if (data.indexOf(':') < 0) {
    Serial.println("Invalid data format (no colons)");
    return false;
  }
  
  // 解析格式: x:y:w:h:classid:confidence:label
  int colonIndex1 = data.indexOf(':');
  int colonIndex2 = data.indexOf(':', colonIndex1 + 1);
  int colonIndex3 = data.indexOf(':', colonIndex2 + 1);
  int colonIndex4 = data.indexOf(':', colonIndex3 + 1);
  int colonIndex5 = data.indexOf(':', colonIndex4 + 1);
  int colonIndex6 = data.indexOf(':', colonIndex5 + 1);
  
  if (colonIndex1 > 0 && colonIndex2 > 0 && colonIndex3 > 0 && 
      colonIndex4 > 0 && colonIndex5 > 0 && colonIndex6 > 0) {
    
    int x = data.substring(0, colonIndex1).toInt();
    int y = data.substring(colonIndex1 + 1, colonIndex2).toInt();
    int width = data.substring(colonIndex2 + 1, colonIndex3).toInt();
    int height = data.substring(colonIndex3 + 1, colonIndex4).toInt();
    int classId = data.substring(colonIndex4 + 1, colonIndex5).toInt();
    float confidence = data.substring(colonIndex5 + 1, colonIndex6).toFloat();
    String label = data.substring(colonIndex6 + 1);
    
    // 打印解析后的数据
    Serial.printf("Parsed data: X=%d, Y=%d, W=%d, H=%d, ClassID=%d, Conf=%.2f, Label=%s\n", 
                  x, y, width, height, classId, confidence, label.c_str());
    
    // 获取数字标签
    detectedNumber = label.toInt();
    if (detectedNumber == 0 && label != "0") {
      // 若无法转为数字，则使用原始类别ID
      detectedNumber = classId + 1;  // 类别ID从0开始，所以+1使其从1开始
    }
    
    // 计算目标中心点
    targetX = x + width / 2;
    targetY = y + height / 2;
    objectDetected = true;
    
    return true;
  }
  
  Serial.println("Invalid colon format");
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