defmodule CANLink.Radio do
  use GenServer
  require Logger

  def start_link(args) do
    GenServer.start_link(__MODULE__, args, name: __MODULE__)
  end

  def send_packet(node_id, pkt) do
    GenServer.cast(__MODULE__, {:send, node_id, pkt})
  end

  def init(args) do
    send(self(), :init_radio)
    {:ok, %{radio: nil, args: args}}
  end

  def handle_info(:init_radio, state) do
    Logger.info("Starting radio")

    case RF69.start_link(state.args) do
      {:ok, radio} ->
        {:noreply, %{state | radio: radio}}

      error ->
        Logger.error("Failed to start radio: #{inspect(error)}")
        {:stop, error, state}
    end
  end

  def handle_info(packet, state) do
    Logger.info(%{radio_packet: packet})
    {:noreply, state}
  end

  def handle_cast({:send, node_id, pkt}, state) do
    RF69.send(state.radio, node_id, false, pkt)
    {:noreply, state}
  end
end
