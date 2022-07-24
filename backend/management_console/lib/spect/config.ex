defmodule Spect.Config do
  use Ecto.Schema

  schema "config" do
    field :version, :integer, default: 1
    field :rgb_channel_1_enable, :boolean
    field :rgb_channel_2_enable, :boolean

    # strip channels
    field :strip_channel_1_enable, :boolean
    field :strip_channel_2_enable, :boolean

    # digital input
    field :digital_input_1_enable, :boolean
    field :digital_input_2_enable, :boolean
    field :digital_input_3_enable, :boolean
    field :digital_input_4_enable, :boolean

    has_many :networks, Spect.Network, foreign_key: :config_id
    has_one :state, Spect.State, foreign_key: :config_id
    timestamps()
  end
end
