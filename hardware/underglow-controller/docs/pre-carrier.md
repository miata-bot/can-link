# Pre Carrier AIO Board

Originally, my vision for this project was just one single board with all the components required to run in a single closure. The global chip shortage made me realize that the design should probably be more modular, that way individual systems or circuits could be replaced without having to redesign the entire board. This is particularly important for

1) the power tree - every time I design around a PMIC, it's out of stock before I finish the design
2) the CAN interface - the automotive electronics market is really wacky right now

## Prototypes

![BoneBlue](/hardware/carrier/images/beagleblue-prototype.jpg)
![BoneBlue Wiring](/hardware/carrier/images/prototype-wiring.jpg)

## Renders

### V1 (04/03/22)

Initial design. Could not source components, notably the main OSD335x CPU

![V1](/hardware/carrier/images/v1-PCB-Render.png)

### V2 (04/10/22)

Redesign using CM4 module with supporting chips for extra features

![V2](/hardware/carrier/images/v2-PCB-Render.png)

### V2 Rev A (04/12/22)

Revision of the intial V2 design to support a TE connectivity case/connector

![V2 RevA](/hardware/carrier/images/v2/v2-RevA-PCB-Render-Front-No-Case.png)
![V2 RevA](/hardware/carrier/images/v2/v2-RevA-PCB-Render-Back-No-Case.png)
![V2 RevA](/hardware/carrier/images/v2/v2-RevA-PCB-Render-Front-Case.png)
![V2 RevA](/hardware/carrier/images/v2/v2-RevA-PCB-Render-Back-Case.png)
