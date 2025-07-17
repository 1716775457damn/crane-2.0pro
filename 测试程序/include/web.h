#ifndef _WEB_H_
#define _WEB_H_

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "laser.h"
#include <Adafruit_NeoPixel.h>
#include "html_page.h" // 包含HTML页面定义

// WiFi credentials - 使用外部定义的变量
extern const char *ssid;     // AP name (defined in wifi_config.h)
extern const char *password; // AP password (defined in wifi_config.h)

// Fixed IP configuration
IPAddress local_IP(192, 168, 4, 1); // Device IP
IPAddress gateway(192, 168, 4, 1);  // Gateway
IPAddress subnet(255, 255, 255, 0); // Subnet mask

// LED pin for distance indication
#define LED_PIN 48 // ESP32-S3 开发板的板载 RGB LED 引脚号
bool led_state = false;

// 使用在task.h中定义的NeoPixel对象和颜色
extern Adafruit_NeoPixel pixels;

// WiFi modes
enum WiFiModeCustom
{
  WIFI_MODE_CUSTOM_AP,    // Access Point mode
  WIFI_MODE_CUSTOM_STA,   // Station mode
  WIFI_MODE_CUSTOM_AP_STA // Mixed mode
};

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncEventSource events("/events");

// Laser sensor data
float web_current_distance = 0; // Renamed to avoid conflict with laser.h
unsigned long last_update = 0;
bool obstacle_detected = false;

// Statistics for debugging
int valid_readings_count = 0;
int error_readings_count = 0;
unsigned long last_valid_reading_time = 0;

/**
 * Setup WiFi in AP mode
 */
void setupWiFiAP()
{
  // Configure ESP32 as access point
  WiFi.mode(WIFI_AP);

  // Configure AP IP address
  WiFi.softAPConfig(local_IP, gateway, subnet);

  // Start AP
  WiFi.softAP(ssid, password);

  Serial.print("AP mode started, IP address: ");
  Serial.println(WiFi.softAPIP());
}

/**
 * Setup WiFi in STA mode
 */
void setupWiFiSTA(const char *sta_ssid, const char *sta_password)
{
  // Make sure STA mode credentials are provided
  if (sta_ssid == NULL || sta_password == NULL)
  {
    Serial.println("Error: STA mode requires WiFi credentials");
    return;
  }

  // Configure ESP32 as station
  WiFi.mode(WIFI_STA);

  // Configure static IP
  if (!WiFi.config(local_IP, gateway, subnet))
  {
    Serial.println("STA mode static IP configuration failed");
  }

  // Connect to WiFi
  WiFi.begin(sta_ssid, sta_password);

  // Wait for connection
  Serial.print("Connecting to WiFi...");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("");
    Serial.println("Connection failed, please check WiFi credentials");
  }
}

/**
 * Setup WiFi in AP_STA mixed mode
 */
void setupWiFiAPSTA(const char *sta_ssid, const char *sta_password)
{
  // Configure ESP32 as mixed mode
  WiFi.mode(WIFI_AP_STA);

  // Configure AP IP address
  WiFi.softAPConfig(local_IP, gateway, subnet);

  // Start AP
  WiFi.softAP(ssid, password);

  Serial.print("AP mode started, IP address: ");
  Serial.println(WiFi.softAPIP());

  // If STA credentials are provided, connect to WiFi
  if (sta_ssid != NULL && sta_password != NULL)
  {
    // Configure static IP
    IPAddress sta_local_IP(192, 168, 1, 200); // Different subnet IP
    IPAddress sta_gateway(192, 168, 1, 1);
    IPAddress sta_subnet(255, 255, 255, 0);

    if (!WiFi.config(sta_local_IP, sta_gateway, sta_subnet))
    {
      Serial.println("STA mode static IP configuration failed");
    }

    // Connect to WiFi
    WiFi.begin(sta_ssid, sta_password);

    // Wait for connection
    Serial.print("Connecting to WiFi...");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("");
      Serial.print("Connected, IP address: ");
      Serial.println(WiFi.localIP());
    }
    else
    {
      Serial.println("");
      Serial.println("Connection failed, please check WiFi credentials");
    }
  }
}

