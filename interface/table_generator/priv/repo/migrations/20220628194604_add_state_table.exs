defmodule TableGenerator.Repo.Migrations.AddStateTable do
  use Ecto.Migration

  def change do
    create table(:state, primary_key: false) do
      add :id, :tinyint, null: false, default: 0, primary_key: true

      # should probably add a constraint for mode
      add :mode, :tinyint, null: false, default: 0

      add :rgb_channel_1_color, :integer, null: false, default: 0
      add :rgb_channel_1_brightness, :integer, null: false, default: 0

      add :rgb_channel_2_color, :integer, null: false, default: 0
      add :rgb_channel_2_brightness, :integer, null: false, default: 0
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
    INSERT INTO state(id) VALUES(0);
    """,
    """
    DELETE FROM state;
    """
  end
end
