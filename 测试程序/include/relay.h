#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>
#include "pins.h" // 引入统一的引脚定义

// 检查继电器引脚是否已定义
#ifndef RELAY_PIN_1
#define RELAY_PIN_1 -1 // 未定义时设为无效引脚
#endif

#ifndef RELAY_PIN_2
#define RELAY_PIN_2 -1 // 未定义时设为无效引脚
#endif

// 继电器状态定义
#define RELAY_ON LOW   // 低电平触发继电器（常见的继电器模块是低电平触发）
#define RELAY_OFF HIGH // 高电平关闭继电器

/**
 * 初始化继电器
 * 将继电器引脚设置为输出模式并初始化为关闭状态
 */
void initRelays()
{
  // 只有在引脚有效时才初始化
  if (RELAY_PIN_1 >= 0)
  {
    pinMode(RELAY_PIN_1, OUTPUT);
    digitalWrite(RELAY_PIN_1, RELAY_OFF);
    Serial.println("Relay 1 initialized on pin " + String(RELAY_PIN_1));
  }
  else
  {
    Serial.println("Relay 1 disabled (pin not defined)");
  }

  if (RELAY_PIN_2 >= 0)
  {
    pinMode(RELAY_PIN_2, OUTPUT);
    digitalWrite(RELAY_PIN_2, RELAY_OFF);
    Serial.println("Relay 2 initialized on pin " + String(RELAY_PIN_2));
  }
  else
  {
    Serial.println("Relay 2 disabled (pin not defined)");
  }
}

/**
 * 控制继电器1的开关状态
 * @param state: 继电器状态，true表示打开，false表示关闭
 */
void controlRelay1(bool state)
{
  if (RELAY_PIN_1 >= 0)
  {
    digitalWrite(RELAY_PIN_1, state ? RELAY_ON : RELAY_OFF);
  }
  else
  {
    Serial.println("Warning: Relay 1 pin not defined, cannot control");
  }
}

/**
 * 控制继电器2的开关状态
 * @param state: 继电器状态，true表示打开，false表示关闭
 */
void controlRelay2(bool state)
{
  if (RELAY_PIN_2 >= 0)
  {
    digitalWrite(RELAY_PIN_2, state ? RELAY_ON : RELAY_OFF);
  }
  else
  {
    Serial.println("Warning: Relay 2 pin not defined, cannot control");
  }
}

/**
 * 打开继电器一段时间后关闭
 * @param relayPin: 继电器引脚
 * @param onTime: 保持打开的时间(毫秒)
 */
void pulseRelay(int relayPin, unsigned long onTime)
{
  digitalWrite(relayPin, RELAY_ON);
  delay(onTime);
  digitalWrite(relayPin, RELAY_OFF);
}

/**
 * 控制继电器执行指定次数的脉冲操作
 * @param relayPin: 继电器引脚
 * @param pulseCount: 脉冲次数
 * @param onTime: 每次脉冲打开时间(毫秒)
 * @param offTime: 每次脉冲关闭时间(毫秒)
 */
void relayPulseSequence(int relayPin, int pulseCount, unsigned long onTime, unsigned long offTime)
{
  for (int i = 0; i < pulseCount; i++)
  {
    digitalWrite(relayPin, RELAY_ON);
    delay(onTime);
    digitalWrite(relayPin, RELAY_OFF);
    if (i < pulseCount - 1)
    {
      delay(offTime);
    }
  }
}

/**
 * 在FreeRTOS中使用继电器的任务示例
 * 此任务将周期性地打开和关闭继电器
 */
void relayTask(void *pvParameters)
{
  // 获取任务参数（继电器引脚）
  int relayPin = *((int *)pvParameters);

  for (;;)
  {
    // 打开继电器
    digitalWrite(relayPin, RELAY_ON);
    vTaskDelay(pdMS_TO_TICKS(1000)); // 延时1秒

    // 关闭继电器
    digitalWrite(relayPin, RELAY_OFF);
    vTaskDelay(pdMS_TO_TICKS(2000)); // 延时2秒
  }
}

/**
 * 启动继电器控制任务
 * @param relayPin: 继电器引脚
 */
void startRelayTask(int relayPin)
{
  // 为任务参数分配静态内存
  static int pin1 = RELAY_PIN_1;
  static int pin2 = RELAY_PIN_2;

  int *pinPtr = (relayPin == RELAY_PIN_1) ? &pin1 : &pin2;

  // 创建继电器控制任务
  TaskHandle_t relayTaskHandle;
  xTaskCreatePinnedToCore(
      relayTask,        // 任务函数
      "RelayTask",      // 任务名称
      2000,             // 栈大小
      (void *)pinPtr,   // 任务参数
      1,                // 任务优先级
      &relayTaskHandle, // 任务句柄
      0                 // 运行的核心
  );
}

#endif // RELAY_H