#ifndef LASER_SENSOR_H
#define LASER_SENSOR_H

#include <Arduino.h>

/**
 * @brief 激光传感器错误码定义
 */
#define LASER_SENSOR_EOK 0      /**< 操作成功 */
#define LASER_SENSOR_ERROR 1    /**< 操作失败 */
#define LASER_SENSOR_ETIMEOUT 2 /**< 通信超时 */
#define LASER_SENSOR_EINVAL 3   /**< 无效参数 */
#define LASER_SENSOR_ERANGE 4   /**< 数据超出范围 */

/**
 * @brief 初始化激光传感器
 * 设置串口和LED指示灯
 */
void laser_sensor_init(void);

/**
 * @brief 读取激光传感器距离
 *
 * @param distance 存储测量距离的指针(单位:mm)
 * @return uint8_t 错误码(0=成功，非0=错误类型)
 */
uint8_t laser_sensor_read(uint16_t *distance);

/**
 * @brief 设置激光传感器LED指示灯状态
 *
 * @param state LED状态(true=开启, false=关闭)
 */
void laser_set_led(bool state);

/**
 * @brief 获取激光传感器最后一次通信状态
 *
 * @return uint8_t 最后通信状态(0=成功，非0=错误类型)
 */
uint8_t laser_sensor_get_status(void);

#endif