defmodule GlowBot.Repo do
  use Ecto.Repo,
    otp_app: :glow_bot,
    adapter: Ecto.Adapters.SQLite3
end
