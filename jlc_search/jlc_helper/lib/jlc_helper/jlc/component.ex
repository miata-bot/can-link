defmodule JlcHelper.JLC.Component do
  use Ecto.Schema
  import Ecto.Changeset
  @primary_key false
  schema "parts" do
    field :"First Category", :string
    field :"Second Category", :string
    field :Datasheet, :string
    field :Description, :string
    field :"Solder Joint", :string
    field :"Library Type", :string
    field :"Manufacturer", :string
    field :"MFR.Part", :string
    field :Package, :string
    field :"LCSC Part", :string
    field :Price, :string
    field :Stock, :string

  end

  @doc false
  def changeset(component, attrs) do
    component
    |> cast(attrs, [:"LCSC Part", :"First Category", :"Second Category", :"MFR.Part", :Package, :"Solder Joint", :"Manufacturer", :"Library Type", :Description, :Datasheet, :Price, :Stock])
    |> validate_required([:"LCSC Part", :"First Category", :"Second Category", :"MFR.Part", :Package, :"Solder Joint", :"Manufacturer", :"Library Type", :Description, :Datasheet, :Price, :Stock])
  end

  defimpl Elasticsearch.Document do
    def id(part), do: Map.get(part, :"LCSC Part")
    def routing(_), do: false
    def encode(part) do
      part
      |> Map.take([:"LCSC Part", :"First Category", :"Second Category", :"MFR.Part", :Package, :"Solder Joint", :"Manufacturer", :"Library Type", :Description, :Datasheet, :Price, :Stock])
    end
  end

  import Ecto.Query

  def build_index do
    {:ok, chunks} = JlcHelper.Repo.transaction(fn ->
      JlcHelper.Repo.stream(from part in JlcHelper.JLC.Component)
      |> Stream.map(fn part ->
        Elasticsearch.Document.encode(part)
      end)
      # |> Stream.chunk_every(1000)
      # |> Flow.from_enumerable()
      |> Enum.to_list()
    end, timeout: :infinity)
    chunks
    |> Flow.from_enumerable()
    |> Flow.map_batch(fn batch ->
      Meilisearch.Documents.add_or_replace("parts", batch)
      batch
    end)
    |> Enum.to_list()
    # |> List.first()

    # JlcHelper.Repo.transaction(fn ->
    #   JlcHelper.Repo.stream(from part in JlcHelper.JLC.Component, limit: 1000)
    #   |> Stream.chunk_every(500)

    #   |> Flow.from_enumerable()
    #   |> Flow.map_batch(fn parts ->
    #     parts
    #   end)
    #   |> Enum.to_list()
    # end, timeout: :infinity)
    # Meilisearch.Indexes.delete("parts")

    # |> Flow.map(fn part ->
    #   Elasticsearch.Document.encode(part)
    # end)
    # |> Flow.map(fn parts ->
    #   Meilisearch.Documents.add_or_replace("parts", )
    # end)

  end
end
