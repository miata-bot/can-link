defmodule TableGenerator.FWUpdate do
  def update(path) do
    bin = File.read!(path)
    {:ok, uart} = Circuits.UART.start_link()
    :ok = Circuits.UART.open(uart, "ttyACM0", speed: 115200)
    Circuits.UART.write(uart, <<0x1::8, 60::little-16>>)
    upload(uart, bin)
    Circuits.UART.write(uart, <<0x4::8>>)
  end

  def upload(uart, <<chunk::binary-size(60), rest::binary>>) do
    Circuits.UART.write(uart, <<0x7::8, chunk::binary>>)
    upload(uart, rest)
  end

  def upload(uart, chunk) when byte_size(chunk) <= 60 do
    Circuits.UART.write(uart, <<0x7::8, chunk::binary>>)
  end
end
