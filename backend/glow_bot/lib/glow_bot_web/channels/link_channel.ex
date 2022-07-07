defmodule GlowBotWeb.LinkChannel do
  use GlowBotWeb, :channel
  alias GlowBotWeb.Presence

  def join(_, _params, socket) do
    send(self(), :after_join)
    socket.endpoint.subscribe("event")
    {:ok,
     socket
     |> assign(:user_id, "cone")}
  end

  def handle_info(%{topic: "event", payload: payload}, socket) do
    IO.inspect(payload, label: "payload")
    push(socket, "event", payload)
    {:noreply, socket}
  end

  def handle_info(:after_join, socket) do
    {:ok, _} =
      Presence.track(socket, socket.assigns.user_id, %{
        online_at: inspect(System.system_time(:second))
      })

    push(socket, "presence_state", Presence.list(socket))
    {:noreply, socket}
  end
end
