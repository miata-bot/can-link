defmodule JlcHelper.Meilsearch do
  use Tesla

  plug Tesla.Middleware.BaseUrl, "http://127.0.0.1:7700"
  plug Tesla.Middleware.Headers, []
  plug Tesla.Middleware.JSON

  def user_repos(login) do
    get("/users/" <> login <> "/repos")
  end
end
