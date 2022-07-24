defmodule Spect.Network.Identity do
  use Ecto.Schema

  schema "network_identity" do
    belongs_to :network, Spect.Network, foreign_key: :network_id
    belongs_to :node, Spect.Node
  end
end
