defmodule Spect.Network.Node do
  use Ecto.Schema

  schema "nodes" do
    belongs_to :network, Spect.Network
    field :name, :string
  end
end
