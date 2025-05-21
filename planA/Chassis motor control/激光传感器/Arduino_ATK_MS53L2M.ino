/**
 ****************************************************************************************************
 * @file        ATK_MS53L2M_ESP32.ino
 * @brief       ATK-MS53L2M激光传感器ESP32S3移植版（WiFi + 网页显示）
 * @note        支持ASCII接口模式，提供WiFi AP和网页界面
 * @version     V2.0
 * @date        2024-05-23
 ****************************************************************************************************
 */

// 引入必要的库
#include <WiFi.h>
#include <WebServer.h>

/* 引脚定义 - 根据ESP32S3板子实际情况调整 */
#define RX_PIN 18             /* 传感器TXD连接到ESP32S3的RX */
#define TX_PIN 17             /* 传感器RXD连接到ESP32S3的TX */
#define SENSOR_SERIAL Serial2 /* 使用ESP32S3的Serial2 */
#define LED_PIN 2             /* 板载LED引脚 */

/* WiFi设置 */
#define WIFI_SSID "LaserSensor_AP" // WiFi名称
#define WIFI_PASSWORD "12345678"   // WiFi密码(至少8位)

/* 错误码定义 */
#define ATK_MS53L2M_EOK 0      /* 操作成功 */
#define ATK_MS53L2M_ERROR 1    /* 操作失败 */
#define ATK_MS53L2M_ETIMEOUT 2 /* 超时错误 */

/* 缓冲区定义 */
#define RX_BUF_SIZE 128 /* 接收缓冲区大小 */
char g_rx_buf[RX_BUF_SIZE];
uint16_t g_rx_cnt = 0;

// 当前距离值
uint16_t current_distance = 0;

// 创建网页服务器对象
WebServer server(80);

/**
 * @brief       清空接收缓冲区
 * @param       无
 * @retval      无
 */
void atk_ms53l2m_uart_rx_restart(void)
{
  while (SENSOR_SERIAL.available())
  {
    SENSOR_SERIAL.read();
  }
  g_rx_cnt = 0;
}

/**
 * @brief       读取传感器距离
 * @param       distance: 指向距离存储变量的指针
 * @retval      ATK_MS53L2M_EOK或错误码
 */
uint8_t atk_ms53l2m_get_distance(uint16_t *distance)
{
  /* 清空接收缓冲区 */
  atk_ms53l2m_uart_rx_restart();
  memset(g_rx_buf, 0, RX_BUF_SIZE);

  /* 等待接收数据 */
  uint32_t start_time = millis();
  g_rx_cnt = 0;
  bool data_received = false;

  /* 接收一段时间内的所有数据 */
  while (millis() - start_time < 1000)
  {
    while (SENSOR_SERIAL.available() && g_rx_cnt < RX_BUF_SIZE - 1)
    {
      char c = SENSOR_SERIAL.read();
      g_rx_buf[g_rx_cnt++] = c;
      data_received = true;
    }

    /* 如果已经收到一些数据并且停止接收一段时间，认为一帧结束 */
    if (data_received && !SENSOR_SERIAL.available())
    {
      delay(50);
      if (!SENSOR_SERIAL.available())
      {
        break;
      }
    }

    delay(5);
  }

  /* 确保字符串结束 */
  g_rx_buf[g_rx_cnt] = '\0';

  /* 如果没有接收到数据，返回超时错误 */
  if (g_rx_cnt == 0)
  {
    return ATK_MS53L2M_ETIMEOUT;
  }

  /* 专门查找"d:"后面的数字 */
  char *d_pos = strstr(g_rx_buf, "d:");
  if (d_pos != NULL)
  {
    d_pos += 2; // 跳过"d:"

    /* 跳过空格 */
    while (*d_pos == ' ' && *d_pos != '\0')
    {
      d_pos++;
    }

    /* 读取数字 */
    if (isdigit(*d_pos))
    {
      *distance = atoi(d_pos);
      return ATK_MS53L2M_EOK;
    }
  }

  /* 如果没有找到"d:"，尝试直接查找数字 */
  char *ptr = g_rx_buf;
  while (*ptr)
  {
    if (isdigit(*ptr) && (ptr == g_rx_buf || !isdigit(*(ptr - 1))))
    {
      *distance = atoi(ptr);
      if (*distance > 0 && *distance < 10000) // 合理的距离范围
      {
        return ATK_MS53L2M_EOK;
      }
    }
    ptr++;
  }

  return ATK_MS53L2M_ERROR;
}

/**
 * @brief       初始化LED指示灯
 * @param       无
 * @retval      无
 */
void led_init(void)
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

