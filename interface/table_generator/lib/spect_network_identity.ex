defmodule SpectNetworkIdentity do
  use Ecto.Schema

  schema "network_identity" do
    belongs_to :network, SpectNetwork, foreign_key: :network_id
    belongs_to :node, SpectNode
  end
end
