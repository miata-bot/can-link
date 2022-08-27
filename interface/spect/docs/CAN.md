# CAN Data Format

## CAN ID

| CAN ID | Data                    |
| ------ | ----------------------- |
| 0x69   | command::8, payload::56 |

Command is 1 byte, the payload can take up the remaining 7 bytes.

## Commands

| Command            | Name  | Data                                             |
| ------------------ | ----- | ------------------------------------------------ |
| 0x0::8             | PWR   | rgb0::1, rgb1::1, addr0::1, addr1::1             |
| 0x1::7, channel::1 | RGB   | r::8, g::8, b::8                                 |
| 0x2::7, channel::1 | Fade  | time_ms:: 16, r::8, g::8, b::8, r::8, g::8, b::8 |
| 0x3::7, channel::1 | Pixel | address::16, r::8, g::8, b::8                    |
| 0x4::7, channel::1 | Fill  | start::16, end::16, r::8, g::8, b::8             |
| 0x5::7, channel::1 | Blit  | --                                               |
| 0x6::8             | Mode  | mode::8                                          |

| Command | Description |
| ------- | ----------- |
| PWR     | controls which features are enabled. each feature is represented by a single bit |
| RGB     | Set a RGB channel color. 24 bit color |
| Fade    | Fade over `time_ms` from one 24 bit color to another |
| Pixel   | Set a pixel on an addressable channel to a 24 bit color |
| Fill    | Fill an addressable channel from `start` to `end` with a 24 bit color |
| Blit    | Show the current buffer |
| Mode    | change mode. See Modes section |

## Modes

| ID              | Name |
| ---             | ---- |
| 0x0             | Manual Control Mode |
| 0x1::7, txrx::1 | Radio Control Mode txrx=0 for "sender" txrx=1 for "receiver |
| 0x4             | Lua Scripting mode |
| 0x5::3, anim::5 | animation mode. 0=rainbow, 1-31=reserved for future use |
