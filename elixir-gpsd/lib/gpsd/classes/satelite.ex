defmodule GPSd.Class.SATELITE do
  alias GPSd.Class.SATELITE

  @behaviour GPSd.Class
  import GPSd.Class, warn: false

  defstruct [
    :PRN,
    :az,
    :el,
    :ss,
    :used,
    :gnssid,
    :svid,
    :sigid,
    :freqid,
    :health
  ]

  def decode(message) do
    satelite = populate_data(%SATELITE{}, message)
    {:ok, satelite}
  end
end
