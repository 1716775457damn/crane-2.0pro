#include <AccelStepper.h>
// 步进电机1参数
#define STEP_PIN_1 18
#define DIR_PIN_1 8
#define SLEEP_PIN_1 3

// 步进电机2参数
#define STEP_PIN_2 15
#define DIR_PIN_2 16
#define SLEEP_PIN_2 17

// 电源电压检测引脚
#define ADC_PIN 7

// 定义步进电机对象
AccelStepper stepper1(AccelStepper::DRIVER, STEP_PIN_1, DIR_PIN_1); // 修正构造函数参数
AccelStepper stepper2(AccelStepper::DRIVER, STEP_PIN_2, DIR_PIN_2); // 修正构造函数参数

// 读取电源电压
int Get_ADC_Average(char times)
{
  long int temp = 0;
  char i;
  for(i=0; i<times; i++)
  {
    temp += analogRead(ADC_PIN);
  }
  return (int)temp/times;
}

void controlStepper(AccelStepper &stepper, float speed, float acceleration, int steps)
{
    // 步进电机1引脚初始化
    pinMode(SLEEP_PIN_1, OUTPUT);
    pinMode(DIR_PIN_1, OUTPUT);
    pinMode(STEP_PIN_1, OUTPUT);
    
    // 步进电机2引脚初始化
    pinMode(SLEEP_PIN_2, OUTPUT);
    pinMode(DIR_PIN_2, OUTPUT);
    pinMode(STEP_PIN_2, OUTPUT);
    
    // 唤醒步进电机
    digitalWrite(SLEEP_PIN_1, HIGH);
    digitalWrite(SLEEP_PIN_2, HIGH);
    
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

// 读取电压任务
void voltage_task(void *pvParameters)
{
    for (;;)
    {
        long int ADC_Val = Get_ADC_Average(5);                      // 每五次取一次ADC的平均值
        long int Voltage_Val = (long int)(ADC_Val*5*11*100/1023);   // 转换为电源电压值
        Serial.print("当前电池电压为：");
        Serial.print(Voltage_Val/100);
        Serial.print(".");
        Serial.print(Voltage_Val%100);
        Serial.println("V");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
