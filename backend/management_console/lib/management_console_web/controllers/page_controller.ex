defmodule ManagementConsoleWeb.PageController do
  use ManagementConsoleWeb, :controller

  def index(conn, _params) do
    render(conn, "index.html")
  end
end