/**
 * @brief       LED指示灯切换状态
 * @param       无
 * @retval      无
 */
void led_toggle(void)
{
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

/**
 * @brief       初始化WiFi AP模式
 * @param       无
 * @retval      无
 */
void initWiFi()
{
  // 配置静态IP地址
  IPAddress local_IP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  // 设置ESP32的WiFi AP模式
  WiFi.mode(WIFI_AP);

  // 设置静态IP
  WiFi.softAPConfig(local_IP, gateway, subnet);

  // 启动AP
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("WiFi AP模式已启动");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);
  Serial.print("密码: ");
  Serial.println(WIFI_PASSWORD);
  Serial.print("IP地址: ");
  Serial.println(WiFi.softAPIP());
}

/**
 * @brief       处理网页根路径请求
 * @param       无
 * @retval      无
 */
void handleRoot()
{
  String html = "<!DOCTYPE HTML><html>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>激光传感器监控</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f0f2f5; color: #333; }";
  html += ".container { max-width: 800px; margin: 0 auto; padding: 20px; }";
  html += ".header { text-align: center; padding: 20px 0; background-color: #1890ff; color: white; border-radius: 8px 8px 0 0; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }";
  html += ".card { background-color: white; border-radius: 0 0 8px 8px; padding: 20px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); margin-bottom: 20px; }";
  html += ".value-box { font-size: 72px; text-align: center; margin: 30px 0; color: #1890ff; font-weight: bold; }";
  html += ".unit { font-size: 24px; color: #888; }";
  html += ".footer { text-align: center; margin-top: 20px; color: #888; font-size: 14px; }";
  html += ".status { display: inline-block; width: 10px; height: 10px; border-radius: 50%; background-color: #52c41a; margin-right: 5px; }";
  html += ".status-text { font-size: 16px; color: #555; text-align: center; margin-bottom: 20px; }";
  html += "</style>";
  html += "<script>";
  html += "function updateDistance() {";
  html += "  fetch('/distance')";
  html += "    .then(response => response.text())";
  html += "    .then(data => {";
  html += "      document.getElementById('distance').innerText = data;";
  html += "    });";
  html += "}";
  html += "setInterval(updateDistance, 500);"; // 每500ms更新一次
  html += "</script>";
  html += "</head>";
  html += "<body>";
  html += "<div class='container'>";
  html += "<div class='header'>";
  html += "<h1>ATK-MS53L2M 激光传感器</h1>";
  html += "</div>";
  html += "<div class='card'>";
  html += "<div class='status-text'><span class='status'></span>系统状态: 正常</div>";
  html += "<div class='value-box'><span id='distance'>";
  html += current_distance;
  html += "</span> <span class='unit'>mm</span></div>";
  html += "</div>";
  html += "<div class='footer'>ESP32S3 + ATK-MS53L2M &copy; 2024</div>";
  html += "</div>";
  html += "</body>";
  html += "</html>";

  server.send(200, "text/html", html);
}

/**
 * @brief       处理距离数据请求
 * @param       无
 * @retval      无
 */
void handleDistance()
{
  server.send(200, "text/plain", String(current_distance));
}

/**
 * @brief       初始化Web服务器
 * @param       无
 * @retval      无
 */
void initWebServer()
{
  server.on("/", handleRoot);
  server.on("/distance", handleDistance);

  server.begin();
  Serial.println("Web服务器已启动");
  Serial.println("访问地址: http://192.168.4.1");
}

void setup()
{
  /* 初始化调试串口 */
  Serial.begin(115200);
  delay(500);
  Serial.println("\r\n========== ATK-MS53L2M 激光传感器 + WiFi网页监控 ==========");

  /* 初始化LED */
  led_init();

  /* 初始化传感器串口 */
  SENSOR_SERIAL.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  delay(200);

  /* 清空可能的缓存数据 */
  atk_ms53l2m_uart_rx_restart();

  /* 初始化WiFi AP */
  initWiFi();

  /* 初始化Web服务器 */
  initWebServer();

  Serial.println("系统初始化完成");
}

void loop()
{
  static unsigned long last_read_time = 0;
  uint16_t distance;
  uint8_t ret;

  /* 处理Web服务器请求 */
  server.handleClient();

  /* 每200ms读取一次距离 */
  if (millis() - last_read_time > 200)
  {
    last_read_time = millis();

    /* 读取距离 */
    ret = atk_ms53l2m_get_distance(&distance);
    if (ret == ATK_MS53L2M_EOK)
    {
      current_distance = distance;
      led_toggle(); /* 成功读取一次数据则切换LED状态 */
    }
  }

  /* 小延迟，避免占用过多CPU */
  delay(10);
}