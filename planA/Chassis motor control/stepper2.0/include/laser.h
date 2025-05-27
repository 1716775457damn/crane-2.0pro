#ifndef _LASER_H_
#define _LASER_H_

// 激光传感器引脚定义
#define LASER_RX_PIN 16
#define LASER_TX_PIN 17

// 函数声明
void laser_init(void);
float jiguang(void);

#endif // _LASER_H_ 