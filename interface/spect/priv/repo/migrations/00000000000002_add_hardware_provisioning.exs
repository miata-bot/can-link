defmodule Spect.Repo.Migrations.AddHardwareProvisioning do
  use Ecto.Migration

  def change do
    create table(:hardware_channels) do
      add :strip_enable, :boolean, default: true, null: false
      add :strip_length, :integer, default: 150, null: false
      add :rgb_enable, :boolean, default: true, null: false
      add :friendly_name, :string, default: "output", null: false
    end

    execute "INSERT INTO hardware_channels DEFAULT VALUES;",
            "DELETE FROM hardware_channels WHERE id=1;"

    execute "INSERT INTO hardware_channels DEFAULT VALUES;",
            "DELETE FROM hardware_channels WHERE id=2;"

    execute """
            CREATE TRIGGER hardware_channel_no_insert
            BEFORE INSERT ON config
            WHEN (SELECT COUNT(*) FROM hardware_channels) >= 2
            BEGIN
                SELECT RAISE(FAIL, 'Only 2 hardware channel tables may exist');
            END;
            """,
            """
            DROP TRIGGER 'hardware_channel_no_insert';
            """

    create table(:hardware_triggers) do
      add :enable, :boolean, default: false, null: false
      add :friendly_name, :string, default: "input", null: false
    end

    execute "INSERT INTO hardware_triggers DEFAULT VALUES;",
            "DELETE FROM hardware_triggers WHERE id=1;"

    execute "INSERT INTO hardware_triggers DEFAULT VALUES;",
            "DELETE FROM hardware_triggers WHERE id=2;"

    execute "INSERT INTO hardware_triggers DEFAULT VALUES;",
            "DELETE FROM hardware_triggers WHERE id=3;"

    execute "INSERT INTO hardware_triggers DEFAULT VALUES;",
            "DELETE FROM hardware_triggers WHERE id=4;"

    execute """
            CREATE TRIGGER hardware_trigger_no_insert
            BEFORE INSERT ON config
            WHEN (SELECT COUNT(*) FROM hardware_triggers) >= 4
            BEGIN
                SELECT RAISE(FAIL, 'Only 4 hardware trigger tables may exist');
            END;
            """,
            """
            DROP TRIGGER 'hardware_trigger_no_insert';
            """

    create table(:hardware) do
      add :channel1, references(:hardware_channels), null: false, default: 1
      add :channel2, references(:hardware_channels), null: false, default: 2
      add :trigger1, references(:hardware_triggers), null: false, default: 1
      add :trigger2, references(:hardware_triggers), null: false, default: 2
      add :trigger3, references(:hardware_triggers), null: false, default: 3
      add :trigger4, references(:hardware_triggers), null: false, default: 4
    end

    execute "INSERT INTO hardware DEFAULT VALUES;", "DELETE FROM hardware;"

    execute """
            CREATE TRIGGER hardware_no_insert
            BEFORE INSERT ON config
            WHEN (SELECT COUNT(*) FROM hardware) >= 1
            BEGIN
                SELECT RAISE(FAIL, 'Only One hardware configuration table may exist');
            END;
            """,
            """
            DROP TRIGGER 'hardware_no_insert';
            """
  end
end
