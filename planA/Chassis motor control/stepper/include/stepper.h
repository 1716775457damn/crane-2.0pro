#include <AccelStepper.h>
// 步进电机1参数
#define STEP_PIN_1 14
#define DIR_PIN_1 12
#define ENABLE_PIN_1 13

// 步进电机2参数
// #define STEP_PIN_2 16
// #define DIR_PIN_2 17
// #define ENABLE_PIN_2 5


// 定义步进电机对象
AccelStepper stepper1(AccelStepper::DRIVER, STEP_PIN_1, DIR_PIN_1); // 修正构造函数参数
// AccelStepper stepper2(AccelStepper::DRIVER, STEP_PIN_2, DIR_PIN_2); // 修正构造函数参数

void controlStepper(AccelStepper &stepper, float speed, float acceleration, int steps)
{
    pinMode(ENABLE_PIN_1, OUTPUT);
    pinMode(ENABLE_PIN_2, OUTPUT);
    digitalWrite(ENABLE_PIN_1, LOW);
    digitalWrite(ENABLE_PIN_2, LOW);
    stepper.setMaxSpeed(speed);
    stepper.setAcceleration(acceleration);
    stepper.moveTo(steps); // 设置目标位置

    // 运行电机直到到达目标位置或发生错误
    while (stepper.distanceToGo() != 0)
    {
        stepper.run();
    }
}

// void hook()
// {
//     controlStepper(stepper1, 1150, 2000, 1000); // up
//     controlStepper(stepper2, 2000, 2000, 2000); // right
//     controlStepper(stepper1, 2000, 2000, 900);  // down
//     controlStepper(stepper1, 970, 2000, 1000);  // up
// }
// void place()
// {
//     // controlStepper(stepper1, 2000, 2000, stepsToMove1); // down
//     // controlStepper(stepper1, 970, 2000, stepsToMove1);  // up
//     // controlStepper(stepper2, 2000, 2000, stepsToMove1); // left
//     // controlStepper(stepper1, 2000, 1000, stepsToMove1); // down
//     // controlStepper(stepper1, 1150, 2000, stepsToMove1); // up
//     // controlStepper(stepper2, 2000, 2000, stepsToMove1); // right
//     // controlStepper(stepper1, 2000, 1000, stepsToMove1); // down
// }
void task3(void *pvParameters)
{
    int maxSpeed1 = 1000;
    int acceleration1 = 2000;
    int stepsToMove1 = 2000;
    for (;;)
    {
        controlStepper(stepper1, maxSpeed1, acceleration1, stepsToMove1);
    }
}

void task4(void *pvParameters)
{
    // 任务1实现，使用之前定义的参数
    int maxSpeed2 = 1000;
    int acceleration2 = 2000;
    int stepsToMove2 = 2000;
    for (;;)
    {
        controlStepper(stepper2, maxSpeed2, acceleration2, stepsToMove2);
        // 任务1每2秒执行一次
        // vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
