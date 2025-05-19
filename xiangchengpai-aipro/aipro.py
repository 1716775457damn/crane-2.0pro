import cv2
import numpy as np
import random
import pytesseract
from PIL import Image
import RPi.GPIO as GPIO
import time
import serial

# 定义常量
MAX_BOXES = 6

# 定义步进电机引脚（假设四个步进电机用于不同的功能）
# 后轮驱动电机
DRIVE_MOTOR_PINS = [17, 18, 27, 22]
# 前轮转向电机
STEERING_MOTOR_PINS = [5, 6, 13, 19]
# 升降电机（用于货箱升降）
LIFT_MOTOR_PINS = [26, 12, 16, 20]
# 抓取电机（用于货箱抓取，也可用舵机代替）
GRAB_MOTOR_PINS = [21, 23, 24, 25]

SERVO_PIN = 23  # 如果抓取机构使用舵机，则使用此引脚

# 初始化 GPIO 引脚
GPIO.setmode(GPIO.BCM)
for pin_set in [DRIVE_MOTOR_PINS, STEERING_MOTOR_PINS, LIFT_MOTOR_PINS, GRAB_MOTOR_PINS]:
    for pin in pin_set:
        GPIO.setup(pin, GPIO.OUT)


# 如果使用舵机进行抓取，则注释掉 GRAB_MOTOR_PINS 的设置，并设置舵机引脚
# GPIO.setup(SERVO_PIN, GPIO.OUT)

# 定义货箱和置物纸垛类
class Box:
    def __init__(self, box_id, x, y):
        self.id = box_id
        self.x = x
        self.y = y


class Pallet:
    def __init__(self, pallet_id, x, y):
        self.id = pallet_id
        self.x = x
        self.y = y


boxes = []
pallets = []
sign_results = [[0, 0] for _ in range(MAX_BOXES)]


# 初始化场地
def init_environment():
    global pallets
    # 初始化置物纸垛位置（根据场地实际情况设置）
    pallet_positions = [
        (0, 0), (100, 0), (200, 0), (300, 0), (400, 0), (500, 0)
    ]
    for i, pos in enumerate(pallet_positions):
        pallets.append(Pallet(i + 1, pos[0], pos[1]))


# 模拟抽签过程
def simulate_sign():
    global sign_results
    for i in range(MAX_BOXES):
        sign_results[i][0] = i + 1  # 货架位置
        sign_results[i][1] = random.randint(1, MAX_BOXES)  # 随机货箱编号
    # 输出抽签结果
    print("抽签结果:")
    for i in range(MAX_BOXES):
        print(f"货架 {sign_results[i][0]}: 货箱 {sign_results[i][1]}")


# 使用OCR识别货箱编号
def identify_boxes_with_ocr():
    global boxes
    cap = cv2.VideoCapture(0)  # 使用USB摄像头
    for i in range(MAX_BOXES):
        ret, frame = cap.read()
        if not ret:
            print("Failed to grab frame")
            continue

        # 图像预处理
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        _, thresh = cv2.threshold(gray, 150, 255, cv2.THRESH_BINARY)
        contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        for contour in contours:
            x, y, w, h = cv2.boundingRect(contour)
            if w > 50 and h > 50:  # 过滤掉小的轮廓
                roi = gray[y:y + h, x:x + w]
                # OCR识别
                box_id = pytesseract.image_to_string(roi,
                                                     config='--psm 10 --oem 3 -c tessedit_char_whitelist=0123456789')
                if box_id.isdigit():
                    boxes.append(Box(int(box_id), x, y))
                    print(f"识别到货箱 {box_id} 在位置 ({x}, {y})")

    cap.release()


# 步进电机控制函数
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


# 控制舵机
def control_servo(angle):
    pwm = GPIO.PWM(SERVO_PIN, 100)
    pwm.start(2.5)  # 初始位置
    pwm.ChangeDutyCycle(angle)
    time.sleep(0.5)
    pwm.stop()


# 初始化激光测距传感器
def init_laser_sensor():
    return serial.Serial('/dev/ttyS0', 9600, timeout=1)


# 读取激光测距传感器的距离
def read_laser_distance(sensor):
    sensor.write(b'measure\n')
    time.sleep(0.1)
    response = sensor.readline().decode().strip()
    try:
        distance = int(response.split(':')[-1])
        return distance
    except:
        return -1


# 小车前进
def move_forward(steps):
    control_stepper_motor(DRIVE_MOTOR_PINS, 1, steps)


# 小车后退
def move_backward(steps):
    control_stepper_motor(DRIVE_MOTOR_PINS, -1, steps)


# 小车转向
def turn(direction, steps):
    if direction == 'left':
        control_stepper_motor(STEERING_MOTOR_PINS, 1, steps)
    elif direction == 'right':
        control_stepper_motor(STEERING_MOTOR_PINS, -1, steps)


# 升降货箱
def lift_box(height_steps):
    control_stepper_motor(LIFT_MOTOR_PINS, 1, height_steps)


# 降低货箱
def lower_box(height_steps):
    control_stepper_motor(LIFT_MOTOR_PINS, -1, height_steps)


# 抓取货箱
def grab_box():
    # 如果使用步进电机进行抓取
    control_stepper_motor(GRAB_MOTOR_PINS, 1, 100)
    # 如果使用舵机进行抓取，请使用以下代码
    # control_servo(12.5)  # 假设 12.5 是抓取角度


# 释放货箱
def release_box():
    # 如果使用步进电机进行抓取
    control_stepper_motor(GRAB_MOTOR_PINS, -1, 100)
    # 如果使用舵机进行抓取，请使用以下代码
    # control_servo(2.5)  # 假设 2.5 是释放角度


# 移动机器人到目标位置
def move_robot_to_target(target_x, target_y, laser_sensor):
    current_x, current_y = 0, 0  # 假设小车从原点开始
    dx = target_x - current_x
    dy = target_y - current_y

    # 计算移动步骤（这里简化为直接使用 dx 和 dy 作为步骤数）
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


# 执行任务
def execute_mission():
    print("执行任务...")
    laser_sensor = init_laser_sensor()
    simulate_sign()
    identify_boxes_with_ocr()

    for i in range(len(boxes)):
        box = boxes[i]
        # 根据任务要求选择置物纸垛
        if box.id == 2:
            # 特殊处理：将2号箱码垛在其他纸垛中的任意一个
            possible_pallets = [p for p in pallets if p.id != 2]
            pallet = random.choice(possible_pallets)
            pallet_id = pallet.id
        else:
            # 其他货箱直接放置在对应的纸垛上
            pallet_id = box.id

        # 移动到货箱位置
        move_robot_to_target(box.x, box.y, laser_sensor)
        lower_box(100)  # 降低货箱抓取机构
        grab_box()  # 抓取货箱
        lift_box(100)  # 升高货箱抓取机构

        # 移动到置物纸垛位置
        move_robot_to_target(pallets[pallet_id - 1].x, pallets[pallet_id - 1].y, laser_sensor)
        lower_box(100)  # 降低货箱抓取机构
        release_box()  # 释放货箱
        lift_box(100)  # 升高货箱抓取机构

        # 返回起始位置
        move_robot_to_target(0, 0, laser_sensor)


# 主函数
if __name__ == "__main__":
    init_environment()
    execute_mission()
    GPIO.cleanup()