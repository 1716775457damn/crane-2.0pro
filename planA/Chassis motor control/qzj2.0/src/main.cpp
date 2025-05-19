#include <Arduino.h>
#include <ESP32Servo.h>

Servo servo_1; // 创建Servo对象
Servo servo_2;
// 初始化舵机的中间位置
const int middlePosition = 40;
int servoPosition = middlePosition; // 当前舵机位置
const int ledPin = 2;
bool ledState = false; // 使用布尔类型来表示LED状态
unsigned long lastMillis = 0;
int Last_Command = 0;
// int ENA1 = 16;  // 使能信号的io口
// int PUL1 = 5; // 脉冲信号的io口
// int DIR1 = 17; // 方向信号的io口
// int x1;
// const int stepsPerRevolution1 = 200; // 每转一圈的步数（根据具体电机参数调整）
// const int speed1 = 5;                // 脉冲间隔（单位：毫秒）
// 函数声明
void turnServoLeft(int degrees);
void turnServoRight(int degrees);


// void maichong1(int times, int speed) // times是脉冲的数量，speed是脉冲间隔，对应着电机的速度
// {
//   for (x1 = 0; x1 < times; x1++)
//   {
//     digitalWrite(PUL1, HIGH);
//     delayMicroseconds(200); // 这个函数单位为微秒
//     digitalWrite(PUL1, LOW);
//     delayMicroseconds(200); // 驱动器说明书规定了脉冲信号的持续时间，在规定的时间内选择尽量小的数值
//     delay(speed);           // 前后两个脉冲之间的间隔
//   }
// }
// void stepMotorContro1(int circles, int speed)
// {
//   // 确保传入的圈数是整数
//   int circles1;
//   circles1 = abs(circles);

//   // 根据方向设置方向引脚的状态
//   // 假设正转使用LOW，反转使用HIGH
//   // 注意：具体的电平和您的硬件设计有关，这里需要根据实际情况调整
//   digitalWrite(DIR1, (circles > 0) ? LOW : HIGH);

//   // 循环转动指定的圈数
//   for (int i = 0; i < circles1; i++)
//   {
//     // 调用已有的脉冲函数来驱动电机转动一圈
//     maichong1(stepsPerRevolution1, speed);

//     // 延时一段时间，防止电机过热或其他问题
//     delay(10);
//   }
// }

// 定义左转函数
void turnServoLeft_1(int degrees)
{
  int newAngle = servoPosition - degrees;
  newAngle = constrain(newAngle, 0, 180); // 确保角度不超过舵机的物理限制
  servo_1.write(newAngle);
  servoPosition = newAngle; // 更新当前舵机位置
}

// 定义右转函数
void turnServoRight_1(int degrees)
{
  int newAngle = servoPosition + degrees;
  newAngle = constrain(newAngle, 0, 180); // 确保角度不超过舵机的物理限制
  servo_1.write(newAngle);
  servoPosition = newAngle; // 更新当前舵机位置
}

void turnServoLeft_2(int degrees)
{
  int newAngle = servoPosition - degrees;
  newAngle = constrain(newAngle, 0, 180); // 确保角度不超过舵机的物理限制
  servo_2.write(newAngle);
  servoPosition = newAngle; // 更新当前舵机位置
}

// 定义右转函数
void turnServoRight_2(int degrees)
{
  int newAngle = servoPosition + degrees;
  newAngle = constrain(newAngle, 0, 180); // 确保角度不超过舵机的物理限制
  servo_2.write(newAngle);
  servoPosition = newAngle; // 更新当前舵机位置
}
// 将字符转换为对应的整数值
// int hexCharToInt(char c)
// {
//   if (c >= '0' && c <= '9')
//   {
//     return c - '0';
//   }
//   else if (c >= 'A' && c <= 'F')
//   {
//     return 10 + c - 'A';
//   }
//   else if (c >= 'a' && c <= 'f')
//   {
//     return 10 + c - 'a';
//   }
//   else
//   {
//     return 0; // 如果不是十六进制字符，返回0
//   }
// }

// 将整数值转换为十六进制字符
// char intToHexChar(int value)
// {
//   if (value < 10)
//   {
//     return '0' + value;
//   }
//   else
//   {
//     return 'A' + (value - 10);
//   }
// }

// // 根据十六进制值控制舵机
// void turnServoBasedOnHexValue(char hexValue)
// {
//   // 根据十六进制值映射到舵机的角度范围
//   int angle = map(hexValue, '3', '4', 0, 180);
//   servo.write(angle); // 设置舵机到新的角度
// }

// 闪烁LED以示错误
void blinkLED()
{
  digitalWrite(ledPin, HIGH);
  delay(200);
  digitalWrite(ledPin, LOW);
}

void setup()
{
  Serial.begin(9600);      // 初始化串口通信
  servo_1.attach(18);        // 舵机连接到GPIO 18
  servo_2.attach(19);
  pinMode(ledPin, OUTPUT); // 设置LED引脚为输出模式
  digitalWrite(ledPin, ledState);
}

void loop()
{
  // 检查是否已经过了1秒
  if (millis() - lastMillis > 10)
  {
    // 保存当前时间作为下一次检查的基准
    lastMillis = millis();

    // 检查串口是否有数据
    if (Serial.available() > 0)
    {
      // 读取数据

      int command = Serial.read();
      Serial.print(command);
      Last_Command = command;
      command = Serial.read();
      if (command != Last_Command)
      {
        command = Last_Command;
        switch (command)
        {
        case 0x72:
          // blinkLED();
          turnServoLeft_1(1);
          break;
        case 0x6c:
          // blinkLED();
          turnServoRight_1(1); // 舵机右转
          break;
        case 0x66:
          // 下
          blinkLED();
          turnServoRight_2(1);
          break;
        case 0x62:
          // 上
          blinkLED();
          turnServoLeft_2(1);
          break;
        }
      }
    }
  }
}
