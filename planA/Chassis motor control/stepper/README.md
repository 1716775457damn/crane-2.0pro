# 激光传感器与电机控制系统

该项目基于ESP32S3，整合ATK-MS53L2M激光传感器、步进电机和舵机，提供完整的传感器读取和电机控制功能。

## 功能特点

- 实时读取激光传感器距离数据
- 精准控制步进电机位置和舵机角度
- 模块化设计，便于扩展和复用
- 提供完整的API文档和示例代码

## 硬件连接

### 激光传感器接线 (ATK-MS53L2M)
- VCC → ESP32 3.3V/5V (优先5V)
- GND → ESP32 GND
- TXD → ESP32 GPIO18 (RX)
- RXD → ESP32 GPIO17 (TX)

### 步进电机驱动接线
- DIR → ESP32 GPIO12 (方向)
- STEP → ESP32 GPIO14 (脉冲)
- ENA → ESP32 GPIO13 (使能)
- VCC、GND接驱动器电源

### 舵机接线
- 信号线 → ESP32 GPIO15
- VCC → 5V电源
- GND → GND

## 软件架构

项目采用PlatformIO框架开发，文件结构如下：

- `src/main.cpp`：主程序入口，包含测试代码
- `src/laser_sensor.cpp`：激光传感器驱动代码
- `src/stepper_control.cpp`：步进电机控制代码
- `src/servo_control.cpp`：舵机控制代码
- `include/laser_sensor.h`：激光传感器头文件
- `include/stepper_control.h`：步进电机控制头文件
- `include/servo_control.h`：舵机控制头文件
- `docs/API_Reference.md`：详细API文档

## 使用方法

1. 将代码编译上传到ESP32S3开发板
2. 打开串口监视器 (115200波特率)，查看测试输出信息
3. 测试程序会自动执行以下测试：
   - 激光传感器测试：读取并显示距离值
   - 步进电机测试：按照预设顺序移动步进电机
   - 舵机测试：控制舵机在不同角度之间平滑移动
   - 综合测试：根据距离值自动控制步进电机和舵机

## 库接口说明

### 激光传感器
```cpp
// 初始化
void laser_sensor_init();

// 读取距离 (返回错误码)
uint8_t laser_sensor_read(uint16_t *distance);
```

### 步进电机
```cpp
// 初始化
void stepper_init();

// 移动指定步数 (正数顺时针，负数逆时针)
void stepper_move_steps(int32_t steps);

// 主循环中调用的控制函数
void stepper_loop();

// 设置速度
void stepper_set_speed(uint16_t steps_per_second);
```

### 舵机
```cpp
// 初始化舵机，指定控制引脚
void servo_init(uint8_t pin);

// 立即设置舵机角度(0-180度)
void servo_set_angle(uint8_t angle);

// 平滑转动到指定角度，speed为速度(1-10)
void servo_sweep_to(uint8_t target_angle, uint8_t speed);

// 主循环中调用的控制函数
void servo_loop();
```

详细API文档请参阅 `docs/API_Reference.md`。

## 开发备注

- 激光传感器使用ASCII数据格式，从"d: XX mm"格式中提取距离值
- 步进电机控制使用DIR/STEP接口，适用于大多数步进电机驱动器
- 舵机控制使用ESP32的PWM功能，支持标准50Hz舵机

## 扩展开发

可通过以下方式扩展系统功能：
- 添加新的传感器驱动
- 扩展电机控制算法
- 增加闭环控制逻辑

## 许可证
MIT 