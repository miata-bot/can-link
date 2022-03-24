defmodule CANLink.RGB do
  alias CANLink.PWM
  use GenServer
  require Logger

  @all_channels [:led_r, :led_g, :led_b]

  def start_link(opts) do
    GenServer.start_link(__MODULE__, opts, name: __MODULE__)
  end

  def brightness_fade(color \\ :red) do
    on()
    set_color(color)
    for i <- 0..255 do
      CANLink.RGB.set_brightness(i)
      Process.sleep(1)
    end
    for i <- 255..0 do
      CANLink.RGB.set_brightness(i)
      Process.sleep(1)
    end
  end

  def on() do
    GenServer.call(__MODULE__, :on)
  end

  def off() do
    GenServer.call(__MODULE__, :off)
  end

  def set_color(val) do
    GenServer.call(__MODULE__, {:set_color, val})
  end

  def set_brightness(val) do
    GenServer.call(__MODULE__, {:set_brightness, val})
  end

  def init(_opts) do
    @all_channels
    |> Enum.each(fn channel ->
      PWM.enable(channel, false)
    end)

    state = %{
      color: :white,
      brightness: 100
    }

    {:ok, state}
  end

  def handle_call(:on, _from, state) do
    set(state.color, state.brightness)

    @all_channels
    |> Enum.each(fn channel ->
      PWM.enable(channel, true)
    end)

    {:reply, :ok, state}
  end

  def handle_call(:off, _from, state) do
    @all_channels
    |> Enum.each(fn channel ->
      PWM.enable(channel, false)
    end)

    {:reply, :ok, state}
  end

  def handle_call({:set_color, val}, _from, state) do
    set(val, state.brightness)

    {:reply, :ok, %{state | color: val}}
  end

  def handle_call({:set_brightness, val}, _from, state) do
    set(state.color, val)

    {:reply, :ok, %{state | brightness: val}}
  end

  defp set(color, brightness) do
    rgb_val = rgb_from_color(color, brightness)

    Enum.zip([:led_r, :led_g, :led_b], rgb_val)
    |> Enum.each(fn {channel, val} ->
      duty_cycle = floor(PWM.period(channel) * val / 255)
      PWM.duty_cycle(channel, duty_cycle)
    end)
  end

  defp rgb_from_color(val, brightness) do
    max = 255 * brightness / 100

    case val do
      :white -> [max, max, max]
      :red -> [max, 0, 0]
      :green -> [0, max, 0]
      :blue -> [0, 0, max]
      :yellow -> [max, max, 0]
      :cyan -> [0, max, max]
      :magenta -> [max, 0, max]
      {r, g, b} -> [r, g, b]
      [r, g, b] -> [r, g, b]
    end
  end
end
