defmodule JlcHelperWeb.ComponentLive.Index do
  use JlcHelperWeb, :live_view

  alias JlcHelper.JLC
  alias JlcHelper.JLC.Component

  @impl true
  def mount(_params, _session, socket) do
    {:ok, assign(socket, :componens, list_componens())}
  end

  @impl true
  def handle_params(params, _url, socket) do
    {:noreply, apply_action(socket, socket.assigns.live_action, params)}
  end

  defp apply_action(socket, :edit, %{"id" => id}) do
    socket
    |> assign(:page_title, "Edit Component")
    |> assign(:component, JLC.get_component!(id))
  end

  defp apply_action(socket, :new, _params) do
    socket
    |> assign(:page_title, "New Component")
    |> assign(:component, %Component{})
  end

  defp apply_action(socket, :index, _params) do
    socket
    |> assign(:page_title, "Listing Componens")
    |> assign(:component, nil)
  end

  @impl true
  def handle_event("delete", %{"id" => id}, socket) do
    component = JLC.get_component!(id)
    {:ok, _} = JLC.delete_component(component)

    {:noreply, assign(socket, :componens, list_componens())}
  end

  defp list_componens do
    JLC.list_componens()
  end
end
