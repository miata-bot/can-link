defmodule Generator do
  def gen do
    names = File.read!("pins.txt")
    |> String.split("\n")
    |> Enum.map(&String.split(&1, " "))
    |> Enum.with_index(1)
    |> Enum.map(fn
      {[num, name, "I" | _], _} -> [num, name, "input"]
      {[num, name, "ETH_I" | _], _} -> [num, name, "input"]
      {[num, name, "O" | _], _} -> [num, name, "output"]
      {[num, name, "ETH_O" | _], _} -> [num, name, "output"]
      {[num, name, "I/O" | _], _} -> [num, name, "bidirectional"]
      {[num, name, "USB_I/O" | _], _} -> [num, name, "bidirectional"]
      {[num, "GND", "-" | _], _} -> [num, "GND", "power_in"]
      {[num, name, "PWR_I" | _], _} -> [num, name, "power_in"]
      {[num, name | _], _} -> [num, name, "unspecified"]
    end)
    pins = for [num, name, type] <- names do
      num = String.to_integer(num)
      """
      (pin #{type} line (at 0 -#{num*2} 0) (length 2.54)
        (name "#{name}" (effects (font (size 1.27 1.27))))
        (number "#{num}" (effects (font (size 1.27 1.27))))
      )
      """
    end
    """
    (kicad_symbol_lib (version 20211014) (generator kicad_symbol_editor)
    (symbol "phyCORE-i.MX6UL" (in_bom yes) (on_board yes)
      (property "Reference" "U" (id 0) (at 0 0 0)
        (effects (font (size 1.27 1.27)))
      )
      (property "Value" "phyCORE-i.MX6UL" (id 1) (at 0 0 0)
        (effects (font (size 1.27 1.27)))
      )
      (property "Footprint" "" (id 2) (at 0 0 0)
        (effects (font (size 1.27 1.27)) hide)
      )
      (property "Datasheet" "" (id 3) (at 0 0 0)
        (effects (font (size 1.27 1.27)) hide)
      )
      (symbol "phyCORE-i.MX6UL_0_0"
      #{pins}
    )
    )
    )
    """
  end
end
