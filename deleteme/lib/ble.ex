defmodule Deleteme.Bluetooth do
  @behaviour :gen_statem
  require Logger

  alias BlueHeron.Peripheral

  @behaviour BlueHeron.GATT.Server

  alias Deleteme.Bluetooth.{
    DeviceInfoService,
    GenericAccessService,
    GenericAttributeService,
  }

  defmodule LocationService do
    alias BlueHeron.GATT.{Characteristic, Characteristic.Descriptor, Service}
require Logger
    def service do
      Service.new(%{
        id: __MODULE__,
        type: 0x7513F64B70772005F0F11F8664DD00FB,
        characteristics: [
          Characteristic.new(%{
            id: {__MODULE__, :location},
            type: 0x1899B193DE16DE234EBDA5447327B12F,
            properties: 0b0010010,
            descriptor: Descriptor.new(%{
              permissions: 0x10
            })
          })
        ]
      })
    end

    def write(_, _), do: "error"

    def read(:location) do
      Logger.info "read"
      {lat, lon} = Radio.location()
      <<lat::little-float-64, lon::little-float-64>>
    end

    def subscribe(:location) do
      Logger.info("subscribe");
      Phoenix.PubSub.subscribe(Deleteme.PubSub, "location")
    end

    def unsubscribe(:location) do
      Logger.info("unsubscribe");

      Phoenix.PubSub.unsubscribe(Deleteme.PubSub, "location")
    end

    def notify(%{location: %{lattitude: lat, longitude: lon}}) do
      Logger.info "notify"
      <<lat::little-float-64, lon::little-float-64>>
    end
  end

  def disable() do
    :gen_statem.cast(__MODULE__, :disable)
  end

  def enable() do
    :gen_statem.cast(__MODULE__, :enable)
  end

  @impl BlueHeron.GATT.Server
  def profile() do
    [
      GenericAttributeService.service(),
      GenericAccessService.service(),
      DeviceInfoService.service(),
      LocationService.service(),
    ]
  end

  # will be needed for encryption
  # GATT Characteristic and Object Type 0x2B44 Current Session
  # GATT Characteristic and Object Type 0x2B45 Session

  @impl BlueHeron.GATT.Server
  def read({mod, id}) when is_atom(mod), do: mod.read(id)

  @impl BlueHeron.GATT.Server
  def write({mod, id}, value) when is_atom(mod), do: mod.write(id, value)

  def subscribe({mod, id}) when is_atom(mod), do: :gen_statem.call(__MODULE__, {:subscribe, mod, id})
  def unsubscribe({mod, id}) when is_atom(mod), do: :gen_statem.call(__MODULE__, {:unsubscribe, mod, id})

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

  @impl :gen_statem
  def callback_mode() do
    :state_functions
  end

  @impl :gen_statem
  def init(_) do
    data = %{port: "ttyS0", context: nil, peripheral: nil}
    actions = [{:next_event, :internal, :init}]
    {:ok, :reset, data, actions}
  end

  @impl :gen_statem
  def terminate(_reason, _state, _data) do
    :ok
  end

  # TODO set typespecs for the state functions

  # reset state
  def reset(:internal, :init, data) do
    case BlueHeron.transport(%BlueHeronTransportUART{
      device: "ttyS0",
      uart_opts: [speed: 115_200]
      }) do
      {:ok, context} ->
        Logger.info("Bluetooth firmware loaded")
        actions = [{:next_event, :internal, :init}]
        data = %{data | context: context}
        {:next_state, :peripheral, data, actions}

      {:error, reason} ->
        Logger.error("Failed to start Bluetooth transport: #{inspect(reason)}")
        actions = [{{:timeout, :init}, 5000, nil}]
        {:keep_state, data, actions}
    end
  end

  def reset(:cast, :enable, data) do
    actions = [{:next_event, :internal, :init}]
    {:keep_state, data, actions}
  end

  # if bluetooth fails to start, it times out here.
  # this will reload reset the module and reload firmware
  def reset({:timeout, :init}, _, data) do
    actions = [{:next_event, :internal, :init}]
    {:keep_state, data, actions}
  end

  def peripheral(:internal, :init, data) do
    case Peripheral.start_link(data.context, __MODULE__) do
      {:ok, peripheral} ->
        Logger.info("Started Bluetooth peripheral")
        data = %{data | peripheral: peripheral}
        actions = [{:next_event, :internal, :init}]
        {:next_state, :advertise, data, actions}

      {:error, reason} ->
        Logger.error("Failed to start Bluetooth peripheral: #{inspect(reason)}")
        actions = [{{:timeout, :init}, 5000, nil}]
        {:keep_state, data, actions}
    end
  end

  # if the peripheral fails to start, something else is probably super
  # wrong, so this probably doesn't help that much, but at least it looks
  # nice.
  def peripheral({:timeout, :init}, _, data) do
    actions = [{:next_event, :internal, :init}]
    {:keep_state, data, actions}
  end

  # Calls to BlueHeron.Peripheral will fail p catastrophically
  # if the module is out to lunch. Ideally the supervision tree
  # will handle this, but i'm not sure how well it might handle
  # it. There may need to be a separate bluetooth supervisor.
  # also
  # should maybe make this into a with chain or something. I'm not sure
  # what the return values of these funcs are, but it's probably `:ok`
  def advertise(:internal, :init, data) do
    serial = get_serial(data)

    try do
      Peripheral.set_advertising_parameters(data.peripheral, %{})

      # Advertising Data Flags: BR/EDR not supported, GeneralConnectable
      # Complete Local Name
      # Incomplete List of 128-bit Servive UUIDs
      advertising_data =
        <<0x02, 0x01, 0b00000110>> <>
          <<byte_size(serial) + 1, 0x09>> <>
          serial <>
          <<0x11, 0x06, <<0x42A31ABD030C4D5CA8DF09686DD16CC0::little-128>>::binary>>

      Peripheral.set_advertising_data(data.peripheral, advertising_data)

      Peripheral.start_advertising(data.peripheral)
      Logger.info("Started advertising Bluetooth peripheral")
      actions = []
      {:keep_state_and_data, actions}
    catch
      _, _ ->
        Logger.error("Advertising failed. Resetting module")
        actions = [{:next_event, :internal, :init}]
        data = %{data | context: nil, peripheral: nil}
        {:next_state, :reset, data, actions}
    end
  end

  def advertise(:info, {Peripheral, :connected}, data) do
    Logger.info("New BLE connection")
    # BlueHeron.Peripheral.exchange_mtu(data.peripheral, 247)
    actions = []
    {:next_state, :connected, data, actions}
  end

  # technically not possible but also that's not true.
  def advertise(:info, {Peripheral, :disconnected}, data) do
    Logger.info("BLE device disconnected")
    # restart advertising after a disconnect.
    actions = [{:next_event, :internal, :init}]
    {:keep_state, data, actions}
  end

  def connected({:call, from}, {:subscribe, mod, id}, data) do
    _ = mod.subscribe(id)
    {:keep_state_and_data, [{:reply, from, :ok}]}
  end

  def connected({:call, from}, {:unsubscribe, mod, id}, data) do
    _ = mod.unsubscribe(id)
    {:keep_state_and_data, [{:reply, from, :ok}]}
  end

  def connected(:info, {BlueHeron.Peripheral, :disconnected}, data) do
    Logger.info("BLE device disconnected")
    # restart advertising after a disconnect.
    actions = [{:next_event, :internal, :init}]
    {:next_state, :advertise, data, actions}
  end

  def connected(:info, %{topic: "location", payload: payload}, data) do
    notification = LocationService.notify(payload)
    Peripheral.nofify(data.peripheral, LocationService, {LocationService, :location}, notification)
    :keep_state_and_data
  end

  def connected(:cast, :enable, _data) do
    {:keep_state_and_data, []}
  end

  # todo, this should use config from app env
  def get_serial(_data) do
    with {serial, 0} = System.cmd("boardid", []) do
      "rp-" <> String.trim(serial)
    end
  end
end
