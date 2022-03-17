defmodule HaltechCANProtocol.CSVUtil do
  def to_integer("0x" <> string) do
    {value, ""} = Integer.parse(string, 16)
    value
  end

  def parse_position(position) do
    cond do
      match?(<<_l, ":", _r>>, position) ->
        <<l::binary-1, ":", r::binary-1>> = position
        {:bit, [String.to_integer(l), String.to_integer(r)]}

      String.contains?(position, ":") ->
        range =
          position
          |> String.split("-")
          |> Enum.map(&String.trim/1)
          |> Enum.map(&String.split(&1, ":"))
          |> Enum.map(&Enum.map(&1, fn d -> String.to_integer(d) end))

        {:bit_range, range}

      String.contains?(position, "-") ->
        range =
          position
          |> String.split("-")
          |> Enum.map(&String.trim/1)
          |> Enum.map(&String.to_integer/1)

        {:range, range}

      match?(<<_>>, position) ->
        String.to_integer(position)

      true ->
        raise "could not parse position"
    end
  end

  def parse_sign("Unsigned"), do: :unsigned
  def parse_sign("Unsiged"), do: :unsigned
  def parse_sign("Signed"), do: :signed

  def parse_units(unit), do: unit

  def parse_conversion("y := " <> mapping) do
    map =
      mapping
      |> String.split(",")
      |> Enum.map(&String.trim/1)
      |> Map.new(fn map ->
        [num, value] = String.split(map, "=")
        {String.to_integer(num), value}
      end)

    {:enum, map}
  end

  def parse_conversion("y = " <> _ = conversion) do
    # lol lazy
    {:conversion, Code.string_to_quoted!(conversion)}
  end

  defmacro compile_parser(canid, data) do
    quote bind_quoted: [canid: canid, data: data] do
      def parse_frame!(unquote(canid), unquote(compile_pattern(data))) do
        unquote(compile_calculations(data))
        binding()
      end
    end
  end

  def compile_pattern(data, acc \\ [])

  def compile_pattern([{{:range, [a, b]}, :unsigned, name, _unit, _conversion} | rest], acc) do
    var = into_var(name)

    quoted =
      quote do
        unquote(var) :: unsigned - size(unquote((b - a + 1) * 8))
      end

    compile_pattern(rest, [quoted | acc])
  end

  def compile_pattern([{{:range, [a, b]}, :signed, name, _unit, _conversion} | rest], acc) do
    var = into_var(name)

    quoted =
      quote do
        unquote(var) :: signed - size(unquote((b - a + 1) * 8))
      end

    compile_pattern(rest, [quoted | acc])
  end

  def compile_pattern([{{:bit, [_a, _b]}, :unsigned, name, _unit, _conversion} | rest], acc) do
    var = into_var(name)

    quoted =
      quote do
        unquote(var) :: 1
      end

    compile_pattern(rest, [quoted | acc])
  end

  def compile_pattern([{_byte, :signed, name, _unit, _conversion} | rest], acc) do
    var = into_var(name)

    quoted =
      quote do
        unquote(var) :: signed - 8
      end

    compile_pattern(rest, [quoted | acc])
  end

  def compile_pattern([{_byte, :unsigned, name, _unit, _conversion} | rest], acc) do
    var = into_var(name)

    quoted =
      quote do
        unquote(var) :: unsigned - 8
      end

    compile_pattern(rest, [quoted | acc])
  end

  def compile_pattern([], acc) do
    quote do
      <<unquote_splicing(Enum.reverse(acc))>>
    end
  end

  def compile_calculations(data, acc \\ [])

  def compile_calculations([{_, _, name, _unit, {:enum, map}} | rest], acc) do
    var = into_var(name)

    quoted =
      quote do
        unquote(var) = unquote(Macro.escape(map))[unquote(var)]
      end

    compile_calculations(rest, [quoted | acc])
  end

  def compile_calculations([{_, _, name, _unit, {:conversion, conversion}} | rest], acc) do
    {:=, meta, ops} = conversion
    hacked_ops = hack_ops(ops, into_var(name))
    compile_calculations(rest, [{:=, meta, hacked_ops} | acc])
  end

  def compile_calculations([], acc) do
    quote do
      (unquote_splicing(acc))
    end
  end

  def hack_ops(ops, var, acc \\ [])

  def hack_ops([{_, _, nil} | rest], var, acc) do
    hack_ops(rest, var, [var | acc])
  end

  def hack_ops([{op, meta, ops} | rest], var, acc) do
    hack_ops(rest, var, [{op, meta, hack_ops(ops, var)} | acc])
  end

  def hack_ops([value | rest], var, acc) do
    hack_ops(rest, var, [value | acc])
  end

  def hack_ops([], _, acc), do: Enum.reverse(acc)

  def into_var(name) do
    var =
      name
      # don't ask
      |> String.replace(:binary.compile_pattern(<<194, 160>>), "")
      |> String.replace(" ", "")
      |> String.replace(["(", ")"], "")
      |> String.replace(["-"], "")
      |> Macro.underscore()
      |> String.to_atom()

    {var, [], nil}
  end
end
