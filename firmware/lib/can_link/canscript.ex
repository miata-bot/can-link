defmodule CANLink.CANScript do
  use GenServer

  def start_link(args) do
    GenServer.start_link(__MODULE__, args, name: __MODULE__)
  end

  def set(key, value) do
    GenServer.call(__MODULE__, {:set, key, value})
  end

  def get(key) do
    GenServer.call(__MODULE__, {:get, key})
  end

  def load_script(script) do
    GenServer.call(__MODULE__, {:load, script})
  end

  def call_script() do
    GenServer.call(__MODULE__, :pcallk)
  end

  def init(_) do
    port =
      Port.open({:spawn_executable, '/home/connor/workspace/sixtyeightplus.one/can-link/canscript/zig-out/bin/canscript'}, [
        {:packet, 2},
        :nouse_stdio,
        :binary,
        :exit_status
      ])

    {:ok, %{port: port}}
  end

  def handle_call({:set, key, value}, _from, state) do
    case value do
      f64 when is_float(f64) ->
        Port.command(state.port, <<0, key, 2, value::little-float-64>>)

      d8 when d8 <= 255 ->
        Port.command(state.port, <<0, key, 1, value>>)

      i8 when i8 <= 128 and i8 >= -128 ->
        Port.command(state.port, <<0, key, 0, value::little-signed>>)
    end

    {:reply, :ok, state}
  end

  def handle_call({:get, key}, _from, state) do
    Port.command(state.port, <<1, key>>)

    receive do
      {_port, {:data, <<1, ^key, 0, value>>}} ->
        {:reply, {:ok, value}, state}

      {_port, {:data, <<1, ^key, 1, value::little-signed>>}} ->
        {:reply, {:ok, value}, state}

      {_port, {:data, <<1, ^key, 2, value::little-float-64>>}} ->
        {:reply, {:ok, value}, state}
    end
  end

  def handle_call({:load, file}, _from, state) do
    Port.command(state.port, <<2, file::binary>>)

    receive do
      {_port, {:data, <<2, 0>>}} ->
        {:reply, :ok, state}

      {_port, {:data, <<2, 1, error::binary>>}} ->
        {:reply, {:error, error}, state}
    end
  end

  def handle_call(:pcallk, _from, state) do
    Port.command(state.port, <<3>>)

    receive do
      {_port, {:data, <<3, 0>>}} ->
        {:reply, :ok, state}

      {_port, {:data, <<3, 1, error::binary>>}} ->
        {:reply, {:error, error}, state}
    end
  end
end
