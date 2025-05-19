# 香橙派aipro物流小车控制系统

## 一、系统概述

本系统基于香橙派aipro开发板实现了一套完整的物流小车自主控制系统，包括视觉识别、路径规划、障碍物检测与货箱搬运功能。系统采用Python语言编写，使用OpenCV处理图像，实现小车在物流场景中的全自动运行。

## 二、目录结构

```
planB/
├── xiangchengpai-aipro/   # 香橙派控制代码目录
│   └── aipro.py           # 主程序代码
└── readme.md              # 本文档
```

## 三、硬件配置

### 主控单元
- **开发板**：香橙派 aipro(20T)
- **处理器**：全志 H6 四核 Cortex-A53 CPU
- **内存**：24GB RAM
- **存储**：使用TF卡存储系统和程序

### 传感器系统
- **视觉系统**：USB摄像头（1080p分辨率）
- **测距系统**：激光测距传感器（串口通信）

### 执行系统
- **驱动电机**：四个步进电机（用于小车移动）
  - GPIO引脚定义：[17, 18, 27, 22]
- **转向系统**：步进电机（用于前轮转向）
  - GPIO引脚定义：[5, 6, 13, 19]
- **升降机构**：步进电机（用于货箱升降）
  - GPIO引脚定义：[26, 12, 16, 20]
- **抓取机构**：步进电机或舵机（用于货箱抓取）
  - GPIO引脚定义：[21, 23, 24, 25] 或 舵机引脚23

## 四、软件架构

### 核心模块
1. **环境初始化模块**：`init_environment()` 
   - 设置场地坐标系
   - 初始化置物纸垛位置信息

2. **任务规划模块**：`simulate_sign()` 
   - 生成或接收任务指令
   - 规划搬运顺序和路径

3. **视觉识别模块**：`identify_boxes_with_ocr()`
   - 利用OpenCV进行图像处理
   - 使用Pytesseract进行OCR数字识别
   - 识别货箱位置和编号

4. **运动控制模块**：
   - 步进电机控制：`control_stepper_motor()`
   - 基础动作函数：`move_forward()`, `move_backward()`, `turn()`
   - 复合动作：`move_robot_to_target()`

5. **搬运执行模块**：
   - 升降控制：`lift_box()`, `lower_box()`
   - 抓取控制：`grab_box()`, `release_box()`

6. **障碍物检测模块**：
   - 激光测距：`read_laser_distance()`
   - 碰撞避免：距离阈值判断与停止机制

### 软件流程
系统工作流程由`execute_mission()`函数协调，主要步骤如下：
1. 初始化系统环境与传感器
2. 解析任务（模拟抽签或实际任务输入）
3. 识别场地上的货箱位置与编号
4. 按照任务要求，依次执行搬运任务：
   - 移动到货箱位置
   - 抓取货箱
   - 移动到目标纸垛位置
   - 释放货箱
5. 返回起始位置，完成任务

## 五、核心算法实现

### 1. 步进电机控制
步进电机采用8步序列控制方式，实现精确的角度控制：
```python
def control_stepper_motor(motor_pins, direction, steps):
    sequence = [
        [1, 0, 0, 1],
        [1, 0, 0, 0],
        [1, 1, 0, 0],
        [0, 1, 0, 0],
        [0, 1, 1, 0],
        [0, 0, 1, 0],
        [0, 0, 1, 1],
        [0, 0, 0, 1]
    ]
    seq_index = 0
    for _ in range(steps):
        for pin in range(4):
            GPIO.output(motor_pins[pin], sequence[seq_index][pin])
        seq_index = (seq_index + direction) % 8
        time.sleep(0.001)
```

### 2. 货箱识别
使用OCR技术识别货箱编号，结合轮廓检测确定位置：
```python
def identify_boxes_with_ocr():
    cap = cv2.VideoCapture(0)
    ret, frame = cap.read()
    if ret:
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        _, thresh = cv2.threshold(gray, 150, 255, cv2.THRESH_BINARY)
        contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        
        for contour in contours:
            x, y, w, h = cv2.boundingRect(contour)
            if w > 50 and h > 50:
                roi = gray[y:y + h, x:x + w]
                box_id = pytesseract.image_to_string(roi, config='--psm 10 --oem 3 -c tessedit_char_whitelist=0123456789')
                if box_id.isdigit():
                    boxes.append(Box(int(box_id), x, y))
```

