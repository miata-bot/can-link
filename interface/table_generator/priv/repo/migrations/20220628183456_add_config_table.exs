defmodule TableGenerator.Repo.Migrations.AddConfigTable do
  use Ecto.Migration

  def change do
    create table(:config, primary_key: false) do
      add :id, :tinyint, null: false, default: 0, primary_key: true
      add :version, :integer, default: 1
      timestamps()
    end

    execute """
    CREATE TRIGGER config_no_insert
    BEFORE INSERT ON config
    WHEN (SELECT COUNT(*) FROM config) >= 1
    BEGIN
        SELECT RAISE(FAIL, 'Only One configuration Table may exist');
    END;
    """,
    """
    DROP TRIGGER 'config_no_insert';
    """

    now = NaiveDateTime.to_iso8601(NaiveDateTime.utc_now())

    execute """
    INSERT INTO config(id, inserted_at, updated_at) VALUES(0, \'#{now}\', \'#{now}\');
    """,
    """
    DELETE FROM config;
    """
  end
end
