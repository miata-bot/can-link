import Config

config :spect,
  ecto_repos: [Spect.Repo]

config :spect, Spect.Repo,
  database: "database.db",
  show_sensitive_error: true
