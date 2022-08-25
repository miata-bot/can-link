defmodule Spect.MixProject do
  use Mix.Project
  @source_url "https://github.com/miata-bot/can-link/"

  def project do
    [
      app: :spect,
      version: "0.1.0",
      elixir: "~> 1.12",
      start_permanent: Mix.env() == :prod,
      elixirc_paths: elixirc_paths(Mix.env()),
      docs: docs(),
      deps: deps()
    ]
  end

  # Specifies which paths to compile per environment.
  defp elixirc_paths(:test), do: ["lib", "test/support"]
  defp elixirc_paths(_), do: ["lib"]

  # Run "mix help compile.app" to learn about applications.
  def application do
    [
      extra_applications: [:logger]
    ]
  end

  defp docs do
    [
      extras: ["README.md"],
      main: "readme",
      source_url: @source_url,
      source_url_pattern: "#{@source_url}/blob/#{get_commit_sha()}/interface/spect/%{path}#L%{line}"
    ]
  end

  defp get_commit_sha() do
    System.cmd("git", ~w|show -s --format=%h|)
    |> elem(0)
    |> String.trim()
  end

  # Run "mix help deps" to learn about dependencies.
  defp deps do
    [
      {:ecto, "~> 3.8"},
      {:ecto_sqlite3, "~> 0.7.7"},
      {:circuits_uart, "~> 1.4"},
      {:ex_doc, "~> 0.28.5"}
    ]
  end
end
