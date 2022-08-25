defmodule Spect.Repo.Migrations.AddSectionsTable do
  use Ecto.Migration

  def change do
    # there will always be 8 sections total.
    # normal config: two each for each bumper,
    #                one each for either side,
    #                one for each RGB.
    # section constraings:
    #   * may not overlap
    #   * may not be longer than the channel length
    create table(:sections) do
      add :start, :integer, null: false
      add :end, :integer, null: false
    end

    create unique_index(:sections, :start)
    create unique_index(:sections, :end)

    execute """
            CREATE TRIGGER sections_overlap_insert
            AFTER INSERT ON sections
            WHEN (
              SELECT COUNT(*) FROM sections section1
              WHERE NEW.start >  section1.start
                AND NEW.start <= section1.end
            ) > 0
            BEGIN
              SELECT RAISE(ROLLBACK, 'sections may not overlap');
            END;
            """,
            "DROP TRIGGER sections_overlap_insert;"

    execute """
            CREATE TRIGGER sections_overlap_update
            AFTER UPDATE ON sections
            WHEN (
              SELECT COUNT(*) FROM sections section1
              WHERE NEW.start >  section1.start
                AND NEW.start <= section1.end
            ) > 0
            BEGIN
              SELECT RAISE(ROLLBACK, 'sections may not overlap');
            END;
            """,
            "DROP TRIGGER sections_overlap_update;"

    execute """
            CREATE TRIGGER sections_max
            BEFORE INSERT ON sections
            WHEN (SELECT COUNT(*) FROM sections) >= 8
            BEGIN
              SELECT RAISE(FAIL, 'only 8 sections');
            END;
            """,
            "DROP TRIGGER sections_max;"

    execute "INSERT INTO sections (id,start,end) VALUES(1,   0, 30);",
            "DELETE FROM slot_sections WHERE id=1"

    execute "INSERT INTO sections (id,start,end) VALUES(2,  31,110);",
            "DELETE FROM slot_sections WHERE id=2"

    execute "INSERT INTO sections (id,start,end) VALUES(3, 111,150);",
            "DELETE FROM slot_sections WHERE id=3"

    execute "INSERT INTO sections (id,start,end) VALUES(4, 151,180);",
            "DELETE FROM slot_sections WHERE id=4"

    execute "INSERT INTO sections (id,start,end) VALUES(5, 181,260);",
            "DELETE FROM slot_sections WHERE id=5"

    execute "INSERT INTO sections (id,start,end) VALUES(6, 261,311);",
            "DELETE FROM slot_sections WHERE id=6"

    execute "INSERT INTO sections (id,start,end) VALUES(7, 312,312);",
            "DELETE FROM slot_sections WHERE id=7"

    execute "INSERT INTO sections (id,start,end) VALUES(8, 313,313);",
            "DELETE FROM slot_sections WHERE id=8"

    # TODO: could delete the insert trigger after this since
    # it's impossible to insert more
  end
end
