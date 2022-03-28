defmodule CANLink.PWM do
  @moduledoc """
  Basic control

  pwm:
     ehrpwm1A == pwmchip2/pwm0 (LED R)
     ehrpwm1B == pwmchip2/pwm1 (LED G)
     ehrpwm2A == pwmchip0/pwm0 (LED B)
     ehrpwm2B == pwmchip0/pwm1 (A)
  """

  @pwms [
    led_r: {2, 0},
    led_g: {2, 1},
    led_b: {0, 0},
  ]

  # Period for 25kHz PWM
  @period 40_000

  def init() do
    @pwms
    |> Enum.each(fn {_pwm, {chip, pin}} ->
      File.write("/sys/class/pwm/pwmchip#{chip}/export", to_string(pin))
      File.write("/sys/class/pwm/pwmchip#{chip}/pwm#{pin}/period", to_string(@period))
    end)
  end

  def period(pwm, period) do
    {chip, pwm} = Keyword.fetch!(@pwms, pwm)
    File.write("/sys/class/pwm/pwmchip#{chip}/pwm#{pwm}/period", to_string(period))
  end

  def period(pwm) do
    {chip, pwm} = Keyword.fetch!(@pwms, pwm)

    File.read!("/sys/class/pwm/pwmchip#{chip}/pwm#{pwm}/period")
    |> String.trim()
    |> String.to_integer()
  end

  def duty_cycle(pwm, duty_cycle) do
    {chip, pwm} = Keyword.fetch!(@pwms, pwm)
    File.write("/sys/class/pwm/pwmchip#{chip}/pwm#{pwm}/duty_cycle", to_string(duty_cycle))
  end

  def duty_cycle(pwm) do
    {chip, pwm} = Keyword.fetch!(@pwms, pwm)

    File.read!("/sys/class/pwm/pwmchip#{chip}/pwm#{pwm}/duty_cycle")
    |> String.trim()
    |> String.to_integer()
  end

  def enable(pwm, enable) do
    {chip, pwm} = Keyword.fetch!(@pwms, pwm)
    enable = if enable, do: 1, else: 0
    File.write("/sys/class/pwm/pwmchip#{chip}/pwm#{pwm}/enable", to_string(enable))
  end

  def enable(pwm) do
    {chip, pwm} = Keyword.fetch!(@pwms, pwm)
    "1" == File.read!("/sys/class/pwm/pwmchip#{chip}/pwm#{pwm}/enable") |> String.trim()
  end
end
