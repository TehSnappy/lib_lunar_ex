defmodule LibLunarEx.Observation do
  defstruct lat: nil,
            lon: nil,
            j_date: nil,
            tz: nil,
            bodies: []

  alias LibLunarEx.{Body, Observer, Observation}

  alias LibLunarEx.{
    Sun,
    Moon,
    Mercury,
    Moon,
    Venus,
    Mars,
    Jupiter,
    Saturn,
    Neptune,
    Uranus
  }

  alias LibLunarEx.Constellations

  @supported_planets %{
    sun: Sun,
    moon: Moon,
    mercury: Mercury,
    venus: Venus,
    mars: Mars,
    jupiter: Jupiter,
    saturn: Saturn,
    neptune: Neptune,
    uranus: Uranus
  }

  def supported_bodies, do: Map.keys(@supported_planets)

  def make_observation(%Observer{} = observer) do
    utc_date =
      observer.date_time
      |> Timex.Timezone.convert("Etc/UTC")

    j_date =
      Timex.Calendar.Julian.julian_date(
        utc_date.year,
        utc_date.month,
        utc_date.day,
        utc_date.hour,
        utc_date.minute,
        utc_date.second
      )

    timezone = Timex.Timezone.get(observer.tz, utc_date)
    tz_diff = Timex.Timezone.diff(utc_date, timezone) / 3600.0

    %Observation{lat: observer.lat, lon: observer.lon, j_date: j_date, tz: tz_diff}
  end

  def add_body(%Observation{} = obs, :moon) do
    bod =
      obs
      |> Body.from(:moon)
      |> Moon.ra_declination()
      |> Moon.rise_set_times()
      |> Moon.phases()
      |> Constellations.locate()

    %{obs | bodies: [bod | obs.bodies]}
  end

  def add_body(%Observation{} = obs, name) do
    target_mod = Map.fetch!(@supported_planets, name)

    bod =
      obs
      |> Body.from(name)
      |> target_mod.ra_declination()
      |> target_mod.rise_set_times()
      |> Constellations.locate()

    %{obs | bodies: [bod | obs.bodies]}
  end
end
