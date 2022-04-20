Mix.install([:nimble_csv])
defmodule Generator do
  def generate do
    data = NimbleCSV.RFC4180.parse_string(File.read!("/Users/connor/Downloads/tabula-IMX6ULZCEC.csv"))
    units = Enum.reduce(data, %{a: [], b: [], c: [], d: []}, fn
      [name, pins, "—" | _], state ->
        pins = String.split(pins, ", ")
        |> Enum.map(fn
          pin ->
            """
            (pin input line (at <%= x %> <%= y %> 0) (length 2.54)
              (name "#{name}" (effects (font (size 1.27 1.27))))
              (number "#{pin}" (effects (font (size 1.27 1.27))))
            )
            """
        end)
        %{state | a: state.a ++ pins}
      ["BOOT_" <> _ = name, pin | _], state ->
        pin = """
        (pin input line (at <%= x %> <%= y %> 0) (length 2.54)
          (name "#{name}" (effects (font (size 1.27 1.27))))
          (number "#{pin}" (effects (font (size 1.27 1.27))))
        )
        """
        %{state | a: [pin | state.a]}
      ["CCM_" <> _ = name, pin | _], state ->
        pin = """
        (pin input line (at <%= x %> <%= y %> 0) (length 2.54)
          (name "#{name}" (effects (font (size 1.27 1.27))))
          (number "#{pin}" (effects (font (size 1.27 1.27))))
        )
        """
        %{state | a: [pin | state.a]}
      ["DRAM" <> _ = name, pin, _, _, _, _, "Output" | _], state ->
        pin = """
        (pin output line (at <%= x %> <%= y %> 0) (length 2.54)
          (name "#{name}" (effects (font (size 1.27 1.27))))
          (number "#{pin}" (effects (font (size 1.27 1.27))))
        )
        """
        %{state | b: [pin | state.b]}
    ["DRAM" <> _ = name, pin, _, _, _, _, "Input" | _], state ->
        pin = """
        (pin input line (at <%= x %> <%= y %> 0) (length 2.54)
          (name "#{name}" (effects (font (size 1.27 1.27))))
          (number "#{pin}" (effects (font (size 1.27 1.27))))
        )
        """
        %{state | b: [pin | state.b]}
      ["GPIO" <> _ = name, pin | _], state ->
        pin = """
        (pin bidirectional line (at <%= x %> <%= y %> 0) (length 2.54)
          (name "#{name}" (effects (font (size 1.27 1.27))))
          (number "#{pin}" (effects (font (size 1.27 1.27))))
        )
        """
        %{state | c: [pin | state.c]}
      ["I2C" <> _ = name, pin | _], state ->
        pin = """
        (pin bidirectional line (at <%= x %> <%= y %> 0) (length 2.54)
          (name "#{name}" (effects (font (size 1.27 1.27))))
          (number "#{pin}" (effects (font (size 1.27 1.27))))
        )
        """
        %{state | c: [pin | state.c]}
      ["JTAG_" <> _ = name, pin | _], state ->
        pin = """
        (pin bidirectional line (at <%= x %> <%= y %> 0) (length 2.54)
          (name "#{name}" (effects (font (size 1.27 1.27))))
          (number "#{pin}" (effects (font (size 1.27 1.27))))
        )
        """
        %{state | d: [pin | state.d]}
      ["KPP_" <> _ = name, pin | _], state ->
        pin = """
        (pin bidirectional line (at <%= x %> <%= y %> 0) (length 2.54)
          (name "#{name}" (effects (font (size 1.27 1.27))))
          (number "#{pin}" (effects (font (size 1.27 1.27))))
        )
        """
        %{state | d: [pin | state.d]}
      ["NAND_" <> _ = name, pin | _], state ->
        pin = """
        (pin bidirectional line (at <%= x %> <%= y %> 0) (length 2.54)
          (name "#{name}" (effects (font (size 1.27 1.27))))
          (number "#{pin}" (effects (font (size 1.27 1.27))))
        )
        """
        %{state | d: [pin | state.d]}
      ["RTC_" <> _ = name, pin | _], state ->
        pin = """
        (pin bidirectional line (at <%= x %> <%= y %> 0) (length 2.54)
          (name "#{name}" (effects (font (size 1.27 1.27))))
          (number "#{pin}" (effects (font (size 1.27 1.27))))
        )
        """
        %{state | d: [pin | state.d]}
      ["SD" <> _ = name, pin | _], state ->
        pin = """
        (pin bidirectional line (at <%= x %> <%= y %> 0) (length 2.54)
          (name "#{name}" (effects (font (size 1.27 1.27))))
          (number "#{pin}" (effects (font (size 1.27 1.27))))
        )
        """
        %{state | d: [pin | state.d]}
      ["SNVS_" <> _ = name, pin | _], state ->
        pin = """
        (pin unspecified line (at <%= x %> <%= y %> 0) (length 2.54)
          (name "#{name}" (effects (font (size 1.27 1.27))))
          (number "#{pin}" (effects (font (size 1.27 1.27))))
        )
        """
        %{state | a: [pin | state.a]}
      ["TEST_" <> _ = name, pin | _], state ->
        pin = """
        (pin unspecified line (at <%= x %> <%= y %> 0) (length 2.54)
          (name "#{name}" (effects (font (size 1.27 1.27))))
          (number "#{pin}" (effects (font (size 1.27 1.27))))
        )
        """
        %{state | a: [pin | state.a]}
      ["ONOFF" <> _ = name, pin | _], state ->
          pin = """
          (pin unspecified line (at <%= x %> <%= y %> 0) (length 2.54)
            (name "#{name}" (effects (font (size 1.27 1.27))))
            (number "#{pin}" (effects (font (size 1.27 1.27))))
          )
          """
          %{state | a: [pin | state.a]}
      ["POR" <> _ = name, pin | _], state ->
            pin = """
            (pin unspecified line (at <%= x %> <%= y %> 0) (length 2.54)
              (name "#{name}" (effects (font (size 1.27 1.27))))
              (number "#{pin}" (effects (font (size 1.27 1.27))))
            )
            """
            %{state | a: [pin | state.a]}
      ["UART" <> _ = name, pin | _], state ->
        pin = """
        (pin bidirectional line (at <%= x %> <%= y %> 0) (length 2.54)
          (name "#{name}" (effects (font (size 1.27 1.27))))
          (number "#{pin}" (effects (font (size 1.27 1.27))))
        )
        """
        %{state | c: [pin | state.c]}
      ["USB_" <> _ = name, pin | _], state ->
        pin = """
        (pin bidirectional line (at <%= x %> <%= y %> 0) (length 2.54)
          (name "#{name}" (effects (font (size 1.27 1.27))))
          (number "#{pin}" (effects (font (size 1.27 1.27))))
        )
        """
        %{state | c: [pin | state.c]}
      ["XTAL" <> _ = name, pin | _], state ->
        pin = """
        (pin bidirectional line (at <%= x %> <%= y %> 0) (length 2.54)
          (name "#{name}" (effects (font (size 1.27 1.27))))
          (number "#{pin}" (effects (font (size 1.27 1.27))))
        )
        """
        %{state | a: [pin | state.a]}
    end)
    |> Enum.map(fn
      {:a, pins} ->
        pins = Enum.with_index(pins, 1)
        |> Enum.map(fn {pin, index} ->
        EEx.eval_string(pin, [x: 0, y: 2.54 * index])
        end)
        """
        (symbol "MCIMX6Z0DVM09AB_1_1"
        #{Enum.join(pins, "\n")}
        )
        """
      {:b, pins} ->
        pins = Enum.with_index(pins, 1)
        |> Enum.map(fn {pin, index} ->
        EEx.eval_string(pin, [x: 0, y: 2.54 * index])
        end)
        """
        (symbol "MCIMX6Z0DVM09AB_2_1"
        #{Enum.join(pins, "\n")}
        )
        """
      {:c, pins} ->
        pins = Enum.with_index(pins, 1)
        |> Enum.map(fn {pin, index} ->
        EEx.eval_string(pin, [x: 0, y: 2.54 * index])
        end)
        """
        (symbol "MCIMX6Z0DVM09AB_3_1"
        #{Enum.join(pins, "\n")}
        )
        """
      {:d, pins} ->
        pins = Enum.with_index(pins, 1)
        |> Enum.map(fn {pin, index} ->
        EEx.eval_string(pin, [x: 0, y: 2.54 * index])
        end)
        """
        (symbol "MCIMX6Z0DVM09AB_4_1"
        #{Enum.join(pins, "\n")}
        )
        """
    end)

    template = """
    (kicad_symbol_lib (version 20211014) (generator kicad_symbol_editor)
      (symbol "MCIMX6Z0DVM09AB" (in_bom yes) (on_board yes)
        (property "Reference" "U" (id 0) (at 0 10.16 0)
          (effects (font (size 1.27 1.27)))
        )
        (property "Value" "MCIMX6Z0DVM09AB" (id 1) (at 0 7.62 0)
          (effects (font (size 1.27 1.27)))
        )
        (property "Footprint" "" (id 2) (at 0 0 0)
          (effects (font (size 1.27 1.27)) hide)
        )
        (property "Datasheet" "" (id 3) (at 0 0 0)
          (effects (font (size 1.27 1.27)) hide)
        )
        (property "ki_locked" "" (id 4) (at 0 0 0)
          (effects (font (size 1.27 1.27)))
        )
        #{Enum.join(units, "\n")}
      )
    )
    """
    File.write!("/Users/connor/workspace/connorrigby/haltech-link/imx6uz-dev/library/CPU_NXP_IMX6ULLZ-generated.kicad_sym", template)
  end
end
