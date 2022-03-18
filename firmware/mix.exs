defmodule CANLink.MixProject do
  use Mix.Project

  @app :can_link
  @version "0.1.0"
  @all_targets [:bbb]

  def project do
    [
      app: @app,
      version: @version,
      elixir: "~> 1.9",
      archives: [nerves_bootstrap: "~> 1.10"],
      start_permanent: Mix.env() == :prod,
      build_embedded: true,
      deps: deps(),
      releases: [{@app, release()}],
      preferred_cli_target: [run: :host, test: :host]
    ]
  end

  # Run "mix help compile.app" to learn about applications.
  def application do
    [
      mod: {CANLink.Application, []},
      extra_applications: [:logger, :runtime_tools]
    ]
  end

  # Run "mix help deps" to learn about dependencies.
  defp deps do
    [
      # Dependencies for all targets
      {:nerves, "~> 1.7.4", runtime: false},
      {:nimble_csv, "~> 1.2", runtime: false},
      {:shoehorn, "~> 0.7.0"},
      {:ring_logger, "~> 0.8.1"},
      {:toolshed, "~> 0.2.13"},
      {:gen_stage, "~> 1.1"},
      {:extty, "~> 0.2.1"},
      {:circuits_uart, "~> 1.4"},
      {:decompilerl, github: "aerosol/decompilerl", only: :dev, targets: :host},
      {:circuits_gpio, "~> 1.0", targets: @all_targets, override: true},
      {:circuits_spi, "~> 1.3", targets: @all_targets, override: true},
      # {:rf69, github: "connorrigby/elixir-rf69", targets: @all_targets},
      {:rf69, path: "../elixir-rf69", targets: @all_targets},

      # Dependencies for all targets except :host
      {:nerves_runtime, "~> 0.11.3", targets: @all_targets},
      {:nerves_pack, "~> 0.6.0", targets: @all_targets},
      {:vintage_net_wizard, "~> 0.2", targets: @all_targets},
      {:nerves_time_rtc_abracon, "~> 0.2.1", targets: @all_targets},

      # {:can, github: "tonyrog/can", targets: @all_targets},
      {:ng_can, path: "../ng_can", targets: @all_targets},
      # {:blue_heron, "~> 0.3", path: "/home/connor/workspace/keeplabs/elias/firmware/../hal/../../blue_heron", targets: @all_targets, override: true},
      # {:blue_heron_transport_uart,
      #  path: "/home/connor/workspace/keeplabs/elias/firmware/../hal/../../blue_heron_transport_uart", targets: @all_targets, override: true},
      # {:blue_heron_ti_wl18xx, path: "/home/connor/workspace/keeplabs/elias/firmware/../hal/../../blue_heron_ti_wl18xx", targets: :bbb, override: true},

      # Dependencies for specific targets
      # NOTE: It's generally low risk and recommended to follow minor version
      # bumps to Nerves systems. Since these include Linux kernel and Erlang
      # version updates, please review their release notes in case
      # changes to your application are needed.
      # {:nerves_system_bbb_can, path: "../nerves_system_bbb_can", tag: "v0.2.0", runtime: false, targets: :bbb}
      {:nerves_system_bbb_can, github: "ConnorRigby/nerves_system_bbb_can", tag: "v0.3.0", runtime: false, targets: :bbb}
    ]
  end

  def release do
    [
      overwrite: true,
      # Erlang distribution is not started automatically.
      # See https://hexdocs.pm/nerves_pack/readme.html#erlang-distribution
      cookie: "#{@app}_cookie",
      include_erts: &Nerves.Release.erts/0,
      steps: [&Nerves.Release.init/1, :assemble],
      strip_beams: Mix.env() == :prod or [keep: ["Docs"]]
    ]
  end
end
