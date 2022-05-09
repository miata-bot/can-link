# ESP32-S3 Firmware

Source code for the main processor firmware

## What does it do?

Handles communication and control of the external peripherals:

| Peripheral | Description |
| - | - |
| RP2040 LED Controller | Handles the 6 outputs, and individually addressable LEDs |
| SDCard | Handles storage of config and plugins |
| WiFi | not used currently |
| BLE | Configuration via phone |
| Motor Control | generic motor controller. maybe pop up headlights? |
| RFM69 | handles syncing between multiple controllers |
| CAN | Handles communication with ECU |
