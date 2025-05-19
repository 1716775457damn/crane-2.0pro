#include <AccelStepper.h>
// 步进电机1参数
#define STEP_PIN_1 18
#define DIR_PIN_1 19
#define ENABLE_PIN_1 21

// 步进电机2参数
#define STEP_PIN_2 16
#define DIR_PIN_2 17
#define ENABLE_PIN_2 5

// 定义步进电机对象
AccelStepper stepper1(AccelStepper::DRIVER, STEP_PIN_1, DIR_PIN_1);
AccelStepper stepper2(AccelStepper::DRIVER, STEP_PIN_2, DIR_PIN_2);

void controlStepper(AccelStepper &stepper, int &maxSpeed, int &acceleration, int &stepsToMove)
{
    pinMode(ENABLE_PIN_1, OUTPUT);
    pinMode(ENABLE_PIN_2, OUTPUT);
    digitalWrite(ENABLE_PIN_1, LOW);
    digitalWrite(ENABLE_PIN_2, LOW);
    // 设置步进电机参数
    stepper.setMaxSpeed(maxSpeed);
    stepper.setAcceleration(acceleration);
    stepper.moveTo(stepsToMove);

    // 运行电机直到到达目标位置
    while (stepper.distanceToGo() != 0)
    {
        stepper.run();
    }

}

void task1(void *pvParameters)
{
    int maxSpeed1 = 1000;
    int acceleration1 = 2000;
    int stepsToMove1 = 2000;
    for (;;)
    {
        controlStepper(stepper1, maxSpeed1, acceleration1, stepsToMove1);
        }
}

void task2(void *pvParameters)
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