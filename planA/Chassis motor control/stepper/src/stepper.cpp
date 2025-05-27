// #include <Arduino.h>
// #include "stepper.h"

// // 电机参数上限设置
// const float maxSpeed = 18000.0;    // 最大速度 (steps per second)
// const float acceleration = 5000.0; // 加速度 (steps per second^2)

// // 定义步进电机对象
// AccelStepper stepper1(MOTOR_INTERFACE_TYPE, STEPPER1_PIN_STEP, STEPPER1_PIN_DIR);

// /**
//  * 步进电机初始化
//  */
// void stepper_init(void) {
//   // 初始化步进电机
//   pinMode(STEPPER1_PIN_ENABLE, OUTPUT);
//   digitalWrite(STEPPER1_PIN_ENABLE, LOW); // 使能电机
  
//   stepper1.setMaxSpeed(maxSpeed);
//   stepper1.setAcceleration(acceleration);
  
//   Serial.println("Stepper motor initialized");
// }

// /**
//  * 步进电机控制函数
//  * @param stepper 步进电机对象
//  * @param speed 最大速度(步/秒)
//  * @param accel 加速度(步/秒²)
//  * @param steps 移动的步数(正值为正向，负值为反向)
//  * @return 启动成功返回true
//  */
// bool controlStepper(AccelStepper &stepper, float speed, float accel, int32_t steps) {
//   // 设置速度和加速度（不超过上限）
//   stepper.setMaxSpeed(speed > maxSpeed ? maxSpeed : speed);
//   stepper.setAcceleration(accel > acceleration ? acceleration : accel);
  
//   // 计算目标位置（相对当前位置移动指定步数）
//   long currentPosition = stepper.currentPosition();
//   long targetPosition = currentPosition + steps;
//   stepper.moveTo(targetPosition);
  
//   Serial.printf("Stepper running: Speed=%.2f, Accel=%.2f, Steps=%ld\n",
//               speed, accel, steps);
  
//   return true;
// }

// /**
//  * 步进电机循环处理函数，必须在loop中调用
//  */
// void stepper_loop(void) {
//   stepper1.run();
// } 