defmodule SpectState do
  use Ecto.Schema

  schema "state" do
    belongs_to :config, SpectConfig
    field :mode, :integer
    field :rgb_channel_1_color, :integer
    field :rgb_channel_1_brightness, :integer
    field :rgb_channel_2_color, :integer
    field :rgb_channel_2_brightness, :integer
  end
end
