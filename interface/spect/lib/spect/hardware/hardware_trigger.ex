defmodule Spect.HardwareTrigger do
  @moduledoc """
  Descirbes one of the 4 available input chcannels.
  There will always be 4 channels, no more no less.
  """

  use Ecto.Schema

  @typedoc "which id a trigger is assosiated with. 1-4"
  @type hardware_trigger_id() :: 1 | 2 | 3 | 4

  @typedoc "describes the configuration of one of the 4 hardware inputs"
  @type t() :: %__MODULE__{
          id: hardware_trigger_id(),
          enable: boolean(),
          friendly_name: String.t()
        }

  schema "hardware_triggers" do
    field :enable, :boolean
    field :friendly_name, :string
  end
end
