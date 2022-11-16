defmodule JlcHelper.Application do
  # See https://hexdocs.pm/elixir/Application.html
  # for more information on OTP Applications
  @moduledoc false

  use Application

  @impl true
  def start(_type, _args) do
    children = [
      # Start the Ecto repository
      JlcHelper.Repo,
      # Start the Telemetry supervisor
      JlcHelperWeb.Telemetry,
      # Start the PubSub system
      {Phoenix.PubSub, name: JlcHelper.PubSub},
      # Start the Endpoint (http/https)
      JlcHelperWeb.Endpoint,
      # Start a worker by calling: JlcHelper.Worker.start_link(arg)
      # {JlcHelper.Worker, arg}
      JlcHelper.ElasticsearchCluster
    ]

    # See https://hexdocs.pm/elixir/Supervisor.html
    # for other strategies and supported options
    opts = [strategy: :one_for_one, name: JlcHelper.Supervisor]
    Supervisor.start_link(children, opts)
  end

  # Tell Phoenix to update the endpoint configuration
  # whenever the application is updated.
  @impl true
  def config_change(changed, _new, removed) do
    JlcHelperWeb.Endpoint.config_change(changed, removed)
    :ok
  end
end
