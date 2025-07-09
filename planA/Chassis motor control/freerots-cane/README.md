# ESP32底盘控制与传感器系统使用说明

该项目是基于ESP32的智能底盘控制系统，包含底盘运动控制、步进电机控制和激光测距功能。系统基于FreeRTOS多任务框架，提供简单易用的API接口。

## 1. 硬件连接

### 1.1 底盘控制连接

底盘控制通过串口通信，使用ESP32的以下引脚：
- **TX引脚**: 13
- **RX引脚**: 14

```
ESP32                   底盘控制板
---------------------------------
GPIO13 (TX) ------->  RX 
GPIO14 (RX) <-------  TX
GND        <------>  GND
```

### 1.2 激光传感器连接

使用ATK-MS53L2M激光测距模块，通过UART与ESP32通信：

| ATK-MS53L2M | ESP32 |
|-------------|-------|
| 红色(VCC) | 3.3V或5V |
| 黑色(GND) | GND |
| 黄色(TXD) | GPIO16 (LASER_RX_PIN) |
| 白色(RXD) | GPIO17 (LASER_TX_PIN) |

注意：模块的TXD需要连接到ESP32的RX引脚，RXD需要连接到ESP32的TX引脚。

### 1.3 步进电机连接

库支持控制最多四个步进电机，引脚定义如下：

```
// 电机1引脚
#define SLEEP1  4
#define DIR1    5
#define STEP1   6

// 电机2引脚
#define SLEEP2  7
#define DIR2    15
#define STEP2   16

// 电机3引脚
#define SLEEP3  42
#define DIR3    41
#define STEP3   40

// 电机4引脚
#define SLEEP4  39
#define DIR4    38
#define STEP4   37
```

## 2. 底盘控制API使用示例

### 2.1 初始化

```cpp
#include "chassis.h"

void setup() {
    // 初始化底盘控制串口
    chassis_serial_init();
}
```

### 2.2 简单控制

最简单的控制方式是使用预定义的动作函数：

```cpp
// 底盘前进(默认5秒)
move_forward();

// 底盘后退(默认5秒)
move_backward();

// 底盘左移(默认5秒)
move_left();

// 底盘右移(默认5秒)
move_right();

// 底盘停止
stop_chassis();
```

### 2.3 带参数的控制

如果需要自定义持续时间，可以使用：

```cpp
// 方向枚举值: FORWARD, BACKWARD, LEFT, RIGHT, STOP
move_chassis(FORWARD, 3000);   // 前进3秒
move_chassis(BACKWARD, 2000);  // 后退2秒
move_chassis(LEFT, 1500);      // 左移1.5秒
```

### 2.4 灵活控制

自定义距离和速度：

```cpp
// 方向(1=前进,-1=后退,0=停止,2=左移,3=右移)，距离，速度(值越小越快)
control_chassis(1, 8000, 30);    // 前进，距离8000，速度30
control_chassis(-1, 5000, 20);   // 后退，距离5000，速度20(更快)
```

## 3. 激光测距API使用示例

### 3.1 一行代码调用方式

使用`laser_easy.h`库提供的简易API：

```cpp
#include "laser_easy.h"

void setup() {
    Serial.begin(115200);
    
    // 初始化激光传感器
    initLaser();
}

void loop() {
    // 获取激光测距数据
    float distance = getLaserDistance();
    
    // 检查前方是否有障碍物(默认阈值300mm)
    if (hasObstacle()) {
        Serial.println("检测到障碍物!");
    }
    
    // 获取距离等级(0:错误, 1:非常近, 2:较近, 3:中等, 4:较远)
    int level = getDistanceLevel();
    
    // 一键测距(自动初始化并获取距离)
    float quick_distance = oneShotDistance(true);  // 参数true表示打印结果
    
    delay(500);
}
```

### 3.2 传统调用方式

```cpp
#include "laser.h"

void setup() {
    Serial.begin(115200);
    
    // 初始化激光传感器
    laser_init();
}

void loop() {
    // 使用jiguang()函数获取测距
    float distance = jiguang();
    
    // 使用get_laser_distance()函数获取测距
    float distance2 = get_laser_distance();
    
    if (distance >= 0) {
        Serial.print("距离: ");
        Serial.print(distance);
        Serial.println(" mm");
    }
    
    delay(500);
}
```

## 4. 步进电机API使用示例

### 4.1 初始化

```cpp
#include "StepperMotor.h"

void setup() {
    // 初始化步进电机
    initSteppers();
}

void loop() {
    // 确保在主循环或任务中定期调用updateSteppers()
    updateSteppers();
}
```

### 4.2 控制单个步进电机

