# Carrier Board

This is the "top" half of the device. It does power management, CAN communication and output control.

![pcb-front](/hardware/carrier/images/v3/v3-front.png)

![pcb-front](/hardware/carrier/images/v3/v3-back.png)

## Major Components

| Name | Description |
| ---- | ----------- |
| RP2040 | Main MCU. dual core ARM M0 |
| 3.3v automotive regulator | Power supply for radio, CAN transciever, CAN interface and RP2040 |
| 5.0V automotive regulator | Power supply for the `platform` board |
| RFM69 Packet radio | Packet radio for wireless syncronization |
| Motor driver | 2 motor output motor driver |
| Dual footprint MCP2515 CAN interface | Very popular CAN interface IC. Both packages are placed in the case that one cannot be obtained |
| CAN transciever | driver for CAN communication |
| 2 Buttons | what can i say about buttons |
| 3 status LEDS | control em however u want |
| WS15 LED | first node in a chain of individually addressable LEDS |
