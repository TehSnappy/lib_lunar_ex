defmodule LibLunarEx.SunTest do
  use ExUnit.Case

  alias LibLunarEx.TestHelper
  alias LibLunarEx.Body
  alias LibLunarEx.Sun
  alias LibLunarEx.Constellations
  alias LibLunarEx.Observer

  setup do
    TestHelper.std_setup()
  end

  test "gets the sunrise and sunset", %{observation: observation} do
    assert %Body{
             prev_rise: "2018-09-24T06:42:02-04:00",
             next_set: "2018-09-24T18:46:17-04:00"
           } = Sun.rise_set_times(%Body{from: observation})
  end

  test "gets the Sun's ra and dec", %{observation: observation} do
    assert %Body{
             dec: %{d: -23, m: 33, s: 31},
             ra: %{h: 20, m: 26, s: 46}
           } = Sun.ra_declination(%Body{from: observation})
  end

  test "gets the Sun's constellation", %{observation: observation} do
    %Body{constellation: const} =
      %Body{from: observation}
      |> Sun.ra_declination()
      |> Constellations.locate()

    assert const == "Capricornus"
  end

  describe "sun edge cases" do
    test "try another dates", %{observer: observer} do
      observer =
        "2018-08-11T05:56:46-04:00 America/New_York"
        |> Observer.set_observer_date_time(observer)

      observation = LibLunarEx.observe(observer, [:sun])
      %Body{prev_rise: rise} =
               LibLunarEx.get_body(observation, :sun)

      assert rise == "2018-08-10T05:57:29-04:00"
    end

    test "get the rise and set despite the time zones", %{observer: observer} do
      observer =
        "2018-10-10 22:42:44-04:00 America/New_York"
        |> Observer.set_observer_date_time(observer)

      observation = LibLunarEx.observe(observer, [:sun])
      %Body{prev_rise: rise, prev_set: set} =
               LibLunarEx.get_body(observation, :sun)

      assert rise == "2018-10-10T06:58:32-04:00"
      assert set == "2018-10-10T18:19:50-04:00"
    end

    test "the sunrise just after midnight", %{observer: observer} do
      observer =
        "2018-10-10 00:01:01-04:00 America/New_York"
        |> Observer.set_observer_date_time(observer)

      observation = LibLunarEx.observe(observer, [:sun])

      %Body{next_rise: rise, next_set: set} =
               LibLunarEx.get_body(observation, :sun)

      assert rise == "2018-10-10T06:58:32-04:00"
      assert set == "2018-10-10T18:19:50-04:00"
    end

    test "the sunrise just before midnight", %{observer: observer} do
      observer =
        "2018-10-10 23:59:59-04:00 America/New_York"
        |> Observer.set_observer_date_time(observer)

      observation = LibLunarEx.observe(observer, [:sun])
      %Body{prev_rise: rise, prev_set: set} =
               LibLunarEx.get_body(observation, :sun)

      assert rise == "2018-10-10T06:58:32-04:00"
      assert set == "2018-10-10T18:19:50-04:00"
    end
  end
end
