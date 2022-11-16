defmodule JlcHelperWeb.PageController do
  use JlcHelperWeb, :controller

  def index(conn, _params) do
    render(conn, "index.html")
  end
end
