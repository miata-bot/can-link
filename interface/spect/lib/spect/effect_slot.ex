defmodule Spect.EffectSlot do
  @moduledoc """
  Represents an effect running the device.

  * Effects are implemented as Lua scripts internally.
  * Mode is a bitfield described below
  * There are 8 effect slots total

  Each slot may use *up to* or *all of* the 8 `sections`
  on the device. See the `sections` document for a
  description of how that works.

  ## Mode Bitfield
  | 7             | 6          | 5         | 4 | 3 | 2 | 1 | 0 |
  | ------------- | ---------- | --------- | - | - | - | - | - |
  | effect enable | radio sync | can sync  | - | - | - | - | - |

  Bits 4-8 are reserved for future use

  * `effect_enalbe` - enable or disable this effect slot
  * `radio_sync`    - enable or disable syncing of this slot on the network
  * `can_sync`      - enable or disable syncing of this slot via CAN
  """
  use Ecto.Schema

  schema "effect_slots" do
    field :mode, :integer
    field :script_name, :string
  end
end
