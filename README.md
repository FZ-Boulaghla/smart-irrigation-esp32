# Smart Irrigation System with ESP32

This project implements an intelligent irrigation system using an ESP32 board. 
It monitors soil moisture and controls a water pump through a RESTful API. 
An LCD screen displays real-time data locally.

## 🚀 Features

- Wi-Fi connectivity
- REST API with two endpoints:
  - `POST /api/pump` to turn the pump ON/OFF
  - `GET /api/soilMoisture` to read soil humidity
- Soil moisture monitoring using analog input
- Pump control via relay
- LCD I2C display for moisture and pump status

## 🧰 Hardware Requirements

- ESP32 development board
- Soil moisture sensor (analog)
- Relay module
- DC pump (optional)
- LCD I2C 16x2 display
- Jumper wires and breadboard
- Wi-Fi network

## ⚙️ Wiring

| Component        | ESP32 Pin |
|------------------|-----------|
| Relay IN         | GPIO 4    |
| Soil sensor OUT  | GPIO 33   |
| LCD SDA          | GPIO 21   |
| LCD SCL          | GPIO 22   |

**Note**: Double-check I2C pins depending on your ESP32 board.

## Setup
Install the required libraries in Arduino IDE:

-WiFi.h

-WebServer.h

-LiquidCrystal_I2C.h

-ArduinoJson.h

Upload the code to your ESP32.

Connect to the IP address shown on the LCD screen.

Use Postman or another HTTP client to interact with the endpoints.
