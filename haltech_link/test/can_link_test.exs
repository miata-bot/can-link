defmodule CANLinkTest do
  use ExUnit.Case
  doctest CANLink

  test "greets the world" do
    assert CANLink.hello() == :world
  end
end
