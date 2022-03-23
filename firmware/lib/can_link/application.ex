defmodule CANLink.Application do
  # See https://hexdocs.pm/elixir/Application.html
  # for more information on OTP Applications
  @moduledoc false

  use Application

  require Logger

  @ifname "wlan0"
  @app :can_link

  @impl true
  def start(_type, _args) do
    maybe_start_distribution()
    # See https://hexdocs.pm/elixir/Supervisor.html
    # for other strategies and supported options
    opts = [strategy: :one_for_one, name: CANLink.Supervisor]

    children =
      [
        {Phoenix.PubSub, name: CANLink.PubSub},
        CANLink.EngineData
        # Children for all targets
        # Starts a worker by calling: CANLink.Worker.start_link(arg)
        # {CANLink.Worker, arg},
      ] ++ children(target())

    Supervisor.start_link(children, opts)
  end

  # List all child processes to be supervised
  def children(:host) do
    [
      # Children that only run on the host
      # Starts a worker by calling: CANLink.Worker.start_link(arg)
      # {CANLink.Worker, arg},
      # {CANLink.RGB, [tty: "/dev/cu.usbserial-110"]}
    ]
  end

  def children(_target) do
    :os.cmd('killall -9 gpsd')
    :os.cmd('gpsd /dev/ttyS1 --nowait')
    # maybe_start_wifi_wizard()
    gpio_pin = Application.get_env(:can_link, :button_pin, 68)

    [
      {CANLink.CAN, []},
      {CANLink.Button, gpio_pin},
      {CANLink.Radio,
       [
         spi_bus_name: "spidev1.0",
         irq_pin: 49
         #  encrypt_key: <<161, 156, 95, 234, 11, 63, 65, 0, 72, 57, 168, 102, 210, 235, 14, 22>>
       ]},
      {CANLink.GPS, []},
      {CANLink.BLE, [port: "ttyS3", enable: 28]}
      # {CANLink.RGB, [tty: "ttyUSB0"]}
    ]
  end

  def target() do
    Application.get_env(:can_link, :target)
  end

  def maybe_start_distribution() do
    _ = :os.cmd('epmd -daemon')
    {:ok, hostname} = :inet.gethostname()

    case Node.start(:"#{@app}@#{hostname}.local") do
      {:ok, _pid} ->
        Node.set_cookie(:KFUYVQEXKNOEJXOBANFE)
        Logger.info("Distribution started at #{@app}@#{hostname}.local")

      {:error, {:already_started, _}} ->
        Node.set_cookie(:KFUYVQEXKNOEJXOBANFE)
        Logger.info("Distribution started at #{@app}@#{hostname}.local")

      _error ->
        Logger.error("Failed to start distribution")
    end
  end

  def maybe_start_wifi_wizard() do
    with true <- has_wifi?() || :no_wifi,
         true <- wifi_configured?() || :not_configured,
         true <- has_networks?() || :no_networks do
      # By this point we know there is a wlan interface available
      # and already configured with networks. This would normally
      # mean that you should then skip starting the WiFi wizard
      # here so that the device doesn't start the WiFi wizard after
      # every reboot.
      #
      # However, for the example we want to always run the
      # WiFi wizard on startup. Comment/remove the function below
      # if you want a more typical experience skipping the wizard
      # after it has been configured once.
      VintageNetWizard.run_wizard(on_exit: {__MODULE__, :on_wizard_exit, []})
      false
    else
      :no_wifi ->
        Logger.error("[#{inspect(__MODULE__)}] Device does not support WiFi - Skipping wizard start")

      status ->
        info_message(status)
        VintageNetWizard.run_wizard(on_exit: {__MODULE__, :on_wizard_exit, []})
    end
  end

  def has_wifi?() do
    @ifname in VintageNet.all_interfaces()
  end

  def wifi_configured?() do
    @ifname in VintageNet.configured_interfaces()
  end

  def has_networks?() do
    VintageNet.get_configuration(@ifname)[:vintage_net_wifi][:networks] != []
  end

  defp info_message(status) do
    msg =
      case status do
        :not_configured -> "WiFi has not been configured"
        :no_networks -> "WiFi was configured without any networks"
      end

    Logger.info("[#{inspect(__MODULE__)}] #{msg} - Starting WiFi Wizard")
  end
end
