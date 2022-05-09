# RP2040 (Pico) Firmware

Source code for the firmware that runs on the RP2040 Coprocessor.

## What does it do?

This firmware (and device) is responsible for the following core functions:

* Low side output for PWMing RGB LEDs
* Individually addressable LED scripting (also known as Neopixel)

## How does it work?

The RP2040 is connected to the main ESP32-S3 MCU via SPI. The ESP32-S3 also controls the RP2040's `RUN` pin meaning it can turn it on and off. The commands are not yet implemented, but I'm planning ahead to have:

| Name  | opcode (uint8) | arguments (var) | response | description |
| - | - | - | - | - |
| ping |  0x00 | - | okay=0x1 error=0x0 |test for life |
| low side pwm | 0x01 | [index (see below), frequency (u16), duty cycle (u16)] | okay=0x1 error=0x0 | Set PWM config for one of the outputs |
| addressable led control | 0x02 | [address, r, g, b, a] |okay=0x1 error=0x0 | Set an LED color |

## Low Side Output Index

There are 6 outputs controllable by this command. They are indexed 0..6. The 7th bit is used to enable or disable the channel entirely.
