defmodule CANLink.GPS do
  use GenServer
  require Logger

  def start_link(args) do
    GenServer.start_link(__MODULE__, args, name: __MODULE__)
  end

  def get_location do
    GenServer.call(__MODULE__, :get_location)
  end

  def broadcast(lat, lon) do
    Phoenix.PubSub.broadcast(CANLink.PubSub, "location", %{topic: "location", payload: %{latitude: lat, longitude: lon}})
  end

  def broadcast do
    {lat, lon} = get_location()
    broadcast(lat, lon)
  end

  def init(_) do
    {:ok, pipe} = GPSd.Pipe.start_link(controlling_process: self())
    {:ok, %{gpsd: pipe, lat: 0, lon: 0}}
  end

  def handle_info(%GPSd.Class.TPV{lat: lat, lon: lon}, state) when is_number(lat) and is_number(lon) do
    broadcast(lat, lon)
    # Logger.info(%{lat: lat, lon: lon})
    # CANLink.Radio.send_packet(0, <<lat::float-little-64, lon::float-little-64>>)
    {:noreply, %{state | lat: lat, lon: lon}}
  end
  def handle_info(_message, state) do
    # Logger.info(%{gps: message})
    {:noreply, state}
  end

  def handle_call(:get_location, _from, state) do
    {:reply, {state.lat, state.lon}, state}
  end
end
