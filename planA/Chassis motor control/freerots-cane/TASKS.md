# ESP32智能拐杖 - 任务说明文档

本文档详细介绍了ESP32智能拐杖项目中使用的FreeRTOS任务系统及其功能，帮助开发者理解系统的工作流程和任务调度机制。

## 任务系统概述

该项目基于FreeRTOS实现多任务并行处理，通过任务调度机制完成不同功能模块的协调工作。主要任务包括：

- 障碍物检测任务
- 路径导航任务
- 步进电机控制任务
- 舵机控制任务
- 电源电压监测任务

## 核心任务说明

### 主控制任务 (task_00)

该任务是系统启动后的第一个任务，负责初始化舵机位置并启动基本的探测任务。

```c
void task_00(void *pvParameters){
    servo1(105);  // 初始化舵机位置
    xTaskCreatePinnedToCore(task_0, "Task_0", 2000, NULL, 1, NULL, 0);  // 创建任务0并分配到核心0
    vTaskDelete(NULL);  // 删除当前任务
}
```

### 基本探测任务 (task_0)

该任务是系统的核心任务，负责持续监测超声波传感器数据和循迹状态，并根据不同情况触发相应的动作任务。

主要功能：
1. 通过超声波传感器检测前方障碍物
2. 根据障碍物状态和计数器值调用不同的处理任务
3. 当无障碍物时执行循迹导航功能

```c
void task_0(void *pvParameters)
{
    while (1)
    {
        chaosheng = measureDistanceAndSetState();  // 测量距离并设置状态
        if (chaosheng == 1)  // 无障碍物
        {
            b = tracing();  // 执行循迹导航
            if (b = 3)  // 当循迹模块检测到特定路线时
            {
                xTaskCreatePinnedToCore(task_001, "Task001", 2000, NULL, 1, NULL, 0);
                vTaskDelete(NULL);
            }
        }
        else if (chaosheng == 0)  // 有障碍物
        {
            // 根据计数器a的值执行不同的操作序列
            switch (a)
            {
            case 0:
                // 第一次探测到障碍物，执行抓取动作1
                Serial.print("0,10,10");  // 停止移动
                delay(30);
                xTaskCreatePinnedToCore(task_1, "Task1", 2000, NULL, 1, NULL, 0);  // 勾物体1.0
                a += 1;
                vTaskDelete(NULL);
                break;
            case 1:
                // 第二次探测到障碍物，执行放置动作1
                xTaskCreatePinnedToCore(task_first, "Task_first", 2000, NULL, 1, NULL, 1);  // 放物体1.0
                a += 1;
                vTaskDelete(NULL);
                break;
            case 2:
                // 第三次探测到障碍物，执行存储动作
                xTaskCreatePinnedToCore(task_second, "Task_second", 2000, NULL, 1, NULL, 0);  // 存储2.0
                a += 1;
                vTaskDelete(NULL);
                break;
            case 3:
                // 第四次探测到障碍物，执行抓取和升起动作
                xTaskCreatePinnedToCore(task_third, "Task_0", 2000, NULL, 1, NULL, 1);  // 勾，升3.0
                a += 1;            
                vTaskDelete(NULL);
                break;
            case 4:
                // 第五次探测到障碍物，执行第二次放置动作
                xTaskCreatePinnedToCore(task_fourth, "Task_0", 2000, NULL, 1, NULL, 1);  // 放2.0
                a += 1;
                vTaskDelete(NULL);
                break;
            case 5:
                // 第六次探测到障碍物，执行第三次放置动作
                xTaskCreatePinnedToCore(task_fifth, "Task_0", 2000, NULL, 1, NULL, 1);  // 放3.0
                a += 1;
                vTaskDelete(NULL);
                break;
            }
        }
    }
}
```

### 方向控制任务 (task_001)

该任务负责控制智能拐杖的方向导航，根据计数器e的值执行不同的方向控制。

```c
void task_001(void *pvParameters)
{
    e += 1;
    switch (e)
    {
    case 1:
        xTaskCreatePinnedToCore(task_301, "Task301", 2000, NULL, 1, NULL, 0);  // 向左转
        e += 1;
        vTaskDelete(NULL);
        break;
    case 2: 
        xTaskCreatePinnedToCore(task_302, "Task302", 2000, NULL, 1, NULL, 0);  // 向右转
        e += 1;
        vTaskDelete(NULL);
        break;
    case 3:
        xTaskCreatePinnedToCore(task_102, "Task102", 2000, NULL, 1, NULL, 0);
        e += 1;
        vTaskDelete(NULL);
        break;
    }
    xTaskCreatePinnedToCore(task_0, "Task_0", 2000, NULL, 1, NULL, 0);
    vTaskDelete(NULL);
}
```

## 功能性任务说明

### 物体抓取任务 (task_1)

该任务负责控制舵机和步进电机实现物体的抓取动作。

```c
void task_1(void *pvParameters)
{
    servo1(65);  // 设置舵机角度为65度（抓取位置）
    delay(30);
    controlStepper(stepper1, 950, 1800, 3700);  // 控制步进电机1上升到指定位置
    xTaskCreatePinnedToCore(task_0, "Task_0", 2000, NULL, 1, NULL, 0);
    vTaskDelete(NULL);
}
```

### 物体放置任务 (task_first)

该任务负责控制步进电机和小车移动来放置物体，并执行后续动作。

