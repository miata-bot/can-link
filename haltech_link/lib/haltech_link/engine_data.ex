defmodule HaltechLink.EngineData do
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
  def handle_cast({:dispatch, frames}, state) do
    state = Enum.reduce(frames, state, &handle_frame/2)
    {:noreply, state}
  end

  def handle_frame({id, frame}, state) do
    try do
      data = MegasquirtCANProtocol.parse_frame!(id, frame)
      Map.merge(state, Map.new(data))
    catch
      _, _ ->
        Logger.error("Dropped frame: #{inspect(frame)}")
        state
    end
  end

  def handle_frame({id, frame}, state) do
    Logger.warn("unknown frame id: #{inspect(id)} #{inspect(frame)}")
    state
  end
end
