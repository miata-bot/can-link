defmodule TableGenerator.Assembler do
  defstruct instructions: [],
            symbols: %{},
            data_start_address: 0

  def assemble(contents) when is_binary(contents) do
    original = String.split(contents, "\n")

    tokens =
      original
      |> Enum.zip(1..Enum.count(original))
      |> strip_comments()
      |> tokenize([])

    {symbols, data_start_address} =
      Enum.reduce(tokens, {%{}, 0}, fn
        {{:identifier, name, _value}, _line}, {symbols, address} ->
          # {Map.put(symbols, name, address), [{{:identifier, name, value}, line} | instructions],
          #  address + byte_size(value)}
          #  {Map.put(symbols, name, address), instructions, address + byte_size(value)}
           {Map.put(symbols, name, address), address}

        {{:instruction, a, b}, line}, {symbols, address} ->
          instr =
            {{:instruction, parse_instruction(a, symbols, line),
              parse_instruction(b, symbols, line)}, line}

          size = instr_size(instr)
          {symbols,  address + size}
      end)

      instructions = Enum.reduce(tokens,  [], fn
        {{:instruction, a, b}, line}, instructions ->
          instr =
            {{:instruction, parse_instruction(a, symbols, line),
              parse_instruction(b, symbols, line)}, line}

          [instr | instructions]
        {{:identifier, _name, _value}, _line}, instructions ->
          instructions
      end)

      data_section = Enum.reduce(tokens, {[], 0}, fn
        {{:instruction, _a, _b}, _line}, {data, address} ->
          data
        {{:identifier, name, ""}, _line}, {data, adress} ->
          data
        {{:identifier, name, value}, _line}, {data, address} ->

      end)

    %__MODULE__{
      instructions: Enum.reverse(instructions),
      symbols: symbols,
      data_start_address: data_start_address
    }
    |> write_file()
  end

  defp write_file(assembly) do
    Enum.map(assembly.instructions, fn
      {{:instruction, a, b}, _} ->
        assemble_instruction(a) <> assemble_instruction(b)
    end)

  end

  defp assemble_instruction({:nop, []}), do: <<0x0>>
  defp assemble_instruction({:halt, []}), do: <<0x1>>
  defp assemble_instruction({:cls, []}), do: <<0x3>>
  defp assemble_instruction({:fill, [r1]}), do: <<0x4, assemble_register(r1)>>
  defp assemble_instruction({:ld, [r1, value]}) when value <= 0xffffffff, do: <<0x5, assemble_register(r1), value::32>>
  defp assemble_instruction({:flush, []}), do: <<0x6>>
  defp assemble_instruction({:set, [r1, r2]}) when r1 in [:x, :y, :za, :zb, :tm, :lp], do: <<0x7, assemble_register(r1), assemble_register(r2)>>
  defp assemble_instruction({:set, [value, r2]}), do: <<0x8, value::8, assemble_register(r2)>>
  defp assemble_instruction({:inc, [r1]}), do: <<0x9, assemble_register(r1)>>
  defp assemble_instruction({:dec, [r1]}), do: <<0xA, assemble_register(r1)>>
  defp assemble_instruction({:jnz, [r1, address]}), do: <<0xB, assemble_register(r1), address::24>>

  defp assemble_register(:x), do: 0x1
  defp assemble_register(:y), do: 0x2
  defp assemble_register(:za), do: 0x3
  defp assemble_register(:zb), do: 0x4
  defp assemble_register(:tm), do: 0x5
  defp assemble_register(:lp), do: 0x6

  defp instr_size({{:instruction, a, b}, _line}) do
    instr_size(a) + instr_size(b)
  end

  defp instr_size({_, operands}) do
    operand_size =
      Enum.map(operands, fn
        register when register in [:x, :y, :za, :zb, :tm, :lp] -> 8
        value when value <= 0xFFFFFFFF -> 32
      end)
      |> Enum.sum()

    operand_size + 8
  end

  @registers ~W(X Y ZA ZB TM LP)

  def parse_instruction(["NOP"], _, _line), do: {:nop, []}
  def parse_instruction(["CLS"], _, _line), do: {:cls, []}
  def parse_instruction(["HALT"], _, _line), do: {:halt, []}

  def parse_instruction(["FILL", register], _, line) do
    {:fill, [parse_register(register, line)]}
  end

  def parse_instruction(["LD", register, value], _, line) do
    {:ld, [parse_register(register, line), parse_value(value, line)]}
  end

  def parse_instruction(["FLUSH"], _, _line) do
    {:flush, []}
  end

  def parse_instruction(["SET", r1, r2], _, line) when r1 in @registers and r2 in @registers do
    {:set, [parse_register(r1, line), parse_register(r2, line)]}
  end

  def parse_instruction(["SET", value, r2], _, line) when r2 in @registers do
    {:set, [parse_value(value, line), parse_register(r2, line)]}
  end

  def parse_instruction(["INC", r1], _, line) when r1 in @registers do
    {:inc, [parse_register(r1, line)]}
  end

  def parse_instruction(["DEC", r1], _, line) when r1 in @registers do
    {:dec, [parse_register(r1, line)]}
  end

  def parse_instruction(["JNZ", r1, identifier_or_value], symbols, line) when r1 in @registers do
    {:jnz,
     [parse_register(r1, line), parse_identifier_or_value(identifier_or_value, symbols, line)]}
  end

  def parse_instruction(unk, _symbols, line) do
    raise "unknown instruction #{Enum.join(unk, " ")} at line #{line}"
  end

  def parse_register("X", _), do: :x
  def parse_register("Y", _), do: :y
  def parse_register("ZA", _), do: :za
  def parse_register("ZB", _), do: :zb
  def parse_register("TM", _), do: :tm
  def parse_register("LP", _), do: :lp

  def parse_register(unknown, line),
    do: raise("failed to parse #{unknown} register at line #{line}")

  def parse_value("#" <> color_code, line) do
    case Integer.parse(color_code, 16) do
      {int, ""} -> int
      _ -> raise "Failed to assemble \##{color_code} as a valid color value at line #{line}"
    end
  end

  def parse_value("0x" <> value, line) do
    case Integer.parse(value, 16) do
      {int, ""} -> int
      _ -> raise "Failed to assemble \##{value} as a valid value at line #{line}"
    end
  end

  def parse_value(value, line) do
    case Integer.parse(value) do
      {int, ""} -> int
      _ -> raise "Failed to assemble \##{value} as a valid value at line #{line}"
    end
  end

  def parse_identifier_or_value("." <> identifier, symbols, line) do
    address = symbols[identifier] || raise "unknown symbol #{identifier} at line #{line}"
    address
  end

  def parse_identifier_or_value(value, line) do
    parse_value(value, line)
  end

  # don't look too hard at this, sorry
  defp tokenize([{content, line} | rest], tokens) do
    case String.trim(content) do
      "." <> identifier ->
        [name, value] = String.split(identifier, ":", parts: 2)
        tokenize(rest, [{{:identifier, String.trim(name), String.trim(value)}, line} | tokens])

      instruction ->
        [a, b] = String.split(instruction, "-", parts: 2)
        [a, b] = [String.trim(a), String.trim(b)]
        [a, b] = [String.split(a, " "), String.split(b, " ")]
        tokenize(rest, [{{:instruction, a, b}, line} | tokens])
    end
  end

  defp tokenize([], tokens), do: Enum.reverse(tokens)

  defp strip_comments(lines) do
    Enum.map(lines, fn {content, line} ->
      {maybe_strip_comment(content, ""), line}
    end)
    |> Enum.reject(fn
      {"", _} -> true
      _ -> false
    end)
  end

  defp maybe_strip_comment(";" <> _, line), do: line

  defp maybe_strip_comment(<<c::binary-1, rest::binary>>, line),
    do: maybe_strip_comment(rest, line <> c)

  defp maybe_strip_comment(<<>>, line), do: line
end
