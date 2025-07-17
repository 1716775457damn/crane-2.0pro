#ifndef _PINS_H_
#define _PINS_H_

//=========================
// Grayscale Sensor Pins
//=========================
#define GRAY_SENSOR_R2 10 // Rightmost sensor
#define GRAY_SENSOR_R1 11 // Right sensor
#define GRAY_SENSOR_M 12  // Middle sensor
#define GRAY_SENSOR_L1 13 // Left sensor
#define GRAY_SENSOR_L2 14 // Leftmost sensor

//=========================
// RGB LED Pin
//=========================
#define RGB_LED_PIN 48

//=========================
// Serial Communication Pins
//=========================
// Host communication serial pins
#define HOST_SERIAL_TX 20
#define HOST_SERIAL_RX 21
#define HOST_BAUD_RATE 115200

// Chassis control serial pins
#define CHASSIS_SERIAL_TX 19
// #define CHASSIS_SERIAL_RX 17  // 移除底盘串口RX引脚，只使用TX
#define CHASSIS_BAUD_RATE 115200

//=========================
// Servo Pins
//=========================
#define SERVO_PIN_1 18 // Servo 1 pin
#define SERVO_PIN_2 8  // Servo 2 pin
#define SERVO_PIN_3 3  // Servo 3 pin

//=========================
// Stepper Motor Pins
//=========================
// Stepper motor controller pins
#define SLEEP1 4
#define DIR1 5
#define STEP1 6

#define SLEEP2 7
#define DIR2 15
#define STEP2 16

#define SLEEP3 42
#define DIR3 41
#define STEP3 40

#define SLEEP4 39
#define DIR4 38
#define STEP4 37

//=========================
// Relay Pins
//=========================
// #define RELAY_PIN_1 11 // Relay 1 control pin
// #define RELAY_PIN_2 22 // Relay 2 control pin (修改避免与串口冲突)

//=========================
// Ultrasonic Sensor Pins
// //=========================
// #define ECHO_PIN 33
// #define TRIG_PIN 25

//=========================
// Laser Sensor Pins (只保留一个激光传感器)
//=========================
#define LASER_RX_PIN 36 // Laser sensor receive pin
#define LASER_TX_PIN 35 // Laser sensor transmit pin
// #define LASER2_RX_PIN 46 // 移除第二个激光传感器
// #define LASER2_TX_PIN 9  // 移除第二个激光传感器

//=========================
// Power Detection Pin
//=========================
// #define ADC_PIN 7  // Power voltage detection pin

#endif // _PINS_H_