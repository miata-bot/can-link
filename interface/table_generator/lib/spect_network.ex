defmodule SpectNetwork do
  use Ecto.Schema

  schema "networks" do
    belongs_to :config, SpectConfig
    field :key, :string
    has_many :nodes, SpectNode, foreign_key: :network_id
  end
end
