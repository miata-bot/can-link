defmodule JlcHelper.Repo.Migrations.CreateComponens do
  use Ecto.Migration

  def change do
    create table(:componens) do
      add :part, :string
      add :category1, :string
      add :category2, :string
      add :mpn, :string
      add :package, :string
      add :joins, :integer
      add :manufacturer, :string
      add :library_type, :string
      add :description, :string
      add :datasheet, :string
      add :price, :float
      add :stock, :integer

      timestamps()
    end
  end
end
