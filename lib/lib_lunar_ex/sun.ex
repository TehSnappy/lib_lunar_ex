defmodule LibLunarEx.Sun do
  alias LibLunarEx.{Body, Observation, Support}

  def ra_declination(%Body{from: %Observation{lat: lat, lon: lon, j_date: date}} = body) do
    %{ra: ra, dec: dec} = Support.body_ra_and_dec(4, lat, lon, date)
    struct(body, ra: ra, dec: dec)
  end

  def rise_set_times(%Body{from: %Observation{lat: lat, lon: lon, j_date: date, tz: tz}} = body) do
    %{prev_rise: prev_rise,
      prev_set: prev_set,
      next_rise: next_rise,
      next_set: next_set,
    } = Support.body_rise_set_times(3, lat, lon, date, tz)

    struct(body, prev_rise: prev_rise, prev_set: prev_set, next_rise: next_rise, next_set: next_set)
  end
end
