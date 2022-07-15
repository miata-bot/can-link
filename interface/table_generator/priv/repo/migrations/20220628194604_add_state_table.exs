defmodule TableGenerator.Repo.Migrations.AddStateTable do
  use Ecto.Migration

  def change do
    create table(:state) do
      add :config_id, references(:config), null: false

      # should probably add a constraint for mode
      add :mode, :tinyint, null: false, default: 0

      add :mode_solid_channel0, :integer, null: false, default: 0xff000000
      add :mode_solid_channel1, :integer, null: false, default: 0

      add :mode_rainbow_length, :integer, null: false, default: 300
      add :mode_rainbow_delay_time, :integer, null: false, default: 500

      add :mode_pulse_length, :integer, null: false, default: 300
      add :mode_pulse_pulsewidth, :integer, null: false, default: 1000
    end

    execute """
    CREATE TRIGGER state_no_insert
    BEFORE INSERT ON state
    WHEN (SELECT COUNT(*) FROM state) >= 1
    BEGIN
        SELECT RAISE(FAIL, 'Only one state table may exist');
    END;
    """,
    """
    DROP TRIGGER 'state_no_insert';
    """

    execute """
    INSERT INTO state(id, config_id) VALUES(0, 0);
    """,
    """
    DELETE FROM state;
    """
  end
end
