defmodule Deleteme.Bluetooth.GenericAttributeService do
  alias BlueHeron.GATT.{Characteristic, Characteristic.Descriptor, Service}

  def service do
    # GATT Service 0x1801 Generic Attribute
    Service.new(%{
      id: __MODULE__,
      type: 0x1801,
      characteristics: [
        # GATT Characteristic and Object Type 0x2A05 Service Changed
        Characteristic.new(%{
          id: {__MODULE__, :service_changed},
          type: 0x2A05,
          properties: 0x20,
          # descriptor:
          #   Descriptor.new(%{
          #     permissions: 0b00000010
          #   })
        })
      ]
    })
  end

  def write(_, _), do: "error"

  def read(:service_changed) do
    <<>>
  end
end
