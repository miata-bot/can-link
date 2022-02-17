defmodule GPSd.Class do
  @type class :: map()
  @callback decode(map()) :: {:ok, class} | {:error, term()}

  def populate_data(struct, message) do
    # lazy hack
    Enum.reduce(Map.keys(struct) -- [:__struct__], struct, fn key, struct ->
      %{struct | key => Map.get(message, to_string(key))}
    end)
  end

  def populate_array(struct, field, class) do
    Map.update!(struct, field, fn data ->
      Enum.map(data || [], fn item ->
        {:ok, updated} = class.decode(item)
        updated
      end)
    end)
  end

  # we do a little metaprogramming.
  # defines a function that calls GPSd.{class}.decode(message)
  # for every known class
  classes = ~w(ERROR VERSION TPV SKY)

  for class <- classes do
    def decode_message(%{"class" => unquote(class)} = message) do
      :"Elixir.GPSd.Class.#{unquote(class)}".decode(message)
    end
  end

  def decode_message(%{"class" => class}) do
    {:error, {:unknown_class, class}}
  end

  def decode_message(unknown) do
    {:error, {:unknown_message, unknown}}
  end

  def generate(class) do
    file = Path.join(["lib", "gpsd", "classes", String.downcase(class) <> ".ex"])

    File.write!(file, """
    defmodule GPSd.Class.#{class} do
      alias GPSd.Class.#{class}

      @behaviour GPSd.Class
      import GPSd.Class, warn: false
      defstruct []

      def decode(%{"class" => "#{class}"} = message) do
        #{String.downcase(class)} = populate_data(%#{class}{}, message)
        {:ok, #{String.downcase(class)}}
      end
    end
    """)
  end
end
