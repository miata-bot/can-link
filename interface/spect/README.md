# Software Developer Documentation

These documents are intended for
software developers of potential contributors to
the project's firmware.

## Getting started

First follow the [The getting started guide](Getting-Started.md).
This will take you thru setting up a dev environment to
flash and debug firmware.

## Build Firmware

```bash
cd project_root/firmware/spect
idf.py build
```

## Flash Firmware

1) hold the download button
2) power the device on

```bash
cd project_root/firmware/spect
idf.py flash
```

## Monitor Firmware

```bash
cd project_root/firmware/spect
idf.py monitor
```
