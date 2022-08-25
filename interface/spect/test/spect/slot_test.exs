defmodule Spect.SlotTest do
  use Spect.DataCase

  test "only 8 slots may exist", %{database: database} do
    sections =
      Spect.Repo.with_repo(database, fn %{repo: repo} ->
        repo.all(Spect.Section)
      end)

    assert Enum.count(sections) == 8

    assert_raise Exqlite.Error, ~r/only 8 sections/, fn ->
      Spect.Repo.with_repo(database, fn %{repo: repo} ->
        repo.insert(%Spect.Section{start: 420, end: 420})
      end)
    end

    sections =
      Spect.Repo.with_repo(database, fn %{repo: repo} ->
        repo.all(Spect.Section)
      end)

    assert Enum.count(sections) == 8
  end

  test "sections may not overlap", %{database: database} do
    section1 =
      Spect.Repo.with_repo(database, fn %{repo: repo} ->
        repo.get(Spect.Section, 1)
      end)

    section2 =
      Spect.Repo.with_repo(database, fn %{repo: repo} ->
        repo.get(Spect.Section, 2)
      end)

    assert section1.start == 0
    assert section1.end == 30

    assert section2.start == 31
    assert section2.end == 110

    assert_raise Exqlite.Error, ~r/sections may not overlap/, fn ->
      Spect.Repo.with_repo(database, fn %{repo: repo} ->
        changeset = Spect.Section.changeset(section2, %{start: 28})
        repo.update(changeset)
      end)
    end
  end
end
