defmodule ManagementConsole.Repo do
  use Ecto.Repo,
    otp_app: :management_console,
    adapter: Ecto.Adapters.SQLite3
end
