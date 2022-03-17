defmodule CANLink.EngineData do
  use GenServer
  require Logger

  def start_link(args) do
    GenServer.start_link(__MODULE__, args, name: __MODULE__)
  end

  def dispatch(frames) do
    GenServer.cast(__MODULE__, {:dispatch, frames})
  end

  @impl GenServer
  def init(_args) do
    {:ok, %{}}
  end

  @impl GenServer
  def handle_cast({:dispatch, frames}, old_state) do
    state = Enum.reduce(frames, old_state, &handle_frame/2)

    if(old_state[:throttle_position] != state[:throttle_position]) do
      tps = round(state[:throttle_position] || 0.0)
      CANLink.RGB.set_color(tps, 0, 0)
    end

    # Logger.info(%{tps: tps})
    {:noreply, state}
  end

  def handle_frame({id, frame}, state) do
    try do
      # data = MegasquirtCANProtocol.parse_frame!(id, frame)
      data = HaltechCANProtocol.parse_frame!(id, frame)
      Map.merge(state, Map.new(data))
    catch
      _, _ ->
        # Logger.error("Dropped frame: #{inspect(frame)}")
        state
    end
  end

  def handle_frame({id, frame}, state) do
    Logger.warn("unknown frame id: #{inspect(id)} #{inspect(frame)}")
    state
  end
end
