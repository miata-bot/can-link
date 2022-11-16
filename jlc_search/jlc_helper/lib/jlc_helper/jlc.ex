defmodule JlcHelper.JLC do
  @moduledoc """
  The JLC context.
  """

  import Ecto.Query, warn: false
  alias JlcHelper.Repo

  alias JlcHelper.JLC.Component

  NimbleCSV.define(JLCCSV, [])

  def import_csv(path) do
    path
    |> File.stream!()
    |> Stream.map(fn line -> Tds.Encoding.decode(line, "gbk") end)
    |> Enum.join("\n")
    |> JLCCSV.parse_string()
    |> Stream.map(fn
      [part, category1, category2, mpn, package, joints, manufacturer, library_type, description, datasheet, price, stock] ->
        part
    end)
  end

  @doc """
  Returns the list of componens.

  ## Examples

      iex> list_componens()
      [%Component{}, ...]

  """
  def list_componens do
    Repo.all(Component)
  end

  @doc """
  Gets a single component.

  Raises `Ecto.NoResultsError` if the Component does not exist.

  ## Examples

      iex> get_component!(123)
      %Component{}

      iex> get_component!(456)
      ** (Ecto.NoResultsError)

  """
  def get_component!(id), do: Repo.get!(Component, id)

  @doc """
  Creates a component.

  ## Examples

      iex> create_component(%{field: value})
      {:ok, %Component{}}

      iex> create_component(%{field: bad_value})
      {:error, %Ecto.Changeset{}}

  """
  def create_component(attrs \\ %{}) do
    %Component{}
    |> Component.changeset(attrs)
    |> Repo.insert()
  end

  @doc """
  Updates a component.

  ## Examples

      iex> update_component(component, %{field: new_value})
      {:ok, %Component{}}

      iex> update_component(component, %{field: bad_value})
      {:error, %Ecto.Changeset{}}

  """
  def update_component(%Component{} = component, attrs) do
    component
    |> Component.changeset(attrs)
    |> Repo.update()
  end

  @doc """
  Deletes a component.

  ## Examples

      iex> delete_component(component)
      {:ok, %Component{}}

      iex> delete_component(component)
      {:error, %Ecto.Changeset{}}

  """
  def delete_component(%Component{} = component) do
    Repo.delete(component)
  end

  @doc """
  Returns an `%Ecto.Changeset{}` for tracking component changes.

  ## Examples

      iex> change_component(component)
      %Ecto.Changeset{data: %Component{}}

  """
  def change_component(%Component{} = component, attrs \\ %{}) do
    Component.changeset(component, attrs)
  end
end
