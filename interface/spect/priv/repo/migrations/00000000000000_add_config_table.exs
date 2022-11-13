defmodule Spect.Repo.Migrations.AddConfigTable do
  use Ecto.Migration

  def change do
    # execute """
    # PRAGMA journal_mode = 'DELETE';
    # """

    create table(:config, primary_key: false) do
      add :id, :tinyint, null: false, default: 0, primary_key: true
      add :version, :integer, default: 1
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

    execute """
            INSERT INTO config(id) VALUES(0);
            """,
            """
            DELETE FROM config;
            """
  end
end
