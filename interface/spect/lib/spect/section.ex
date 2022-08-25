defmodule Spect.Section do
  use Ecto.Schema
  import Ecto.Changeset

  schema "sections" do
    field :start, :integer
    field :end, :integer
  end

  def changeset(section, attrs) do
    section
    |> cast(attrs, [:start, :end])
    |> unique_constraint(:start)
    |> unique_constraint(:end)
  end
end
