defmodule LibLunarEx.MixProject do
  use Mix.Project

  def project do
    [
      app: :lib_lunar_ex,
      version: "0.1.0",
      elixir: "~> 1.6",
      start_permanent: Mix.env() == :prod,
      compilers: [:elixir_make] ++ Mix.compilers(),
      deps: deps(),
      package: package()
    ]
  end

  # Run "mix help compile.app" to learn about applications.
  def application do
    [
      extra_applications: [:logger],
      mod: {LibLunarEx.Application, []}
    ]
  end

  # Run "mix help deps" to learn about dependencies.
  defp deps do
    [
      {:porcelain, "~> 2.0"},
      {:elixir_make, "~> 0.5", runtime: false},
      {:timex, git: "https://github.com/bitwalker/timex.git"},
    ]
  end

  defp package do
    [
      files: [
        "lib", "LICENSE", "mix.exs", "README.md", "src/*.[ch]", "Makefile"
      ]
    ]
  end

end
