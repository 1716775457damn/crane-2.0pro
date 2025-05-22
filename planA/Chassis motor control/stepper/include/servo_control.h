#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Arduino.h>

/**
 * @brief 初始化舵机
 *
 * @param pin 舵机控制信号引脚
 */
void servo_init(uint8_t pin);

/**
 * @brief 设置舵机角度(立即执行)
 *
 * @param angle 舵机角度(0-180°)
 */
void servo_set_angle(uint8_t angle);

/**
 * @brief 平滑转动舵机到指定角度(非阻塞)
 *
 * @param angle 目标角度(0-180°)
 * @param speed 转动速度(1-10, 1最慢, 10最快)
 */
void servo_sweep_to(uint8_t angle, uint8_t speed);

/**
 * @brief 获取当前舵机角度
 *
 * @return uint8_t 当前角度(0-180°)
 */
uint8_t servo_get_angle();

/**
 * @brief 舵机控制循环函数(在主循环中调用)
 * 用于实现平滑转动功能
 */
void servo_loop();

#endif