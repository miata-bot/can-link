defmodule GPSd.Class.TPV do
  import GPSd.Class
  @behaviour GPSd.Class

  defstruct [
    :device,
    :mode,
    :status,
    :time,
    :altHAE,
    :altMSL,
    :alt,
    :climb,
    :datum,
    :depth,
    :dgpsAge,
    :dgpsSta,
    :epc,
    :epd,
    :eph,
    :ept,
    :epx,
    :epy,
    :epv,
    :geoidSep,
    :lat,
    :leapseconds,
    :lon,
    :track,
    :magtrack,
    :magvar,
    :speed,
    :ecefx,
    :ecefy,
    :ecefz,
    :ecefpAcc,
    :ecefvx,
    :ecefvy,
    :ecefvz,
    :ecefvAcc,
    :sep,
    :relD,
    :relE,
    :relN,
    :velD,
    :velE,
    :velN,
    :wanglem,
    :wangler,
    :wanglet,
    :wspeedr,
    :wspeedt
  ]

  def decode(%{"class" => "TPV", "mode" => 0}) do
    tpv = %__MODULE__{mode: :unknown}
    {:ok, tpv}
  end

  def decode(%{"class" => "TPV", "mode" => 1} = message) do
    tpv =
      %__MODULE__{mode: :nofix}
      |> populate_data(message)

    {:ok, tpv}
  end

  def decode(%{"class" => "TPV", "mode" => 2} = message) do
    tpv =
      %__MODULE__{mode: :"2d"}
      |> populate_data(message)

    {:ok, tpv}
  end

  def decode(%{"class" => "TPV", "mode" => 3} = message) do
    tpv =
      %__MODULE__{mode: :"3d"}
      |> populate_data(message)

    {:ok, tpv}
  end
end
