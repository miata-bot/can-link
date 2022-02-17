# UNDERGLOW / RGB SYNC PROJECT

I want to make a device that can syncronize underglow
or other RGB lights between two or more cars. Below is a high-level list of all the things I want out of the project.

0) This is not a for profit product, I'm building this for fun.
1) The device should have a CAN interface for aftermarket ECUs and OBD2 capable cars. This notably leaves out interfacing older pre-obd2 cars right now. See [Non CAN enabled applications](#non-can-enabled-systems) for justification and workarounds. See [ECU Support](#ecu-support) for a list of planned support.
2) The device should be WiFi/Bluetooth capable, which means it should be a Linux capable platform. See the [Hardware](#hardware) section.
3) Every component should be open source where applicable. See the [Licensing](#license) section.
4) It doesn't need to be the cheapest thing. See the [Pricing](#pricing).
5) The device should work offline. See the [Offline Use](#offline-use).
6) The device should create a mesh network to sync devices together. See [Packet Radio](#packet-radio).

## Non CAN Enabled Applications

I decided early on that I simply do not care about this enough to spend time on it. A potential solution to this is an external hardware one. That is, If one wants to inerface a non CAN enabled system, all they would need to do is design their own interface system, and then put their own CAN layer on top. This turns the issue into a software problem, where the interface may be added as a decoder plugin. See the [Firmware Plugin System](#Firmware-Plugin-System) for more info.

## ECU Support

I've written code to support several ECUs already. Much care was put into the system to define how decoding works. It's not "feature complete" yet, but the scafolding is put in place already for dynamic decoder definition. What this means is that I will add support for a handful of common systems up front as I've already done, but adding new ones should be quite easy.

Here's the current supported platform matrix:

| ECU        | Coded? | Tested? | Note  |
| ---------- | ------ | ------- | ----- |
| MegaSquirt | yes    | yes     | Broadcast protocol with default ID |
| Haltech    | yes    | yes     | Broadcast protocol |
| Maxx       | yes    | no      | Default broadcast protocol, too extensible to test every combo? |
| OBD2       | no     | no      | Enabled in Linux, need to decode messages somehow |
| Speeduino  | no     | no      | Very similar to MS proto iirc |

## Hardware

I've tried this project a few times in the past, and every single time I wish I just started with Linux. I'm sure it's possible to do in an RTOS or even an Arduino system etc. It's just to hard to reason about a system with so much connectivity for me. This puts a much higher price tag because the components selected for the board are just by nature quite a bit more expensive. See the pricing section.

Here's a table for the "main" components. See the full BOM for exact component selection.

| Part Name | Purpose | Required? |
| ----------| --------| --------- |
| OSD358x      | Main CPU, RAM, PMIC               | yes                 |
| CAN Xceiver  | CAN interface                     | yes (but maybe no?) |
| BT / WIFI    | Phone Companion app               | no                  |
| Packet radio | External syncing with other nodes | yes   |
| LIPO Charger | Battery backup | no |
| GPS          | Accurate positioning | no |
| Compass      | Direction            | no |
| IMU          | Logging?             | no |
| USB1         | Mass storage, configuration | yes |
| USB2         | debug, firmware dev  | yes |

## Firmware

The firmware is the most interesting part to me. I've proof of concepted all the core functions. Below is a list of the main technology buzzwords used. See the SBOM for a more exact list of all techs used. (this file doesn't exist yet).

* Embedded Linux Kernel
* Buildroot - framework used to create an embedded Linux distribution with UBoot bootloader, devicetrees, and root file system.
* Nerves - framework built on top of Buildroot to create embedded Linux firmware using the Elixir programming language.
* Elixir - programming language built on Erlang/OTP to build the main firmware.
* Flutter - UI Toolkit for building native apps using the Dart programming language.
* Dart - programming language developed for Flutter.
* Lua - programming language that will be used for the main plugin system.
* Zig - programming language to build the Lua interface.
* SocketCAN - Linux API for interfacing CAN Bus

## Plugin System

The main interface for the firmware is a Lua scripting environment. All of the core firmware exists to support and supplement this system. The primary reason for this is purely because I just want to build it. There are other reasons, including but not limited to:

* Lua is easier to learn and write.
* I can build a community plugin repo?
* I have friends that want to contribute and already know Lua.
* I want anyone to be able to write their own plugins.
* It's just cool.

## Packet Radio

Devices use a RF69 Packet radio to syncronize together. My goal is to keep this system as simple as possible. There should be an accompanying document for the implementation, but the gist of it is that there should be single "producer" node on a network, and many consumers. There may be many networks, they should not interfere. This is accomplished easily via the nature of the packet radios not delivering messages that cannot be decrypted. A network id will just be it's encryption key. This means that networks can be created and destroyed dynamically without collision provided a adequite IDing scheme. UUIDs should work for easy serialization and sharing via QR code or similar. Could also use the ATECCA chip's public key.

One potential idea I have with the radio is to create a "remote" that can control a device similar to the chinese kits. This is of course another piece of hardware that needs to be designed.

## Configuration Interface

There are three ways to configure the device. I plan on shipping a "default" configuration that should just work without any extra setup steps.

The device has a USB mass storage interface that allows storing arbitrary files. I want the main functionality to be implemented in "plugins" (see that section) that can be loaded at runtime. I will ship a default set of plugins to do the some common stuff.

The device has a USB serial port. This means it can be interfaced using existing tools. The first one that comes to mind is Tuner Studio. The INI file could even be stored on the mass storage interface.

The device has a BLE radio. This allows a mobile phone to configure the device via an app.

## Mobile Application

I added BLE onto the device to allow developing a mobile application. I don't really want to make this application myself. I'm hoping this is something I can trick open source passer-bys into building. I've used Flutter to develop a proof of concept application that interfaces the device but it's far from complete. If you're reading this and are interested, get in touch.

## Offline Use

One of my main goals for this device is to have it working in time for [Miatas at the Gap](https://www.gapmiatas.com/) at the end of the year. This means it cannot rely on an active internet connection for use. This does not mean that internet cannot be used, that's why i put a WiFi chip on the board. I don't have any firm planned features that require internet right now, but I do have some ideas. This requirement lead me to use a packet radio for syncing systems together in a meshing network. See [Packet Radio](#packet-radio).

## License

The entire project will be open source. Although I don't expect many direct contributions, I still want the ability for others to be involved with the development easily.

Hardware should be Creative Commons. Software will be mostly MIT or Apache 2. I haven't decied which one yet. Probably Apache 2.

## Pricing

I don't really expect this to be a viable product. The final "price" will probably be whatever it costs to make the device. I may do a croudfunded run of the device depending on what the final BOM turns out to cost. Right now my gut tells me each board will cost about $125 to produce. The passive components are pretty cheap, but there's a lot of them. The CPU is really hard to get right now due to the global chip shortage, same with the WiFi module etc. The connectors are also in short supply.

In the end I don't really care about the price of the device as long as I still find it fun to work on. If I get bored, or interest fades on the idea, I make no commitment to continue producing the device etc.
