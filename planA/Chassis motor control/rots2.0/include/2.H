#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <AccelStepper.h>

// 步进电机参数
#define STEP_PIN 18   // 步进信号引脚
#define DIR_PIN 19    // 方向信号引脚
#define ENABLE_PIN 21 // 使能信号引脚

// 定义步进电机对象
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// 电机参数设置
int maxSpeed;     // 最大速度 (steps per second)
int acceleration; // 加速度 (steps per second^2)
int stepsToMove;  // 移动步数

// SSID & Password
const char *ssid = "ovo";          // 你的SSID
const char *password = "twx20051"; // 你的密码

// DNS服务器和Web服务器的端口
const byte DNS_PORT = 53;
DNSServer dnsServer;
WebServer server(80);

// LED引脚设置为GPIO 15,这里需要改成自己的gpio端口
const int ledPin = 2;

// 设置HTML页面内容
String HTML = R"delim(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>ESP32 Control Panel</title>
  <style>
    body {
      font-family: 'Arial', sans-serif;
      background-color: #f4f4f4;
      margin: 0;
      padding: 20px;
      text-align: center;
    }
    h1 {
      color: #333;
    }
    .button {
      display: inline-block;
      padding: 10px 20px;
      margin: 5px;
      background-color: #4CAF50;
      color: white;
      text-decoration: none;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      transition: background-color 0.3s;
    }
    .button:hover {
      background-color: #45a049;
    }
    .control-group {
      margin-bottom: 20px;
    }
    .response-message {
      margin-top: 20px;
      color: #28a745;
    }
  </style>
</head>
<body>
  <h1>ESP32 Control Panel</h1>
  
  <div class='control-group'>
    <button class='button led-control' data-action='LED/on'>Open LED</button>
    <button class='button led-control' data-action='LED/off'>Close LED</button>
  </div>
  
  <div class='control-group'>
    <button class='button stepper-control' data-action='stepper/move1'>Move forward</button>
    <button class='button stepper-control' data-action='stepper/move2'>Move backward</button>
    <button class='button stepper-control' data-action='stepper/move3'>Move forward</button>
    <button class='button stepper-control' data-action='stepper/move4'>Move backward1</button>
    <button class='button stepper-control' data-action='stepper/stop'>Stop Motor</button>
  </div>
  
  <div id='response-message' class='response-message'></div>

  <script>
    document.addEventListener('DOMContentLoaded', function() {
      var ledControls = document.querySelectorAll('.led-control');
      var stepperControls = document.querySelectorAll('.stepper-control');
      var responseMessage = document.getElementById('response-message');

      ledControls.forEach(function(button) {
        button.addEventListener('click', function(event) {
          sendRequest(this.getAttribute('data-action'), function(message) {
            responseMessage.textContent = message;
          });
        });
      });

      stepperControls.forEach(function(button) {
        button.addEventListener('click', function(event) {
          sendRequest(this.getAttribute('data-action'), function(message) {
            responseMessage.textContent = message;
          });
        });
      });

      function sendRequest(url, callback) {
        var xhr = new XMLHttpRequest();
        xhr.open('GET', url, true);
        xhr.onload = function() {
          if (this.status === 200) {
            var response = JSON.parse(this.responseText);
            callback(response.status, response.message);
          } else {
            callback("error", "Error: " + this.status);
          }
        };
        xhr.send();
      }
    });
  </script>
</body>
</html>)delim";

// 初始化步进电机
// 处理步进电机移动请求
void handleRoot()
{
    server.send(200, "text/html", HTML);
}
// 处理步进电机移动请求
void controlStepper(AccelStepper &stepper, float speed, float acceleration, int steps)
{
    stepper.setMaxSpeed(speed);
    stepper.setAcceleration(acceleration);
    stepper.moveTo(steps); // 设置目标位置
    while (stepper.distanceToGo() != 0)
    {
        stepper.run();
    }
}
void handleStepperMove_1()
{
    controlStepper(stepper, 1500, 1800, 2000);
    server.send(200, "text/html", "<p>Moved forward</p><a href=\"/\">return</a>");
}

void handleStepperMove_2()
{
    controlStepper(stepper, 1500, 1800, -2000); // 注意这里是负数，表示向后移动
    server.send(200, "text/html", "<p>Moved backward</p><a href=\"/\">return</a>");
}
void handleStepperMove_3()
{
    controlStepper(stepper, 1500, 1800, 2000);
    server.send(200, "text/html", "<p>Moved forward</p><a href=\"/\">return</a>");
}

void handleStepperMove_4()
{
    controlStepper(stepper, 1500, 1800, -2000); // 注意这里是负数，表示向后移动
    server.send(200, "text/html", "<p>Moved backward</p><a href=\"/\">return</a>");
}
// 处理步进电机停止请求
void handleStepperStop()
{
    stepper.stop();
    Serial.println("Motor stopped.");
}

void handleLedOn()
{
    digitalWrite(ledPin, HIGH);
    server.send(200, "text/html", "<p>LED Opened</p><a href=\"/\">return</a>"); // 修正了这里的文本
}

void handleLedOff()
{
    digitalWrite(ledPin, LOW);
    server.send(200, "text/html", "<p>LED Closed</p><a href=\"/\">return</a>");
}

// 在主控件文件 (main.cpp) 的顶部或在相应的头文件中

void setup()
{
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW); // 默认LED关闭

    // 设置ESP32为AP模式
    WiFi.softAP(ssid, password);
    Serial.println("ESP32作为Access Point启动");

    // 启动DNS服务器，将所有域名请求重定向到本地
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    // 设置Web服务器路由
    server.on("/", handleRoot);
    server.on("/LED/on", handleLedOn);
    server.on("/LED/off", handleLedOff);
    server.on("/stepper/move1", handleStepperMove_1);
    server.on("/stepper/move2", handleStepperMove_2);
    server.on("/stepper/stop", handleStepperStop);
    // 捕获所有未定义的请求并重定向到根目录
    server.onNotFound([]()
                      { server.send(200, "text/html", HTML); });

    server.begin();
    Serial.println("Web服务器已启动");
}

void loop()
{
    dnsServer.processNextRequest(); // 处理DNS重定向
    server.handleClient();          // 处理HTTP请求

    // 检查步进电机是否需要运行
    if (stepper.distanceToGo() != 0)
    {
        stepper.run();
    } // 处理HTTP请求
}
