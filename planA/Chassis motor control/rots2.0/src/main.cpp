#include <AccelStepper.h>
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stepper.h>
#include <sensor.h>

TaskHandle_t task1Handle, task2Handle;

void setup()
{
    Serial.begin(115200);
    xTaskCreatePinnedToCore(task1, "Task1", 4000, NULL, 1, &task1Handle, 0);
    // 创建任务2
    xTaskCreatePinnedToCore(task2, "Task2", 4000, NULL, 1, &task2Handle, 1);
}

void loop()
{

}