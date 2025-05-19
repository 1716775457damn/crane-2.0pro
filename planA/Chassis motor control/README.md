# 底盘控制系统技术文档

## 一、系统概述

底盘控制系统是物流小车的移动控制核心，负责实现小车的精确运动和路径执行。本系统提供多种控制方案和驱动方式，可根据不同需求选择适合的实现方式。

## 二、目录结构

```
Chassis motor control/
├── 底盘控制-basic/                # 基础控制方案（串口通信控制）
│   ├── controltest.ino           # 基础测试程序
│   └── ReadMe接线说明.txt          # 接线说明文档
├── stepper/                      # AccelStepper库控制方案
│   └── src/
│       └── main.cpp              # 主程序代码
├── qzj/                          # L298N驱动方案
│   └── src/
│       └── main.cpp              # 主程序代码
├── qzj2.0/                       # 升级版方案（带舵机控制）
│   └── src/
│       └── main.cpp              # 主程序代码
├── rots2.0/                      # 机器人控制系统2.0
└── README.md                     # 本文档
```

## 三、实现方案对比

| 方案 | 控制器 | 驱动方式 | 适用场景 | 特点 |
|------|--------|----------|----------|------|
| 底盘控制-basic | Arduino MEGA2560 | 串口通信+专用驱动板 | 精确运动控制 | 简单易用，适合快速部署 |
| stepper | ESP32 | AccelStepper库+驱动板 | 高精度定位 | 加减速平滑，精确控制 |
| qzj | ESP32 | L298N模块直驱 | 一般移动控制 | 成本低，适合原型开发 |
| qzj2.0 | ESP32 | L298N+舵机组合 | 带舵机的复合控制 | 功能丰富，适合多机构协调 |

## 四、硬件配置详情

### 1. 底盘控制-basic方案：
- **控制器**：Arduino MEGA2560
- **通信接口**：UART（TX0/RX0）
- **驱动方式**：专用步进电机驱动板
- **接线方案**：
  ```
  Arduino MEGA2560    驱动板
  TX0 --------------> RX
  RX0 --------------> TX
  VCC -------------> VCC
  GND -------------> GND
  ```
- **电源要求**：5V（控制板），24V（电机驱动）

### 2. stepper方案：
- **控制器**：ESP32
- **电机控制**：步进电机
- **引脚定义**：
  ```cpp
  #define STEP_PIN 18        // 步进信号引脚
  #define DIR_PIN 19         // 方向信号引脚
  #define ENABLE_PIN 21      // 使能信号引脚
  ```
- **控制参数**：
  ```cpp
  const float maxSpeed = 1150.0;     // 最大速度
  const float acceleration = 2000.0; // 加速度
  const int stepsToMove = 10000;     // 移动步数
  ```

### 3. qzj方案：
- **控制器**：ESP32
- **驱动芯片**：L298N
- **引脚定义**：
  ```cpp
  const int motorAin1 = 17; // 电机A的输入1
  const int motorAin2 = 16; // 电机A的输入2
  const int motorBin1 = 5;  // 电机B的输入1
  const int motorBin2 = 18; // 电机B的输入2
  const int enA = 19;       // 电机A的使能引脚
  const int enB = 21;       // 电机B的使能引脚
  ```

### 4. qzj2.0方案：
- **控制器**：ESP32
- **执行机构**：舵机
- **引脚设置**：
  ```cpp
  // 舵机引脚定义
  Servo servo_1;
  Servo servo_2;
  const int middlePosition = 40; // 舵机中间位置
  ```

## 五、软件功能实现

### 1. 基础底盘控制（controltest.ino）：
底盘控制的基本实现使用了简单的状态机设计，通过case语句实现不同动作序列：

```cpp
void loop() {
  switch (FlowControl) {    
    case 0:    // 按位置向前进
      Serial.print("7,8000,30"); // 前进8000步，速度为30
      delay(5000);
      FlowControl = 1;
      break;
     
    case 1:    // 按位置向后运动
      Serial.print("8,8000,30"); // 后退8000步，速度为30
      delay(5000);
      FlowControl = 2;
      break;
      
    // ... 更多动作
  }
}
```

### 2. 步进电机精确控制（stepper/src/main.cpp）：
使用AccelStepper库实现步进电机的精确控制，包括加减速：

```cpp
void loop() {
  // 设置目标位置并启动电机
  stepper.moveTo(stepsToMove);

  // 运行电机
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }

  delay(1000);

  // 反向运动
  stepper.moveTo(-stepsToMove);
  // ... 执行反向运动
}
```

