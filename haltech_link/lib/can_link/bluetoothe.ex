defmodule CANLink.Bluetooth do
  @behaviour :gen_statem
  require Logger

  @behaviour BlueHeron.GATT.Server
  alias BlueHeron.GATT.{Characteristic, Service}
  @firmware_file "/lib/firmware/ti-connectivity/TIInit_11.8.32.bts"

  def disable() do
    :gen_statem.cast(__MODULE__, :disable)
  end

  def enable() do
    :gen_statem.cast(__MODULE__, :enable)
  end

  @impl BlueHeron.GATT.Server
  def profile() do
    [
      Service.new(%{
        id: :gap,
        type: 0x1800,
        characteristics: [
          Characteristic.new(%{
            id: {:gap, :device_name},
            type: 0x2A00,
            properties: 0b0000010
          }),
          Characteristic.new(%{
            id: {:gap, :appearance},
            type: 0x2A01,
            properties: 0b0000010
          })
        ]
      }),
      Service.new(%{
        id: :nerves_firmware_config,
        type: 0x42A31ABD030C4D5CA8DF09686DD16CC0,
        characteristics: [
          Characteristic.new(%{
            id: {:nerves_firmware_config, "wifi_reset"},
            type: 0x3EB9876E658C43E596D1B6ED13364BEC,
            properties: 0b0001000
          })
        ]
      })
    ]
  end

  @impl BlueHeron.GATT.Server
  def read({:gap, :device_name}) do
    with {:ok, hostname} <- :inet.gethostname() do
      to_string(hostname)
    else
      _ -> "error"
    end
  end

  def read({:gap, :appearance}) do
    # The GAP service must have an appearance attribute,
    # whose value must be picked from this document: https://specificationrefs.bluetooth.com/assigned-values/Appearance%20Values.pdf
    # This is the standard apperance value for "Medicine Delivery"
    <<0x0D80::little-16>>
  end

  @impl BlueHeron.GATT.Server
  def write({:nerves_firmware_config, name}, value) do
    Logger.info("BLE write #{name} => #{inspect(value)}")
    :ok
  end

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
  def init(args) do
    port = Keyword.fetch!(args, :port)
    enable = Keyword.fetch!(args, :enable)
    data = %{port: port, enable: enable, context: nil, peripheral: nil}
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
    config = BlueHeron.Wilink8.init(@firmware_file, data.enable, data.port)

    case BlueHeron.transport(config) do
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
    case BlueHeron.Peripheral.start_link(data.context, __MODULE__) do
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
      BlueHeron.Peripheral.set_advertising_parameters(data.peripheral, %{})

      # Advertising Data Flags: BR/EDR not supported, GeneralConnectable
      # Complete Local Name
      # Incomplete List of 128-bit Servive UUIDs
      advertising_data =
        <<0x02, 0x01, 0b00000110>> <>
          <<byte_size(serial) + 1, 0x09>> <>
          serial <>
          <<0x11, 0x06, <<0x42A31ABD030C4D5CA8DF09686DD16CC0::little-128>>::binary>>

      BlueHeron.Peripheral.set_advertising_data(data.peripheral, advertising_data)

      BlueHeron.Peripheral.start_advertising(data.peripheral)
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

  def advertise(:info, {BlueHeron.Peripheral, :connected}, data) do
    Logger.info("New BLE connection")
    actions = []
    {:next_state, :connected, data, actions}
  end

  # technically not possible but also that's not true.
  def advertise(:info, {BlueHeron.Peripheral, :disconnected}, data) do
    Logger.info("BLE device disconnected")
    # restart advertising after a disconnect.
    actions = [{:next_event, :internal, :init}]
    {:keep_state, data, actions}
  end

  def advertise(:cast, :disable, data) do
    do_disable(data)
    {:next_state, :reset, data, []}
  end

  def connected(:info, {BlueHeron.Peripheral, :disconnected}, data) do
    Logger.info("BLE device disconnected")
    # restart advertising after a disconnect.
    actions = [{:next_event, :internal, :init}]
    {:next_state, :advertise, data, actions}
  end

  def connected(:cast, :disable, data) do
    do_disable(data)
    {:next_state, :reset, data, []}
  end

  def connected(:cast, :enable, _data) do
    {:keep_state_and_data, []}
  end

  # todo, this should use config from app env
  def get_serial(_data) do
    "bbblue"
  end

  def do_disable(data) do
    {:ok, pin_ref} = Circuits.GPIO.open(data.enable, :output)
    Circuits.GPIO.write(pin_ref, 0)
  end
end