```cpp
// 控制电机1以速度100运行200步（正向）
runStepper(1, 100, 200);

// 控制电机2以速度150反向运行300步
runStepper(2, 150, -300);

// 控制电机3以速度200运行，不等待其停止
runStepper(3, 200, 500, false);
```

### 4.3 同步控制多个步进电机

```cpp
// 同步控制电机1和电机2
// 电机1: 速度100, 200步
// 电机2: 速度150, 300步
runSteppers(100, 200, 150, 300);

// 同步控制四个电机
runAllSteppers(100, 200, 150, 300, 200, 400, 250, 500);
```

## 5. 实用组合示例

### 5.1 激光测距控制步进电机

```cpp
#include "laser_easy.h"
#include "StepperMotor.h"

void setup() {
    Serial.begin(115200);
    initLaser();
    initSteppers();
}

void loop() {
    // 更新步进电机状态
    updateSteppers();
    
    // 获取激光测距数据
    float distance = getLaserDistance();
    
    // 当距离大于500mm时，让1号步进电机以200的速度正转
    if (distance > 500) {
        // 检查电机是否已经在运行
        if (!isStepperRunning(1)) {
            // 启动电机，连续转动模式(步数=1)
            runStepper(1, 200, 1, false);
            Serial.println("距离大于500mm，电机开始运转");
        }
    } 
    // 当距离小于等于500mm时，停止电机
    else if (distance >= 0 && distance <= 500) {
        stopStepper(1);
        Serial.println("距离小于等于500mm，电机停止");
    }
    
    delay(100);
}
```

### 5.2 一行代码实现距离控制步进电机

我们提供了一个封装好的函数`controlStepperByDistance`，可以一行代码实现激光测距控制步进电机：

```cpp
#include "laser_easy.h"
#include "StepperMotor.h"

// 函数声明
float controlStepperByDistance(uint8_t motorNumber, float targetDistance, 
                               uint8_t speed, bool isGreaterThan = true,
                               float slowDownThreshold = 100.0, uint8_t slowSpeed = 80);

void setup() {
    Serial.begin(115200);
    initLaser();
    initSteppers();
}

void loop() {
    // 更新步进电机状态
    updateSteppers();
    
    // 一行代码控制：当距离>500mm时1号电机正转，接近时自动减速
    float distance = controlStepperByDistance(1, 500.0, 200);
    
    // 显示当前距离
    if (distance >= 0) {
        Serial.print("距离: ");
        Serial.print(distance);
        Serial.println(" mm");
    }
    
    delay(100);
}
```

函数参数说明：
- `motorNumber`: 电机号(1-4)
- `targetDistance`: 目标距离(mm)
- `speed`: 电机速度(1-255)
- `isGreaterThan`: true=距离大于阈值时运行，false=距离小于阈值时运行
- `slowDownThreshold`: 减速阈值(mm)，接近目标距离多少时减速
- `slowSpeed`: 减速时的速度(1-255)

### 5.3 障碍物检测与避障

```cpp
#include "laser_easy.h"
#include "chassis.h"

void setup() {
    Serial.begin(115200);
    chassis_serial_init();
    initLaser();
}

void loop() {
    // 获取激光测距数据
    float distance = getLaserDistance();
    
    // 检查是否有障碍物
    if (hasObstacle(500)) {  // 500mm内有障碍物
        Serial.println("检测到障碍物，向右转向");
        move_right();  // 向右移动避开障碍物
        delay(2000);
        stop_chassis();
    } else {
        // 无障碍物，继续前进
        move_forward();
        delay(1000);
    }
}
```

### 5.2 使用FreeRTOS多任务

```cpp
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "laser_easy.h"
#include "chassis.h"

// 激光测距任务
void laserTask(void *pvParameters) {
    initLaser();
    
    while (1) {
        float distance = getLaserDistance();
        
        if (distance >= 0) {
            Serial.print("距离: ");
            Serial.print(distance);
            Serial.println(" mm");
        }
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// 底盘控制任务
void chassisTask(void *pvParameters) {
    chassis_serial_init();
    
    while (1) {
        move_forward();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        
        move_backward();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        
        stop_chassis();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void setup() {
    Serial.begin(115200);
    
    // 创建激光测距任务
    xTaskCreate(
        laserTask,
        "LaserTask",
        2048,
        NULL,
        1,
        NULL
    );
    
    // 创建底盘控制任务
    xTaskCreate(
        chassisTask,
        "ChassisTask",
        2048,
        NULL,
        1,
        NULL
    );
}

void loop() {
    // 主循环为空，所有操作在任务中执行
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}
``` 