defmodule LibLunarEx.Body do
  defstruct ra: nil,
            dec: nil,
            ecliptic_lon: nil,
            constellation: nil,
            zodiac: nil,
            phases: nil,
            current_phase: nil,
            name: nil,
            next_rise: nil,
            next_set: nil,
            prev_rise: nil,
            prev_set: nil,
            from: nil

  alias LibLunarEx.{Body, Observation, Support}

  defmacro __using__(_) do
    quote do
      def ra_declination(%Body{from: %Observation{lat: lat, lon: lon, j_date: date}} = body) do
        %{ra: ra, dec: dec} = Support.body_ra_and_dec(planet_number(), lat, lon, date)
        struct(body, ra: ra, dec: dec)
      end

      def rise_set_times(
            %Body{from: %Observation{lat: lat, lon: lon, j_date: date, tz: tz}} = body
          ) do

        %{prev_rise: prev_rise,
          prev_set: prev_set,
          next_rise: next_rise,
          next_set: next_set,
        } = Support.body_rise_set_times(planet_number(), lat, lon, date, tz)

        struct(body, prev_rise: prev_rise, prev_set: prev_set, next_rise: next_rise, next_set: next_set)
      end
    end
  end

  def from(%Observation{} = obs, name) do
    %Body{
      name: name,
      from: %{obs | bodies: []}
    }
  end
end
