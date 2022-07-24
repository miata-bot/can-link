defmodule TableGenerator.Repo.Migrations.AddHardwareProvisioning do
  use Ecto.Migration

  def change do
    alter table(:config) do
      # rgb channels
      add :rgb_channel_1_enable, :boolean, default: true, null: false
      add :rgb_channel_2_enable, :boolean, default: false, null: false

      # strip channels
      add :strip_channel_1_enable, :boolean, default: true, null: false
      add :strip_channel_2_enable, :boolean, default: false, null: false

      # strip length
      add :strip_channel_1_length, :integer, default: 150, null: false
      add :strip_channel_2_length, :integer, default: 150, null: false

      # digital input
      add :digital_input_1_enable, :boolean, default: false, null: false
      add :digital_input_2_enable, :boolean, default: false, null: false
      add :digital_input_3_enable, :boolean, default: false, null: false
      add :digital_input_4_enable, :boolean, default: false, null: false
    end

    execute """
    UPDATE config SET version = version + 1;
    """, """
    UPDATE config SET version = version - 1;
    """
  end
end
