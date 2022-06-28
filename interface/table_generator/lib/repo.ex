defmodule TableGenerator.Repo do
  use Ecto.Repo, otp_app: :table_generator, adapter: Ecto.Adapters.SQLite3
end
