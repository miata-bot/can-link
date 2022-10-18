defmodule Spect.Hardware do
  @moduledoc """
  Describes the hardware configuration.
  only one instance of this table will ever exist.
  """

  use Ecto.Schema

  @typedoc "there can only be one table, so ID is always 0"
  @type hardware_id() :: 0

  @type t() :: %__MODULE__{
          id: hardware_id(),
          hardware_channel1: Spect.HardwareChannel.t(),
          hardware_channel2: Spect.HardwareChannel.t(),
          hardware_trigger1: Spect.HardwareTrigger.t(),
          hardware_trigger2: Spect.HardwareTrigger.t(),
          hardware_trigger3: Spect.HardwareTrigger.t(),
          hardware_trigger4: Spect.HardwareTrigger.t(),
          channel1: Spect.HardwareChannel.hardware_channel_id(),
          channel2: Spect.HardwareChannel.hardware_channel_id(),
          trigger1: Spect.HardwareTrigger.hardware_trigger_id(),
          trigger2: Spect.HardwareTrigger.hardware_trigger_id(),
          trigger3: Spect.HardwareTrigger.hardware_trigger_id(),
          trigger4: Spect.HardwareTrigger.hardware_trigger_id()
        }

  schema "hardware" do
    belongs_to :hardware_channel1, Spect.HardwareChannel, foreign_key: :channel1
    belongs_to :hardware_channel2, Spect.HardwareChannel, foreign_key: :channel2
    belongs_to :hardware_trigger1, Spect.HardwareTrigger, foreign_key: :trigger1
    belongs_to :hardware_trigger2, Spect.HardwareTrigger, foreign_key: :trigger2
    belongs_to :hardware_trigger3, Spect.HardwareTrigger, foreign_key: :trigger3
    belongs_to :hardware_trigger4, Spect.HardwareTrigger, foreign_key: :trigger4
  end
end
