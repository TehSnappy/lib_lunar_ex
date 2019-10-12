defmodule LibLunarEx.UranusTest do
  use ExUnit.Case

  alias LibLunarEx.Constellations
  alias LibLunarEx.TestHelper
  alias LibLunarEx.Uranus
  alias LibLunarEx.Body

  setup do
    TestHelper.std_setup()
  end

  test "gets Uranus' ra and dec", %{observation: observation} do
    assert %Body{
             dec: %{d: 11, m: 26, s: 41},
             ra: %{h: 1, m: 57, s: 54}
           } = Uranus.ra_declination(%Body{from: observation})
  end

  test "gets Uranus' constellation", %{observation: observation} do
    %Body{constellation: const} =
      Uranus.ra_declination(%Body{from: observation})
      |> Constellations.locate()

    assert const == "Aries"
  end
end
