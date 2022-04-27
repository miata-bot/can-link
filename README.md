# Underglow Sync Device

Very much work in progress software and hardware project built with [Nerves](https://www.nerves-project.org/), [Flutter](https://flutter.dev/), [Kicad](https://www.kicad.org/), [Zig](https://ziglang.org/), [Lua](https://www.lua.org) and pretty much any other tech buzz word you might want. See [The requirements doc](Requirements.md) for the guts of the project.

![render](/hardware/carrier/images/v2-RevA-PCB-Render-Front-Case.png)

## TLDR

Here's a [Twitter thread](https://twitter.com/pressy4pie/status/1504309482054029315) showing a demo of the ECU integration.
[video link](https://video.twimg.com/ext_tw_video/1504309386151301125/pu/vid/720x1280/zwIcUEVZBSSQAASE.mp4?tag=12).

Here's a [Twitter thread](https://twitter.com/pressy4pie/status/1506779115122139137) showing a demo of the GPS and companion application. [video link](https://video.twimg.com/ext_tw_video/1506778163891740673/pu/vid/720x1630/oxYdaMkSowD5uJmU.mp4?tag=12).

I've written up a few posts on the progress of the project

* [Original hardware design](https://cone.codes/posts/can-link/)
* [Short post on using PWM to drive LEDS](https://cone.codes/posts/nerves-pwm/)
* [Hardware redesign](https://cone.codes/posts/can-link-pt-2/)
* [Hardware reredesign](https://cone.codes/posts/can-link-pt-3/)
* [Hardware rereredesign](https://cone.codes/posts/can-link-pt-4/)
* [Designing a Development test board](https://cone.codes/posts/imx6-devboard/)

## Description

This README acts as a root for all the jillion projects contained within the repo. There is no inherit relationship between any folders. Some are Elixir, some are flutter, etc. One day clean it up. Too much proof of concepting going on right now.

| Folder Name | Description | Language | License |
| ------------| ----------- | -------- | ------- |
| [hardware](hardware/README.md) | Main hardware designs. Contains schematics, layouts, boms, manufacturing gerbers etc. | KiCad | Creative Commons |
| [firmware](firmware/README.md) | Main Nerves Firmware project. | Elixir | MIT |
| [socket_can](elixir-socket-can/README.md) | CAN interface library fork | Elixir | ??? |
| [canscript](canscript/README.md) | Main Lua environment and runtime. | Zig | MIT |
| [can_companion](can_companion/README.md) | proof of concept mobile app for interfacing GPS | Flutter | MIT |
| [elixir-GPSd](elixir-gpsd/README.md) | GPS library | Elixir | MIT |

There are auxilary projects involved in the development of this one. Notably:

* [nerves_system_bbb_can](https://github.com/connorrigby/nerves_system_bbb_can) - Nerves base system for this device. There are so many changes now, that it should be moved here.
* [elixir-rf69](https://github.com/connorrigby/elixir-rf69) - Elixir library for interfacing the RF69 Radio
