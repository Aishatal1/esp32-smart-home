# ESP32 Smart Home System

Web-controlled smart home system using ESP32. Control lights, curtains, and doorbell from any browser on the same WiFi network.

## Features

- **Lights:** ON/OFF control via relay (active-LOW)
- **Curtains:** Smooth servo motor control (0° to 180°)
- **Doorbell:** Physical button press triggers buzzer and updates status
- **Web Interface:** Control from any device browser
- **Status API:** JSON endpoint for real-time device states

## Hardware Used

| Component | Purpose |
|-----------|---------|
| ESP32 D1 Mini | Main controller with WiFi |
| 2x SG90 Servo Motors | Open/close curtains |
| 5V 2-channel Relay | Control lights |
| Push button | Doorbell |
| Passive buzzer | Sound notification |

## Pin Configuration

| Component | ESP32 Pin |
|-----------|-----------|
| Living Light Relay | GPIO14 (D5) |
| Bedroom Light Relay | GPIO12 (D6) |
| Living Curtain Servo | GPIO13 (D7) |
| Bedroom Curtain Servo | GPIO17 |
| Doorbell Button | GPIO5 (D1) |
| Buzzer | GPIO2 (D4) |

## How to Use

1. Upload the code to ESP32 using PlatformIO
2. Connect ESP32 to WiFi (credentials in code)
3. Find ESP32 IP address in serial monitor
4. Open browser to that IP address
5. Click buttons to control your home

## API Endpoints

| Endpoint | Action |
|----------|--------|
| `/living_on` | Living light ON |
| `/living_off` | Living light OFF |
| `/living_window_open` | Open living curtains |
| `/living_window_close` | Close living curtains |
| `/status` | Get JSON status |

## Author

Aisha Taleb - Computer Engineering, Palestine Ahliya University
