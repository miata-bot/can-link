defmodule Mix.Tasks.Reload do
  use Mix.Task
  require Logger

  def run(opts) do
    {args, extra, _} = OptionParser.parse(opts, strict: [hostname: :string, app: :string], aliases: [a: :app, h: :hostname])
    unless Enum.empty?(extra), do: Mix.raise "Unknown args: #{inspect extra}"
    {:ok, _} = Node.start(:reloader@localhost)
    Node.set_cookie(:KFUYVQEXKNOEJXOBANFE)
    Logger.info("Distribution started at reloader@localhost")
    remote_node = :"deleteme@#{args[:hostname]}"
    true = Node.connect(remote_node)
    Logger.info("Distribution connected to deleteme@#{args[:hostname]}")
    app = String.to_existing_atom(args[:app] || "rf69")
    :ok = Application.ensure_loaded(app)
    {:ok, modules} = :application.get_key(app, :modules)

    for module <- modules do
      IEx.Helpers.nl([remote_node], module)
      Logger.info("Reloaded #{module}")
    end

    # :rpc.call(remote_node, Application, :stop, [:deleteme])
    # :rpc.call(remote_node, Application, :ensure_all_started, [:deleteme])
  end
end
