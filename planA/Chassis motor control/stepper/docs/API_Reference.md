# 传感器与执行器接口文档

## 1. 激光传感器 (laser_sensor.h)

激光传感器模块用于读取ATK-MS53L2M激光传感器的距离数据。

### 接口函数:

#### 初始化
```cpp
void laser_sensor_init();
```
- **功能**: 初始化激光传感器模块，设置串口和LED
- **参数**: 无
- **返回值**: 无
- **注意**: 使用固定引脚 RX=18, TX=17

#### 读取距离
```cpp
uint8_t laser_sensor_read(uint16_t *distance);
```
- **功能**: 读取激光传感器的距离值
- **参数**: 
  - `distance`: 用于存储读取到的距离值的指针(单位: mm)
- **返回值**: 
  - `LASER_SENSOR_EOK(0)`: 读取成功
  - `LASER_SENSOR_ERROR(1)`: 读取失败
  - `LASER_SENSOR_ETIMEOUT(2)`: 超时错误
- **示例**:
```cpp
uint16_t distance;
if (laser_sensor_read(&distance) == LASER_SENSOR_EOK) {
    Serial.print("距离: ");
    Serial.print(distance);
    Serial.println(" mm");
}
```

#### LED控制
```cpp
void led_init();
void led_toggle();
```
- **功能**: 初始化并控制板载LED
- **参数**: 无
- **返回值**: 无
- **注意**: LED使用GPIO 2引脚

## 2. 步进电机控制 (stepper_control.h)

步进电机控制模块用于精确控制步进电机的位置和运动。

### 接口函数:

#### 初始化
```cpp
void stepper_init();
```
- **功能**: 初始化步进电机控制，设置引脚模式
- **参数**: 无
- **返回值**: 无
- **注意**: 使用固定引脚 DIR=12, STEP=14, ENABLE=13

#### 移动步数
```cpp
void stepper_move_steps(int32_t steps);
```
- **功能**: 控制步进电机移动指定步数
- **参数**: 
  - `steps`: 移动的步数，正数顺时针，负数逆时针
- **返回值**: 无
- **注意**: 该函数仅启动运动，实际运动在stepper_loop()中执行

#### 控制循环
```cpp
void stepper_loop();
```
- **功能**: 步进电机控制循环，负责执行实际的步进脉冲
- **参数**: 无
- **返回值**: 无
- **注意**: 必须在主循环中定期调用此函数

#### 设置速度
```cpp
void stepper_set_speed(uint16_t steps_per_second);
```
- **功能**: 设置步进电机移动速度
- **参数**: 
  - `steps_per_second`: 步进电机速度(步/秒)，范围1-5000
- **返回值**: 无
- **示例**:
```cpp
stepper_set_speed(2000);  // 设置速度为2000步/秒
```

#### 停止
```cpp
void stepper_stop();
```
- **功能**: 停止步进电机运动
- **参数**: 无
- **返回值**: 无

## 3. 舵机控制 (servo_control.h)

舵机控制模块用于精确控制舵机的角度和运动。

### 接口函数:

#### 初始化
```cpp
void servo_init(uint8_t pin);
```
- **功能**: 初始化舵机，设置控制引脚
- **参数**: 
  - `pin`: 舵机信号引脚
- **返回值**: 无
- **示例**:
```cpp
servo_init(15);  // 使用GPIO 15控制舵机
```

#### 设置角度
```cpp
void servo_set_angle(uint8_t angle);
```
- **功能**: 立即设置舵机角度
- **参数**: 
  - `angle`: 目标角度(0-180度)
- **返回值**: 无
- **注意**: 此函数会立即将舵机移动到指定角度

#### 平滑转动
```cpp
void servo_sweep_to(uint8_t target_angle, uint8_t speed);
```
- **功能**: 平滑地将舵机转动到指定角度
- **参数**: 
  - `target_angle`: 目标角度(0-180度)
  - `speed`: 转动速度(1-10)，1最慢，10最快
- **返回值**: 无
- **注意**: 需要在主循环中调用servo_loop()来执行实际转动

#### 控制循环
```cpp
void servo_loop();
```
- **功能**: 舵机控制循环，负责执行平滑转动
- **参数**: 无
- **返回值**: 无
- **注意**: 必须在主循环中定期调用此函数

#### 获取当前角度
```cpp
uint8_t servo_get_angle();
```
- **功能**: 获取当前舵机角度
- **参数**: 无
- **返回值**: 当前舵机角度(0-180度) 