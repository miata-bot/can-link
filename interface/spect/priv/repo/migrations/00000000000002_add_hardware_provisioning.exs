defmodule Spect.Repo.Migrations.AddHardwareProvisioning do
  use Ecto.Migration

  def change do
    create table(:hardware_channels) do
      add :strip_enable, :boolean, default: true
      add :strip_length, :integer, default: 150
      add :rgb_enable, :boolean, default: true
    end

    execute "INSERT INTO hardware_channels DEFAULT VALUES;",
            "DELETE FROM hardware_channels WHERE id=1;"

    execute "INSERT INTO hardware_channels DEFAULT VALUES;",
            "DELETE FROM hardware_channels WHERE id=2;"

    create table(:hardware_triggers) do
      add :enable, :boolean, default: false
    end

    execute "INSERT INTO hardware_triggers DEFAULT VALUES;",
            "DELETE FROM hardware_triggers WHERE id=1;"

    execute "INSERT INTO hardware_triggers DEFAULT VALUES;",
            "DELETE FROM hardware_triggers WHERE id=2;"

    execute "INSERT INTO hardware_triggers DEFAULT VALUES;",
            "DELETE FROM hardware_triggers WHERE id=3;"

    execute "INSERT INTO hardware_triggers DEFAULT VALUES;",
            "DELETE FROM hardware_triggers WHERE id=4;"

    create table(:hardware) do
      add :channel1, references(:hardware_channels), null: false, default: 1
      add :channel2, references(:hardware_channels), null: false, default: 2

      add :trigger1, references(:hardware_triggers), null: false, default: 1
      add :trigger2, references(:hardware_triggers), null: false, default: 2
      add :trigger3, references(:hardware_triggers), null: false, default: 3
      add :trigger4, references(:hardware_triggers), null: false, default: 4
    end

    execute "INSERT INTO hardware DEFAULT VALUES;", "DELETE FROM hardware;"
  end
end
