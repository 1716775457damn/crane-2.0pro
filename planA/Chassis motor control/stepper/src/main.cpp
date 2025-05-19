// #include <Arduino.h>

// int ENA = 32; // 使能信号的io口
// int PUL = 25; // 脉冲信号的io口
// int DIR = 33; // 方向信号的io口
// int x;

// // 步进电机参数设置
// const int stepsPerRevolution = 1000; // 每转一圈的步数（根据具体电机参数调整）
// const int speed = 1;                // 脉冲间隔（单位：毫秒）

// void maichong(int times, int speed) // times是脉冲的数量，speed是脉冲间隔，对应着电机的速度
// {
//   for (x = 0; x < times; x++)
//   {
//     digitalWrite(PUL, HIGH);
//     delayMicroseconds(200); // 这个函数单位为微秒
//     digitalWrite(PUL, LOW);
//     delayMicroseconds(200); // 驱动器说明书规定了脉冲信号的持续时间，在规定的时间内选择尽量小的数值
//     delay(speed);           // 前后两个脉冲之间的间隔
//   }
// }

// void setup()
// {
//   pinMode(PUL, OUTPUT);   // 设置io口输出
//   pinMode(DIR, OUTPUT);   // 设置io口输出
//   pinMode(ENA, OUTPUT);   // 设置io口输出
//   digitalWrite(ENA, LOW); // LOW表示使能开启，HIGH表示使能关闭
// }

// void loop()
// {
//   digitalWrite(DIR, HIGH);              // 设置电机正转方向
//   maichong(stepsPerRevolution, speed); // 调用脉冲函数，转动一圈
//   // delay(2000);                         // 每隔10秒转一圈
// }

#include <AccelStepper.h>

// 步进电机参数
#define MOTOR_INTERFACE_TYPE 1 // 接口类型，1为单步模式
#define STEP_PIN 18            // 步进信号引脚
#define DIR_PIN 19           // 方向信号引脚
#define ENABLE_PIN 21         // 使能信号引脚

// 定义步进电机对象
AccelStepper stepper(MOTOR_INTERFACE_TYPE, STEP_PIN, DIR_PIN);

// 电机参数设置
const float maxSpeed = 1150.0;     // 最大速度 (steps per second)
const float acceleration = 2000.0; // 加速度 (steps per second^2)
const int stepsToMove = 10000;     // 移动步数

void setup()
{
  // 初始化步进电机
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW); // 使能电机

  stepper.setMaxSpeed(maxSpeed);
  stepper.setAcceleration(acceleration);

  Serial.begin(9600);
}

void loop()
{
  // 设置目标位置并启动电机
  stepper.moveTo(stepsToMove);

  // 运行电机
  while (stepper.distanceToGo() != 0)
  {
    stepper.run();
  }

  delay(1000); // 等待1秒

  // 反向运动
  stepper.moveTo(-stepsToMove);

  // 运行电机
  while (stepper.distanceToGo() != 0)
  {
    stepper.run();
  }

  delay(10000); // 等待1秒
}