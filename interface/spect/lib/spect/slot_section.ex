defmodule Spect.SlotSection do
  use Ecto.Schema

  schema "slot_sections" do
    belongs_to :section, Spect.Section
    belongs_to :slot, Spect.EffectSlot, foreign_key: :slot_id
  end
end
