defmodule LibLunarEx do
  @moduledoc """
  Documentation for LibLunarEx.
  """

  alias LibLunarEx.Observer
  alias LibLunarEx.Observation

  @doc """
  Hello world.

  """
  def get_observer(lat, lon, date, tz), do: Observer.create(lat, lon, date, tz)
  def get_observer(%{} = attr, date), do: Observer.create(attr, date)

  def observe(%Observer{} = obs, :daily),
    do: observe_bodies(obs, [:moon, :sun])

  def observe(%Observer{} = obs, :visible) do
    obs = observe_bodies(obs, Observation.supported_bodies())

    %{obs | bodies: Enum.reject(obs.bodies, fn b -> is_nil(b.dec) || b.dec.d < 0 end)}
  end

  def observe(%Observer{} = obs, planet_list), do: observe_bodies(obs, planet_list)

  def get_body(%Observation{bodies: bodies}, name) do
    Enum.find(bodies, :missing, fn b -> b.name == name end)
  end

  defp observe_bodies(%Observer{} = observer, observation_list) when is_list(observation_list) do
    observation = Observation.make_observation(observer)
    Enum.reduce(observation_list, observation, &Observation.add_body(&2, &1))
  end

  defp observe_bodies(%Observer{} = observer, planet) do
    observer
    |> Observation.make_observation()
    |> Observation.add_body(planet)
  end
end
