defmodule Spect.Section do
  @moduledoc """
  Section of one entire strip
  There are 8 of these total that can be distributed
  into 8 different Effects and 4 different Triggers
  """

  use Ecto.Schema
  import Ecto.Changeset

  @type section_id() :: non_neg_integer()

  @typedoc "start/end of a section of the entire buffer"
  @type t() :: %__MODULE__{
    id: section_id(),
    start: non_neg_integer(),
    end: non_neg_integer()
  }

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
