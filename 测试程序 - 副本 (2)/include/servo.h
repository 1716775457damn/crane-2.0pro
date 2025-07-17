#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>
#include "pins.h" // Import pin definitions

// 声明外部函数（在main.cpp中定义）
extern void debugPrint(const String &message);

/**
 * @brief Control Servo 1 to rotate to specified angle
 * @param angle Angle value (0-360)
 */
void servo1(int angle)
{
    // Ensure angle is within valid range
    if (angle < 0)
        angle = 0;
    if (angle > 360)
        angle = 360;

    debugPrint("Servo 1 moving to angle: " + String(angle));

    // Send 50 pulses
    pinMode(SERVO_PIN_1, OUTPUT);
    for (int i = 0; i < 50; i++)
    {
        // Convert angle to pulse width for 360-degree servo
        // 0 degrees = 500us, 360 degrees = 2500us
        int pulsewidth = (angle * 2000 / 360) + 500; // Convert angle to pulse width (500-2500)
        digitalWrite(SERVO_PIN_1, HIGH);             // Set servo pin to HIGH
        delayMicroseconds(pulsewidth);               // Delay for pulse width microseconds
        digitalWrite(SERVO_PIN_1, LOW);              // Set servo pin to LOW
        delayMicroseconds(20000 - pulsewidth);
    }
    delay(100);
}

/**
 * @brief Control Servo 2 to rotate to specified angle
 * @param angle Angle value (0-360)
 */
void servo2(int angle)
{
    // Ensure angle is within valid range
    if (angle < 0)
        angle = 0;
    if (angle > 360)
        angle = 360;

    debugPrint("Servo 2 moving to angle: " + String(angle));

    // Send 50 pulses
    pinMode(SERVO_PIN_2, OUTPUT);
    for (int i = 0; i < 50; i++)
    {
        // Convert angle to pulse width for 360-degree servo
        // 0 degrees = 500us, 360 degrees = 2500us
        int pulsewidth = (angle * 2000 / 360) + 500; // Convert angle to pulse width (500-2500)
        digitalWrite(SERVO_PIN_2, HIGH);             // Set servo pin to HIGH
        delayMicroseconds(pulsewidth);               // Delay for pulse width microseconds
        digitalWrite(SERVO_PIN_2, LOW);              // Set servo pin to LOW
        delayMicroseconds(20000 - pulsewidth);
    }
    delay(100);
}

/**
 * @brief Control Servo 3 to rotate to specified angle
 * @param angle Angle value (0-360)
 */
void servo3(int angle)
{
    // Ensure angle is within valid range
    if (angle < 0)
        angle = 0;
    if (angle > 360)
        angle = 360;

    debugPrint("Servo 3 moving to angle: " + String(angle));

    // Send 50 pulses
    pinMode(SERVO_PIN_3, OUTPUT);
    for (int i = 0; i < 50; i++)
    {
        // Convert angle to pulse width for 360-degree servo
        // 0 degrees = 500us, 360 degrees = 2500us
        int pulsewidth = (angle * 2000 / 360) + 500; // Convert angle to pulse width (500-2500)
        digitalWrite(SERVO_PIN_3, HIGH);             // Set servo pin to HIGH
        delayMicroseconds(pulsewidth);               // Delay for pulse width microseconds
        digitalWrite(SERVO_PIN_3, LOW);              // Set servo pin to LOW
        delayMicroseconds(20000 - pulsewidth);
    }
    delay(100);
}

/**
 * @brief Control all three servos to rotate to specified angles simultaneously
 * @param angle1 Servo 1 angle value (0-360)
 * @param angle2 Servo 2 angle value (0-360)
 * @param angle3 Servo 3 angle value (0-360)
 */
void servoAll(int angle1, int angle2, int angle3)
{
    // Ensure angles are within valid range
    if (angle1 < 0)
        angle1 = 0;
    if (angle1 > 360)
        angle1 = 360;
    if (angle2 < 0)
        angle2 = 0;
    if (angle2 > 360)
        angle2 = 360;
    if (angle3 < 0)
        angle3 = 0;
    if (angle3 > 360)
        angle3 = 360;

    debugPrint("Moving all servos to angles: " + String(angle1) + ", " + String(angle2) + ", " + String(angle3));

    // Convert to pulse widths for 360-degree servos
    // 0 degrees = 500us, 360 degrees = 2500us
    int pulsewidth1 = (angle1 * 2000 / 360) + 500;
    int pulsewidth2 = (angle2 * 2000 / 360) + 500;
    int pulsewidth3 = (angle3 * 2000 / 360) + 500;

    // Set pin modes
    pinMode(SERVO_PIN_1, OUTPUT);
    pinMode(SERVO_PIN_2, OUTPUT);
    pinMode(SERVO_PIN_3, OUTPUT);

    // Send 50 pulses
    for (int i = 0; i < 50; i++)
    {
        // Control servo 1
        digitalWrite(SERVO_PIN_1, HIGH);
        delayMicroseconds(pulsewidth1);
        digitalWrite(SERVO_PIN_1, LOW);

        // Control servo 2
        digitalWrite(SERVO_PIN_2, HIGH);
        delayMicroseconds(pulsewidth2);
        digitalWrite(SERVO_PIN_2, LOW);

        // Control servo 3
        digitalWrite(SERVO_PIN_3, HIGH);
        delayMicroseconds(pulsewidth3);
        digitalWrite(SERVO_PIN_3, LOW);

        // Wait for next cycle
        delayMicroseconds(20000 - max(pulsewidth1, max(pulsewidth2, pulsewidth3)));
    }
    delay(100);
}

/**
 * @brief Initialize all servos to specified positions
 * @param angle1 Servo 1 initial angle (0-360), default is 90 degrees
 * @param angle2 Servo 2 initial angle (0-360), default is 90 degrees
 * @param angle3 Servo 3 initial angle (0-360), default is 90 degrees
 * @param moveToPosition Whether to move servos to specified positions during initialization, default is false
 */
void initServos(int angle1 = 90, int angle2 = 90, int angle3 = 90, bool moveToPosition = false)
{
    debugPrint("Initializing servos...");

    // Set pin modes
    pinMode(SERVO_PIN_1, OUTPUT);
    pinMode(SERVO_PIN_2, OUTPUT);
    pinMode(SERVO_PIN_3, OUTPUT);

    // Only move servos when moveToPosition is true
    if (moveToPosition)
    {
        // Set all servos to initial positions simultaneously
        servoAll(angle1, angle2, angle3);
        debugPrint("Servos moved to initial positions");
    }
    else
    {
        debugPrint("Servo pins initialized, but not moved (safety feature)");
    }

    debugPrint("Servo initialization complete!");
}

#endif
