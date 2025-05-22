#ifndef STEPPER_CONTROL_H
#define STEPPER_CONTROL_H

#include <Arduino.h>

/**
 * @brief 步进电机控制库 - 错误码定义
 */
#define STEPPER_OK 0    /**< 操作成功 */
#define STEPPER_ERROR 1 /**< 操作失败 */
#define STEPPER_BUSY 2  /**< 电机忙 */

/**
 * @brief 初始化步进电机控制器
 * 默认设置引脚和禁用电机
 */
void stepper_init(void);

/**
 * @brief 设置步进电机速度
 *
 * @param steps_per_second 步进电机速度(步/秒), 范围1-5000
 */
void stepper_set_speed(uint16_t steps_per_second);

/**
 * @brief 移动步进电机指定步数
 * 正值表示正向转动，负值表示反向转动
 *
 * @param steps 移动步数，范围INT32_MIN到INT32_MAX
 */
void stepper_move_steps(int32_t steps);

/**
 * @brief 立即停止步进电机
 */
void stepper_stop(void);

/**
 * @brief 获取步进电机是否正在运动
 *
 * @return true 正在运动
 * @return false 静止状态
 */
bool stepper_is_moving(void);

/**
 * @brief 步进电机运行循环
 * 必须在主循环中持续调用此函数，以实现电机转动
 */
void stepper_loop(void);

#endif // STEPPER_CONTROL_H