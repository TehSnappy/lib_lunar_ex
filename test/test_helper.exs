ExUnit.start()

defmodule LibLunarEx.TestHelper do
  alias Timex.Calendar.Julian
  alias LibLunarEx.Observation

  def set_observation_date_time(julian_date_time, %Observation{} = obs)
      when is_float(julian_date_time) do
    %{
      obs | j_date: julian_date_time
    }
  end

  def set_observation_date_time(date_time_string, %Observation{} = obs)
      when is_binary(date_time_string) do
    ndt =
      date_time_string
      |> Timex.parse!("{ISO:Extended}")
      |> Timex.Timezone.convert("Etc/UTC")

    %{
      obs
      | j_date: Julian.julian_date(ndt.year, ndt.month, ndt.day, ndt.hour, ndt.minute, ndt.second)
    }
  end

  def std_setup do
    {:ok, ndt} = NaiveDateTime.new(2018, 9, 24, 20, 0, 0)
    dt = DateTime.from_naive!(ndt, "Etc/UTC")

    [
      location: %{
        lat: "40.9277",
        lon: "-73.1333"
      },
      datetime: dt,
      observer: LibLunarEx.get_observer(40.9277, -73.1333, dt, "America/New_York"),
      observation: %Observation{
        lat: "40.927",
        lon: "-73.133",
        j_date: Julian.julian_date(2018, 9, 24, 12, 0, 0),
        tz: -4
      }
    ]
  end
end
