defmodule JlcHelper.JLCFixtures do
  @moduledoc """
  This module defines test helpers for creating
  entities via the `JlcHelper.JLC` context.
  """

  @doc """
  Generate a component.
  """
  def component_fixture(attrs \\ %{}) do
    {:ok, component} =
      attrs
      |> Enum.into(%{
        category1: "some category1",
        category2: "some category2",
        datasheet: "some datasheet",
        description: "some description",
        joins: 42,
        library_type: "some library_type",
        manufacturer: "some manufacturer",
        mpn: "some mpn",
        package: "some package",
        part: "some part",
        price: 120.5,
        stock: 42
      })
      |> JlcHelper.JLC.create_component()

    component
  end
end
