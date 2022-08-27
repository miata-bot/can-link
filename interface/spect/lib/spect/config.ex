defmodule Spect.Config do
  @moduledoc """
  Root level device configuration.
  Versioned for introspection in the future.

  Contians configuration for the spect radio network.
  See the network document for more information.
  """
  use Ecto.Schema

  @typedoc "Current config version. Currently unused"
  @type version :: non_neg_integer()

  @typedoc "Config.id will always be zero"
  @type config_id :: 0

  @typedoc "Root level device configuration. Mostly a container for other records."
  @type t() :: %__MODULE__{
    id: config_id,
    version: version(),
    network: Spect.Network.t(),
    network_id: Spect.Network.network_id(),
    network_identity: Spect.Network.Identity.t(),
    network_identity_id: Spect.Network.Identity.network_identity_id(),
    network_leader: Spect.Network.Leader.t(),
    network_leader_id: Spect.Network.Leader.network_leader_id()
  }

  schema "config" do
    field :version, :integer, default: 1
    belongs_to :network, Spect.Network
    belongs_to :network_identity, Spect.Network.Identity
    belongs_to :network_leader, Spect.Network.Leader
  end
end
