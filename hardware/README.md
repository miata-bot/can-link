# Hardware Design

Here you'll find the hub for all the sub modules that make up the entire device. There are two major components, the `carrier` board, and the `platform` board. In short, the `carrier` has all the components to run "standalone". It primarily handles power management and interfacing the actual loaded outputs.
The `platform` board has the full Linux capable SOC and related components, USB ports, power sequencer, SD interface, WiFi, BT etc. I debated calling this the `communications` board.

| Folder | Description |
| -------| ----------- |
| [`carrier`](/hardware/carrier/README.md) | Carrier board |
| [`platform-i.mx6`](/hardware/platform-i.mx6/README.md) | i.mx6 platform board |
