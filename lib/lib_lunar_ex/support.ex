defmodule LibLunarEx.Support do
  require Logger
  alias LibLunarEx.Support
  alias Porcelain.Result

  @target Mix.target()
  
  def body_rise_set_times(planet_no, lat, lng, date, tz) do
    proc_params = ["-n #{lng}", "-l #{lat}", "-d #{date}", "-p #{planet_no}", "-t #{tz}"]
    Support.execute_with(:riseset, proc_params)
  end

  def body_ra_and_dec(planet_no, lat, lng, date) do
    proc_params = ["-n #{lng}", "-l #{lat}", "-d #{date}", "-p #{planet_no}"]
    Support.execute_with(:locations, proc_params)
  end

  def execute_with(app, params) do
    path = Support.executable_path(app)
    params = ["-c#{:code.priv_dir(:lib_lunar_ex)}" | params]

    case Porcelain.exec(path, params, out: :iodata, async_in: true) do
      %Result{err: nil, out: [_, data], status: 0} ->
        data |> Support.strip_length_int() |> :erlang.binary_to_term()

      %Result{err: err, out: [_, data], status: status} ->
        Logger.error("received exec error #{err} status: #{status}")
        Logger.error("received exec error #{inspect(data, charlists: :as_binary)}")
        %{err: :exec_error, desc: status}

      _ ->
        %{err: :unknown}
    end
  end

  def strip_length_int(<<_lng::size(16), data::binary>>) do
    data
  end

  def executable_path(:riseset) do
    (:code.priv_dir(:lib_lunar_ex) ++ obj_dir() ++ '/riseset') |> List.to_string()
  end

  def executable_path(:locations) do
    (:code.priv_dir(:lib_lunar_ex) ++ obj_dir() ++ '/planet_loc') |> List.to_string()
  end

  def executable_path(:phases) do
    (:code.priv_dir(:lib_lunar_ex) ++ obj_dir() ++ '/moonphase') |> List.to_string()
  end

  def obj_dir do
    "/#{@target |> Atom.to_string()}" |> String.to_charlist()
  end
end
