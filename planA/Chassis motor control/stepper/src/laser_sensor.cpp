#include "laser_sensor.h"

// 引脚定义
#define RX_PIN 18             // 传感器TXD连接到ESP32S3的RX
#define TX_PIN 17             // 传感器RXD连接到ESP32S3的TX
#define SENSOR_SERIAL Serial2 // 使用ESP32S3的Serial2
#define LED_PIN 2             // 板载LED引脚

// 缓冲区定义
#define RX_BUF_SIZE 64 // 减小缓冲区以节省内存
static char g_rx_buf[RX_BUF_SIZE];
static uint16_t g_rx_cnt = 0;
static uint8_t g_last_status = LASER_SENSOR_EOK;

// 清空接收缓冲区
static void uart_rx_restart(void)
{
    while (SENSOR_SERIAL.available())
    {
        SENSOR_SERIAL.read();
    }
    g_rx_cnt = 0;
}

// 初始化LED指示灯
static void led_init(void)
{
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
}

// LED指示灯切换状态
static void led_toggle(void)
{
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

// 初始化激光传感器
void laser_sensor_init(void)
{
    // 初始化LED
    led_init();

    // 初始化传感器串口
    SENSOR_SERIAL.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
    delay(100); // 减少延迟时间

    // 清空可能的缓存数据
    uart_rx_restart();

    // 测试LED指示灯
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(LED_PIN, HIGH);
        delay(50);
        digitalWrite(LED_PIN, LOW);
        delay(50);
    }

    Serial.println("[LASER] Initialization complete");
}

// 读取传感器距离
uint8_t laser_sensor_read(uint16_t *distance)
{
    if (distance == NULL)
    {
        g_last_status = LASER_SENSOR_EINVAL;
        return g_last_status;
    }

    static uint32_t last_success_time = 0;

    // 清空接收缓冲区
    uart_rx_restart();
    memset(g_rx_buf, 0, RX_BUF_SIZE);

    // 减少等待时间，使用更高效的方式接收数据
    uint32_t start_time = millis();
    bool data_received = false;
    g_rx_cnt = 0;

    while ((millis() - start_time) < 300)
    { // 减少超时时间
        if (SENSOR_SERIAL.available())
        {
            while (SENSOR_SERIAL.available() && g_rx_cnt < RX_BUF_SIZE - 1)
            {
                g_rx_buf[g_rx_cnt++] = SENSOR_SERIAL.read();
                data_received = true;
            }

            if (data_received)
            {
                // 收到数据后等待短暂时间看是否有更多数据
                delay(30);
                if (!SENSOR_SERIAL.available())
                {
                    break;
                }
            }
        }
        delay(2); // 减少循环延迟
    }

    // 确保字符串结束
    g_rx_buf[g_rx_cnt] = '\0';

    // 如果没有接收到数据，返回超时错误
    if (g_rx_cnt == 0)
    {
        g_last_status = LASER_SENSOR_ETIMEOUT;
        return g_last_status;
    }

    // 直接查找"d:"后面的数字
    char *d_pos = strstr(g_rx_buf, "d:");
    if (d_pos != NULL)
    {
        d_pos += 2; // 跳过"d:"
        while (*d_pos == ' ' && *d_pos != '\0')
            d_pos++;

        if (isdigit(*d_pos))
        {
            *distance = atoi(d_pos);

            // 检查数据范围合理性
            if (*distance > 10000)
            {
                g_last_status = LASER_SENSOR_ERANGE;
                return g_last_status;
            }

            // 只在值变化或间隔较长时输出调试信息
            static uint16_t last_distance = 0;
            if (*distance != last_distance || (millis() - last_success_time) > 3000)
            {
                Serial.printf("[LASER] Distance: %u mm\n", *distance);
                last_distance = *distance;
                last_success_time = millis();
            }
            led_toggle(); // 成功读取后切换LED
            g_last_status = LASER_SENSOR_EOK;
            return g_last_status;
        }
    }

    // 备用解析方式
    char *ptr = g_rx_buf;
    while (*ptr)
    {
        if (isdigit(*ptr) && (ptr == g_rx_buf || !isdigit(*(ptr - 1))))
        {
            *distance = atoi(ptr);
            if (*distance > 0 && *distance < 10000)
            {
                Serial.printf("[LASER] Distance (backup method): %u mm\n", *distance);
                led_toggle();
                g_last_status = LASER_SENSOR_EOK;
                return g_last_status;
            }
        }
        ptr++;
    }

    // 偶尔打印错误信息并显示收到的数据
    static uint32_t last_error_time = 0;
    if (millis() - last_error_time > 5000)
    {
        Serial.printf("[LASER] Parse error, data: %s\n", g_rx_buf);
        last_error_time = millis();
    }

    g_last_status = LASER_SENSOR_ERROR;
    return g_last_status;
}

// 设置LED状态
void laser_set_led(bool state)
{
    digitalWrite(LED_PIN, state ? HIGH : LOW);
}

// 获取最后一次传感器状态
uint8_t laser_sensor_get_status(void)
{
    return g_last_status;
}