defmodule GlowBotWeb.PageController do
  use GlowBotWeb, :controller

  def index(conn, _params) do
    render(conn, "index.html")
  end
end
