#ifndef STEPPER_CONTROL_H
#define STEPPER_CONTROL_H

#include <Arduino.h>

// 最大支持的步进电机数量
#define MAX_STEPPER_NUM 4

// 步进电机初始化
void stepper_init(void);

// 设置步进电机最大速度 (步/秒)
void stepper_set_speed(uint16_t steps_per_second);

// 设置步进电机加速度 (步/秒²)
void stepper_set_acceleration(float accel);

// 移动到绝对位置
void stepper_move_to(int32_t position);

// 相对移动指定步数
void stepper_move_steps(int32_t steps);

// 停止步进电机
void stepper_stop(void);

// 获取步进电机是否正在运动
bool stepper_is_moving(void);

// 获取当前位置
int32_t stepper_get_position(void);

// 获取到目标位置的距离
int32_t stepper_distance_to_go(void);

// 步进电机控制循环，需要在主循环中调用
void stepper_loop(void);

// 配置步进电机引脚
void stepper_config_pins(uint8_t stepper_id, uint8_t step_pin, uint8_t dir_pin, uint8_t enable_pin);

// 切换当前选择的步进电机
bool select_stepper(uint8_t stepper_id);

// 简化的一键调用函数 - 控制指定电机以指定速度和加速度移动到指定位置
// stepper_id: 步进电机编号 (0-3)
// max_speed: 最大速度 (步/秒)
// accel: 加速度 (步/秒²)
// position: 目标绝对位置
// 返回值: 成功返回true，失败返回false
bool stepper_run(uint8_t stepper_id, uint16_t max_speed, float accel, int32_t position);

#endif // STEPPER_CONTROL_H