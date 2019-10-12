defmodule LibLunarEx.Moon do
  alias LibLunarEx.{Body, Observation, Support}

  use LibLunarEx.Body

  def planet_number(), do: 10

  def phases(%Body{from: %Observation{} = obs} = body) do
    phases = moon_phases(obs)

    %{body | phases: Map.delete(phases, :phase), current_phase: Map.fetch!(phases, :phase)}
  end

  defp moon_phases(%Observation{j_date: date, tz: tz}) do
    proc_params = ["-d #{date}", "-t #{tz}"]
    Support.execute_with(:phases, proc_params)
  end
end
