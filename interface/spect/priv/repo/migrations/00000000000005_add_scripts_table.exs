defmodule Spect.Repo.Migrations.AddScriptsTable do
  use Ecto.Migration

  def change do
    create table(:scripts) do
      add :name, :string, null: false
      add :description, :string, null: false
      add :content, :binary, null: false
    end

    create index(:scripts, :name)

    alter table(:effect_slots) do
      add :script_id, references(:scripts)
    end
  end
end
