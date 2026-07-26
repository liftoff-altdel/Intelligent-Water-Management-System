# Intelligent Water Management and Monitoring System

An ESP32-based intelligent water management system designed to automate water level regulation, detect leakage, and monitor water quality. The project uses two ESP32 microcontrollers to separate control tasks from water quality monitoring, resulting in a modular and scalable system.

---

## Features

- Automatic water level control using PID
- Real-time water level measurement with an ultrasonic sensor
- Leakage detection using dual flow sensors
- Water quality monitoring using TDS and turbidity sensors
- Pump control through an L298N motor driver
- Modular architecture using two ESP32 boards
- Designed for future IoT integration

---

## System Architecture

### ESP32-1: Water Level & Leakage Controller
This controller is responsible for maintaining the water level and detecting leakage.

Components:
- ESP32
- HC-SR04 Ultrasonic Sensor
- 2 × YF-S201 Flow Sensors
- L298N Motor Driver
- DC Water Pumps

Functions:
- Measures tank water level
- Executes PID control
- Controls inlet and outlet pumps
- Detects leakage by comparing flow sensor readings

Code:
- `ESP32_Code/Water_Level_Leakage_Controller.ino`

---

### ESP32-2: Water Quality Monitoring
This controller continuously monitors water quality after the water reaches the receiver tank.

Components:
- ESP32
- TDS Sensor
- Turbidity Sensor

Functions:
- Measures Total Dissolved Solids (TDS)
- Measures turbidity
- Displays processed sensor readings

Code:
- `ESP32_Code/Water_Quality_Monitoring.ino`

---

## Repository Structure

```
Intelligent-Water-Management-System
│
├── README.md
├── ESP32_Code
│   ├── Water_Level_Leakage_Controller.ino
│   ├── Water_Quality_Monitoring.ino
│   └── Code_Description.md
├── Report
├── Documentation
└── Videos
```

---

## Hardware Used

- 2 × ESP32 Development Boards
- HC-SR04 Ultrasonic Sensor
- 2 × YF-S201 Flow Sensors
- TDS Sensor
- Turbidity Sensor
- L298N Motor Driver
- DC Water Pumps
- Breadboard and Jumper Wires
- Power Supply

---

## Working Principle

1. ESP32-1 measures the water level using the ultrasonic sensor.
2. A PID controller regulates the inlet and outlet pumps to maintain the desired water level.
3. Dual flow sensors monitor the water flow and help identify possible leakage.
4. Water entering the receiver tank is monitored by ESP32-2.
5. ESP32-2 measures TDS and turbidity to assess water quality.
6. The system can be extended with IoT platforms such as Blynk or ThingSpeak for remote monitoring.

---

## Future Improvements

- Cloud-based IoT monitoring
- Mobile application
- Automatic valve control
- Predictive leak detection
- Data logging and analytics

---

## Author

Sandesh Kumar Jha

Manipal Institute of Technology
