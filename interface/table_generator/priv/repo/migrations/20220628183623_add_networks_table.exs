defmodule TableGenerator.Repo.Migrations.AddNetworksTable do
  use Ecto.Migration

  def change do
    create table(:networks, primary_key: false) do
      add :config_id, references(:config), null: false
      add :id, :tinyint, null: false, default: 100, primary_key: true
      add :key, :string
    end

    execute """
    INSERT INTO networks(id, config_id, key) VALUES(100, 0, 'testkey');
    """,
    """
    DELETE FROM networks;
    """

    create table(:nodes) do
      add :network_id, references(:networks), null: false
      add :name, :string
    end

    create unique_index(:nodes, [:network_id, :name], comment: "node names must be unique per network")
    # SQLite3 doesn't support constraints
    # create constraint(:nodes, "node_id_must_not_be_zero", check: "id > 0", comment: "Node ID must not be zero")

    # preload the intital topology
    execute """
    INSERT INTO nodes(id, network_id, name) VALUES(1, 100, 'cone');
    """
    execute """
    INSERT INTO nodes(id, network_id, name) VALUES(2, 100, '9');
    """
    execute """
    INSERT INTO nodes(id, network_id, name) VALUES(3, 100, 'jake');
    """
    execute """
    INSERT INTO nodes(id, network_id, name) VALUES(4, 100, 'dax');
    """
    execute """
    INSERT INTO nodes(id, network_id, name) VALUES(5, 100, 'kyler');
    """
    execute """
    INSERT INTO nodes(id, network_id, name) VALUES(6, 100, 'tyler');
    """
    execute """
    INSERT INTO nodes(id, network_id, name) VALUES(7, 100, 'dibs');
    """
    execute """
    INSERT INTO nodes(id, network_id, name) VALUES(8, 100, 'easyy');
    """
    # -- two empty slots for gen1
    execute """
    INSERT INTO nodes(id, network_id) VALUES(9, 100);
    """
    execute """
    INSERT INTO nodes(id, network_id) VALUES(10, 100);
    """

    create table(:network_leader) do
      add :network_id, references(:networks), null: false
      add :node_id, references(:nodes), null: false
    end

    execute """
    CREATE TRIGGER only_one_network_leader_per_network
    BEFORE INSERT ON network_leader
    WHEN (SELECT COUNT(*) FROM network_leader WHERE network_id = NEW.network_id) >= 1
    BEGIN
        SELECT RAISE(FAIL, 'Only one network_leader may exist');
    END;
    """,
    """
    DROP TRIGGER 'only_one_network_leader_per_network';
    """

    execute """
    INSERT INTO network_leader(id, network_id, node_id) VALUES(1, 100, 1);
    """,
    """
    DELETE FROM network_leader;
    """

    create table(:network_identity) do
      add :network_id, references(:networks), null: false
      add :node_id, references(:nodes), null: false
    end

    execute """
    CREATE TRIGGER only_one_network_identity_per_network
    BEFORE INSERT ON network_identity
    WHEN (SELECT COUNT(*) FROM network_identity WHERE network_id = NEW.network_id) >= 1
    BEGIN
        SELECT RAISE(FAIL, 'Only one network_identity may exist');
    END;
    """,
    """
    DROP TRIGGER 'only_one_network_identity_per_network';
    """

    # this will create an network_identity of cone
    execute """
    INSERT INTO network_identity(id, network_id, node_id) VALUES(1, 100, 1);
    """,
    """
    DELETE FROM network_identity;
    """

    # update config table and set the default network to 100
    alter table(:config) do
      add :network_id, references(:networks)
      add :network_identity_id, references(:network_identity)
      add :network_leader_id, references(:network_leader)
    end

    execute """
    UPDATE config SET network_id = 100;
    """
    execute """
    UPDATE config SET network_identity_id = 1;
    """
    execute """
    UPDATE config SET network_leader_id = 1;
    """
    execute """
    UPDATE config SET version = version + 1;
    """
  end
end
