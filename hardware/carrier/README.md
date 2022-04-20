# Hardware Designfiles

## Prototypes

![BoneBlue](images/beagleblue-prototype.jpg)
![BoneBlue Wiring](images/prototype-wiring.jpg)

## Renders

### V1 (04/03/22)

Initial design. Could not source components, notably the main OSD335x CPU

![V1](images/v1-PCB-Render.png)

### V2 (04/10/22)

Redesign using CM4 module with supporting chips for extra features

![V2](images/v2-PCB-Render.png)

### V2 Rev A (04/12/22)

Revision of the intial V2 design to support a TE connectivity case/connector

![V2 RevA](images/v2-RevA-PCB-Render-Front-No-Case.png)
![V2 RevA](images/v2-RevA-PCB-Render-Back-No-Case.png)
![V2 RevA](images/v2-RevA-PCB-Render-Front-Case.png)
![V2 RevA](images/v2-RevA-PCB-Render-Back-Case.png)

## Features and Core Components

|name                |function                       |
|--------------------|-------------------------------|
|CM4 Module          |Main CPU                       |
|RP2040              |GPIO expander                  |
|MCP2515             |CAN Interface                  |
|RFM69 Packet Radio  |Sub GHz radio for local syncing|
|12 Low side drivers |PWM capable for driving LEDs   |
|4 TTL level outputs |Individually addressable LEDs  |
