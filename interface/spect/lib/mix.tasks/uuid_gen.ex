defmodule Mix.Tasks.UuidGen do
  @moduledoc false
  @shortdoc false
  use Mix.Task

  def run([name]) do
    uuid = Ecto.UUID.bingenerate()
    {:ok, uuid_str} = Ecto.UUID.cast(uuid)

    [top, bottom] =
      uuid
      |> :erlang.binary_to_list()
      |> Enum.reverse()
      |> Enum.map(&Integer.to_string(&1, 16))
      |> Enum.map(&Kernel.<>("0x", &1))
      |> Enum.split(8)
      |> Tuple.to_list()
      |> Enum.map(&Enum.join(&1, ","))

    # |> Enum.join(", ")
    """
    /* #{uuid_str} */
    static const ble_uuid128_t #{name} =
      BLE_UUID128_INIT(#{top},
                       #{bottom});
    """
    |> IO.puts()
  end
end
