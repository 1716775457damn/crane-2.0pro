#ifndef _LASER_H_
#define _LASER_H_

#include <Arduino.h>

// 激光传感器引脚定义
#define LASER_RX_PIN 16
#define LASER_TX_PIN 17

// 函数声明
void laser_init(void);
float jiguang(void);

/**
 * 激光传感器初始化
 */
inline void laser_init(void)
{
    // 初始化激光传感器串口
    Serial2.begin(115200, SERIAL_8N1, LASER_RX_PIN, LASER_TX_PIN);
    Serial.println("Laser sensor initialized");
}

/**
 * 获取激光传感器测量的实时距离
 * @return 测量的距离，单位mm
 */
inline float jiguang(void)
{
    if (Serial2.available())
    {
        String data = "";
        // 等待数据完全接收
        delay(10);

        while (Serial2.available())
        {
            char c = Serial2.read();
            if (c == '\n' || c == '\r')
                break;
            data += c;
        }

        // 清空剩余数据
        while (Serial2.available())
            Serial2.read();

        // 解析数据
        if (data.length() > 0)
        {
            float distance = data.toFloat();
            return distance;
        }
    }

    // 如果没有真实数据，返回-1表示读取失败
    return -1.0;
}

#endif // _LASER_H_