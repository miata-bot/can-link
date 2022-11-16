defmodule JlcHelperWeb.ComponentLive.Show do
  use JlcHelperWeb, :live_view

  alias JlcHelper.JLC

  @impl true
  def mount(_params, _session, socket) do
    {:ok, socket}
  end

  @impl true
  def handle_params(%{"id" => id}, _, socket) do
    {:noreply,
     socket
     |> assign(:page_title, page_title(socket.assigns.live_action))
     |> assign(:component, JLC.get_component!(id))}
  end

  defp page_title(:show), do: "Show Component"
  defp page_title(:edit), do: "Edit Component"
end
