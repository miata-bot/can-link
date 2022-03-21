defmodule GPSd.Class.VERSION do
  @behaviour GPSd.Class
  import GPSd.Class

  defstruct [:release, :rev, :proto_major, :proto_minor]

  def decode(%{"class" => "VERSION"} = message) do
    version = populate_data(%__MODULE__{}, message)

    with {:ok, release} <- Version.parse(version.release),
         {:ok, rev} <- Version.parse(version.rev) do
      {:ok, %__MODULE__{version | release: release, rev: rev}}
    end
  end
end
