defmodule GPSd.Pipe do
  use GenServer
  require Logger
  import GPSd.Class, only: [decode_message: 1]

  def start_link(args, opts \\ [name: __MODULE__]) do
    GenServer.start_link(__MODULE__, args, opts)
  end

  def init(args) do
    pid = Keyword.fetch!(args, :controlling_process)
    exe = System.find_executable("gpspipe") || raise "Could not find gpspipe"

    port =
      Port.open({:spawn_executable, exe}, [
        {:args, ["-w"]},
        {:line, 1024},
        :exit_status,
        :stderr_to_stdout,
        :binary
      ])

    {:ok, %{port: port, controlling_process: pid, buffer: <<>>}}
  end

  def handle_info({_, {:data, {:noeol, line}}}, state) do
    {:noreply, %{state | buffer: state.buffer <> line}}
  end

  def handle_info({_, {:data, {:eol, line}}}, state) do
    with {:ok, data} <- Jason.decode(state.buffer <> line),
         {:ok, message} <- decode_message(data) do
      send(state.controlling_process, message)
      {:noreply, %{state | buffer: <<>>}}
    else
      _error ->
        {:noreply, state}
    end
  end

  def handle_info({_, {:exit_status, status}}, state) do
    {:stop, {:exit_status, status}, state}
  end
end
