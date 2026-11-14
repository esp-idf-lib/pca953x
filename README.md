# esp-idf-lib/pca953x

[![Build examples](https://github.com/esp-idf-lib/pca953x/actions/workflows/build.yml/badge.svg)](https://github.com/esp-idf-lib/pca953x/actions/workflows/build.yml)
[![Build docs](https://github.com/esp-idf-lib/pca953x/actions/workflows/build-docs.yml/badge.svg)](https://github.com/esp-idf-lib/pca953x/actions/workflows/build-docs.yml)
[![Validation](https://github.com/esp-idf-lib/pca953x/actions/workflows/validate-component.yml/badge.svg)](https://github.com/esp-idf-lib/pca953x/actions/workflows/validate-component.yml)

Driver for PCA9530/PCA9533 I2C-bus LED dimmers.

* [Documentation](https://esp-idf-lib.github.io/pca953x/)
* [Repository](https://github.com/esp-idf-lib/pca953x)
* [Issues](https://github.com/esp-idf-lib/pca953x/issues)
* [Discussions and questions](https://github.com/esp-idf-lib/core/discussions)
* [Component page at the ESP Component Registry](https://components.espressif.com/components/esp-idf-lib/pca953x)

## Supported devices

| Part | Outputs | Address |
| --- | --- | --- |
| PCA9530 | 2 | `0x60` when `A0` is grounded, `0x61` when `A0` is tied to VDD |
| PCA9533/01 | 4 | `0x62`, fixed |
| PCA9533/02 | 4 | `0x63`, fixed |

The driver uses 7-bit addresses; the datasheets list them as 8-bit write
addresses (`0xC0`, `0xC2`, `0xC4` and `0xC6`).

Both devices have open-drain outputs that can drive an LED directly, without a series resistor on the low side, and an `INT` input is not present: the parts are output-only expanders with a PWM and a blink engine.

## How it works

Every output is controlled by two bits of the LED selector register:

| Value | Meaning |
| --- | --- |
| `00` | Output is high-impedance, the LED is off and the pin can be read as an input |
| `01` | Output is driven low, the LED is on |
| `10` | Output is modulated with the rate and the duty cycle of PWM0 |
| `11` | Output is modulated with the rate and the duty cycle of PWM1 |

The two PWM generators are shared: every output that selects PWM0 blinks with the same period and the same duty cycle, and the same holds for PWM1. The period is `(prescaler + 1) / 152` seconds, so it is between 6.6 ms (152 Hz) and 1.684 s, and the duty cycle is `PWM / 256`. A period short enough, above roughly 100 Hz, makes the LED look dimmed rather than blinking; a long period makes it blink.

## Using the driver

For the everyday use there is a small set of functions:

* `pca953x_set_brightness()` turns a LED on, dims it or turns it off, the brightness goes from 0, dark, to `PCA953X_BRIGHTNESS_MAX`, fully lit; the two extremes switch the output to the steady modes, so the LED does not toggle at all;
* `pca953x_get_brightness()` reports that value back, whichever way the LED is driven;
* `pca953x_blink()` lights a LED in pulses and takes the period in milliseconds and the brightness of the pulses on the same scale, where the extremes mean off and steady on;
* `pca953x_get_blink()` reports the period and the brightness of a blinking LED.

The dimming runs at the shortest period the chip can produce, about 6.6 ms or 152 Hz, so a dimmed LED looks steady rather than blinking, and the prescaler is not something the caller has to think about. The chip generates the pulses with two generators for all of its outputs, so the dimmed outputs share one brightness and the blinking outputs share one period and one brightness: the last call wins for every output that uses the same generator. Outputs that are switched on or off steadily do not depend on the generators and can be used freely.

The registers are still reachable where the simple functions are not enough:

* `pca953x_set_mode()` and `pca953x_get_mode()` select the source of one output;
* `pca953x_set_pwm()` and `pca953x_get_pwm()` set and read the duty cycle of a generator in register units;
* `pca953x_set_prescaler()` and `pca953x_get_prescaler()` set and read the prescaler of a generator;
* `pca953x_set_blink_period()` and `pca953x_get_blink_period()` work with the period in milliseconds and round it up to the closest value the prescaler can produce;
* `pca953x_get_inputs()` and `pca953x_get_input()` read the state of the pins, which is what a pin configured as `PCA953X_MODE_OFF` is for.

## Example

The [default](examples/default) example sweeps the first output from dark to full brightness and back, blinks the second one at half brightness and reads the pin levels back.

## Installation

```sh
idf.py add-dependency esp-idf-lib/pca953x
```

## Support

For questions and discussions about the component, please use
[Discussions](https://github.com/esp-idf-lib/core/discussions)
at [esp-idf-lib/core](https://github.com/esp-idf-lib/core).

## Contributing

Please read [CONTRIBUTING.md](https://github.com/esp-idf-lib/core/blob/main/CONTRIBUTING.md)
at [esp-idf-lib/core](https://github.com/esp-idf-lib/core).
