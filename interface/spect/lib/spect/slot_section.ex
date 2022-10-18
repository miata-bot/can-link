defmodule Spect.SlotSection do
  @moduledoc """
  Join table between section and slot
  """

  use Ecto.Schema

  @type slot_section_id() :: non_neg_integer()

  @typedoc "join table on section and slot"
  @type t() :: %__MODULE__{
          id: slot_section_id(),
          section: Spect.Section.t(),
          section_id: Spect.Section.section_id(),
          slot: Spect.EffectSlot.t(),
          slot_id: Spect.EffectSlot.slot_id()
        }

  schema "slot_sections" do
    belongs_to :section, Spect.Section
    belongs_to :slot, Spect.EffectSlot, foreign_key: :slot_id
  end
end
