defmodule HaltechLink.Button do
  use GenServer
  require Logger

  @moduledoc """
  This GenServer starts the wizard if a button is depressed for long enough.
  """

  alias Circuits.GPIO

  @doc """
  Start the button monitor
  Pass an index to the GPIO that's connected to the button.
  """
  @spec start_link(non_neg_integer()) :: GenServer.on_start()
  def start_link(gpio_pin) do
    GenServer.start_link(__MODULE__, gpio_pin)
  end

  @impl true
  def init(gpio_pin) do
    _ = File.write("/sys/class/leds/beaglebone:green:usr0/trigger", "none")

    _ = File.write("/sys/class/leds/beaglebone:green:usr3/trigger", "none")
    _ = File.write("/sys/class/leds/beaglebone:green:usr3/trigger", "oneshot")
    _ = File.write("/sys/class/leds/beaglebone:green:usr3/invert", "0")

    {:ok, gpio} = GPIO.open(gpio_pin, :input)
    :ok = GPIO.set_interrupts(gpio, :both)
    {:ok, %{pin: gpio_pin, gpio: gpio, wizard_started: false}}
  end

  @impl true
  def handle_info({:circuits_gpio, gpio_pin, _timestamp, 0}, %{pin: gpio_pin} = state) do
    Logger.debug("button pressed")
    _ = File.write("/sys/class/leds/beaglebone:green:usr3/invert", "1")
    # Button pressed. Start a timer to launch the wizard when it's long enough
    {:noreply, state, 5_000}
  end

  @impl true
  def handle_info({:circuits_gpio, gpio_pin, _timestamp, 1}, %{pin: gpio_pin} = state) do
    Logger.debug("button released")
    _ = File.write("/sys/class/leds/beaglebone:green:usr3/invert", "0")

    # Button released. The GenServer timer is implicitly cancelled by receiving this message.
    {:noreply, state}
  end

  @impl true
  def handle_info(:timeout, %{wizard_started: false} = state) do
    _ = File.write("/sys/class/leds/beaglebone:green:usr3/trigger", "pattern")
    _ = File.write("/sys/class/leds/beaglebone:green:usr3/pattern", "0 1000 0 0 255 2000 255 0")
    :ok = VintageNetWizard.run_wizard(on_exit: {GenServer, :cast, [self(), :on_wizard_exit]})
    {:noreply, %{state | wizard_started: true}}
  end

  def handle_info(:timeout, %{wizard_started: true} = state) do
    _ = VintageNetWizard.stop_wizard()
    {:noreply, state}
  end

  @impl true
  def handle_cast(:on_wizard_exit, state) do
    # This function is used as a callback when the WiFi Wizard
    # exits which is useful if you need to do work after
    # configuration is done, like restart web servers that might
    # share a port with the wizard, etc etc
    Logger.info("WiFi Wizard stopped")
    _ = File.write("/sys/class/leds/beaglebone:green:usr3/trigger", "oneshot")
    _ = File.write("/sys/class/leds/beaglebone:green:usr3/invert", "0")
    {:noreply, %{state | wizard_started: false}}
  end
end
