# This file is responsible for configuring your application
# and its dependencies with the aid of the Config module.
#
# This configuration file is loaded before any dependency and
# is restricted to this project.

# General application configuration
import Config

config :jlc_helper,
  ecto_repos: [JlcHelper.Repo]

# Configures the endpoint
config :jlc_helper, JlcHelperWeb.Endpoint,
  url: [host: "localhost"],
  render_errors: [view: JlcHelperWeb.ErrorView, accepts: ~w(html json), layout: false],
  pubsub_server: JlcHelper.PubSub,
  live_view: [signing_salt: "zA95J7fY"]

# Configure esbuild (the version is required)
config :esbuild,
  version: "0.14.0",
  default: [
    args:
      ~w(js/app.js --bundle --target=es2017 --outdir=../priv/static/assets --external:/fonts/* --external:/images/*),
    cd: Path.expand("../assets", __DIR__),
    env: %{"NODE_PATH" => Path.expand("../deps", __DIR__)}
  ]

# Configures Elixir's Logger
config :logger, :console,
  format: "$time $metadata[$level] $message\n",
  metadata: [:request_id]

# Use Jason for JSON parsing in Phoenix
config :phoenix, :json_library, Jason

config :tesla, adapter: Tesla.Adapter.Hackney

config :meilisearch,
  endpoint: "http://127.0.0.1:7700"

# Import environment specific config. This must remain at the bottom
# of this file so it overrides the configuration defined above.
import_config "#{config_env()}.exs"
