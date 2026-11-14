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

The driver provides access to the modes, the duty cycles, the prescalers and
the pin levels, see the reference below.

 .. doxygenfile:: pca953x.h
