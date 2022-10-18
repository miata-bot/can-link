defmodule Spect.NetworkNode do
  @moduledoc """
  Join between Network and Node to allow for
  the same node database to be used for
  multiple networks. This table is populated
  with information about a node per this network
  """

  use Ecto.Schema

  @typedoc "Name of the node on this network"
  @type node_name() :: String.t() | nil

  @typedoc "join of network and node"
  @type t() :: %__MODULE__{
          network_id: Spect.Network.network_id(),
          network: Spect.Network.t(),
          node_id: Spect.Network.network_id(),
          node: Spect.Network.Node.t(),
          name: node_name()
        }

  schema "network_nodes" do
    belongs_to :network, Spect.Network
    belongs_to :node, Spect.Network.Node
    field :name, :string
  end
end
