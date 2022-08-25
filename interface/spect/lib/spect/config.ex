defmodule Spect.Config do
  @moduledoc """
  Root level device configuration.
  Versioned for introspection in the future.

  Contians configuration for the spect radio network.
  See the network document for more information.
  """
  use Ecto.Schema

  schema "config" do
    field :version, :integer, default: 1
    belongs_to :network, Spect.Network
    belongs_to :network_identity, Spect.Network.Identity
    belongs_to :network_leader, Spect.Network.Leader
  end
end
