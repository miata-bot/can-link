defmodule GlowBotDiscord.Connection do
  use Nostrum.Consumer

  alias Nostrum.Api
  @endpoint GlowBotWeb.Endpoint

  def install_commands(guild_id) do
    command = %{
      name: "change color",
      description: "change rgb color",
      options: [

      ]
    }
    Nostrum.Api.create_guild_application_command(guild_id, command)
  end

  def start_link do
    Consumer.start_link(__MODULE__)
  end

  def handle_event({:MESSAGE_CREATE, msg, _ws_state}) do
    case msg.content do
      "?rgb " <> color ->
        @endpoint.broadcast("event", "change_color", %{color: color})
        # Api.create_message(msg.channel_id, "I copy and pasted this code")

      _ ->
        :ignore
    end
  end

  # Default event handler, if you don't include this, your consumer WILL crash if
  # you don't have a method definition for each event type.
  def handle_event(_event) do
    :noop
  end
end
