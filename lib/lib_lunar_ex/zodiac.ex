defmodule LibLunarEx.Zodiac do
  alias LibLunarEx.Body

  def locate(%Body{ecliptic_lon: nil} = body), do: body

  def locate(%Body{ecliptic_lon: ecliptic_lon} = body) do
    %{body | zodiac: name_for_location(ecliptic_lon)}
  end

  defp name_for_location(ecliptic_lon) do
    cond do
      ecliptic_lon < 30 -> "Aries"
      ecliptic_lon < 60 -> "Taurus"
      ecliptic_lon < 90 -> "Gemini"
      ecliptic_lon < 120 -> "Cancer"
      ecliptic_lon < 150 -> "Leo"
      ecliptic_lon < 180 -> "Virgo"
      ecliptic_lon < 210 -> "Libra"
      ecliptic_lon < 240 -> "Scorpio"
      ecliptic_lon < 270 -> "Sagittarius"
      ecliptic_lon < 300 -> "Capricorn"
      ecliptic_lon < 330 -> "Aquarius"
      ecliptic_lon < 360 -> "Pisces"
      true -> :data_error
    end
  end
end
