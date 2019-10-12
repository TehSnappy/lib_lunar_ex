defmodule LibLunarEx.NeptuneTest do
  use ExUnit.Case

  alias LibLunarEx.Constellations
  alias LibLunarEx.TestHelper
  alias LibLunarEx.Neptune
  alias LibLunarEx.Body

  setup do
    TestHelper.std_setup()
  end

  test "gets Neptune's ra and dec", %{observation: observation} do
    assert %Body{
             dec: %{d: -7, m: 4, s: 39},
             ra: %{h: 23, m: 3, s: 56}
           } = Neptune.ra_declination(%Body{from: observation})
  end

  test "gets Neptune's constellation", %{observation: observation} do
    %Body{constellation: const} =
      Neptune.ra_declination(%Body{from: observation})
      |> Constellations.locate()

    assert const == "Aquarius"
  end
end
