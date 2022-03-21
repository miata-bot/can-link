defmodule GPSd.Class.SKY do
  alias GPSd.Class.SKY

  @behaviour GPSd.Class
  import GPSd.Class, warn: false

  defstruct [
    :device,
    :time,
    :gdop,
    :hdop,
    :pdop,
    :tdop,
    :vdop,
    :xdop,
    :ydop,
    :nSat,
    :uSat,
    :satellites
  ]

  def decode(%{"class" => "SKY"} = message) do
    sky =
      populate_data(%SKY{}, message)
      |> populate_array(:satellites, GPSd.Class.SATELITE)

    {:ok, sky}
  end
end
