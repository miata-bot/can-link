defmodule HaltechLink.CAN do
  @behaviour :gen_statem
  require Logger

  @doc false
  def child_spec(opts) do
    %{
      id: __MODULE__,
      start: {__MODULE__, :start_link, [opts]},
      type: :worker,
      restart: :permanent,
      shutdown: 500
    }
  end

  @doc false
  def start_link(args) do
    :gen_statem.start_link({:local, __MODULE__}, __MODULE__, args, [])
  end

  def send_frame(<<_, _, _, _>> = id, data) when is_binary(data) do
    <<id::size(32)>> = id
    frame = {id, data}
    :gen_statem.cast(__MODULE__, {:send_frame, frame})
  end

  @impl :gen_statem
  def callback_mode() do
    :state_functions
  end

  @impl :gen_statem
  def init(_args) do
    data = %{can_port: nil}
    actions = [{:next_event, :internal, :init}]
    :os.cmd('ip link set can0 up type can bitrate 1000000')
    # :os.cmd('ip link set can0 up')
    {:ok, :reset, data, actions}
  end

  @impl :gen_statem
  def terminate(_reason, _state, _data) do
    :ok
  end

  def reset(:internal, :init, data) do
    # _ = :os.cmd('ip link set can0 down')
    # _ = Process.sleep(100)
    # _ = :os.cmd('/sbin/ip link set can0 up type can bitrate 125000')
    actions = [{:next_event, :internal, :init}]
    {:next_state, :open, data, actions}
  end

  def open(:internal, :init, data) do
    {:ok, can_port} = Ng.Can.start_link()
    Ng.Can.open(can_port, "can0")
    data = %{data | can_port: can_port}
    actions = [{:next_event, :internal, :await_read}]
    {:next_state, :await_read, data, actions}
  end

  def await_read(:internal, :await_read, data) do
    Ng.Can.await_read(data.can_port)
    :keep_state_and_data
  end

  def await_read(:info, {:can_frames, "can0", frames}, data) do
    # Logger.info(%{can_frames: frames})
    HaltechLink.EngineData.dispatch(frames)
    actions = [{:next_event, :internal, :await_read}]
    {:keep_state, data, actions}
  end

  def await_read(:cast, {:send_frame, frame}, data) do
    Ng.Can.write(data.can_port, frame)
    actions = []
    {:keep_state, data, actions}
  end
end
