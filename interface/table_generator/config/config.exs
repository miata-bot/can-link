import Config

config :table_generator,
  ecto_repos: [TableGenerator.Repo]

config :table_generator, TableGenerator.Repo,
  database: "database.db"
