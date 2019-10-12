defmodule LibLunarEx.JupiterTest do
  use ExUnit.Case

  alias LibLunarEx.TestHelper
  alias LibLunarEx.Jupiter
  alias LibLunarEx.Constellations
  alias LibLunarEx.Body

  setup do
    TestHelper.std_setup()
  end

  test "gets Jupiter's ra and dec", %{observation: observation} do
    assert %Body{
             dec: %{d: -17, m: 9, s: 27},
             ra: %{h: 15, m: 13, s: 39}
           } = Jupiter.ra_declination(%Body{from: observation})
  end

  test "gets Jupiter's constellation", %{observation: observation} do
    assert %Body{constellation: "Libra"} =
             %Body{from: observation}
             |> Jupiter.ra_declination()
             |> Constellations.locate()
  end

  test "gets the Jupiter's rise and set", %{observation: observation} do
    assert %Body{
             next_rise: "2018-09-24T18:58:13-04:00",
             prev_set: "2018-09-24T05:58:28-04:00"
           } = Jupiter.rise_set_times(%Body{from: observation})
  end

end
