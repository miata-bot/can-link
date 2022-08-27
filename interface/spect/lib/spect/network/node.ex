defmodule Spect.Network.Node do
  @moduledoc "Radio node"
  use Ecto.Schema

  @typedoc """
  Represents a node on the network. Node Info is unknown by
  default, it must be gathered at runtime. Each network
  may have 1023 nodes technically, but one node
  is indistinguishable from another without knowing
  what the currently configured network is.
  """
  @type t() :: %__MODULE__{
    id: node_id(),
    rssi: rssi(),
    last_seen: last_seen()
  }

  schema "nodes" do
    field :rssi, :integer
    field :last_seen, :integer
  end
end
