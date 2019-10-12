defmodule LibLunarEx.MoonTest do
  use ExUnit.Case

  alias LibLunarEx.TestHelper
  alias LibLunarEx.Body
  alias LibLunarEx.Moon
  alias LibLunarEx.Constellations
  alias Timex.Calendar.Julian

  setup do
    TestHelper.std_setup()
  end

  test "gets the Moons rise and set", %{observation: observation} do
    assert %Body{
             next_rise: "2018-09-24T18:58:13-04:00",
             prev_set: "2018-09-24T05:58:28-04:00"
           } = Moon.rise_set_times(%Body{from: observation})
  end

  test "gets the Moon's ra and dec", %{observation: observation} do
    assert %Body{
             dec: %{d: -6, m: 31, s: 24},
             ra: %{h: 23, m: 41, s: 32}
           } = Moon.ra_declination(%Body{from: observation})
  end

  test "gets the Moon's constellation", %{observation: observation} do
    %Body{constellation: const} =
      Moon.ra_declination(%Body{from: observation})
      |> Constellations.locate()

    assert const == "Aquarius"
  end

  test "gets the moon phases", %{observation: observation} do
    %Body{phases: phases} = Moon.phases(%Body{from: observation})
    assert phases[:previous_full] == "2018-08-26T07:56:13-04:00"
    assert phases[:next_full] == "2018-09-24T22:52:26-04:00"
  end

  describe "moon edge cases" do
    test "moonrise jumps day on DST change", %{observation: observation} do
      obs =
        "2018-11-07 08:47:27-05:00 America/New_York"
        |> TestHelper.set_observation_date_time(observation)

      %Body{
         next_set: next_set,
       } = Moon.rise_set_times(%Body{from: observation})

      assert next_set == "2018-09-24T05:58:28-04:00"
    end

    test "the constellation becomes Virgo", %{observation: observation} do
      obs =
        "2018-10-10 08:42:44-04:00 America/New_York"
        |> TestHelper.set_observation_date_time(observation)

      assert %Body{constellation: "Virgo"} =
               %Body{from: obs}
               |> Moon.ra_declination()
               |> Constellations.locate()
    end

    test "the constellation becomes Libra", %{observation: observation} do
      obs =
        "2018-10-10 09:32:44-04:00 America/New_York"
        |> TestHelper.set_observation_date_time(observation)

      assert %Body{constellation: "Libra"} =
               Moon.ra_declination(%Body{from: obs})
               |> Constellations.locate()
    end
  end

  describe "Moon Phases" do
    test "previous new moon", %{observation: observation} do
      assert %Body{phases: %{previous_new: "2018-09-09T14:01:27-04:00"}} 
        = Moon.phases(%Body{from: observation})
    end

    test "next new moon", %{observation: observation} do
      assert %Body{phases: %{next_new: "2018-10-08T23:46:52-04:00"}} 
        = Moon.phases(%Body{from: observation})
    end

    test "previous full moon", %{observation: observation} do
      assert %Body{phases: %{previous_full: "2018-08-26T07:56:13-04:00"}} 
        = Moon.phases(%Body{from: observation})
    end

    test "next full moon", %{observation: observation} do
      assert %Body{phases: %{next_full: "2018-09-24T22:52:26-04:00"}} 
        = Moon.phases(%Body{from: observation})
    end

    test "previous first quarter", %{observation: observation} do
      assert %Body{phases: %{previous_first: "2018-09-16T19:15:00-04:00"}} 
        = Moon.phases(%Body{from: observation})
    end

    test "previous last quarter", %{observation: observation} do
      assert %Body{phases: %{previous_last: "2018-09-02T22:37:26-04:00"}} 
        = Moon.phases(%Body{from: observation})
    end

    test "next first quarter", %{observation: observation} do
      assert %Body{phases: %{next_first: "2018-10-16T14:01:34-04:00"}} 
        = Moon.phases(%Body{from: observation})
    end

    test "next last quarter", %{observation: observation} do
      assert %Body{phases: %{next_last: "2018-10-02T05:45:28-04:00"}} 
        = Moon.phases(%Body{from: observation})
    end
  end

  describe "current Moon Phase" do
    test "Full on 9/24/2018", %{observation: observation} do
      observation =
        Julian.julian_date(2018, 9, 24, 4, 0, 0)
        |> TestHelper.set_observation_date_time(observation)

      assert %Body{current_phase: "Full"} = Moon.phases(%Body{from: observation})
    end

    test "New on 10/8/2018", %{observation: observation} do
      observation =
        Julian.julian_date(2018, 10, 8, 4, 0, 0)
        |> TestHelper.set_observation_date_time(observation)

      assert %Body{current_phase: "New"} = Moon.phases(%Body{from: observation})
    end

    test "Waning Gibbous on 9/28/2018", %{observation: observation} do
      observation =
        Julian.julian_date(2018, 9, 28, 4, 0, 0)
        |> TestHelper.set_observation_date_time(observation)

      assert %Body{current_phase: "Waning Gibbous"} = Moon.phases(%Body{from: observation})
    end

    test "Last Quarter on 10/2/2018", %{observation: observation} do
      observation =
        Julian.julian_date(2018, 10, 2, 4, 0, 0)
        |> TestHelper.set_observation_date_time(observation)

      assert %Body{current_phase: "Last Quarter"} = Moon.phases(%Body{from: observation})
    end

    test "Waning Crescent on 10/7/2018", %{observation: observation} do
      observation =
        Julian.julian_date(2018, 10, 7, 4, 0, 0)
        |> TestHelper.set_observation_date_time(observation)

      assert %Body{current_phase: "Waning Crescent"} = Moon.phases(%Body{from: observation})
    end

    test "First Quarter on 10/16/2018", %{observation: observation} do
      observation =
        Julian.julian_date(2018, 10, 16, 4, 0, 0)
        |> TestHelper.set_observation_date_time(observation)

      assert %Body{current_phase: "First Quarter"} = Moon.phases(%Body{from: observation})
    end

    test "Waxing Crescent on 10/12/2018", %{observation: observation} do
      observation =
        Julian.julian_date(2018, 10, 12, 4, 0, 0)
        |> TestHelper.set_observation_date_time(observation)

      assert %Body{current_phase: "Waxing Crescent"} = Moon.phases(%Body{from: observation})
    end

    test "Waxing Gibbous on 10/18/2018", %{observation: observation} do
      observation =
        Julian.julian_date(2018, 10, 18, 4, 0, 0)
        |> TestHelper.set_observation_date_time(observation)

      assert %Body{current_phase: "Waxing Gibbous"} = Moon.phases(%Body{from: observation})
    end
  end
end
