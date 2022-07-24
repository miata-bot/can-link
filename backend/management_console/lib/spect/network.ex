defmodule Spect.Network do
  use Ecto.Schema

  schema "networks" do
    belongs_to :config, Spect.Config
    field :key, :string
    has_many :nodes, Spect.Network.Node, foreign_key: :network_id
    has_one :network_identity, Spect.Network.Identity, foreign_key: :network_id
  end
end
