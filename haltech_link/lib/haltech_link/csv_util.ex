defmodule CSVUtil do
  def to_bool(str) do
    case String.trim(str) do
      "-" -> false
      "Y" -> true
    end
  end

  def to_units(unit) do
    case String.trim(unit) do
      "s" ->
        :seconds

      "ms" ->
        :milliseconds

      "RPM" ->
        :rpm

      "deg BTDC" ->
        :degrees_btdc

      "-" ->
        nil

      "AFR" ->
        :afr

      "kPa" ->
        :kpa

      "deg F" ->
        :deg_f

      "%" ->
        :percent

      "V" ->
        :volts

      "step duty%" ->
        :duty_percent

      "deg" ->
        :degrees

      "%/s" ->
        :percent_per_second

      "kPa/s" ->
        :kpa_per_second

      "RPM/s" ->
        :rpm_per_second

      "g/s" ->
        :grams_per_second

      "us" ->
        :microseconds

      "ms-2" ->
        :milliseconds_1

      "ms-1" ->
        :milliseconds_2

      "BTDC" ->
        :degrees_btdc

      "cc/min" ->
        :cc_per_minute

      "l/km" ->
        :_per_km

      "A" ->
        :amps

      "min" ->
        :minutes

      "mmin" ->
        :milliminutes

      "km" ->
        :kilometers

      "m" ->
        :meters

      unknown ->
        raise "unknown unit #{unknown}"
    end
  end

  def to_integer(str) do
    case String.split(String.trim(str), " ") do
      ["-"] ->
        nil

      [str] ->
        {int, ""} = Integer.parse(str)
        int

      [str1, str2] ->
        {int1, ""} = Integer.parse(str1)
        {int2, ""} = Integer.parse(str2)
        {int1, int2}
    end
  end

  # def parse_frame!(unquote(group), error) do
  #   raise "Could not parse frame. This is probably a parser-generator bug: #{inspect(error)}"
  # end

  defmacro compile_parser(group, data) do
    quote bind_quoted: [data: data, group: group] do
      def parse_frame!(unquote(group), unquote(compile_pattern(data))) do
        unquote(compile_calculations(data))
        binding()
      end
    end
  end

  def compile_pattern(data, acc \\ [])

  def compile_pattern([{_, _, size, true, name, _function, _units, _multiply, _divide, _add, _ms2?} | rest], acc) do
    var = into_var(name)

    quoted =
      quote do
        unquote(var) :: signed - size(unquote(size * 8))
      end

    compile_pattern(rest, [quoted | acc])
  end

  def compile_pattern([{_, _, size, false, name, _function, _units, _multiply, _divide, _add, _ms2?} | rest], acc) do
    var = into_var(name)

    quoted =
      quote do
        unquote(var) :: unsigned - size(unquote(size * 8))
      end

    compile_pattern(rest, [quoted | acc])
  end

  def compile_pattern([], acc) do
    quote do
      <<unquote_splicing(Enum.reverse(acc))>>
    end
  end

  def compile_calculations(data, acc \\ [])

  def compile_calculations([{_, _, _size, _signed?, name, _function, _units, multiply, divide, add, _ms2?} | rest], acc) do
    case into_var(name) do
      {:_, _, _} ->
        compile_calculations(rest, acc)

      var ->
        quoted =
          quote do
            unquote(var) = unquote(var) * unquote(multiply) / unquote(divide) + unquote(add)
          end

        compile_calculations(rest, [quoted | acc])
    end
  end

  def compile_calculations([], acc) do
    quote do
      (unquote_splicing(acc))
    end
  end

  def into_var("-"), do: {:_, [], nil}
  def into_var(name), do: {String.to_atom(String.downcase(name)), [], nil}
end
