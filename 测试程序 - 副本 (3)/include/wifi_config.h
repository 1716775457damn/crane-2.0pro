#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <WiFi.h>
#include <EEPROM.h>
#include <Arduino.h>

// 声明外部函数
extern void debugPrint(const String &message);
extern void debugPrintAll(const String &message);

// EEPROM配置
#define EEPROM_SIZE 512
#define EEPROM_INITIALIZED_FLAG 0xAA
#define EEPROM_MAGIC_BYTE_ADDR 0
#define EEPROM_SSID_ADDR 1
#define EEPROM_PASS_ADDR 34
#define EEPROM_REFRESH_RATE_ADDR 100

// WiFi配置（全局变量）
const char* ssid = "ServoController";     // WiFi接入点名称
const char* password = "12345678";        // WiFi密码

// 写入字符串到EEPROM
void writeStringToEEPROM(int startAddr, const String &str) {
  int len = str.length();
  // 存储字符串长度（1字节）
  EEPROM.write(startAddr, len);
  
  // 存储字符串内容
  for (int i = 0; i < len; i++) {
    EEPROM.write(startAddr + 1 + i, str[i]);
  }
}

// 从EEPROM读取字符串
String readStringFromEEPROM(int startAddr) {
  // 读取字符串长度
  int len = EEPROM.read(startAddr);
  
  // 验证长度合理性
  if (len > 100 || len < 0) {
    return ""; // 长度不合理，返回空字符串
  }
  
  // 读取字符串内容
  char buffer[101]; // 最多100字符 + 结束符
  for (int i = 0; i < len; i++) {
    buffer[i] = EEPROM.read(startAddr + 1 + i);
  }
  buffer[len] = '\0'; // 添加字符串结束符
  
  return String(buffer);
}

// EEPROM 操作函数
void initEEPROM() {
  // 初始化EEPROM
  EEPROM.begin(EEPROM_SIZE);
  
  // 检查是否初始化过
  if (EEPROM.read(EEPROM_MAGIC_BYTE_ADDR) != EEPROM_INITIALIZED_FLAG) {
    debugPrintAll("First run, initializing EEPROM with default settings");
    
    // 写入默认的WiFi设置
    writeStringToEEPROM(EEPROM_SSID_ADDR, "ServoController");
    writeStringToEEPROM(EEPROM_PASS_ADDR, "12345678");
    
    // 写入默认的刷新率
    EEPROM.write(EEPROM_REFRESH_RATE_ADDR, 5); // 5 * 100 = 500ms
    
    // 设置初始化标志
    EEPROM.write(EEPROM_MAGIC_BYTE_ADDR, EEPROM_INITIALIZED_FLAG);
    EEPROM.commit();
  }
}

// 加载WiFi设置
void loadWiFiSettings() {
  String savedSSID = readStringFromEEPROM(EEPROM_SSID_ADDR);
  String savedPass = readStringFromEEPROM(EEPROM_PASS_ADDR);
  
  // 仅当有有效值时才使用它们
  if (savedSSID.length() > 0 && savedPass.length() >= 8) {
    ssid = savedSSID.c_str();
    password = savedPass.c_str();
    debugPrintAll("Loaded WiFi settings from EEPROM: SSID=" + savedSSID);
  } else {
    debugPrintAll("No valid WiFi settings in EEPROM, using defaults");
  }
}

// 获取保存的刷新率
int getRefreshRate() {
  int rate = EEPROM.read(EEPROM_REFRESH_RATE_ADDR);
  // 验证范围
  if (rate < 1 || rate > 50) {
    rate = 5; // 默认500ms
  }
  return rate * 100;
}

// 保存设置到EEPROM
void saveSettingsToEEPROM(const String &newSSID, const String &newPass, int refreshRate) {
  writeStringToEEPROM(EEPROM_SSID_ADDR, newSSID);
  writeStringToEEPROM(EEPROM_PASS_ADDR, newPass);
  EEPROM.write(EEPROM_REFRESH_RATE_ADDR, refreshRate / 100);
  EEPROM.commit();
  debugPrintAll("Settings saved to EEPROM");
}

// 初始化WiFi
void initWiFi() {
  // 初始化EEPROM并加载设置
  initEEPROM();
  loadWiFiSettings();
  
  // 创建接入点
  WiFi.softAP(ssid, password);
  
  IPAddress myIP = WiFi.softAPIP();
  debugPrintAll("AP started with IP: " + myIP.toString());
  debugPrintAll("SSID: " + String(ssid) + ", Password: " + String(password));
}

#endif // WIFI_CONFIG_H 