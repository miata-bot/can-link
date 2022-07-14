defmodule SpectConfig do
  use Ecto.Schema
  # @primary_key false
  # @timestamps false

  schema "config" do
    # field :id, :integer, primary_key: true
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

    has_many :networks, SpectNetwork, foreign_key: :config_id
    has_one :state, SpectState, foreign_key: :config_id
    timestamps()
  end
end
