defmodule CANLink.BLE.GenericAccessService do
  alias BlueHeron.GATT.{Characteristic, Service}

  def service do
    # GATT Service 0x1800 Generic Access
    Service.new(%{
      id: __MODULE__,
      type: 0x1800,
      characteristics: [
        # GATT Characteristic and Object Type 0x2A00 Device Name
        Characteristic.new(%{
          id: {__MODULE__, :device_name},
          type: 0x2A00,
          properties: 0b0000010
        }),
        # GATT Characteristic and Object Type 0x2A01 Appearance
        Characteristic.new(%{
          id: {__MODULE__, :appearance},
          type: 0x2A01,
          properties: 0b0000010
        }),
        Characteristic.new(%{
          id: {__MODULE__, :central_address_resolution},
          type: 0x2AA6,
          properties: 0b0000010
        })
      ]
    })
  end

  def write(_, _), do: "error"

  def read(:device_name) do
    with {:ok, hostname} <- :inet.gethostname() do
      to_string(hostname)
    else
      _ -> "error"
    end
  end

  def read(:appearance) do
    # The generic_access service must have an appearance attribute,
    # whose value must be picked from this document: https://specificationrefs.bluetooth.com/assigned-values/Appearance%20Values.pdf
    # This is the standard apperance value for "Medicine Delivery"
    <<0x0D80::little-16>>
  end
end
