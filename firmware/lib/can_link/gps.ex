defmodule CANLink.GPS do
  use GenServer
  require Logger

  def start_link(args) do
    GenServer.start_link(__MODULE__, args, name: __MODULE__)
  end

  def init(_) do
    {:ok, pipe} = GPSd.Pipe.start_link(controlling_process: self())
    {:ok, %{gpsd: pipe, lat: 0, lon: 0}}
  end

  def handle_info(%GPSd.Class.TPV{lat: lat, lon: lon}, state) do
    # if lat != state.lat or lon != state.lon do
    #   Logger.info(%{lattitude: lat, longitude: lon})
    # end
    CANLink.Radio.send_packet(0, <<lat::float-64, lon::float-64>>)
    {:noreply, %{state | lat: lat, lon: lon}}
  end
  def handle_info(_message, state) do
    # Logger.info(%{gps: message})
    {:noreply, state}
  end
end
