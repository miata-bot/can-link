defmodule Spect.Repo.Migrations.AddNetworksTable do
  use Ecto.Migration

  def change do
    create table(:networks, primary_key: false) do
      add :id, :tinyint, null: false, default: 100, primary_key: true
      add :key, :string
    end

    execute "INSERT INTO networks(id) VALUES(100);", "DELETE FROM networks;"
    execute "INSERT INTO networks(id) VALUES(101);", "DELETE FROM networks;"

    create table(:nodes) do
      add :rssi, :integer
      add :last_seen, :integer
    end

    execute """
            CREATE TRIGGER node_id_must_be_non_zero
            BEFORE INSERT ON nodes
            WHEN (
              (NEW.id <= 0) OR
              (NEW.id > 1023)
            )
            BEGIN
              SELECT RAISE(ROLLBACK, 'node id must be nonzero and less than 1023');
            END;
            """,
            "DROP TRIGGER node_id_must_be_non_zero;"

    create table(:network_nodes) do
      add :network_id, references(:networks), null: false, on_delete: :delete_all
      add :node_id, references(:nodes), null: false, on_delete: :delete_all
      add :name, :string, null: false
    end

    # only one node name may exist per network
    create unique_index(:network_nodes, [:network_id, :name])

    # preload the intital topology
    execute "INSERT INTO nodes(id) VALUES(1);", "DELETE FROM nodes WHERE id=1;"
    execute "INSERT INTO nodes(id) VALUES(2);", "DELETE FROM nodes WHERE id=2;"
    execute "INSERT INTO nodes(id) VALUES(3);", "DELETE FROM nodes WHERE id=3;"
    execute "INSERT INTO nodes(id) VALUES(4);", "DELETE FROM nodes WHERE id=4;"
    execute "INSERT INTO nodes(id) VALUES(5);", "DELETE FROM nodes WHERE id=5;"
    execute "INSERT INTO nodes(id) VALUES(6);", "DELETE FROM nodes WHERE id=6;"
    execute "INSERT INTO nodes(id) VALUES(7);", "DELETE FROM nodes WHERE id=7;"
    execute "INSERT INTO nodes(id) VALUES(8);", "DELETE FROM nodes WHERE id=8;"

    execute "INSERT INTO network_nodes(network_id, node_id, name) VALUES(100, 1, 'cone');",
            "DELETE FROM network_nodes WHERE id=1;"

    execute "INSERT INTO network_nodes(network_id, node_id, name) VALUES(100, 2, '9');",
            "DELETE FROM network_nodes WHERE id=1;"

    execute "INSERT INTO network_nodes(network_id, node_id, name) VALUES(100, 3, 'jake');",
            "DELETE FROM network_nodes WHERE id=1;"

    execute "INSERT INTO network_nodes(network_id, node_id, name) VALUES(100, 4, 'dax');",
            "DELETE FROM network_nodes WHERE id=1;"

    execute "INSERT INTO network_nodes(network_id, node_id, name) VALUES(100, 5, 'kyler');",
            "DELETE FROM network_nodes WHERE id=1;"

    execute "INSERT INTO network_nodes(network_id, node_id, name) VALUES(100, 6, 'tyler');",
            "DELETE FROM network_nodes WHERE id=1;"

    execute "INSERT INTO network_nodes(network_id, node_id, name) VALUES(100, 7, 'dibs');",
            "DELETE FROM network_nodes WHERE id=1;"

    execute "INSERT INTO network_nodes(network_id, node_id, name) VALUES(100, 8, 'easyy');",
            "DELETE FROM network_nodes WHERE id=1;"

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

    execute "UPDATE config SET network_id = 100;"
    execute "UPDATE config SET network_identity_id = 1;"
    execute "UPDATE config SET network_leader_id = 1;"
    execute "UPDATE config SET version = version + 1;"
  end
end