/**
 * Setup WiFi mode and IP address
 * @param mode WiFi mode: WIFI_MODE_CUSTOM_AP, WIFI_MODE_CUSTOM_STA, WIFI_MODE_CUSTOM_AP_STA
 * @param sta_ssid WiFi name to connect to in STA mode
 * @param sta_password WiFi password in STA mode
 */
void setupWiFi(WiFiModeCustom mode, const char *sta_ssid = NULL, const char *sta_password = NULL)
{
  // First disconnect from all connections
  WiFi.disconnect(true);
  delay(1000);

  // Call the appropriate setup function based on mode
  if (mode == WIFI_MODE_CUSTOM_AP)
  {
    setupWiFiAP();
  }
  else if (mode == WIFI_MODE_CUSTOM_STA)
  {
    setupWiFiSTA(sta_ssid, sta_password);
  }
  else if (mode == WIFI_MODE_CUSTOM_AP_STA)
  {
    setupWiFiAPSTA(sta_ssid, sta_password);
  }
}

// HTML page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP32 Laser Distance Monitor</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <style>
    html {font-family: Arial, sans-serif; display: inline-block; text-align: center;}
    h2 {font-size: 2.4rem; margin: 20px 0 10px 0;}
    p {font-size: 1.8rem; margin: 8px 0;}
    body {max-width: 600px; margin:0px auto; padding: 20px; background-color: #f8f9fa;}
    .distance {font-size: 3.0rem; font-weight: bold; color: #0275d8;}
    .units {font-size: 1.5rem; color: #6c757d;}
    .status-container {margin: 20px auto; width: 80%; padding: 20px; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1);}
    .status-ok {background-color: #5cb85c; color: white;}
    .status-warning {background-color: #d9534f; color: white;}
    .status-error {background-color: #f0ad4e; color: white;}
    .distance-gauge {width: 100%; background-color: #f1f1f1; border-radius: 10px; margin: 15px 0; box-shadow: inset 0 1px 3px rgba(0,0,0,0.1);}
    .distance-bar {height: 30px; background-color: #0275d8; border-radius: 10px; text-align: center; line-height: 30px; color: white; transition: width 0.5s ease-in-out;}
    .connection-status {font-size: 1.0rem; color: #666; margin: 15px 0; padding: 8px; background-color: #f8f9fa; border-radius: 5px;}
    .debug-info {margin-top: 20px; font-size: 0.9rem; color: #666; text-align: left; border-top: 1px solid #ddd; padding-top: 10px;}
    .error-message {color: #d9534f; font-weight: bold; display: none; margin: 10px 0;}
    .btn-reconnect {background-color: #0275d8; color: white; border: none; padding: 8px 15px; border-radius: 5px; cursor: pointer; display: none; margin: 10px auto;}
    .btn-reconnect:hover {background-color: #025aa5;}
    .led-indicator {width: 20px; height: 20px; border-radius: 50%; display: inline-block; margin-right: 10px;}
    .led-on {background-color: #ff0000;}
    .led-off {background-color: #777;}
    .stats-container {margin-top: 15px; padding: 10px; background-color: #f5f5f5; border-radius: 5px; font-size: 0.85rem; text-align: left;}
    @media (max-width: 500px) {
      h2 {font-size: 1.8rem;}
      p {font-size: 1.4rem;}
      .distance {font-size: 2.5rem;}
    }
  </style>
</head>
<body>
  <h2>ESP32 Laser Distance Monitor</h2>
  <p>
    <span class="distance" id="distance">0.00</span>
    <span class="units">mm</span>
  </p>
  <p>
    <span>LED status: </span>
    <span class="led-indicator" id="led-indicator"></span>
    <span id="led-text">Off</span>
  </p>
  <div id="status-box" class="status-container status-ok">
    <p id="status-text">Status: Normal</p>
  </div>
  <div class="distance-gauge">
    <div class="distance-bar" id="distance-bar" style="width: 0%">0%</div>
  </div>
  <p class="connection-status" id="connection-status">Connection status: Waiting...</p>
  <div id="error-message" class="error-message">Connection error, please check your network</div>
  <button id="btn-reconnect" class="btn-reconnect" onclick="reconnectWebSocket()">Reconnect</button>
  
  <div class="debug-info">
    <p>Sensor status: <span id="sensor-status">Unknown</span></p>
    <p>Last update: <span id="last-update">-</span></p>
    <p>Connection attempts: <span id="connection-attempts">0</span></p>
    
    <div class="stats-container">
      <p>Valid readings: <span id="valid-readings">0</span></p>
      <p>Error readings: <span id="error-readings">0</span></p>
      <p>Time since valid reading: <span id="time-since-valid">0</span> ms</p>
    </div>
  </div>
  
  <script>
    // Global variables
    var eventSource = null;
    var lastUpdateTime = new Date();
    var connectionAttempts = 0;
    var reconnectTimeout = null;
    var heartbeatInterval = null;
    var connectionTimeout = null;
    var lastDistance = 0;
    
    // Execute when page loads
    window.addEventListener('load', onLoad);
    
    // Initialize EventSource connection
    function initWebSocket() {
      console.log('Attempting to establish EventSource connection...');
      document.getElementById('connection-status').innerHTML = "Connection status: Connecting...";
      document.getElementById('error-message').style.display = 'none';
      document.getElementById('btn-reconnect').style.display = 'none';
      
      // Clear previous timeout timer
      if (connectionTimeout) {
        clearTimeout(connectionTimeout);
      }
      
      try {
        // Close previous connection
        if (eventSource) {
          eventSource.close();
          eventSource = null;
        }
        
        // Create new connection
        eventSource = new EventSource('/events');
        
        // Set connection timeout (10 seconds)
        connectionTimeout = setTimeout(function() {
          if (eventSource && eventSource.readyState !== 1) { // 1 = OPEN
            console.log('Connection timeout');
            onError(new Error('Connection timeout'));
          }
        }, 10000);
        
        // Set event handlers
        eventSource.onopen = onOpen;
        eventSource.onerror = onError;
        
        // Set message handlers
        eventSource.addEventListener('laser_data', onLaserData);
        eventSource.addEventListener('heartbeat', onHeartbeat);
        eventSource.addEventListener('message', onMessage);
        
        // Update connection attempt count
        connectionAttempts++;
        document.getElementById('connection-attempts').innerHTML = connectionAttempts;
      } catch (e) {
        console.error('Connection error:', e);
        onError(e);
      }
    }
    
    // Connection success callback
    function onOpen(event) {
      console.log('Connection established');
      document.getElementById('connection-status').innerHTML = "Connection status: Connected";
      document.getElementById('error-message').style.display = 'none';
      document.getElementById('btn-reconnect').style.display = 'none';
      
      // Clear connection timeout
      if (connectionTimeout) {
        clearTimeout(connectionTimeout);
        connectionTimeout = null;
      }
      
      // Set heartbeat detection
      if (heartbeatInterval) {
        clearInterval(heartbeatInterval);
      }
      
      // Send heartbeat request every 30 seconds
      heartbeatInterval = setInterval(function() {
        fetch('/ping')
          .then(response => {
            if (!response.ok) {
              throw new Error('Ping failed');
            }
            return response.text();
          })
          .then(data => {
            console.log('Heartbeat response:', data);
          })
          .catch(error => {
            console.error('Heartbeat error:', error);
            // If heartbeat fails and EventSource is disconnected, try reconnect
            if (eventSource && eventSource.readyState === 2) { // 2 = CLOSED
              reconnectWebSocket();
            }
          });
      }, 30000);
    }
    
    // Connection error callback
    function onError(event) {
      console.log('Connection error or closed');
      document.getElementById('connection-status').innerHTML = "Connection status: Disconnected";
      document.getElementById('error-message').style.display = 'block';
      document.getElementById('btn-reconnect').style.display = 'block';
      
      // Clear heartbeat detection
      if (heartbeatInterval) {
        clearInterval(heartbeatInterval);
        heartbeatInterval = null;
      }
      
      // Clear previous reconnect timer
      if (reconnectTimeout) {
        clearTimeout(reconnectTimeout);
      }
      
      // Set automatic reconnect
      reconnectTimeout = setTimeout(reconnectWebSocket, 5000);
    }
    
    // Manual reconnect
    function reconnectWebSocket() {
      // Clear previous reconnect timer
      if (reconnectTimeout) {
        clearTimeout(reconnectTimeout);
        reconnectTimeout = null;
      }
      
      // Try to reconnect
      initWebSocket();
    }
    
    // Handle laser data event
    function onLaserData(event) {
      try {
        var data = JSON.parse(event.data);
        
        // Update distance display
        if (data.distance !== undefined) {
          // Only show valid distance values (greater than 0)
          if (data.distance > 0) {
            // Check if distance has changed
            if (Math.abs(lastDistance - data.distance) > 0.5) {
              document.getElementById('distance').innerHTML = data.distance.toFixed(2);
              lastDistance = data.distance;
              
              // Highlight the change briefly
              document.getElementById('distance').style.color = '#ff0000';
              setTimeout(function() {
                document.getElementById('distance').style.color = '#0275d8';
              }, 300);
            } else {
            document.getElementById('distance').innerHTML = data.distance.toFixed(2);
              lastDistance = data.distance;
            }
            
            // Update progress bar (assuming max range is 2000mm)
            var maxRange = 2000;
            var percentage = Math.min(100, (data.distance / maxRange) * 100);
            var distanceBar = document.getElementById('distance-bar');
            distanceBar.style.width = percentage + "%";
            distanceBar.innerHTML = percentage.toFixed(0) + "%";
            
            // Update LED indicator
            var ledIndicator = document.getElementById('led-indicator');
            var ledText = document.getElementById('led-text');
            if (data.led_on) {
              ledIndicator.className = "led-indicator led-on";
              ledText.innerHTML = "On (Distance < 500mm)";
            } else {
              ledIndicator.className = "led-indicator led-off";
              ledText.innerHTML = "Off";
            }
          }
        }
        
        // Update status box
        var statusBox = document.getElementById('status-box');
        var statusText = document.getElementById('status-text');
        
        if (data.status === "error") {
          document.getElementById('sensor-status').innerHTML = "Error - Using last valid data";
          statusBox.className = "status-container status-error";
          statusText.innerHTML = "Status: Sensor error";
        } else if (data.obstacle) {
          document.getElementById('sensor-status').innerHTML = "Normal";
          statusBox.className = "status-container status-warning";
          statusText.innerHTML = "Status: Obstacle detected!";
        } else {
          document.getElementById('sensor-status').innerHTML = "Normal";
          statusBox.className = "status-container status-ok";
          statusText.innerHTML = "Status: Normal";
        }
        
        // Update statistics
        document.getElementById('valid-readings').innerHTML = data.valid_readings || 0;
        document.getElementById('error-readings').innerHTML = data.error_readings || 0;
        document.getElementById('time-since-valid').innerHTML = data.time_since_valid || 0;
        
        // Update last update time
        lastUpdateTime = new Date();
        document.getElementById('last-update').innerHTML = lastUpdateTime.toLocaleTimeString();
      } catch (e) {
        console.error('Data parsing error:', e);
      }
    }
    
    // Handle heartbeat event
    function onHeartbeat(event) {
      console.log('Heartbeat received');
      // Update last activity time
      lastUpdateTime = new Date();
    }
    
    // Handle general message
    function onMessage(event) {
      console.log('Message received:', event.data);
      // Update last activity time
      lastUpdateTime = new Date();
    }
    
    // Execute when page loads
    function onLoad(event) {
      // Initialize connection
      initWebSocket();
      
      // Check connection status periodically
      setInterval(function() {
        var now = new Date();
        var timeDiff = (now - lastUpdateTime) / 1000; // seconds
        
        if (timeDiff > 10) {
          document.getElementById('connection-status').innerHTML = "Connection status: Data timeout (" + Math.floor(timeDiff) + " sec)";
          
          // If more than 30 seconds with no data and EventSource is disconnected, try reconnect
          if (timeDiff > 30 && (!eventSource || eventSource.readyState !== 1)) {
            console.log('Data timeout, trying to reconnect');
            reconnectWebSocket();
          }
        }
      }, 1000);
      
      // Add page visibility change event handler
      document.addEventListener('visibilitychange', function() {
        if (document.visibilityState === 'visible') {
          // When page becomes visible, check connection status and reconnect if needed
          if (!eventSource || eventSource.readyState !== 1) {
            console.log('Page became visible, checking connection');
            reconnectWebSocket();
          }
        }
      });
    }
  </script>
</body>
</html>
)rawliteral";

/**
 * Initialize Web server
 * Configure ESP32 as AP mode and start Web server
 */
void initWebServer()
{
  // Configure ESP32 as access point with fixed IP 192.168.4.1
  setupWiFi(WIFI_MODE_CUSTOM_AP);

  // 我们使用task.h中的pixels初始化，这里不需要重复初始化
  // LED状态初始化为关闭
  led_state = false;

  // Route handling
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/html", MAIN_page); });

  // Set CORS headers to allow all origins
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

  // Set event source
  events.onConnect([](AsyncEventSourceClient *client)
                   {
    if(client->lastId()){
      Serial.printf("Client reconnected! Last event ID received: %u\n", client->lastId());
    }
    // Send initial event to confirm connection
    client->send("Connection successful", NULL, millis(), 10000);
    
    // Immediately send current data
    String jsonData = "{\"distance\":" + String(web_current_distance, 2) + 
                     ",\"obstacle\":" + String(obstacle_detected ? "true" : "false") + 
                     ",\"led_on\":" + String(led_state ? "true" : "false") + 
                     ",\"valid_readings\":" + String(valid_readings_count) + 
                     ",\"error_readings\":" + String(error_readings_count) + 
                     ",\"time_since_valid\":" + String(millis() - last_valid_reading_time) + 
                     ",\"status\":\"" + String(web_current_distance >= 0 ? "ok" : "error") + "\"}";
    client->send(jsonData.c_str(), "laser_data", millis()); });
  server.addHandler(&events);

  // Add a simple API endpoint that returns JSON data
  server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String jsonResponse = "{\"distance\":" + String(web_current_distance, 2) + 
                         ",\"obstacle\":" + String(obstacle_detected ? "true" : "false") + 
                         ",\"led_on\":" + String(led_state ? "true" : "false") + 
                         ",\"valid_readings\":" + String(valid_readings_count) + 
                         ",\"error_readings\":" + String(error_readings_count) + 
                         ",\"time_since_valid\":" + String(millis() - last_valid_reading_time) + 
                         ",\"status\":\"" + String(web_current_distance >= 0 ? "ok" : "error") + "\"}";
    request->send(200, "application/json", jsonResponse); });

  // Add heartbeat endpoint
  server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", "pong"); });

  // Add endpoint to force sensor reset
  server.on("/reset_sensor", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    bool result = laser_reset();
    if (result) {
      laser_init();
      request->send(200, "text/plain", "Sensor reset successful");
    } else {
      request->send(200, "text/plain", "Sensor reset failed");
    } });

  // Add endpoint to run sensor test
  server.on("/test_sensor", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    laser_test();
    request->send(200, "text/plain", "Sensor test initiated, check serial output"); });

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

/**
 * Update laser sensor data and send to Web clients
 * This function should be called periodically in the main loop
 */
void updateLaserWebData()
{
  static unsigned long last_debug_time = 0;
  static unsigned long last_heartbeat_time = 0;
  static unsigned long last_data_time = 0;

  // Update data every 50ms (was 100ms) for faster updates
  if (millis() - last_data_time > 50)
  {
    last_data_time = millis();

    // Get laser distance data
    float distance = jiguang();

    // Create JSON data
    String jsonData;

    if (distance >= 0)
    {
      // Valid data
      web_current_distance = distance;
      valid_readings_count++;
      last_valid_reading_time = millis();

      // Check if obstacle detected (less than 30cm)
      obstacle_detected = (distance < 300);

      // Check if LED should be on (less than 50cm)
      if (distance < 300)
      {
        // 近距离(<30cm)显示红色
        pixels.setPixelColor(0, pixels.Color(255, 0, 0));
        pixels.show();
        led_state = true;
      }
      else if (distance < 500)
      {
        // 中距离(<50cm)显示蓝色
        pixels.setPixelColor(0, pixels.Color(0, 0, 255));
        pixels.show();
        led_state = true;
      }
      else
      {
        // 远距离(>=50cm)显示绿色
        pixels.setPixelColor(0, pixels.Color(0, 255, 0));
        pixels.show();
        led_state = true;
      }

      jsonData = "{\"distance\":" + String(web_current_distance, 2) +
                 ",\"obstacle\":" + String(obstacle_detected ? "true" : "false") +
                 ",\"led_on\":" + String(led_state ? "true" : "false") +
                 ",\"valid_readings\":" + String(valid_readings_count) +
                 ",\"error_readings\":" + String(error_readings_count) +
                 ",\"time_since_valid\":0" +
                 ",\"status\":\"ok\"}";
    }
    else
    {
      // Invalid data, keep previous distance value but mark status as error
      error_readings_count++;

      // 测距失败时LED闪烁一次表示错误
      pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // 红色
      pixels.show();
      delay(50);
      pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // 关闭
      pixels.show();
      led_state = false;

      jsonData = "{\"distance\":" + String(web_current_distance, 2) +
                 ",\"obstacle\":" + String(obstacle_detected ? "true" : "false") +
                 ",\"led_on\":" + String(led_state ? "true" : "false") +
                 ",\"valid_readings\":" + String(valid_readings_count) +
                 ",\"error_readings\":" + String(error_readings_count) +
                 ",\"time_since_valid\":" + String(millis() - last_valid_reading_time) +
                 ",\"status\":\"error\"}";
    }

    // Send event to all connected clients
    events.send(jsonData.c_str(), "laser_data", millis());
  }

  // Send heartbeat every 10 seconds to keep connection active
  if (millis() - last_heartbeat_time > 10000)
  {
    last_heartbeat_time = millis();
    events.send("", "heartbeat", millis());
  }

  // Brief delay to yield CPU time
  delay(1);
}

/**
 * Start laser Web monitoring task
 * Creates a FreeRTOS task to handle Web server and data updates
 */
void startLaserWebTask()
{
  xTaskCreate(
      [](void *parameter)
      {
        // 先初始化Web服务器，避免与激光传感器初始化冲突
        initWebServer();

        // 然后初始化激光传感器
        if (!laser_init())
        {
          // 删除调试输出
          // Serial.println("Laser sensor initialization failed, trying to continue running...");
        }

        while (true)
        {
          // Update laser data
          updateLaserWebData();

          // Brief delay to prevent watchdog issues
          vTaskDelay(1);
        }
      },
      "LaserWebTask", // Task name
      8192,           // Stack size
      NULL,           // Task parameters
      1,              // Task priority
      NULL            // Task handle
  );

  // 删除调试输出
  // Serial.println("Laser Web monitoring task started");
  // Serial.println("Please connect to WiFi: " + String(ssid) + ", password: " + String(password));
  // Serial.println("Then visit http://192.168.4.1 to view laser sensor data");
}

#endif // _WEB_H_