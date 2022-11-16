defmodule JlcHelperWeb.ComponentLive.FormComponent do
  use JlcHelperWeb, :live_component

  alias JlcHelper.JLC

  @impl true
  def update(%{component: component} = assigns, socket) do
    changeset = JLC.change_component(component)

    {:ok,
     socket
     |> assign(assigns)
     |> assign(:changeset, changeset)}
  end

  @impl true
  def handle_event("validate", %{"component" => component_params}, socket) do
    changeset =
      socket.assigns.component
      |> JLC.change_component(component_params)
      |> Map.put(:action, :validate)

    {:noreply, assign(socket, :changeset, changeset)}
  end

  def handle_event("save", %{"component" => component_params}, socket) do
    save_component(socket, socket.assigns.action, component_params)
  end

  defp save_component(socket, :edit, component_params) do
    case JLC.update_component(socket.assigns.component, component_params) do
      {:ok, _component} ->
        {:noreply,
         socket
         |> put_flash(:info, "Component updated successfully")
         |> push_redirect(to: socket.assigns.return_to)}

      {:error, %Ecto.Changeset{} = changeset} ->
        {:noreply, assign(socket, :changeset, changeset)}
    end
  end

  defp save_component(socket, :new, component_params) do
    case JLC.create_component(component_params) do
      {:ok, _component} ->
        {:noreply,
         socket
         |> put_flash(:info, "Component created successfully")
         |> push_redirect(to: socket.assigns.return_to)}

      {:error, %Ecto.Changeset{} = changeset} ->
        {:noreply, assign(socket, changeset: changeset)}
    end
  end
end
