defmodule SpectNode do
  use Ecto.Schema

  schema "nodes" do
    belongs_to :network, SpectNetwork
    field :name, :string
  end
end
