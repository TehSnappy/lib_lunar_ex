defmodule LibLunarEx.MarsTest do
  use ExUnit.Case

  alias LibLunarEx.Body
  alias LibLunarEx.Mars
  alias LibLunarEx.TestHelper
  alias LibLunarEx.Constellations

  setup do
    TestHelper.std_setup()
  end

  test "gets Mars' ra and dec", %{observation: observation} do
    assert %Body{
             dec: %{d: -23, m: 33, s: 31},
             ra: %{h: 20, m: 26, s: 46}
           } = Mars.ra_declination(%Body{from: observation})
  end

  test "gets Mars' constellation", %{observation: observation} do
    assert %Body{constellation: "Capricornus"} =
             %Body{from: observation}
             |> Mars.ra_declination()
             |> Constellations.locate()
  end
end
