defmodule Spect.ConfigTest do
  use Spect.DataCase

  test "only one config", %{database: db, repo: repo} do
    config =
      repo.with_repo(db, fn %{repo: ^repo} ->
        repo.one!(Spect.Config)
      end)

    assert config.id == 0

    assert_raise Exqlite.Error, ~r/Only One configuration Table may exis/, fn ->
      repo.with_repo(db, fn %{repo: ^repo} ->
        repo.insert!(%Spect.Config{})
      end)
    end
  end
end
