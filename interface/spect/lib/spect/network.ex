defmodule Spect.Network do
  use Ecto.Schema

  schema "networks" do
    field :key, :string
    many_to_many :nodes, Spect.Network.Node, join_through: "network_nodes"
    has_many :network_nodes, Spect.NetworkNode, foreign_key: :network_id
    has_one :network_identity, Spect.Network.Identity, foreign_key: :network_id
    has_one :network_leader, Spect.Network.Leader, foreign_key: :network_id
  end
end
