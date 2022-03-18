defmodule HaltechCANProtocol do
  import HaltechCANProtocol.CSVUtil

  @external_resource "Haltech-CAN-Broadcast-Protocol-V2.35.0.csv"
  {last, acc} =
    File.stream!(@external_resource)
    |> NimbleCSV.RFC4180.parse_stream()
    |> Enum.to_list()
    # line numbers
    |> Enum.with_index(2)
    # sort into buckets indexed by can id
    |> Enum.reduce({{nil, []}, []}, fn
      {["", "", "" | row], line}, {{canid, buffer}, acc} ->
        {{canid, [{row, line} | buffer]}, acc}

      {[canid, rate, direction | row], line}, {{lastcanid, buffer}, acc} ->
        {{{canid, rate, direction}, [{row, line}]}, [{lastcanid, Enum.reverse(buffer)} | acc]}
    end)

  [_ | specs] = Enum.reverse([last | acc])

  parsed =
    Enum.map(specs, fn
      {{canid, _rate, _direction}, data} ->
        {to_integer(canid),
         Enum.map(data, fn
           {[position, sign, channel, units, conversion], _line} ->
             {parse_position(position), parse_sign(sign), channel, parse_units(units), parse_conversion(conversion)}
         end)}
    end)

  @doc "Compiled fun"
  Enum.each(parsed, fn {canid, data} ->
    compile_parser(canid, data)
  end)
end
