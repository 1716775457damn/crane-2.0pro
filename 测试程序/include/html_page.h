#ifndef HTML_PAGE_H
#define HTML_PAGE_H

// HTML页面
const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32-S3 Robot Controller</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: #333;
        }
        .main-container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.3);
            overflow: hidden;
        }
        .header {
            background: linear-gradient(135deg, #4CAF50 0%, #45a049 100%);
            color: white;
            padding: 20px;
            text-align: center;
        }
        .header h1 {
            margin: 0;
            font-size: 2.5em;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .content {
            padding: 20px;
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
        }
        .card {
            background: #f8f9fa;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            transition: transform 0.3s ease;
        }
        .card:hover {
            transform: translateY(-5px);
        }
        .card h3 {
            margin-top: 0;
            color: #4CAF50;
            border-bottom: 2px solid #4CAF50;
            padding-bottom: 10px;
        }
        .button {
            padding: 12px 24px;
            margin: 5px;
            background: linear-gradient(135deg, #4CAF50 0%, #45a049 100%);
            color: white;
            border: none;
            border-radius: 25px;
            cursor: pointer;
            font-size: 14px;
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(76, 175, 80, 0.3);
        }
        .button:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(76, 175, 80, 0.4);
        }
        .button:active {
            transform: translateY(0);
        }
        .button.danger {
            background: linear-gradient(135deg, #f44336 0%, #d32f2f 100%);
            box-shadow: 0 4px 15px rgba(244, 67, 54, 0.3);
        }
        .input-group {
            margin: 10px 0;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        .input-group label {
            min-width: 80px;
            font-weight: bold;
        }
        .input-group input, .input-group select {
            flex: 1;
            padding: 8px 12px;
            border: 2px solid #ddd;
            border-radius: 5px;
            font-size: 14px;
        }

        /* 新增样式：虚拟摇杆和底盘控制 */
        .joystick-container {
            position: relative;
            width: 200px;
            height: 200px;
            margin: 20px auto;
            background: radial-gradient(circle, #f0f0f0 0%, #ddd 100%);
            border-radius: 50%;
            border: 3px solid #4CAF50;
            box-shadow: inset 0 0 20px rgba(0,0,0,0.1);
        }

        .joystick-knob {
            position: absolute;
            width: 60px;
            height: 60px;
            background: linear-gradient(135deg, #4CAF50 0%, #45a049 100%);
            border-radius: 50%;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            cursor: grab;
            box-shadow: 0 4px 15px rgba(76, 175, 80, 0.4);
            transition: all 0.1s ease;
        }

        .joystick-knob:active {
            cursor: grabbing;
            transform: translate(-50%, -50%) scale(1.1);
        }

        .chassis-status {
            background: #e8f5e8;
            border: 2px solid #4CAF50;
            border-radius: 10px;
            padding: 15px;
            margin: 10px 0;
        }

        .status-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
        }

        .status-indicator.green { background: #4CAF50; }
        .status-indicator.red { background: #f44336; }
        .status-indicator.yellow { background: #ff9800; }

        .emergency-stop {
            background: linear-gradient(135deg, #f44336 0%, #d32f2f 100%);
            color: white;
            font-size: 18px;
            font-weight: bold;
            padding: 15px 30px;
            border: none;
            border-radius: 10px;
            cursor: pointer;
            box-shadow: 0 4px 15px rgba(244, 67, 54, 0.4);
            margin: 10px;
        }

        .speed-slider {
            width: 100%;
            height: 8px;
            border-radius: 5px;
            background: #ddd;
            outline: none;
            -webkit-appearance: none;
        }

        .speed-slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: #4CAF50;
            cursor: pointer;
        }

        .speed-display {
            font-size: 24px;
            font-weight: bold;
            color: #4CAF50;
            text-align: center;
            margin: 10px 0;
        }
        .input-group input:focus, .input-group select:focus {
            outline: none;
            border-color: #4CAF50;
        }
        .status-display {
            background: #e8f5e8;
            border: 2px solid #4CAF50;
            border-radius: 10px;
            padding: 15px;
            margin: 10px 0;
            font-family: monospace;
            font-size: 14px;
        }
        .sensor-value {
            font-size: 1.5em;
            font-weight: bold;
            color: #4CAF50;
            text-align: center;
            margin: 10px 0;
        }
        .progress-bar {
            width: 100%;
            height: 20px;
            background: #ddd;
            border-radius: 10px;
            overflow: hidden;
            margin: 10px 0;
        }
        .progress-fill {
            height: 100%;
            background: linear-gradient(90deg, #4CAF50, #45a049);
            transition: width 0.3s ease;
        }
        .grid-2 {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 10px;
        }
        .grid-3 {
            display: grid;
            grid-template-columns: 1fr 1fr 1fr;
            gap: 10px;
        }
        @media (max-width: 768px) {
            .content {
                grid-template-columns: 1fr;
            }
            .grid-2, .grid-3 {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
    <div class="main-container">
        <div class="header">
            <h1>🤖 ESP32-S3 Robot Controller</h1>
            <p>Advanced Multi-Function Robot Control System</p>
        </div>

        <div class="content">
            <!-- System Status Card -->
            <div class="card">
                <h3>📊 System Status</h3>
                <div class="status-display" id="systemStatus">
                    <div>Status: <span id="status">Loading...</span></div>
                    <div>WiFi: <span id="wifiInfo">Connecting...</span></div>
                    <div>Uptime: <span id="uptime">0s</span></div>
                    <div>Memory: <span id="memory">0%</span></div>
                    <div>Temperature: <span id="temperature">0°C</span></div>
                </div>
                <button class="button" onclick="refreshStatus()">🔄 Refresh</button>
            </div>

            <!-- Servo Control Card -->
            <div class="card">
                <h3>🔧 Servo Control</h3>
                <div class="input-group">
                    <label>Servo:</label>
                    <select id="servoSelect">
                        <option value="1">Servo 1</option>
                        <option value="2">Servo 2</option>
                        <option value="3">Servo 3</option>
                    </select>
                </div>
                <div class="input-group">
                    <label>Angle:</label>
                    <input type="range" id="servoAngle" min="0" max="360" value="90" oninput="updateAngleDisplay()">
                    <span id="angleDisplay">90°</span>
                </div>
                <div class="grid-2">
                    <button class="button" onclick="moveServo()">Move Servo</button>
                    <button class="button" onclick="moveAllServos()">Move All</button>
                </div>
                <div class="status-display" id="servoStatus">
                    Servo 1: 90° | Servo 2: 90° | Servo 3: 90°
                </div>
            </div>

            <!-- Sensor Data Card -->
            <div class="card">
                <h3>📡 Sensor Data</h3>
                <div>
                    <h4>Grayscale Sensors</h4>
                    <div class="sensor-value" id="grayValues">0, 0, 0, 0, 0</div>
                    <button class="button" onclick="toggleGraySensor()">Toggle Continuous</button>
                </div>
                <div>
                    <h4>Laser Distance</h4>
                    <div class="sensor-value" id="laserDistance">0 mm</div>
                    <div class="progress-bar">
                        <div class="progress-fill" id="distanceProgress" style="width: 0%"></div>
                    </div>
                    <button class="button" onclick="toggleLaserSensor()">Toggle Continuous</button>
                </div>
            </div>

            <!-- Enhanced Chassis Control Card -->
            <div class="card">
                <h3>🚗 Enhanced Chassis Control</h3>

                <!-- Emergency Stop Button -->
                <div style="text-align: center; margin-bottom: 15px;">
                    <button class="emergency-stop" onclick="emergencyStop()" id="emergencyBtn">
                        🛑 EMERGENCY STOP
                    </button>
                    <button class="button" onclick="clearEmergencyStop()" id="clearEmergencyBtn" style="display: none;">
                        ✅ Clear Emergency
                    </button>
                </div>

                <!-- Chassis Status Display -->
                <div class="chassis-status" id="chassisStatusDisplay">
                    <div><span class="status-indicator green" id="statusIndicator"></span>Status: <span id="chassisState">Stopped</span></div>
                    <div>Speed: <span id="currentSpeed">0</span>% → <span id="targetSpeed">0</span>%</div>
                    <div>Position: X=<span id="posX">0</span>mm, Y=<span id="posY">0</span>mm, θ=<span id="posAngle">0</span>°</div>
                </div>

                <!-- Virtual Joystick -->
                <div class="joystick-container" id="joystickContainer">
                    <div class="joystick-knob" id="joystickKnob"></div>
                </div>

                <!-- Speed Control -->
                <div class="input-group">
                    <label>Max Speed:</label>
                    <input type="range" class="speed-slider" id="chassisSpeed" min="5" max="100" value="50" oninput="updateChassisSpeedDisplay()">
                </div>
                <div class="speed-display" id="chassisSpeedDisplay">50%</div>

                <!-- Traditional Button Controls -->
                <div class="grid-2" style="margin-top: 15px;">
                    <button class="button" onmousedown="startChassisMove('forward')" onmouseup="stopChassisMove()" onmouseleave="stopChassisMove()">↑ Forward</button>
                    <button class="button" onmousedown="startChassisMove('backward')" onmouseup="stopChassisMove()" onmouseleave="stopChassisMove()">↓ Backward</button>
                </div>
                <div style="text-align: center; margin-top: 10px;">
                    <button class="button danger" onclick="stopChassisMove()">⏹ Stop</button>
                </div>

                <div style="text-align: center; color: #666; font-size: 12px; margin-top: 10px;">
                    Enhanced with smooth acceleration and safety features
                </div>
            </div>

            <!-- Stepper Motor Control Card -->
            <div class="card">
                <h3>⚙️ Stepper Motors</h3>
                <div class="input-group">
                    <label>Motor:</label>
                    <select id="motorSelect">
                        <option value="1">Motor 1</option>
                        <option value="2">Motor 2</option>
                        <option value="3">Motor 3</option>
                        <option value="4">Motor 4</option>
                    </select>
                </div>
                <div class="input-group">
                    <label>Steps:</label>
                    <input type="number" id="motorSteps" value="100" min="-10000" max="10000">
                </div>
                <div class="input-group">
                    <label>Speed:</label>
                    <input type="range" id="motorSpeed" min="1" max="200" value="50" oninput="updateSpeedDisplay()">
                    <span id="speedDisplay">50</span>
                </div>
                <div class="grid-2">
                    <button class="button" onclick="controlStepper()">Move Steps</button>
                    <button class="button danger" onclick="stopAllMotors()">Stop All</button>
                </div>
                <div class="grid-2" style="margin-top: 10px;">
                    <button class="button" onmousedown="startContinuousRotation('forward')" onmouseup="stopContinuousRotation()" onmouseleave="stopContinuousRotation()">Continuous Forward</button>
                    <button class="button" onmousedown="startContinuousRotation('backward')" onmouseup="stopContinuousRotation()" onmouseleave="stopContinuousRotation()">Continuous Backward</button>
                </div>
                <div class="grid-2" style="margin-top: 5px;">
                    <button class="button" style="background: #ff6b35;" onmousedown="startFastRotation('forward')" onmouseup="stopContinuousRotation()" onmouseleave="stopContinuousRotation()">Fast Forward</button>
                    <button class="button" style="background: #ff6b35;" onmousedown="startFastRotation('backward')" onmouseup="stopContinuousRotation()" onmouseleave="stopContinuousRotation()">Fast Backward</button>
                </div>
                <div class="grid-2" style="margin-top: 5px;">
                    <button class="button" style="background: #e74c3c;" onmousedown="startTurboRotation('forward')" onmouseup="stopContinuousRotation()" onmouseleave="stopContinuousRotation()">⚡ Turbo Forward</button>
                    <button class="button" style="background: #e74c3c;" onmousedown="startTurboRotation('backward')" onmouseup="stopContinuousRotation()" onmouseleave="stopContinuousRotation()">⚡ Turbo Backward</button>
                </div>
                <div class="status-display" id="motorStatus">
                    All motors stopped
                </div>
            </div>
        </div>
    </div>

    <script>
        let refreshInterval;

        // Initialize page
        window.onload = function() {
            refreshStatus();
            startAutoRefresh();
        };

        function startAutoRefresh() {
            refreshInterval = setInterval(refreshStatus, 2000);
        }

        function refreshStatus() {
            fetch('/system_status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('status').textContent = 'Online';
                    document.getElementById('wifiInfo').textContent = data.wifi_ssid + ' (' + data.wifi_ip + ')';
                    document.getElementById('uptime').textContent = data.uptime + 's';
                    document.getElementById('memory').textContent = data.free_memory + '%';
                    document.getElementById('temperature').textContent = data.cpu_temp + '°C';

                    // Update servo status
                    if (data.servos) {
                        let servoText = '';
                        data.servos.forEach((servo, index) => {
                            servoText += `Servo ${index + 1}: ${servo.angle}° `;
                        });
                        document.getElementById('servoStatus').textContent = servoText;
                    }
                })
                .catch(error => {
                    document.getElementById('status').textContent = 'Error';
                    console.error('Error:', error);
                });
        }

        function updateAngleDisplay() {
            const angle = document.getElementById('servoAngle').value;
            document.getElementById('angleDisplay').textContent = angle + '°';
        }

        function updateSpeedDisplay() {
            const speed = document.getElementById('motorSpeed').value;
            document.getElementById('speedDisplay').textContent = speed;
        }

        function updateChassisSpeedDisplay() {
            const speed = document.getElementById('chassisSpeed').value;
            document.getElementById('chassisSpeedDisplay').textContent = speed + '%';
        }

        function moveServo() {
            const servo = document.getElementById('servoSelect').value;
            const angle = document.getElementById('servoAngle').value;

            fetch('/move_servo', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: `servo=${servo}&angle=${angle}`
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    alert('Servo moved successfully!');
                    refreshStatus();
                } else {
                    alert('Error: ' + data.message);
                }
            })
            .catch(error => {
                alert('Communication error');
                console.error('Error:', error);
            });
        }

        function moveAllServos() {
            const angle = document.getElementById('servoAngle').value;

            fetch('/move_all_servos', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: `angle1=${angle}&angle2=${angle}&angle3=${angle}`
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    alert('All servos moved successfully!');
                    refreshStatus();
                } else {
                    alert('Error: ' + data.message);
                }
            })
            .catch(error => {
                alert('Communication error');
                console.error('Error:', error);
            });
        }

        function toggleGraySensor() {
            fetch('/toggle_gray', { method: 'POST' })
            .then(response => response.json())
            .then(data => {
                alert(data.message);
                if (data.enabled) {
                    startGraySensorUpdates();
                }
            })
            .catch(error => console.error('Error:', error));
        }

        function toggleLaserSensor() {
            fetch('/toggle_laser', { method: 'POST' })
            .then(response => response.json())
            .then(data => {
                alert(data.message);
                if (data.enabled) {
                    startLaserSensorUpdates();
                }
            })
            .catch(error => console.error('Error:', error));
        }

        function startGraySensorUpdates() {
            setInterval(() => {
                fetch('/gray_data')
                .then(response => response.json())
                .then(data => {
                    if (data.values) {
                        document.getElementById('grayValues').textContent = data.values.join(', ');
                    }
                })
                .catch(error => console.error('Error:', error));
            }, 500);
        }

        function startLaserSensorUpdates() {
            setInterval(() => {
                fetch('/laser_data')
                .then(response => response.json())
                .then(data => {
                    if (data.distance !== undefined) {
                        document.getElementById('laserDistance').textContent = data.distance + ' ' + data.unit;
                        const progress = Math.min(100, (data.distance / 2000) * 100);
                        document.getElementById('distanceProgress').style.width = progress + '%';
                    }
                })
                .catch(error => console.error('Error:', error));
            }, 200);
        }

        function controlStepper() {
            const motor = document.getElementById('motorSelect').value;
            const steps = document.getElementById('motorSteps').value;
            const speed = document.getElementById('motorSpeed').value;

            fetch('/control_stepper', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: `motor=${motor}&steps=${steps}&speed=${speed}`
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    alert('Motor command sent successfully!');
                    document.getElementById('motorStatus').textContent = `Motor ${motor} moving ${steps} steps at speed ${speed}`;
                } else {
                    alert('Error: ' + data.message);
                }
            })
            .catch(error => {
                alert('Communication error');
                console.error('Error:', error);
            });
        }

        function stopAllMotors() {
            for (let i = 1; i <= 4; i++) {
                fetch('/control_stepper', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                    body: `motor=${i}&steps=0&speed=0`
                });
            }
            document.getElementById('motorStatus').textContent = 'All motors stopped';
            alert('All motors stopped!');
        }

        // Enhanced chassis control variables
        let chassisInterval = null;
        let joystickActive = false;
        let emergencyStopActive = false;

        // Initialize joystick functionality
        window.addEventListener('load', function() {
            initializeJoystick();
            startChassisStatusUpdate();
        });

        function initializeJoystick() {
            const container = document.getElementById('joystickContainer');
            const knob = document.getElementById('joystickKnob');
            let isDragging = false;
            let centerX = container.offsetWidth / 2;
            let centerY = container.offsetHeight / 2;
            let maxRadius = (container.offsetWidth / 2) - 30;

            function updateJoystick(x, y) {
                const deltaX = x - centerX;
                const deltaY = y - centerY;
                const distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY);

                if (distance <= maxRadius) {
                    knob.style.left = x + 'px';
                    knob.style.top = y + 'px';
                } else {
                    const angle = Math.atan2(deltaY, deltaX);
                    const limitedX = centerX + Math.cos(angle) * maxRadius;
                    const limitedY = centerY + Math.sin(angle) * maxRadius;
                    knob.style.left = limitedX + 'px';
                    knob.style.top = limitedY + 'px';
                }

                // Calculate direction and speed from joystick position
                const normalizedX = (knob.offsetLeft - centerX) / maxRadius;
                const normalizedY = (knob.offsetTop - centerY) / maxRadius;

                if (Math.abs(normalizedY) > 0.1) { // Dead zone
                    const speed = Math.min(Math.abs(normalizedY) * 100, 100);
                    const direction = normalizedY < 0 ? 'forward' : 'backward';
                    joystickChassisMove(direction, speed);
                } else {
                    if (joystickActive) {
                        stopChassisMove();
                    }
                }
            }

            // Mouse events
            knob.addEventListener('mousedown', function(e) {
                isDragging = true;
                joystickActive = true;
                e.preventDefault();
            });

            document.addEventListener('mousemove', function(e) {
                if (isDragging && !emergencyStopActive) {
                    const rect = container.getBoundingClientRect();
                    const x = e.clientX - rect.left;
                    const y = e.clientY - rect.top;
                    updateJoystick(x, y);
                }
            });

            document.addEventListener('mouseup', function() {
                if (isDragging) {
                    isDragging = false;
                    joystickActive = false;
                    // Return knob to center
                    knob.style.left = centerX + 'px';
                    knob.style.top = centerY + 'px';
                    stopChassisMove();
                }
            });

            // Touch events for mobile
            knob.addEventListener('touchstart', function(e) {
                isDragging = true;
                joystickActive = true;
                e.preventDefault();
            });

            document.addEventListener('touchmove', function(e) {
                if (isDragging && !emergencyStopActive) {
                    const rect = container.getBoundingClientRect();
                    const touch = e.touches[0];
                    const x = touch.clientX - rect.left;
                    const y = touch.clientY - rect.top;
                    updateJoystick(x, y);
                    e.preventDefault();
                }
            });

            document.addEventListener('touchend', function() {
                if (isDragging) {
                    isDragging = false;
                    joystickActive = false;
                    knob.style.left = centerX + 'px';
                    knob.style.top = centerY + 'px';
                    stopChassisMove();
                }
            });
        }

        function joystickChassisMove(direction, speed) {
            if (emergencyStopActive) return;

            const maxSpeed = document.getElementById('chassisSpeed').value;
            const actualSpeed = Math.min(speed, maxSpeed);

            fetch('/control_chassis', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: `direction=${direction}&speed=${actualSpeed}`
            });
        }

        function startChassisMove(direction) {
            if (emergencyStopActive) return;

            const speed = document.getElementById('chassisSpeed').value;

            // Stop previous movement
            if (chassisInterval) {
                clearInterval(chassisInterval);
            }

            // Start continuous movement
            chassisInterval = setInterval(() => {
                fetch('/control_chassis', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                    body: `direction=${direction}&speed=${speed}`
                });
            }, 100); // 100ms interval for button control
        }

        function stopChassisMove() {
            if (chassisInterval) {
                clearInterval(chassisInterval);
                chassisInterval = null;
            }

            fetch('/control_chassis', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: 'direction=stop&speed=0'
            });
        }

        function emergencyStop() {
            emergencyStopActive = true;
            stopChassisMove();

            fetch('/emergency_stop', {
                method: 'POST'
            })
            .then(response => response.json())
            .then(data => {
                document.getElementById('emergencyBtn').style.display = 'none';
                document.getElementById('clearEmergencyBtn').style.display = 'inline-block';
                document.getElementById('statusIndicator').className = 'status-indicator red';
                document.getElementById('chassisState').textContent = 'EMERGENCY STOP';
                alert('Emergency stop activated!');
            })
            .catch(error => {
                console.error('Emergency stop error:', error);
            });
        }

        function clearEmergencyStop() {
            fetch('/clear_emergency_stop', {
                method: 'POST'
            })
            .then(response => response.json())
            .then(data => {
                emergencyStopActive = false;
                document.getElementById('emergencyBtn').style.display = 'inline-block';
                document.getElementById('clearEmergencyBtn').style.display = 'none';
                document.getElementById('statusIndicator').className = 'status-indicator green';
                document.getElementById('chassisState').textContent = 'Ready';
                alert('Emergency stop cleared!');
            })
            .catch(error => {
                console.error('Clear emergency stop error:', error);
            });
        }

        function startChassisStatusUpdate() {
            setInterval(() => {
                if (!emergencyStopActive) {
                    fetch('/chassis_status')
                        .then(response => response.json())
                        .then(data => {
                            document.getElementById('currentSpeed').textContent = data.currentSpeed;
                            document.getElementById('targetSpeed').textContent = data.targetSpeed;
                            document.getElementById('posX').textContent = data.position.x;
                            document.getElementById('posY').textContent = data.position.y;
                            document.getElementById('posAngle').textContent = data.position.angle;

                            if (data.isMoving) {
                                document.getElementById('chassisState').textContent = 'Moving';
                                document.getElementById('statusIndicator').className = 'status-indicator yellow';
                            } else if (!data.emergencyStop) {
                                document.getElementById('chassisState').textContent = 'Stopped';
                                document.getElementById('statusIndicator').className = 'status-indicator green';
                            }
                        })
                        .catch(error => {
                            console.error('Chassis status error:', error);
                        });
                }
            }, 500); // Update every 500ms
        }

        // 步进电机连续转动变量
        let motorInterval = null;

        function startContinuousRotation(direction) {
            const motor = document.getElementById('motorSelect').value;
            const speed = document.getElementById('motorSpeed').value;
            const steps = direction === 'forward' ? 200 : -200; // 增加到200步

            // 停止之前的转动
            if (motorInterval) {
                clearInterval(motorInterval);
            }

            // 开始连续转动
            motorInterval = setInterval(() => {
                fetch('/control_stepper', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                    body: `motor=${motor}&steps=${steps}&speed=${speed}`
                });
            }, 50); // 减少到50ms间隔，更流畅

            document.getElementById('motorStatus').textContent = `Motor ${motor} continuous ${direction} at speed ${speed}`;
        }

        function startFastRotation(direction) {
            const motor = document.getElementById('motorSelect').value;
            const speed = Math.min(parseInt(document.getElementById('motorSpeed').value) + 50, 200); // 增加50速度，最大200
            const steps = direction === 'forward' ? 500 : -500; // 更大的步数

            // 停止之前的转动
            if (motorInterval) {
                clearInterval(motorInterval);
            }

            // 开始快速连续转动
            motorInterval = setInterval(() => {
                fetch('/control_stepper', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                    body: `motor=${motor}&steps=${steps}&speed=${speed}`
                });
            }, 30); // 30ms间隔，更快

            document.getElementById('motorStatus').textContent = `Motor ${motor} FAST ${direction} at speed ${speed}`;
        }

        function startTurboRotation(direction) {
            const motor = document.getElementById('motorSelect').value;
            const speed = 255; // 最大速度
            const steps = direction === 'forward' ? 1000 : -1000; // 最大步数

            // 停止之前的转动
            if (motorInterval) {
                clearInterval(motorInterval);
            }

            // 开始超快速连续转动
            motorInterval = setInterval(() => {
                fetch('/control_stepper', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                    body: `motor=${motor}&steps=${steps}&speed=${speed}`
                });
            }, 20); // 20ms间隔，最快

            document.getElementById('motorStatus').textContent = `Motor ${motor} ⚡TURBO⚡ ${direction} at MAX speed`;
        }

        function stopContinuousRotation() {
            if (motorInterval) {
                clearInterval(motorInterval);
                motorInterval = null;
            }

            const motor = document.getElementById('motorSelect').value;
            fetch('/control_stepper', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: `motor=${motor}&steps=0&speed=0`
            });

            document.getElementById('motorStatus').textContent = `Motor ${motor} stopped`;
        }
    </script>
</body>
</html>
)rawliteral";

#endif // HTML_PAGE_H