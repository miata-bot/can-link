defmodule Spect.HardwareChannel do
  @moduledoc """
  Holds the configuration of *one* channel.
  There are always two channels available, there
  will never be more or less.
  """

  use Ecto.Schema

  @typedoc "index of which channel, 1 or 2"
  @type hardware_channel_id() :: 1 | 2

  @typedoc "limited to uint16_t"
  @type strip_length() :: 0..0xFFFF

  @typedoc "Describes one of the two channels"
  @type t() :: %__MODULE__{
          id: hardware_channel_id(),
          strip_enable: boolean(),
          rgb_enable: boolean(),
          strip_length: strip_length(),
          friendly_name: String.t()
        }

  schema "hardware_channels" do
    field :strip_enable, :boolean
    field :strip_length, :integer
    field :rgb_enable, :boolean
    field :friendly_name, :string
  end
end
