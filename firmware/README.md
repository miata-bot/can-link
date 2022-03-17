# CANLink

## rememberme
cmd("/sbin/ip link set can0 up type can bitrate 100000")

```
57  cmd("/sbin/ip link set can0 up type can bitrate 1000000")
58  cmd("cansend can1q 500#1E.10.10")
59  cmd("cansend can1 500#1E.10.10")
60  cmd("cansend can1 500#1E.69.10")
61  cmd("cansend can1 500#1E.69.69.69")
62  cmd("cansend can1 420#1E.69.69.69")
63  {:ok, can_port} = Ng.Can.start_link
64  Ng.Can.open(can_port, "can0", sndbuf: 1024, rcvbuf: 106496)
65  <<id::size(32)>> = <<1,2,3,4>>
66  frame = {id, <<1,2,3,4,5,6,7,8>>}
67  Ng.Can.write(can_port, frame)
68  Ng.Can.await_read(can_port)
69  flush
70  Ng.Can.await_read(can_port)
71  flush
72  Ng.Can.await_read(can_port)
73  flush
74  Ng.Can.await_read(can_port)
```

**TODO: Add description**

## Targets

Nerves applications produce images for hardware targets based on the
`MIX_TARGET` environment variable. If `MIX_TARGET` is unset, `mix` builds an
image that runs on the host (e.g., your laptop). This is useful for executing
logic tests, running utilities, and debugging. Other targets are represented by
a short name like `rpi3` that maps to a Nerves system image for that platform.
All of this logic is in the generated `mix.exs` and may be customized. For more
information about targets see:

https://hexdocs.pm/nerves/targets.html#content

## Getting Started

To start your Nerves app:
  * `export MIX_TARGET=my_target` or prefix every command with
    `MIX_TARGET=my_target`. For example, `MIX_TARGET=rpi3`
  * Install dependencies with `mix deps.get`
  * Create firmware with `mix firmware`
  * Burn to an SD card with `mix firmware.burn`

## Learn more

  * Official docs: https://hexdocs.pm/nerves/getting-started.html
  * Official website: https://nerves-project.org/
  * Forum: https://elixirforum.com/c/nerves-forum
  * Discussion Slack elixir-lang #nerves ([Invite](https://elixir-slackin.herokuapp.com/))
  * Source: https://github.com/nerves-project/nerves
