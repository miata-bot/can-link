defmodule Spect.Repo.Migrations.AddEffectSlotsTable do
  use Ecto.Migration

  def change do
    # there will always be 8 effect_slots total.
    # not all slots must be active.
    # active/inactive is controlled by the mode bitfield
    create table(:effect_slots) do
      add :mode, :integer, default: 0
      add :script_name, :string
    end

    # mode must be a 8 bit integer
    # bits 4-8 are reserved for future use
    execute """
            CREATE TRIGGER effect_mode
            AFTER UPDATE ON effect_slots
            WHEN (
              (NEW.mode > 255) OR
              (NEW.mode <   0) OR
              (NEW.mode & 248) OR
              (((NEW.mode >> 0) & 1) AND NEW.script_name IS NULL)
            ) > 0
            BEGIN
              SELECT RAISE(ROLLBACK, 'invalid slot mode');
            END;
            """,
            "DROP TRIGGER effect_mode;"

    execute "INSERT INTO effect_slots (id,mode) VALUES(1, 0);",
            "DELETE FROM effect_slots WHERE id=1"

    execute "INSERT INTO effect_slots (id,mode) VALUES(2, 0);",
            "DELETE FROM effect_slots WHERE id=2"

    execute "INSERT INTO effect_slots (id,mode) VALUES(3, 0);",
            "DELETE FROM effect_slots WHERE id=3"

    execute "INSERT INTO effect_slots (id,mode) VALUES(4, 0);",
            "DELETE FROM effect_slots WHERE id=4"

    execute "INSERT INTO effect_slots (id,mode) VALUES(5, 0);",
            "DELETE FROM effect_slots WHERE id=5"

    execute "INSERT INTO effect_slots (id,mode) VALUES(6, 0);",
            "DELETE FROM effect_slots WHERE id=6"

    execute "INSERT INTO effect_slots (id,mode) VALUES(7, 0);",
            "DELETE FROM effect_slots WHERE id=7"

    execute "INSERT INTO effect_slots (id,mode) VALUES(8, 0);",
            "DELETE FROM effect_slots WHERE id=8"

    # decodes the mode bitfield
    execute """
            CREATE VIEW effect_mode (id, enable, radio, can) AS
            SELECT
              id,
              ((mode >> 0) & 1),
              ((mode >> 1) & 1),
              ((mode >> 2) & 1)
            FROM effect_slots;
            """,
            "DROP VIEW effect_mode;"

    # slot section is a join between section and effect slot
    create table(:slot_sections) do
      add :section_id, references(:sections)
      add :slot_id, references(:effect_slots)
    end

    # there must not be any section duplicates per effect or trigger.
    create unique_index(:slot_sections, :section_id)

    # slot sections may point to more slot sections.
    # this allows slot sections to be chained together
    alter table(:slot_sections) do
      add :slot_section_id, references(:slot_sections)
    end

    # may not point to a slot_section that is already in use
    create unique_index(:slot_sections, :slot_section_id)

    # there may only be 8 slot_sections total
    # this is enough to allow one slot to take up all
    # 8 sections, or any combination between.
    execute """
            CREATE TRIGGER slot_sections_max
            BEFORE INSERT ON slot_sections
            WHEN (SELECT COUNT(*) FROM slot_sections) >= 8
            BEGIN
              SELECT RAISE(FAIL, 'Only 8 slot_sections may exist');
            END;
            """,
            "DROP TRIGGER slot_sections_max;"
  end
end
