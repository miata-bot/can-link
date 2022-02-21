defmodule HaltechLink.Console do
  use GenServer

  def start_link(args) do
    GenServer.start_link(__MODULE__, args, name: __MODULE__)
  end

  def init(_args) do
    {:ok, pid} = Circuits.UART.start_link()
    :ok = Circuits.UART.open(pid, "/dev/ttyGS0", speed: 115_200, active: true)
    {:ok, tty} = ExTTY.start_link(handler: self())
    {:ok, %{uart: pid, tty: tty}}
  end

  def handle_info({:tty_data, data}, state) do
    Circuits.UART.write(state.uart, data)
    {:noreply, state}
  end

  def handle_info({:circuits_uart, "/dev/ttyGS0", data}, state) do
    ExTTY.send_text(state.tty, data)
    {:noreply, state}
  end
end
