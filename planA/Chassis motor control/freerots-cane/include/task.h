// #include<sensor.h>
// #include<stepper.h>
#include<servo.h>
// #include <StepperControl.h>
#include "StepperMotor.h"
// #include<relay.h>
#include<laser.h>
void task_0(void *pvParameters);
void task_1(void *pvParameters);
void task_101(void *pvParameters);
void task_102(void *pvParameters);
void task_103(void *pvParameters);
void task_104(void *pvParameters);
void task_301(void *pvParameters);
void task_302(void *pvParameters);
void task_first(void *pvParameters);
void task_second(void *pvParameters);
void task_fourth(void *pvParameters);
void task_fifth(void *pvParameters);
void task_third(void *pvParameters);
void task_00(void *pvParameters);
void task_001(void *pvParameters);
//初始化
// int a = 0;
// int e = 0;
// int amount = 0;

void task_00(void *pvParameters)
{
    // 初始化步进电机
    initSteppers();
    
    while (1)
    {
        // 启动电机 - 一行代码控制：电机号,速度,步数(正数顺时针，负数逆时针)
        stepper(1, 100, 1);  // 电机1持续运行(正转)
        stepper(2, 100, 1);  // 电机2持续运行(正转)

        Serial.println("Both motors are running...");

        vTaskDelay(5000 / portTICK_PERIOD_MS); // 让电机持续运行5秒

        // 停止电机
        stopStepper(1);
        stopStepper(2);

        Serial.println("Both motors have stopped.");

        vTaskDelay(1000 / portTICK_PERIOD_MS); // 停止后等待1秒
        
        // 电机1正转2秒，电机2反转2秒
        stepper(1, 100, 200);   // 电机1以速度100顺时针转动2秒
        stepper(2, 100, -200);  // 电机2以速度100逆时针转动2秒
        
        Serial.println("Motors running in opposite directions...");
        
        // 等待电机自动停止(无需手动调用updateSteppers)
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        
        Serial.println("Motors stopped automatically after 2 seconds.");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
// void task_00(void *pvParameters)
// {
//     initSteppers();
//     enableStepper(1);
//     setStepperDirection(1, DIRECTION_CW);
//     setStepperSpeed(1, 100);

//     Serial.begin(115200);
//     Serial.println("Initializing steppers...");

//     xTaskCreate(task_00, "Task_00", 2000, NULL, 1, &task1Handle);
    // servo1(105);
    // // pulseRelay(10, 1000);
    // delay(1000);
    // // if (laser_on())
    // // {
    // //     Serial.print("1,10,10");
    // //     delay(30);
    // stepper(1, 100, 0, true); // 速度降低到 100
    // enableStepper(1);
    // delay(1000); // 等待电机稳定
    // // xTaskCreatePinnedToCore(task_0, "Task_0", 2000, NULL, 1, NULL, 0);
    // vTaskDelete(NULL);
// }
// void task_001(void *pvParameters)
// {

//     // e += 1;
//     // switch (e)
//     // {
//     // case 1:
//     //     xTaskCreatePinnedToCore(task_301, "Task301", 2000, NULL, 1, NULL, 0);//左
//     //     e += 1;
//     //     vTaskDelete(NULL);
//     //     break;
//     // case 2: 
//     //     xTaskCreatePinnedToCore(task_302, "Task302", 2000, NULL, 1, NULL, 0);//右
//     //     e += 1;
//     //     vTaskDelete(NULL);
//     //     break;
//     // case 3:
//     //     xTaskCreatePinnedToCore(task_102, "Task102", 2000, NULL, 1, NULL, 0);
//     //     e += 1;
//     //     vTaskDelete(NULL);
//     //     break;
//     // }
//     //     xTaskCreatePinnedToCore(task_0, "Task_0", 2000, NULL, 1, NULL, 0);
//     //     vTaskDelete(NULL);
// }   

// void task_0(void *pvParameters)
// {
//     // while (1)
//     // {
//     //     chaosheng = measureDistanceAndSetState();
//     //     if (chaosheng == 1)
//     //     {
//     //         b = tracing();
//     //         if (b = 3)
//     //         {
//     //             xTaskCreatePinnedToCore(task_001, "Task001", 2000, NULL, 1, NULL, 0);
//     //             vTaskDelete(NULL);
//     //         }
//     //     }
//     //     else if (chaosheng == 0) // 假设当value达到10时任务1完成
//     //     {
//     //         switch (a)
//     //         {
//     //         case 0:
//     //             Serial.print("0,10,10");
//     //             delay(30);
//     //             xTaskCreatePinnedToCore(task_1, "Task1", 2000, NULL, 1, NULL, 0);//勾1.0
//     //             a += 1;
//     //             vTaskDelete(NULL);
//     //             break;
//     //         case 1:
//     //             xTaskCreatePinnedToCore(task_first, "Task_first", 2000, NULL, 1, NULL, 1);//放1.0
//     //             a += 1;
//     //             vTaskDelete(NULL);
//     //             break;
//     //         case 2:
//     //             xTaskCreatePinnedToCore(task_second, "Task_second", 2000, NULL, 1, NULL, 0);//存储2.0
//     //             a += 1;
//     //             vTaskDelete(NULL);
//     //             break;
//     //         case 3:
//     //             xTaskCreatePinnedToCore(task_third, "Task_0", 2000, NULL, 1, NULL, 1);//勾，升3.0
//     //             a += 1;            
//     //             vTaskDelete(NULL);
//     //             break;
//     //         case 4:
//     //             xTaskCreatePinnedToCore(task_fourth, "Task_0", 2000, NULL, 1, NULL, 1);//放2.0
//     //             a += 1;
//     //             vTaskDelete(NULL);
//     //             break;
//     //         case 5:
//     //             xTaskCreatePinnedToCore(task_fifth, "Task_0", 2000, NULL, 1, NULL, 1);//放3.0
//     //             a += 1;
//     //             vTaskDelete(NULL);
//     //             break;
//     //         default:
//     //             break;
//     //         }
//     //     }
//     // }
// }
// void task_1(void *pvParameters)
// {
//     servo1(65);
//     delay(30);
//     controlStepper(stepper1, 950, 1800, 3700);
//     xTaskCreatePinnedToCore(task_0, "Task_0", 2000, NULL, 1, NULL, 0);
//     vTaskDelete(NULL);
// }


// void task_first(void *pvParameters)
// {
//     Serial.print("0,10,10");
//     delay(30);
//     controlStepper(stepper1, 2000, 1000, 2000);
//     servo1(105);
//     delay(500);
//     Serial.print("5,8000,8"); // 后退
//     delay(2300);
//     Serial.print("8,9200,15"); // 右
//     delay(4500);
//     controlStepper(stepper1, 1000, 1000, 0);
//     // xTaskCreatePinnedToCore(task_101, "Task1", 4000, NULL, 1, NULL, 0);
//     // xTaskCreatePinnedToCore(task_102, "Task2", 4000, NULL, 1, NULL, 1);
//     xTaskCreatePinnedToCore(task_0, "Task_0", 2000, NULL, 1, NULL, 0);
//     vTaskDelete(NULL);
// }
// void task_101(void *pvParameters){
//     controlStepper(stepper1, 950, 2000, 2900); // up
//     vTaskDelete(NULL);
// }
// void task_102(void *pvParameters)
// {
//     delay(6000);
//     controlStepper(stepper2, 2000, 2000, -2600);
//     vTaskDelete(NULL);
// }
// void task_103(void *pvParameters)
// {
//     controlStepper(stepper1, 2000, 2000, 0); // down
//     vTaskDelete(NULL);
// }
// void task_104(void *pvParameters)
// {
//     controlStepper(stepper2, 2000, 2000, 0);
//     vTaskDelete(NULL);
// }

// void task_second(void *pvParameters)
// {
//     Serial.print("0,10,10");
//     delay(30);
//     servo1(65);
//     // controlStepper(stepper1, 950, 1800, 2900);//up
//     // controlStepper(stepper2, 1800, 1800, -2600);
//     xTaskCreatePinnedToCore(task_101, "Task1", 4000, NULL, 1, NULL, 0);
//     xTaskCreatePinnedToCore(task_102, "Task2", 4000, NULL, 1, NULL, 1);
//     controlStepper(stepper1, 1800, 1800, 2000);
//     servo1(105);
//     // controlStepper(stepper1, 1800, 1800, 0);//down
//     // controlStepper(stepper2, 1800, 1800, 0);
//     xTaskCreatePinnedToCore(task_103, "Task3", 4000, NULL, 1, NULL, 0);
//     xTaskCreatePinnedToCore(task_104, "Task4", 4000, NULL, 1, NULL, 1);

//     xTaskCreatePinnedToCore(task_0, "Task_0", 2000, NULL, 1, NULL, 0);
//     vTaskDelete(NULL);
// }
// void task_third(void *pvParameters){
//     Serial.print("0,10,10");
//     delay(30);
//     servo1(105);
//     controlStepper(stepper1, 950, 2000, 6000); // up
// }
// void task_301(void *pvParameters)
// {
//     Serial.print("7,4600,15");
//     xTaskCreatePinnedToCore(task_0, "Task_0", 2000, NULL, 1, NULL, 1);
//     vTaskDelete(NULL);
// }
// void task_302(void *pvParameters)
// {
//     Serial.print("8,4600,15");
//     xTaskCreatePinnedToCore(task_0, "Task_0", 2000, NULL, 1, NULL, 1);
//     vTaskDelete(NULL);
// }
// void task_fourth(void *pvParameters){
//     Serial.print("0,10,10");
//     delay(30);
//     controlStepper(stepper1, 2000, 1000, 4700); // down
//     delay(500);
//     servo1(105);
//     xTaskCreatePinnedToCore(task_0, "Task_0", 2000, NULL, 1, NULL, 1);
//     vTaskDelete(NULL);
// }
// void task_fifth(void *pvParameters){
//     Serial.print("0,10,10");
//     delay(30000);
    
// }