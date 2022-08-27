defmodule Spect.Network.Identity do
  @moduledoc """
  Represents the current device's radio identity. This should be
  set once and never changed.
  """
  use Ecto.Schema

  @typedoc "should never change in the DB"
  @type network_identity_id() :: Spect.Network.Node.node_id()

  @typedoc """
  The current node's identity
  """
  @type t() :: %__MODULE__{
    id: network_identity_id(),
    network: Spect.Network.t(),
    network_id: Spect.Network.network_id(),
    node: Spect.Network.Node.t(),
    node_id: Spect.Network.Node.node_id()
  }

  schema "network_identity" do
    belongs_to :network, Spect.Network, foreign_key: :network_id
    belongs_to :node, Spect.Network.Node
  end
end
