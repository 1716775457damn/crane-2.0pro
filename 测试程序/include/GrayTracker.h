#ifndef GRAY_TRACKER_H
#define GRAY_TRACKER_H

#include <Arduino.h>

/**
 * GrayTracker - Grayscale Sensor Base Class
 * 
 * Base class for grayscale sensor management
 */
class GrayTracker {
protected:
    // Sensor pin definitions
    uint8_t sensorPins[5]; 
    
    // Sensor data
    int sensorValues[5]; // Raw sensor values
    int sensorStates[5]; // Sensor states (0/1)
    int threshold;       // Black/white threshold
    
public:
    // Constructor (default pin configuration can be overridden)
    GrayTracker(uint8_t r2, uint8_t r1, uint8_t m, uint8_t l1, uint8_t l2) {
        sensorPins[0] = r2;
        sensorPins[1] = r1;
        sensorPins[2] = m;
        sensorPins[3] = l1;
        sensorPins[4] = l2;
        
        threshold = 2000; // Default threshold
        
        // Initialize arrays
        for (int i = 0; i < 5; i++) {
            sensorValues[i] = 0;
            sensorStates[i] = 0;
        }
    }
    
    // Initialize
    virtual void begin() {
        // Configure sensor pins as inputs
        for (int i = 0; i < 5; i++) {
            pinMode(sensorPins[i], INPUT);
        }
    }
    
    // Update sensor data
    virtual void update() {
        // Read sensor raw values
        for (int i = 0; i < 5; i++) {
            sensorValues[i] = analogRead(sensorPins[i]);
            sensorStates[i] = (sensorValues[i] > threshold) ? 1 : 0;
        }
    }
    
    // Get raw sensor values
    void getSensorValues(int* values) {
        for (int i = 0; i < 5; i++) {
            values[i] = sensorValues[i];
        }
    }
    
    // Get sensor states (0 or 1)
    void getSensorStates(int* states) {
        for (int i = 0; i < 5; i++) {
            states[i] = sensorStates[i];
        }
    }
    
    // Set threshold
    void setThreshold(int value) {
        threshold = value;
    }
    
    // Get threshold
    int getThreshold() {
        return threshold;
    }
};

#endif // GRAY_TRACKER_H 