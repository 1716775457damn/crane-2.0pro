#ifndef GRAY_TRACKER_DEBUG_H
#define GRAY_TRACKER_DEBUG_H

#include "GrayTracker.h"
#include "comm.h"
#include "pins.h"

/**
 * GrayTrackerDebug - Grayscale Sensor Debug Class
 * 
 * Provides basic grayscale sensor functionality with debugging support
 */
class GrayTrackerDebug {
private:
    // Sensor pin definitions
    uint8_t sensorPinR2;
    uint8_t sensorPinR1;
    uint8_t sensorPinM;
    uint8_t sensorPinL1;
    uint8_t sensorPinL2;
    uint8_t rgbLedPin;
    
    // Sensor data
    int sensorValues[5]; // Raw sensor values
    int sensorStates[5]; // Sensor states (0/1)
    int threshold;       // Black/white threshold
    int tracingState;    // Line tracking state
    bool autoRun;        // Auto run flag
    
public:
    // Sensor state definitions
    enum {
        STRAIGHT = 0,        // Straight line
        LEFT_TURN = 1,       // Left turn
        RIGHT_TURN = 2,      // Right turn
        SHARP_LEFT_TURN = 3, // Sharp left turn
        SHARP_RIGHT_TURN = 4,// Sharp right turn
        ALL_BLACK = 5,       // All black
        ALL_WHITE = 6,       // All white
        CROSSROAD = 7        // Crossroad
    };
    
    // Route modes
    enum RouteMode {
        BASIC_MODE = 0,      // Basic tracking mode
        SLOW_MODE = 1,       // Slow tracking mode
        REVERSE_MODE = 2,    // Reverse tracking mode
        SPECIAL_MODE = 3     // Special effect mode
    };
    
    /**
     * Constructor
     */
    GrayTrackerDebug(
        uint8_t r2 = GRAY_SENSOR_R2, 
        uint8_t r1 = GRAY_SENSOR_R1, 
        uint8_t m = GRAY_SENSOR_M, 
        uint8_t l1 = GRAY_SENSOR_L1, 
        uint8_t l2 = GRAY_SENSOR_L2,
        uint8_t rgb = RGB_LED_PIN
    ) {
        sensorPinR2 = r2;
        sensorPinR1 = r1;
        sensorPinM = m;
        sensorPinL1 = l1;
        sensorPinL2 = l2;
        rgbLedPin = rgb;
        
        threshold = 2000;    // Default threshold
        tracingState = 0;
        autoRun = false;     // Default: auto run disabled
        
        // Initialize arrays
        for (int i = 0; i < 5; i++) {
            sensorValues[i] = 0;
            sensorStates[i] = 0;
        }
    }
    
    /**
     * Initialize
     */
    void begin() {
        // Set pin modes
        pinMode(sensorPinR2, INPUT);
        pinMode(sensorPinR1, INPUT);
        pinMode(sensorPinM, INPUT);
        pinMode(sensorPinL1, INPUT);
        pinMode(sensorPinL2, INPUT);
        pinMode(rgbLedPin, OUTPUT);
        
        // Initialize RGB LED (if needed)
        // pixels.begin();
        // pixels.setBrightness(50);
        // pixels.clear();
        // pixels.show();
        
        Serial.println("Grayscale sensors initialized successfully");
    }
    
    /**
     * Update sensor data
     */
    void update() {
        // Read sensor raw values
        sensorValues[0] = analogRead(sensorPinR2);
        sensorValues[1] = analogRead(sensorPinR1);
        sensorValues[2] = analogRead(sensorPinM);
        sensorValues[3] = analogRead(sensorPinL1);
        sensorValues[4] = analogRead(sensorPinL2);
        
        // Determine sensor states based on threshold
        for (int i = 0; i < 5; i++) {
            sensorStates[i] = (sensorValues[i] > threshold) ? 1 : 0;
        }
        
        // Update tracking state
        updateTracingState();
    }
    
    /**
     * Get sensor raw values
     */
    void getSensorValues(int* values) {
        for (int i = 0; i < 5; i++) {
            values[i] = sensorValues[i];
        }
    }
    
    /**
     * Get sensor states
     */
    void getSensorStates(int* states) {
        for (int i = 0; i < 5; i++) {
            states[i] = sensorStates[i];
        }
    }
    
    /**
     * Get tracking state
     */
    int getTracingState() {
        return tracingState;
    }
    
    /**
     * Set threshold
     */
    void setThreshold(int value) {
        threshold = value;
    }
    
    /**
     * Set auto run state
     */
    void setAutoRun(bool enable) {
        autoRun = enable;
    }
    
    /**
     * Get auto run state
     */
    bool isAutoRunEnabled() {
        return autoRun;
    }
    
    /**
     * Set route mode
     */
    void setRouteMode(RouteMode mode) {
        // Route mode processing logic can be added here
    }
    
    /**
     * Get current route mode
     */
    int getRouteMode() {
        return 0; // Default: return basic mode
    }
    
    /**
     * Detect crossroad
     */
    bool detectCrossroad() {
        // Simple crossroad detection - when multiple sensors detect black line
        int blackCount = 0;
        for (int i = 0; i < 5; i++) {
            if (sensorStates[i] == 1) {
                blackCount++;
            }
        }
        
        return blackCount >= 3;
    }
    
private:
    /**
     * Update tracking state
     */
    void updateTracingState() {
        // Simple rules to determine state:
        // All black state
        if (sensorStates[0] && sensorStates[1] && sensorStates[2] && 
            sensorStates[3] && sensorStates[4]) {
            tracingState = ALL_BLACK;
            return;
        }
        
        // All white state
        if (!sensorStates[0] && !sensorStates[1] && !sensorStates[2] && 
            !sensorStates[3] && !sensorStates[4]) {
            tracingState = ALL_WHITE;
            return;
        }
        
        // Middle sensor on black line, straight state
        if (sensorStates[2]) {
            tracingState = STRAIGHT;
            return;
        }
        
        // Left sensor on black line, right turn state
        if (sensorStates[3]) {
            tracingState = LEFT_TURN;
            return;
        }
        
        // Right sensor on black line, left turn state
        if (sensorStates[1]) {
            tracingState = RIGHT_TURN;
            return;
        }
        
        // Leftmost sensor on black line, sharp right turn state
        if (sensorStates[4]) {
            tracingState = SHARP_LEFT_TURN;
            return;
        }
        
        // Rightmost sensor on black line, sharp left turn state
        if (sensorStates[0]) {
            tracingState = SHARP_RIGHT_TURN;
            return;
        }
        
        // Default: maintain previous state
    }
};

#endif // GRAY_TRACKER_DEBUG_H 
