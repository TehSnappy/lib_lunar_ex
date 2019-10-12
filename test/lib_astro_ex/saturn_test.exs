defmodule LibLunarEx.SaturnTest do
  use ExUnit.Case

  alias LibLunarEx.Constellations
  alias LibLunarEx.Saturn
  alias LibLunarEx.Body
  alias LibLunarEx.TestHelper

  setup do
    TestHelper.std_setup()
  end

  test "gets Saturn's ra and dec", %{observation: observation} do
    assert %Body{
             dec: %{d: -22, m: 45, s: 8},
             ra: %{h: 18, m: 11, s: 3}
           } = Saturn.ra_declination(%Body{from: observation})
  end

  test "gets Saturn's constellation", %{observation: observation} do
    %Body{constellation: const} =
      Saturn.ra_declination(%Body{from: observation})
      |> Constellations.locate()

    assert const == "Sagittarius"
  end

  test "saturn provides location, constellation and rise set times", %{observer: observer} do
    observation = LibLunarEx.observe(observer, :saturn)
    assert %Body{
      ra: ra,
      dec: dec,
      constellation: constellation,
      prev_rise: prev_rise,
      prev_set: prev_set,
      next_rise: next_rise,
      next_set: next_set,
    } = LibLunarEx.get_body(observation, :saturn)

    assert ra
    assert dec
    assert constellation
    assert prev_rise == "2018-09-23T23:48:20-04:00"
    assert next_set == "2018-09-24T14:44:42-04:00"
  end
end
