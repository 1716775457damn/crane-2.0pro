#ifndef _CHASSIS_H_
#define _CHASSIS_H_

#include <Arduino.h>

// 底盘控制串口定义
#define CHASSIS_SERIAL_RX 14
#define CHASSIS_SERIAL_TX 13

// 底盘控制命令定义
#define CHASSIS_FORWARD  7  // 前进
#define CHASSIS_BACKWARD 8  // 后退
#define CHASSIS_LEFT     6  // 左移
#define CHASSIS_RIGHT    5  // 右移//5,9000,10右转90
#define CHASSIS_STOP     0  // 停止

// 方向枚举，使代码更易读
enum ChassisDirection {
    STOP = 0,
    FORWARD = 1,  // 对应命令7
    BACKWARD = 2, // 对应命令8
    LEFT = 3,     // 对应命令6
    RIGHT = 4     // 对应命令5
};

// 标准移动距离和速度的默认值
#define DEFAULT_DISTANCE 8000
#define DEFAULT_SPEED 30
#define DEFAULT_DELAY 5000 // 默认延时5秒

// 函数声明
void chassis_serial_init(void);
void send_chassis_command(const String& command);
String read_chassis_data(void);
void control_chassis_raw(int cmd, int distance, int speed);
void control_chassis(int direction, int distance, int speed);
void test_chassis_motion(void);

// 一行代码控制函数
void move_chassis(ChassisDirection direction, int delay_ms = DEFAULT_DELAY);
void move_forward(void);
void move_backward(void);
void move_left(void);
void move_right(void);
void stop_chassis(void);

/**
 * 初始化底盘控制专用串口(使用引脚13和14)
 * 此串口仅用于底盘控制，不会与其他串口混用
 */
inline void chassis_serial_init(void)
{
    // 使用Serial1作为底盘控制串口，映射到引脚13(TX)和14(RX)
    Serial1.begin(115200, SERIAL_8N1, CHASSIS_SERIAL_RX, CHASSIS_SERIAL_TX);
    Serial.println("Chassis control serial port initialized on pins TX:13, RX:14");
}

/**
 * 发送命令到底盘控制串口
 * 注意：此函数仅用于发送底盘控制命令，不应用于其他用途
 * @param command 要发送的控制命令字符串
 */
inline void send_chassis_command(const String& command)
{
    Serial1.print(command);  // 直接print不加换行
    Serial.print("已发送底盘命令: ");
    Serial.println(command);
}

/**
 * 从底盘控制串口读取数据
 * 注意：此函数仅用于接收底盘状态数据，不应用于其他用途
 * @return 读取到的字符串
 */
inline String read_chassis_data(void)
{
    if (Serial1.available())
    {
        String data = "";
        // 等待数据完全接收
        delay(10);

        while (Serial1.available())
        {
            char c = Serial1.read();
            if (c == '\n' || c == '\r')
                break;
            data += c;
        }

        // 清空剩余数据
        while (Serial1.available())
            Serial1.read();

        return data;
    }

    return ""; // 没有数据时返回空字符串
}

/**
 * 向底盘发送原始控制命令
 * @param cmd 命令号：7=前进, 8=后退, 6=左移, 5=右移, 0=停止
 * @param distance 运动距离(脉冲数)
 * @param speed 速度(值越小速度越快)
 */
inline void control_chassis_raw(int cmd, int distance, int speed)
{
    String command = String(cmd) + "," + String(distance) + "," + String(speed);
    send_chassis_command(command);
}

/**
 * 向底盘发送控制命令(简化版)
 * @param direction 方向：1=前进, -1=后退, 0=停止, 2=左移, 3=右移
 * @param distance 运动距离(脉冲数)
 * @param speed 速度(值越小速度越快)，默认为30
 */
inline void control_chassis(int direction, int distance, int speed = 30)
{
    int cmd = 0;
    
    // 将direction转换为底盘实际命令号
    switch(direction) {
        case 1:  // 前进
            cmd = CHASSIS_FORWARD;
            break;
        case -1: // 后退
            cmd = CHASSIS_BACKWARD;
            break;
        case 2:  // 左移
            cmd = CHASSIS_LEFT;
            break;
        case 3:  // 右移
            cmd = CHASSIS_RIGHT;
            break;
        default: // 停止
            cmd = CHASSIS_STOP;
            break;
    }
    
    // 发送命令
    control_chassis_raw(cmd, distance, speed);
}

/**
 * 底盘移动测试程序
 * 按顺序执行：前进->后退->左移->右移->循环
 */
inline void test_chassis_motion(void)
{
    static int FlowControl = 0;
    
    switch (FlowControl) {    
        case 0:    // 前进
            Serial.println("测试：底盘向前运动");
            control_chassis_raw(CHASSIS_FORWARD, 8000, 30);
            delay(5000); 
            FlowControl = 1;
            break;
         
        case 1:    // 后退  
            Serial.println("测试：底盘向后运动");
            control_chassis_raw(CHASSIS_BACKWARD, 8000, 30);
            delay(5000);
            FlowControl = 2;     
            break;
           
        case 2:    // 左移
            Serial.println("测试：底盘向左平移");
            control_chassis_raw(CHASSIS_LEFT, 8000, 30);
            delay(5000); 
            FlowControl = 3;
            break;
         
        case 3:    // 右移
            Serial.println("测试：底盘向右平移");
            control_chassis_raw(CHASSIS_RIGHT, 8000, 30);
            delay(5000);  
            FlowControl = 0; // 回到开始，循环执行
            break;
    }
}

/**
 * 一行代码控制底盘移动(最简版)
 * @param direction 移动方向: FORWARD, BACKWARD, LEFT, RIGHT, STOP
 * @param delay_ms 移动持续时间(毫秒)，默认5000ms
 */
inline void move_chassis(ChassisDirection direction, int delay_ms)
{
    int cmd = 0;
    
    // 将枚举方向转换为底盘命令
    switch(direction) {
        case FORWARD:
            cmd = CHASSIS_FORWARD;
            Serial.println("底盘: 前进");
            break;
        case BACKWARD:
            cmd = CHASSIS_BACKWARD;
            Serial.println("底盘: 后退");
            break;
        case LEFT:
            cmd = CHASSIS_LEFT;
            Serial.println("底盘: 左移");
            break;
        case RIGHT:
            cmd = CHASSIS_RIGHT;
            Serial.println("底盘: 右移");
            break;
        case STOP:
        default:
            cmd = CHASSIS_STOP;
            Serial.println("底盘: 停止");
            break;
    }
    
    // 发送命令
    control_chassis_raw(cmd, DEFAULT_DISTANCE, DEFAULT_SPEED);
    
    // 如果需要等待
    if(delay_ms > 0) {
        delay(delay_ms);
    }
}

/**
 * 预定义动作: 前进(默认5秒)
 */
inline void move_forward(void) {
    move_chassis(FORWARD, DEFAULT_DELAY);
}

/**
 * 预定义动作: 后退(默认5秒)
 */
inline void move_backward(void) {
    move_chassis(BACKWARD, DEFAULT_DELAY);
}

/**
 * 预定义动作: 左移(默认5秒)
 */
inline void move_left(void) {
    move_chassis(LEFT, DEFAULT_DELAY);
}

/**
 * 预定义动作: 右移(默认5秒)
 */
inline void move_right(void) {
    move_chassis(RIGHT, DEFAULT_DELAY);
}

/**
 * 预定义动作: 停止
 */
inline void stop_chassis(void) {
    move_chassis(STOP, 0);
}

#endif // _CHASSIS_H_ 