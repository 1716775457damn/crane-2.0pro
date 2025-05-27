// #ifndef _STEPPER_H_
// #define _STEPPER_H_

// #include <AccelStepper.h>

// // 步进电机引脚定义
// #define STEPPER1_PIN_STEP 14   // 步进引脚
// #define STEPPER1_PIN_DIR 12    // 方向引脚
// #define STEPPER1_PIN_ENABLE 13 // 使能引脚
// #define MOTOR_INTERFACE_TYPE 1 // 接口类型，1为单步模式

// // 电机参数上限
// extern const float maxSpeed;
// extern const float acceleration;

// // 步进电机对象声明
// extern AccelStepper stepper1;

// // 函数声明
// void stepper_init(void);
// bool controlStepper(AccelStepper &stepper, float speed, float accel, int32_t steps);
// void stepper_loop(void);

// #endif // _STEPPER_H_ 