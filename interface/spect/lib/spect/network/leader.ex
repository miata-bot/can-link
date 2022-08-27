defmodule Spect.Network.Leader do
  @moduledoc """
  Represents the current device's radio leader. Leaders can be changed
  at runtime.
  """

  use Ecto.Schema

  @typedoc "current leader's node id"
  @type network_leader_id() :: Spect.Network.Node.node_id()

  @typedoc """
  The current device's radio leader. The leader
  node broadcasts it's own state as well as some other
  information. See the Radio document for  how the
  protocol acts.
  """
  @type t() :: %__MODULE__{
    id: network_leader_id(),
    network: Spect.Network.t(),
    network_id: Spect.Network.network_id(),
    node: Spect.Network.Node.t(),
    node_id: Spect.Network.Node.node_id()
  }

  schema "network_leader" do
    belongs_to :network, Spect.Network, foreign_key: :network_id
    belongs_to :node, Spect.Node
  end
end