### 3. 路径规划
使用基于坐标的简单路径规划方法，结合激光测距实时避障：
```python
def move_robot_to_target(target_x, target_y, laser_sensor):
    current_x, current_y = 0, 0  # 假设小车从原点开始
    dx = target_x - current_x
    dy = target_y - current_y

    # 计算移动步骤
    move_forward(abs(dx))
    if dy > 0:
        turn('left', abs(dy))
    else:
        turn('right', abs(dy))

    # 检测障碍物
    while True:
        distance = read_laser_distance(laser_sensor)
        if distance == 400:
            print("到达指定距离，停止前进")
            break
        elif distance != -1:
            move_forward(1)
        time.sleep(0.1)
```

## 六、硬件接线说明

### GPIO接线
- **步进电机接线**：
  - 驱动电机：GPIO 17, 18, 27, 22 分别连接电机控制线A, B, C, D
  - 转向电机：GPIO 5, 6, 13, 19 分别连接电机控制线A, B, C, D
  - 升降电机：GPIO 26, 12, 16, 20 分别连接电机控制线A, B, C, D
  - 抓取电机：GPIO 21, 23, 24, 25 分别连接电机控制线A, B, C, D

- **舵机接线**（如使用）：
  - 舵机信号线连接到GPIO 23
  - 舵机电源和地线分别连接到5V电源和GND

### 传感器接线
- **USB摄像头**：直接连接到香橙派aipro的USB端口
- **激光测距传感器**：
  - TX线连接到香橙派UART0的RX（通常是GPIO 15）
  - RX线连接到香橙派UART0的TX（通常是GPIO 14）
  - VCC、GND分别连接到3.3V电源和GND

## 七、使用说明

### 1. 环境配置
在香橙派aipro上需要安装以下软件包：
```bash
sudo apt-get update
sudo apt-get install python3-opencv python3-pip python3-rpi.gpio
pip3 install pytesseract numpy pyserial pillow
sudo apt-get install tesseract-ocr
```

### 2. 程序部署
1. 将程序文件复制到香橙派aipro：
   ```bash
   scp aipro.py user@<香橙派IP地址>:/home/user/
   ```

2. 设置程序权限：
   ```bash
   chmod +x aipro.py
   ```

### 3. 运行程序
1. 连接所有硬件，确保接线正确
2. 运行程序：
   ```bash
   python3 aipro.py
   ```
3. 程序将自动执行初始化、识别和搬运任务

## 八、常见问题与解决方案

### 1. 视觉识别问题
- **症状**：无法正确识别货箱编号
- **解决方案**：
  - 调整摄像头位置，确保光线充足
  - 修改图像处理参数，如阈值（当前为150）
  - 更新OCR配置参数以提高识别率

### 2. 电机控制问题
- **症状**：电机不转动或动作不准确
- **解决方案**：
  - 检查GPIO引脚连接是否正确
  - 调整控制步进电机的延时参数（当前为0.001秒）
  - 增加电机驱动电流以提供足够扭矩

### 3. 路径规划问题
- **症状**：小车未能准确到达目标位置
- **解决方案**：
  - 校准坐标系和物理距离的对应关系
  - 添加编码器反馈以实现闭环控制
  - 实现更复杂的路径规划算法

## 九、扩展与优化

### 1. 系统优化
- 使用多线程处理图像识别和电机控制，提高实时性
- 实现PID控制算法，提高电机控制精度
- 添加IMU传感器，实现更准确的位置和姿态估计

### 2. 功能扩展
- 添加WiFi/蓝牙远程控制功能
- 实现多货箱同时搬运的策略规划
- 集成ROS系统，实现更复杂的机器人控制架构