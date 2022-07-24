defmodule Spect.Repo do
  use Ecto.Repo, otp_app: :management_console, adapter: Ecto.Adapters.SQLite3

  def with_repo(path, fun) do
    default_dynamic_repo = Repo.get_dynamic_repo()

    {:ok, pid} =
      Repo.start_link(
        name: nil,
        database: path,
        pool_size: 1
      )

    try do
      Repo.put_dynamic_repo(pid)
      fun.(%{pid: pid, repo: Repo})
    after
      Repo.put_dynamic_repo(default_dynamic_repo)
      Supervisor.stop(pid)
    end
  end
end
