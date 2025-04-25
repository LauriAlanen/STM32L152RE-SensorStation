# STM32L152RE Sensor Station

## Overview
The STM32L152RE Sensor Station is an embedded system project that integrates multiple environmental sensors to collect, process, and visualize various environmental parameters. Built around the STM32L152RE microcontroller, it provides a platform for real-time environmental monitoring.

![image](https://github.com/user-attachments/assets/51431ee9-8ab1-4629-a41c-886593c60b82)

## Web UI
![image](https://github.com/user-attachments/assets/09b69490-1776-4871-8267-e6ce6b43777d)
![image](https://github.com/user-attachments/assets/8451a06d-d581-4bf2-944d-fe88e14eaf98)



## Features

### Supported Sensors
- **LMT84LP**: Temperature sensor
- **DHT22**: Temperature and humidity sensor
- **NSL19M51**: Light intensity sensor
- **SGP30**: Air quality sensor (VOC and CO₂)

### Communication
- MODBUS protocol for sensor communication
- I2C interface for digital sensors
- UART interface for data transmission
- Python-based master application for data collection and visualization

#### MODBUS Implementation Details
The MODBUS implementation features:
- Fixed 8-byte frame size
- Efficient ring buffer for UART reception
- CRC16 verification for data integrity
- Frame validation and error handling
- Support for both raw and processed sensor data
- Customizable addressing scheme for each slave

#### Master Application Architecture
The Python-based master application provides:
- Modular sensor handling with dedicated classes for each sensor type
- Dynamic MODBUS frame construction and CRC calculation
- Real-time data processing and conversion
- Web-based visualization using Flask
- Responsive dashboard with real-time updates
- Sensor management interface

## Project Structure

```
STM32L152RE-SensorStation/
├── src/
│   ├── Peripherals/        # Hardware interface drivers
│   ├── Sensors/           # Sensor-specific implementations
│   ├── Master/            # Python web application
│   └── main.c            # Main firmware entry point
├── Drivers/               # STM32 HAL and CMSIS
└── README.md             # This file
```

## Hardware Requirements

- STM32L152RE Nucleo Board
- Supported Sensors:
  - LMT84LP Temperature Sensor
  - DHT22 Temperature/Humidity Sensor
  - NSL19M51 Light Sensor
  - SGP30 Air Quality Sensor
- Connection cables and breadboard

## Software Requirements

### Dependencies
- STM32 HAL Drivers
- CMSIS
- Flask (for web interface)
- Pyserial

## Setup Instructions

### Firmware
1. Clone the repository
2. Open the project in STM32CubeIDE
3. Build and flash to the STM32L152RE board

### Web Interface
1. Navigate to the Master directory:
   ```bash
   cd src/Master
   ```
2. Install Python dependencies:
   ```bash
   pip install -r pyserial flask
   ```
3. Run the application:
   ```bash
   python app.py
   ```
4. Access the web interface at `http://localhost:5000`

## Pin Configuration

| Sensor    | Pin  | Interface |
|-----------|------|-----------|
| LMT84LP   | PA0  | ADC      |
| NSL19M51  | PA1  | ADC      |
| DHT22     | PA7  | Digital  |
| SGP30     | PB8/9| I2C      |

## Usage

1. Power up the system
2. Access the web interface
3. Add sensors through the management interface
4. Monitor real-time data on the dashboard
