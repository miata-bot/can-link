defmodule Spect.Network.Node do
  use Ecto.Schema

  schema "nodes" do
    field :rssi, :integer
    field :last_seen, :integer
  end
end
