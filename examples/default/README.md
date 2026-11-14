# Example for `pca953x` driver

## Datasheets

* [PCA9530](https://www.nxp.com/docs/en/data-sheet/PCA9530.pdf)
* [PCA9533](https://www.nxp.com/docs/en/data-sheet/PCA9533.pdf)

## What it does

The example configures a PCA9530, or a PCA9533 if `CONFIG_EXAMPLE_TYPE_PCA9533`
is selected, and drives its outputs.

PWM rate 0 is programmed with the shortest period (about 152 Hz) and a 25%
duty cycle, so the first output looks dimmed. PWM rate 1 is programmed with a
period of 500 ms and a 50% duty cycle, so the second output blinks.

When the example runs on a PCA9533, the third output is switched on steadily
and the fourth one is left in the high-impedance state, where it works as a
GPIO input.

Every two seconds the example prints the mode of each output and the level of
its pin, which shows that the open-drain output is low when the LED is on.

## Wiring

Connect an LED and a resistor between each output and VDD: the outputs are
open-drain, so they sink the current and the LED lights up when the output is
driven low.

| Output | LED |
| --- | --- |
| `LED0` | Dimmed by PWM0 |
| `LED1` | Blinks with PWM1 |
| `LED2` | On, PCA9533 only |
| `LED3` | High-impedance input, PCA9533 only |

Connect `SCL` and `SDA` pins to the following pins with appropriate pull-up
resistors.

| Name | Description | Defaults |
|------|-------------|----------|
| `CONFIG_EXAMPLE_I2C_MASTER_SCL` | GPIO number for `SCL` | "6" for `esp32c3`, "19" for `esp32`, `esp32s2`, and `esp32s3` |
| `CONFIG_EXAMPLE_I2C_MASTER_SDA` | GPIO number for `SDA` | "5" for `esp32c3`, "18" for `esp32`, `esp32s2`, and `esp32s3` |
