defmodule JlcHelper.Repo do
  use Ecto.Repo,
    otp_app: :jlc_helper,
    adapter: Ecto.Adapters.SQLite3
end
