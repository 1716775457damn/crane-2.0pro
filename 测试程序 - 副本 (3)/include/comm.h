#ifndef _COMM_H_
#define _COMM_H_

#include <Arduino.h>
#include "pins.h"  // Import pin definitions
#include "chassis.h" // Import chassis control functions

// Host communication serial port defined in pins.h
// HOST_SERIAL_TX, HOST_SERIAL_RX, HOST_BAUD_RATE

/**
 * Initialize host communication serial port
 * This serial port is dedicated for host communication only
 */
inline void host_serial_init(void)
{
    // Use Serial2 for host communication
    Serial.println("Initializing host communication serial port...");
    
    // First, print pin configuration
    Serial.print("RX Pin: ");
    Serial.println(HOST_SERIAL_RX);
    Serial.print("TX Pin: ");
    Serial.println(HOST_SERIAL_TX);
    Serial.print("Baud Rate: ");
    Serial.println(HOST_BAUD_RATE);
    
    // End any existing serial connection
    Serial2.end();
    delay(100);
    
    // Initialize with just TX pin for now (to avoid RX issues)
    Serial2.begin(HOST_BAUD_RATE, SERIAL_8N1, -1, HOST_SERIAL_TX);
    delay(100);
    
    // Send test message
    Serial2.println("HOST_SERIAL_TEST");
    Serial.println("Host communication serial port initialized (TX only)");
    
    // Now add RX pin
    Serial2.end();
    delay(100);
    Serial2.begin(HOST_BAUD_RATE, SERIAL_8N1, HOST_SERIAL_RX, HOST_SERIAL_TX);
    delay(100);
    
    // Send another test message
    Serial2.println("HOST_SERIAL_TEST_FULL");
    Serial.println("Host communication serial port fully initialized (TX+RX)");
}

/**
 * Send data to host
 * @param data Data to send
 */
inline void send_to_host(const String& data)
{
    Serial2.println(data);  // Add newline for easier parsing by host
    Serial.print("Sent to host: ");
    Serial.println(data);
}

/**
 * Receive command from host
 * Note: This is the global version
 */
inline String receive_from_host_global() {
    static String buffer = "";
    
    // Check if data is available
    while (Serial2.available()) {
        char c = Serial2.read();
        
        // Process carriage return or newline as command terminator
        if (c == '\n' || c == '\r') {
            if (buffer.length() > 0) {
                String command = buffer;
                buffer = "";
                Serial.print("Received host command: ");
                Serial.println(command);
                return command;
            }
        } 
        // Ignore non-printable characters
        else if (c >= 32 && c <= 126) {
            buffer += c;
            
            // Prevent buffer overflow
            if (buffer.length() > 64) {
                buffer = buffer.substring(buffer.length() - 64);
            }
        }
    }
    
    return "";
}

/**
 * Send sensor data to host
 * @param sensor_type Sensor type
 * @param value Sensor value
 */
inline void send_sensor_data(const String& sensor_type, float value)
{
    String data = sensor_type + ":" + String(value);
    send_to_host(data);
}

/**
 * Send grayscale sensor data to host
 * @param values Array of 5 grayscale sensor values
 */
inline void send_gray_sensor_data(int* values)
{
    String data = "GRAY:" + 
                  String(values[0]) + "," + 
                  String(values[1]) + "," + 
                  String(values[2]) + "," + 
                  String(values[3]) + "," + 
                  String(values[4]);
    send_to_host(data);
}

/**
 * Send status information to host
 * @param status Status information
 */
inline void send_status(const String& status)
{
    String data = "STATUS:" + status;
    send_to_host(data);
}

#endif // _COMM_H_ 