defmodule HaltechLinkTest do
  use ExUnit.Case
  doctest HaltechLink

  test "greets the world" do
    assert HaltechLink.hello() == :world
  end
end
