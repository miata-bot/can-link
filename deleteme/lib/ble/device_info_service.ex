defmodule Deleteme.Bluetooth.DeviceInfoService do
  alias BlueHeron.GATT.{Characteristic, Service}

  def service do
    # GATT Service 0x180A Device Information
    Service.new(%{
      id: __MODULE__,
      type: 0x180A,
      characteristics: [
        # GATT Characteristic and Object Type 0x2A23 System ID
        Characteristic.new(%{
          id: {__MODULE__, :system_id},
          type: 0x2A24,
          properties: 0b0000010
        }),
        # GATT Characteristic and Object Type 0x2A24 Model Number String
        Characteristic.new(%{
          id: {__MODULE__, :model_number},
          type: 0x2A24,
          properties: 0b0000010
        }),
        # GATT Characteristic and Object Type 0x2A25 Serial Number String
        Characteristic.new(%{
          id: {__MODULE__, :serial_number},
          type: 0x2A25,
          properties: 0b0000010
        }),
        # GATT Characteristic and Object Type 0x2A26 Firmware Revision String
        Characteristic.new(%{
          id: {__MODULE__, :firmware_revision},
          type: 0x2A26,
          properties: 0b0000010
        }),
        # GATT Characteristic and Object Type 0x2A27 Hardware Revision String
        Characteristic.new(%{
          id: {__MODULE__, :hardware_revision},
          type: 0x2A27,
          properties: 0b0000010
        }),
        # GATT Characteristic and Object Type 0x2A28 Software Revision String
        Characteristic.new(%{
          id: {__MODULE__, :software_revision},
          type: 0x2A28,
          properties: 0b0000010
        }),
        # GATT Characteristic and Object Type 0x2A29 Manufacturer Name String
        Characteristic.new(%{
          id: {__MODULE__, :manufacturer_name},
          type: 0x2A29,
          properties: 0b0000010
        })
      ]
    })
  end

  def write(_, _), do: "error"

  def read(:system_id) do
    "unused"
  end

  def read(:model_number) do
    "rpi"
  end

  def read(:serial_number) do
    Nerves.Runtime.serial_number()
  end

  def read(:hardware_revision) do
    "0w"
  end

  def read(:manufacturer_name) do
    "rpi"
  end

  def read(:software_revision) do
    {:ok, vsn} = :application.get_key(:deleteme, :vsn)
    to_string(vsn)
  end

  def read(:firmware_revision) do
    Nerves.Runtime.KV.get_active("nerves_fw_uuid")
  end
end
