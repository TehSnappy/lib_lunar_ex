defmodule LibLunarExTest do
  use ExUnit.Case
  doctest LibLunarEx

  alias LibLunarEx.TestHelper

  setup do
    TestHelper.std_setup()
  end

  describe "observe_bodies" do
    test "can get odd planets", %{observer: observer} do
      observation = LibLunarEx.observe(observer, [:venus, :uranus])

      assert Enum.count(observation.bodies) == 2
      assert Enum.any?(observation.bodies, fn b -> b.name == :venus end)
      assert Enum.any?(observation.bodies, fn b -> b.name == :uranus end)
    end

    test "can get visible planets", %{observer: observer} do
      observation = LibLunarEx.observe(observer, :visible)

      assert Enum.count(observation.bodies) == 2
      assert Enum.any?(observation.bodies, fn b -> b.name == :mercury end)
      assert Enum.any?(observation.bodies, fn b -> b.name == :uranus end)
    end

    test "can get daily bodies, sun and moon", %{observer: observer} do
      observation = LibLunarEx.observe(observer, :daily)

      assert Enum.count(observation.bodies) == 2
      assert Enum.any?(observation.bodies, fn b -> b.name == :moon end)
      assert Enum.any?(observation.bodies, fn b -> b.name == :sun end)
    end
  end

  describe "get_observer" do
    test "can create an observer", %{location: %{lat: lat, lon: lon}, datetime: datetime} do
      assert LibLunarEx.get_observer(lat, lon, datetime, "America/New_York")
    end
  end
end
