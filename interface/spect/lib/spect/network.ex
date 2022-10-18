defmodule Spect.Network do
  @moduledoc """
  Represents a radio network instance.
  A network is refered to by it's ID which
  is a 10 bit integer.

  ## Encryption

  A network can be encrypted at the radio level. This is enabled
  by having the `key` set. Packets are encrypted entirely, and the
  `network` concept is implemented on top of that, so if a packet
  is encrypted such that it changes the `network id` bits, packets
  will not be routed to the correct network, and also contain nonsense.
  """
  use Ecto.Schema

  @typedoc "network's ID number. 10 bit integer ranging from 1..1023"
  @type network_id :: 1..1023

  @typedoc """
  Encryption key. If set, all packets received on this network
  will be encrypted/decryped by this key. Must be **EXACTLY** 16 bytes
  if set.
  """
  @type key :: binary() | nil

  @typedoc "Current network configuration"
  @type t() :: %__MODULE__{
          id: network_id,
          key: key,
          nodes: [Spect.Network.Node.t()],
          network_nodes: [Spect.NetworkNode.t()],
          network_identity: Spect.Network.Identity.t(),
          network_leader: Spect.Network.Leader.t()
        }

  schema "networks" do
    field :key, :string
    many_to_many :nodes, Spect.Network.Node, join_through: "network_nodes"
    has_many :network_nodes, Spect.NetworkNode, foreign_key: :network_id
    has_one :network_identity, Spect.Network.Identity, foreign_key: :network_id
    has_one :network_leader, Spect.Network.Leader, foreign_key: :network_id
  end
end
