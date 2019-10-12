defmodule LibLunarEx.Observer do
  defstruct lat: nil, lon: nil, date_time: nil, tz: nil

  alias LibLunarEx.Observer

  def create(lat, lon, date, tz) do
    %Observer{lat: lat, lon: lon, tz: tz, date_time: date}
  end

  def create(%{lat: lat, lon: lon}, date) do
    %Observer{lat: lat, lon: lon, date_time: date}
  end

  def create(%{latitude: lat, longitude: lon, tz_name: tz}, datetime) do
    %Observer{lat: lat, lon: lon, tz: tz, date_time: datetime}
  end
  
  def set_observer_date_time(date_time_string, %Observer{} = obs)
      when is_binary(date_time_string) do
    ndt =
      date_time_string
      |> Timex.parse!("{ISO:Extended}")

    %Observer{
      obs | date_time: ndt
    }
  end

end