```c
void task_first(void *pvParameters)
{
    Serial.print("0,10,10");  // 停止移动
    delay(30);
    controlStepper(stepper1, 2000, 1000, 2000);  // 步进电机1移动到指定位置
    servo1(105);  // 设置舵机角度为105度（释放位置）
    delay(500);
    Serial.print("5,8000,8");  // 后退
    delay(2300);
    Serial.print("8,9200,15");  // 右转
    delay(4500);
    controlStepper(stepper1, 1000, 1000, 0);  // 步进电机1回到初始位置
    xTaskCreatePinnedToCore(task_0, "Task_0", 2000, NULL, 1, NULL, 0);
    vTaskDelete(NULL);
}
```

### 步进电机控制子任务

以下任务负责控制步进电机的特定动作，通常由其他任务调用：

```c
void task_101(void *pvParameters){
    controlStepper(stepper1, 950, 2000, 2900);  // 步进电机1上升
    vTaskDelete(NULL);
}

void task_102(void *pvParameters)
{
    delay(6000);
    controlStepper(stepper2, 2000, 2000, -2600);  // 步进电机2向左移动
    vTaskDelete(NULL);
}

void task_103(void *pvParameters)
{
    controlStepper(stepper1, 2000, 2000, 0);  // 步进电机1下降到初始位置
    vTaskDelete(NULL);
}

void task_104(void *pvParameters)
{
    controlStepper(stepper2, 2000, 2000, 0);  // 步进电机2回到初始位置
    vTaskDelete(NULL);
}
```

### 物体存储任务 (task_second)

该任务负责协调多个步进电机同时工作，实现物体的存储功能。

```c
void task_second(void *pvParameters)
{
    Serial.print("0,10,10");  // 停止移动
    delay(30);
    servo1(65);  // 设置舵机角度为65度（抓取位置）
    
    // 创建两个并行任务分别控制两个步进电机
    xTaskCreatePinnedToCore(task_101, "Task1", 4000, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(task_102, "Task2", 4000, NULL, 1, NULL, 1);
    
    controlStepper(stepper1, 1800, 1800, 2000);  // 步进电机1移动到指定位置
    servo1(105);  // 设置舵机角度为105度（释放位置）
    
    // 创建两个并行任务让步进电机回到初始位置
    xTaskCreatePinnedToCore(task_103, "Task3", 4000, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(task_104, "Task4", 4000, NULL, 1, NULL, 1);

    xTaskCreatePinnedToCore(task_0, "Task_0", 2000, NULL, 1, NULL, 0);
    vTaskDelete(NULL);
}
```

### 方向控制任务 (task_301/task_302)

这些任务负责控制小车的方向：

```c
void task_301(void *pvParameters)
{
    Serial.print("7,4600,15");  // 向左转动
    xTaskCreatePinnedToCore(task_0, "Task_0", 2000, NULL, 1, NULL, 1);
    vTaskDelete(NULL);
}

void task_302(void *pvParameters)
{
    Serial.print("8,4600,15");  // 向右转动
    xTaskCreatePinnedToCore(task_0, "Task_0", 2000, NULL, 1, NULL, 1);
    vTaskDelete(NULL);
}
```

## 传感器功能任务

### 超声波测距任务

测量距离并设置系统状态：

```c
int measureDistanceAndSetState()
{
    // 执行多次测量取平均值
    float totalDistance = 0;
    int numReadings = 5;

    for (int i = 0; i < numReadings; i++)
    {
        // 测量距离的代码
        totalDistance += distance;
        delay(50);
    }
    float averageDistance = totalDistance / numReadings;

    // 根据平均距离设置状态
    if (averageDistance <= 9)
    {
        chaosheng = 0;  // 有障碍物
    }
    else
    {
        chaosheng = 1;  // 无障碍物
    }
    return chaosheng;
}
```

### 循迹导航任务

读取循迹传感器并控制小车方向：

```c
int tracing()
{
    tracingcontrol = combinedSensorControl();  // 获取传感器状态
    switch (tracingcontrol)
    {
    case 0:
        Serial.print("2,12,18");  // 向右转
        delay(30);
        b = 0;
        break;
    case 1:
        Serial.print("2,20,20");  // 直行
        delay(30);
        b = 1;
        break;
    case 2:
        Serial.print("2,18,12");  // 向左转
        delay(30);
        b = 2;
        break;
    case 3:
        Serial.print("0,10,10");  // 停止
        delay(30);
        b = 3;
        break;
    }
    return b;
}
```

## 任务调度说明

- 系统使用FreeRTOS的多核任务调度功能，将不同任务分配到ESP32的两个核心上
- 使用`xTaskCreatePinnedToCore`函数将任务固定到特定核心执行，避免资源竞争
- 任务优先级设置为1，栈大小根据任务复杂度分配（通常为2000-4000字节）
- 使用`vTaskDelete(NULL)`在任务完成后删除自身，释放资源
- 系统采用链式任务调用方式，每个任务完成后创建下一个需要执行的任务

## 任务依赖关系

1. **启动序列**: main -> task_00 -> task_0
2. **障碍物检测序列**: task_0 -> task_1/task_first/task_second/...（根据计数器a值）
3. **循迹导航序列**: task_0 -> tracing() -> task_001（当b=3时）
4. **方向控制序列**: task_001 -> task_301/task_302（根据计数器e值）

## 调试和优化建议

1. 使用串口监视器观察任务执行情况和传感器数据
2. 调整任务栈大小以优化内存使用
3. 根据实际需求调整任务优先级
4. 考虑使用信号量或互斥量管理共享资源
5. 调整延时参数以优化系统响应时间和稳定性 