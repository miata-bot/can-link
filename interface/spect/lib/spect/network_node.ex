defmodule Spect.NetworkNode do
  use Ecto.Schema

  schema "network_nodes" do
    belongs_to :network, Spect.Network
    belongs_to :node, Spect.Network.Node
    field :name, :string
  end
end
