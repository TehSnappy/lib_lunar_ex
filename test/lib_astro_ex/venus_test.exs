defmodule LibLunarEx.VenusTest do
  use ExUnit.Case

  alias LibLunarEx.Constellations
  alias LibLunarEx.TestHelper
  alias LibLunarEx.Body
  alias LibLunarEx.Venus

  setup do
    TestHelper.std_setup()
  end

  test "gets Venus' ra and dec", %{observation: observation} do
    assert %Body{
             dec: %{d: -20, m: 4, s: 25},
             ra: %{h: 14, m: 15, s: 18}
           } = Venus.ra_declination(%Body{from: observation})
  end

  test "gets Venus' constellation", %{observation: observation} do
    %Body{constellation: const} =
      Venus.ra_declination(%Body{from: observation})
      |> Constellations.locate()

    assert const == "Libra"
  end
end
