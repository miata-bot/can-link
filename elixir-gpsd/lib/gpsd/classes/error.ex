defmodule GPSd.Class.ERROR do
  defstruct [:message]

  def decode(%{"class" => "ERROR", "message" => message}) do
    {:ok, %__MODULE__{message: message}}
  end
end
