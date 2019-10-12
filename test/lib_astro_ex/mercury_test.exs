defmodule LibLunarEx.MercuryTest do
  use ExUnit.Case

  alias LibLunarEx.TestHelper
  alias LibLunarEx.Body
  alias LibLunarEx.Mercury
  alias LibLunarEx.Constellations

  setup do
    TestHelper.std_setup()
  end

  test "gets Mercury's ra and dec", %{observation: observation} do
    assert %Body{
             dec: %{d: 0, m: 27, s: 0},
             ra: %{h: 12, m: 16, s: 29}
           } = Mercury.ra_declination(%Body{from: observation})
  end

  test "gets Mercury's constellation", %{observation: observation} do
    assert %Body{constellation: "Virgo"} =
             Mercury.ra_declination(%Body{from: observation})
             |> Constellations.locate()
  end
end
