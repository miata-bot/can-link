defmodule TableGenerator.MixProject do
  use Mix.Project

  def project do
    [
      app: :table_generator,
      version: "0.1.0",
      elixir: "~> 1.13",
      start_permanent: Mix.env() == :prod,
      deps: deps()
    ]
  end

  # Run "mix help compile.app" to learn about applications.
  def application do
    [
      extra_applications: [:logger]
    ]
  end

  # Run "mix help deps" to learn about dependencies.
  defp deps do
    [
      {:ecto_sqlite3, "~> 0.7.7"},
      {:circuits_uart, "~> 1.4"}
    ]
  end
end
