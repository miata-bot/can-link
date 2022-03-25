defmodule Mix.Tasks.Reload do
  use Mix.Task
  require Logger
  @app "can_link"

  def run(opts) do
    {args, extra, _} =
      OptionParser.parse(opts,
        strict: [name: :string, hostname: :string, app: :string, restart: :boolean],
        aliases: [n: :name, a: :app, h: :hostname, r: :restart]
      )

    unless Enum.empty?(extra), do: Mix.raise("Unknown args: #{inspect(extra)}")
    {:ok, _} = Node.start(:reloader@localhost)
    Node.set_cookie(:KFUYVQEXKNOEJXOBANFE)
    Logger.info("Distribution started at reloader@localhost")
    remote_node = :"#{args[:name] || "can_link"}@#{args[:hostname]}"
    true = Node.connect(remote_node)
    Logger.info("Distribution connected to #{args[:name] || "can_link"}@#{args[:hostname]}")
    app = String.to_existing_atom(args[:app] || @app)

    Logger.info("Reloading #{app}")
    :ok = Application.ensure_loaded(app)
    {:ok, modules} = :application.get_key(app, :modules)

    for module <- modules do
      IEx.Helpers.nl([remote_node], module)
      Logger.info("Reloaded #{module}")
    end

    if args[:restart] do
      :rpc.call(remote_node, Application, :stop, [app])
      :rpc.call(remote_node, Application, :ensure_all_started, [app])
    end
  end
end
