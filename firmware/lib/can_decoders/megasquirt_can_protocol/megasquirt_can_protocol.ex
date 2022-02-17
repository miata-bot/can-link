defmodule MegasquirtCANProtocol do
  import MegasquirtCANProtocol.CSVUtil

  specs =
    File.stream!("lib/can_decoders/megasquirt_can_protocol/ms-can-proto.csv")
    |> NimbleCSV.RFC4180.parse_stream()
    |> Enum.to_list()
    |> Enum.with_index(2)
    |> Enum.map(fn
      {[group, offset, total_offset, size, signed?, name, function, units, multiply, divide, add, ms2?] = full, line} ->
        try do
          group = to_integer(group)
          offset = to_integer(offset)
          total_offset = to_integer(total_offset)
          size = to_integer(size)
          signed? = to_bool(signed?)
          name = String.trim(name)
          function = String.trim(function)
          units = to_units(units)
          multiply = to_integer(multiply)
          divide = to_integer(divide)
          add = to_integer(add)
          ms2? = to_bool(ms2?)
          {group, offset, total_offset, size, signed?, name, function, units, multiply, divide, add, ms2?}
        catch
          error, reason ->
            raise CompileError,
              file: "ms-can-proto.csv",
              line: line,
              description: """
              caught #{inspect(error)} compiling CSV: #{inspect(full)}
              #{inspect(reason)}
              """
        end

      {unknown, line} ->
        raise CompileError, file: "ms-can-proto.csv", line: line, description: "Unknown line: #{inspect(unknown)}"
    end)
    |> Enum.group_by(
      fn {group, _, _, _, _, _, _, _, _, _, _, _} -> group end,
      fn {_, offset, total_offset, size, signed?, name, function, units, multiply, divide, add, ms2?} ->
        {offset, total_offset, size, signed?, name, function, units, multiply, divide, add, ms2?}
      end
    )
    |> Enum.sort_by(fn {group, _data} -> group end)

  Enum.each(specs, fn {group, data} ->
    compile_parser(1512 + group, data)
  end)
end
