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

The driver wraps the registers:

* `pca953x_set_mode()` and `pca953x_get_mode()` select the source of one output;
* `pca953x_set_pwm()` and `pca953x_get_pwm()` set and read the duty cycle of a rate;
* `pca953x_set_prescaler()` and `pca953x_get_prescaler()` set and read the prescaler of a rate;
* `pca953x_set_blink_period()` and `pca953x_get_blink_period()` work with the period in milliseconds and round it up to the closest value the prescaler can produce;
* `pca953x_get_inputs()` and `pca953x_get_input()` read the state of the pins, which is what a pin configured as `PCA953X_MODE_OFF` is for.

## Example

The [default](examples/default) example dims the first output, blinks the second one and reads the pin levels back.

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
