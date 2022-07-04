defmodule TableGenerator.Assembler do
  defstruct instructions: [],
            symbols: %{},
            data_start_address: 0,
            address: 0,
            bytecode: []

  @address_size 2
  @opperand_size 1
  @immediate_value_size 4
  @opcode_size 1
  @start_address 0

  @op_nop 0x0
  @op_halt 0x1
  @op_ld 0x2
  @op_inc 0x3
  @op_dec 0x4

  @op_cls 0x5
  @op_set_a 0x6
  @op_set_r 0x7
  @op_flush 0x8
  @op_fill  0x9

  @op_jp 0xA
  @op_jpnz 0xB

  def assemble(tokens)

  def assemble(tokens) do
    %__MODULE__{}
    |> parse_tokens(tokens)
    |> assemble_bytecode()
  end

  def parse_tokens(assembler, tokens) do
    %{assembler | instructions: [], address: @start_address}
    |> split_instructions(tokens, [])
    |> evaluate_addresses()
  end

  def split_instructions(assembler, [{:mnemonic, _line, m} | rest], buffer) do
    %{assembler | address: assembler.address + @opcode_size}
    |> add_instruction(buffer)
    |> split_instructions(rest, [m])
  end

  def split_instructions(assembler, [{:opperand, _line, o} | rest], buffer) do
    %{assembler | address: assembler.address + @opperand_size}
    |> split_instructions(rest, [o | buffer])
  end

  def split_instructions(assembler, [{:address, _line, a} | rest], buffer) do
    %{assembler | address: assembler.address + @address_size}
    |> split_instructions(rest, [{:address, a} | buffer])
  end

  def split_instructions(assembler, [{:symbol, _line, a} | rest], buffer) do
    %{assembler | symbols: Map.put(assembler.symbols, a, assembler.address)}
    |> add_instruction(buffer)
    |> split_instructions(rest, [])
  end

  def split_instructions(assembler, [{:value, _line, v} | rest], buffer) when v <= 0xFFFFFFFF do
    # updating this function to take the entire assembler
    %{assembler | address: assembler.address + @immediate_value_size}
    |> split_instructions(rest, [{:value, v} | buffer])
  end

  def split_instructions(assembler, [], buffer) do
    assembler
    |> add_instruction(buffer)
    |> Map.update(:instructions, [], &Enum.reverse/1)
  end

  def add_instruction(assembler, []) do
    assembler
  end

  def add_instruction(%{instructions: instructions} = assembler, buffer) do
    %{assembler | instructions: [Enum.reverse(buffer) | instructions]}
  end

  def evaluate_addresses(assembler) do
    instructions =
      Enum.map(assembler.instructions, &evaluate_instruction_address(&1, assembler.symbols, []))

    %{assembler | instructions: instructions, data_start_address: assembler.address}
  end

  def evaluate_instruction_address([], _symbols, buffer), do: Enum.reverse(buffer)

  def evaluate_instruction_address([{:address, a} | rest], symbols, buffer) do
    evaluate_instruction_address(rest, symbols, [{:address, symbols[a]} | buffer])
  end

  def evaluate_instruction_address([data | rest], symbols, buffer) do
    evaluate_instruction_address(rest, symbols, [data | buffer])
  end

  def assemble_bytecode(assembler) do
    assemble_bytecode(assembler, assembler.instructions)
  end

  def assemble_bytecode(assembler, [[:ld, register, {:value, im}] | rest]) do
    assembler
    |> add_bytecode(<<@op_ld, assemble_register(register), im::unsigned-32>>)
    |> assemble_bytecode(rest)
  end

  def assemble_bytecode(assembler, [[:cls, register]| rest]) do
    assembler
    |> add_bytecode(<<@op_cls, assemble_register(register)>>)
    |> assemble_bytecode(rest)
  end

  def assemble_bytecode(assembler, [[:fill, r1, r2] | rest]) do
    assembler
    |> add_bytecode(<<@op_fill, assemble_register(r1), assemble_register(r2)>>)
    |> assemble_bytecode(rest)
  end

  def assemble_bytecode(assembler, [[:nop] | rest]) do
    assembler
    |> add_bytecode(<<@op_nop>>)
    |> assemble_bytecode(rest)
  end

  def assemble_bytecode(assembler, [[:flush] | rest]) do
    assembler
    |> add_bytecode(<<@op_flush>>)
    |> assemble_bytecode(rest)
  end

  def assemble_bytecode(assembler, [[:halt] | rest]) do
    assembler
    |> add_bytecode(<<@op_halt>>)
    |> assemble_bytecode(rest)
  end

  def assemble_bytecode(assembler, [[:inc, register] | rest]) do
    assembler
    |> add_bytecode(<<@op_inc, assemble_register(register)>>)
    |> assemble_bytecode(rest)
  end

  def assemble_bytecode(assembler, [[:dec, register] | rest]) do
    assembler
    |> add_bytecode(<<@op_dec, assemble_register(register)>>)
    |> assemble_bytecode(rest)
  end

  def assemble_bytecode(assembler, [[:set, r1, r2, r3] | rest]) do
    assembler
    |> add_bytecode(<<@op_set_r, assemble_register(r1), assemble_register(r2),assemble_register(r3)>>)
    |> assemble_bytecode(rest)
  end

  def assemble_bytecode(assembler, [[:jnz, register, {:address, address}] | rest]) do
    assembler
    |> add_bytecode(<<@op_jpnz, assemble_register(register), address::unsigned-16>>)
    |> assemble_bytecode(rest)
  end

  def assemble_bytecode(assembler, []) do
    %{assembler | bytecode: Enum.reverse(assembler.bytecode)}
  end

  def add_bytecode(assembler, bytecode) do
    %{assembler | bytecode: [bytecode | assembler.bytecode]}
  end

  def assemble_register(:X), do: 0x0
  def assemble_register(:Y), do: 0x1
  def assemble_register(:ZA), do: 0x2
  def assemble_register(:ZB), do: 0x3
  def assemble_register(:LP), do: 0x4
  def assemble_register(:TM), do: 0x5
  def assemble_register(:CHN), do: 0x6

end
