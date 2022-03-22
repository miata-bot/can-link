defmodule Radio do
  use GenServer
  require Logger

  def start_link(args) do
    GenServer.start_link(__MODULE__, args, name: __MODULE__)
  end

  def send_packet(node_id, pkt) do
    GenServer.cast(__MODULE__, {:send, node_id, pkt})
  end

  def init(_args) do
    send(self(), :init_radio)
    args = [spi_bus_name: "spidev0.0", irq_pin: 25, node_id: 2,
    # encrypt_key: <<161, 156, 95, 234, 11, 63, 65, 0, 72, 57, 168, 102, 210, 235, 14, 22>>
    ]
    {:ok, %{radio: nil, args: args}}
  end

  def handle_info(:init_radio, state) do
    Logger.info("Starting radio")

    case RF69.start_link(state.args) do
      {:ok, radio} ->
        # Process.send_after(self(), :start_send, 5000)
        {:noreply, %{state | radio: radio}}

      error ->
        Logger.error("Failed to start radio: #{inspect(error)}")
        {:stop, error, state}
    end
  end

  def handle_info(:start_send, state) do
    paylod =  :crypto.strong_rand_bytes(16)
    RF69.send(state.radio, 1, false, paylod)
    Process.send_after(self(), :start_send, 5000)
    {:noreply, state}
  end

  def handle_info(%{payload: <<lat::float-64, lon::float-64>>}, state) do
    Logger.info(%{lattitude: lat, longitude: lon})
    {:noreply, state}
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
