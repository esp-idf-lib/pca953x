pca953x: Driver for PCA9530/PCA9533 I2C-bus LED dimmers
=======================================================

The PCA9530 and the PCA9533 are 2-bit and 4-bit I2C LED dimmers with open-drain
outputs. Every output is controlled by two bits of the LED selector register:

.. list-table::
   :header-rows: 1

   * - Value
     - Meaning
   * - ``00``
     - Output is high-impedance, the LED is off and the pin can be read as an input
   * - ``01``
     - Output is driven low, the LED is on
   * - ``10``
     - Output is modulated with the rate and the duty cycle of PWM0
   * - ``11``
     - Output is modulated with the rate and the duty cycle of PWM1

The two PWM generators are shared by all outputs. The period is
``(prescaler + 1) / 152`` seconds, between 6.6 ms and 1.684 s, and the duty
cycle is ``PWM / 256``. A short period, above roughly 100 Hz, makes the LED
look dimmed; a long period makes it blink.

Everyday use needs four functions. ``pca953x_set_brightness()`` turns a LED
on, dims it or turns it off, the brightness going from 0, dark, to
``PCA953X_BRIGHTNESS_MAX``, fully lit, and the two extremes switch the output
to the steady modes, so the LED does not toggle at all;
``pca953x_get_brightness()`` reports that value back;
``pca953x_blink()`` lights a LED in pulses with a period in milliseconds and a
brightness on the same scale, and ``pca953x_get_blink()`` reports both.

The dimming runs at the shortest period the chip can produce, about 6.6 ms
or 152 Hz, which is far above the flicker fusion threshold, so a dimmed LED
looks steady rather than blinking. The two generators belong to the chip, not
to the outputs, so all dimmed
outputs share one brightness and all blinking outputs share one period and one
brightness, the last call winning for every output that uses the same
generator. Outputs that are switched on or off steadily do not depend on the
generators.

The registers remain available for the cases the simple functions do not
cover: the modes, the duty cycles, the prescalers and the pin levels belong to
the reference below.

 .. doxygenfile:: pca953x.h
