import Config

# Configure your database
#
# The MIX_TEST_PARTITION environment variable can be used
# to provide built-in test partitioning in CI environment.
# Run `mix help test` for more information.
config :management_console, ManagementConsole.Repo,
  database: Path.expand("../management_console_test.db", Path.dirname(__ENV__.file)),
  pool_size: 5,
  pool: Ecto.Adapters.SQL.Sandbox

# We don't run a server during test. If one is required,
# you can enable the server option below.
config :management_console, ManagementConsoleWeb.Endpoint,
  http: [ip: {127, 0, 0, 1}, port: 4002],
  secret_key_base: "o/NLTY06VAQV7rwBY65miRTwFPEXdf+LlCV7tWlpYHCKZUJxBeJ+g/S3Jg49rlb2",
  server: false

# In test we don't send emails.
config :management_console, ManagementConsole.Mailer, adapter: Swoosh.Adapters.Test

# Print only warnings and errors during test
config :logger, level: :warn

# Initialize plugs at runtime for faster test compilation
config :phoenix, :plug_init_mode, :runtime