### 3. 直驱电机控制（qzj/src/main.cpp）：
实现了电机的基本控制功能，包括速度控制和方向控制：

```cpp
// 设置电机速度（正数向前，负数向后）
void setMotorSpeed(int motorPin1, int motorPin2, int speed) {
  if (speed > 0) {
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
  } else if (speed < 0) {
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
  } else {
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
  }
  analogWrite(enA, abs(speed));
  analogWrite(enB, abs(speed));
}
```

### 4. 舵机控制功能（qzj2.0/src/main.cpp）：
实现了舵机的角度控制，用于方向调整或机械臂操作：

```cpp
// 定义左转函数
void turnServoLeft_1(int degrees) {
  int newAngle = servoPosition - degrees;
  newAngle = constrain(newAngle, 0, 180); // 确保角度不超过舵机的物理限制
  servo_1.write(newAngle);
  servoPosition = newAngle; // 更新当前舵机位置
}
```

## 六、控制协议

### 1. 串口控制协议（底盘控制-basic）：
通过串口发送指令控制底盘运动，格式为：`命令ID,参数1,参数2`

| 命令ID | 说明 | 参数1 | 参数2 | 示例 |
|--------|------|-------|-------|------|
| 7 | 前进 | 脉冲数 | 速度 | 7,8000,30 |
| 8 | 后退 | 脉冲数 | 速度 | 8,8000,30 |
| 5 | 左移 | 脉冲数 | 速度 | 5,8000,30 |
| 6 | 右移 | 脉冲数 | 速度 | 6,8000,30 |

### 2. 直接API控制（其他方案）：
其他方案通过函数API直接控制电机和舵机，主要功能包括：

- **电机控制**：
  - 前进/后退
  - 转向
  - 速度调整
  - 停止

- **舵机控制**：
  - 角度设置
  - 转向控制
  - 抓取/释放（如用于机械臂）

## 七、使用指南

### 1. 硬件连接：
- 根据选择的方案，按照相应的接线图连接电机驱动和控制器
- 确保电源连接正确，特别是电机驱动和舵机需要单独供电
- 检查所有连接是否牢固，避免松动导致控制不稳定

### 2. 程序烧录：
- 底盘控制-basic方案：使用Arduino IDE烧录程序到MEGA2560
- 其他方案：使用PlatformIO或Arduino IDE烧录程序到ESP32
- 烧录前检查引脚设置是否与实际接线一致

### 3. 测试流程：
- 先进行单个电机测试，确认方向和速度控制正常
- 测试基本动作（前进、后退、转向）
- 测试复杂动作序列
- 根据测试结果调整参数（如速度、加速度等）

## 八、常见问题与解决方案

### 1. 电机不转动：
- **症状**：发送命令后电机没有响应
- **原因**：接线错误、电源问题、使能信号未激活
- **解决方案**：
  - 检查电源连接，确保驱动板有足够电源
  - 验证使能信号是否正确设置（通常为低电平有效）
  - 检查步进/方向信号是否正确生成

### 2. 运动不精确：
- **症状**：步进电机丢步或位置偏移
- **原因**：机械阻力大、电流不足、加速过快
- **解决方案**：
  - 调整驱动电流，确保足够驱动力矩
  - 减小加速度，避免瞬时扭矩过大
  - 检查机械结构，减少摩擦和阻力

### 3. 舵机抖动：
- **症状**：舵机在目标位置附近抖动
- **原因**：PWM信号不稳定、电源波动、负载过大
- **解决方案**：
  - 使用独立稳定的电源给舵机供电
  - 调整舵机控制代码，减少频繁位置更新
  - 确保舵机负载在额定范围内

## 九、优化与扩展

### 1. 性能优化：
- **电机控制优化**：
  - 实现S形加减速曲线，提高平稳性
  - 添加闭环控制（如编码器反馈），提高精度
  - 优化驱动电流，减少发热和能耗

- **响应性能**：
  - 减少通信延迟
  - 优化控制算法，提高响应速度

### 2. 功能扩展：
- **增加传感器**：
  - 添加编码器实现位置闭环
  - 集成IMU（惯性测量单元）提供姿态信息
  - 加入距离传感器实现避障功能

- **控制接口扩展**：
  - 增加Wi-Fi/蓝牙无线控制功能
  - 实现ROS接口，与其他机器人系统集成 