/*
 * Copyright (c) 2026 Ruslan V. Uss <unclerus@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of itscontributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file pca953x.h
 *
 * ESP-IDF driver for PCA9530/PCA9533 I2C-bus LED dimmers
 *
 * Copyright (c) 2026 Ruslan V. Uss <unclerus@gmail.com>
 *
 * BSD Licensed as described in the file LICENSE
 */
#ifndef __PCA953X_H__
#define __PCA953X_H__

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <i2cdev.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Device type
 *
 * The two devices share the register map and differ in the number of LED
 * outputs: the PCA9530 has two, the PCA9533 has four.
 */
typedef enum
{
    PCA953X_TYPE_PCA9530 = 0, //!< PCA9530, 2 LED outputs
    PCA953X_TYPE_PCA9533,     //!< PCA9533, 4 LED outputs
} pca953x_type_t;

/**
 * @brief I2C addresses
 *
 * The PCA9530 has one hardware address pin (`A0`), and the address depends
 * on its level. The PCA9533/01 and PCA9533/02 versions have fixed addresses.
 * The values are 7-bit addresses, which is what the I2C driver expects; the
 * datasheets show them as 8-bit write addresses with the extra zero bit, so
 * `0xc0`, `0xc2`, `0xc4` and `0xc6` are the same addresses multiplied by two.
 */
#define PCA953X_I2C_ADDR_PCA9530_A0_LOW  0x60 //!< PCA9530, `A0` connected to GND
#define PCA953X_I2C_ADDR_PCA9530_A0_HIGH 0x61 //!< PCA9530, `A0` connected to VDD
#define PCA953X_I2C_ADDR_PCA9533_01      0x62 //!< PCA9533/01
#define PCA953X_I2C_ADDR_PCA9533_02      0x63 //!< PCA9533/02

/**
 * @brief Fully lit brightness
 *
 * The duty cycle register holds 8 bits, so the pulses run from 0 to 255, and
 * 256 is the extreme that leaves no low phase at all, which means the output
 * is steady. The brightness functions accept the whole range, from 0, dark,
 * to this value, fully lit.
 */
#define PCA953X_BRIGHTNESS_MAX 256

/**
 * @brief LED output modes
 *
 * The values match the bits of the LED selector register.
 */
typedef enum
{
    PCA953X_MODE_OFF  = 0, //!< Output is high-impedance, the LED is off, the pin can be read as an input
    PCA953X_MODE_ON   = 1, //!< Output is driven low, the LED is on
    PCA953X_MODE_PWM0 = 2, //!< Output is modulated with the rate and the duty cycle of PWM0
    PCA953X_MODE_PWM1 = 3, //!< Output is modulated with the rate and the duty cycle of PWM1
} pca953x_mode_t;

/**
 * @brief Device descriptor
 */
typedef struct
{
    i2c_dev_t i2c_dev;   //!< I2C device descriptor
    pca953x_type_t type; //!< Device type
} pca953x_t;

/**
 * @brief Initialize device descriptor
 *
 * Default SCL frequency is 400 kHz.
 *
 * @param dev       Pointer to the device descriptor
 * @param type      Device type
 * @param addr      I2C address, one of `PCA953X_I2C_ADDR_*`
 * @param port      I2C port number
 * @param sda_gpio  SDA GPIO
 * @param scl_gpio  SCL GPIO
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_ARG` if an argument is invalid
 */
esp_err_t pca953x_init_desc(pca953x_t *dev, pca953x_type_t type, uint8_t addr, i2c_port_t port,
                            gpio_num_t sda_gpio, gpio_num_t scl_gpio);

/**
 * @brief Free device descriptor
 *
 * @param dev  Pointer to the device descriptor
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_ARG` if the descriptor is NULL
 */
esp_err_t pca953x_free_desc(pca953x_t *dev);

/**
 * @brief Get the mode of an LED output
 *
 * @param dev    Pointer to the device descriptor
 * @param led    LED number, 0..1 for PCA9530, 0..3 for PCA9533
 * @param[out] mode  Output mode
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_ARG` if an argument is invalid
 */
esp_err_t pca953x_get_mode(pca953x_t *dev, uint8_t led, pca953x_mode_t *mode);

/**
 * @brief Set the mode of an LED output
 *
 * To use a pin as a GPIO input, set its mode to `PCA953X_MODE_OFF` and read
 * it with pca953x_get_input().
 *
 * @param dev   Pointer to the device descriptor
 * @param led   LED number, 0..1 for PCA9530, 0..3 for PCA9533
 * @param mode  Output mode
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_ARG` if an argument is invalid
 */
esp_err_t pca953x_set_mode(pca953x_t *dev, uint8_t led, pca953x_mode_t mode);

/**
 * @brief Get the LED selector register
 *
 * The register holds two bits per LED output, in the same order as
 * pca953x_set_mode() stores them.
 *
 * @param dev        Pointer to the device descriptor
 * @param[out] ls    LED selector value
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_ARG` if an argument is invalid
 */
esp_err_t pca953x_get_modes(pca953x_t *dev, uint8_t *ls);

/**
 * @brief Set the LED selector register
 *
 * The reserved bits are written as they are passed, so it is safer to change
 * the outputs one by one with pca953x_set_mode().
 *
 * @param dev  Pointer to the device descriptor
 * @param ls   LED selector value
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_ARG` if an argument is invalid
 */
esp_err_t pca953x_set_modes(pca953x_t *dev, uint8_t ls);

/**
 * @brief Get the prescaler of a PWM rate
 *
 * The period of `rate` is `(prescaler + 1) / 152` seconds, so the frequency
 * is between 152 Hz and 0.59 Hz.
 *
 * @param dev        Pointer to the device descriptor
 * @param rate       PWM rate, 0 or 1
 * @param[out] psc   Prescaler value
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_ARG` if an argument is invalid
 */
esp_err_t pca953x_get_prescaler(pca953x_t *dev, uint8_t rate, uint8_t *psc);

/**
 * @brief Set the prescaler of a PWM rate
 *
 * @param dev   Pointer to the device descriptor
 * @param rate  PWM rate, 0 or 1
 * @param psc   Prescaler value
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_ARG` if an argument is invalid
 */
esp_err_t pca953x_set_prescaler(pca953x_t *dev, uint8_t rate, uint8_t psc);

/**
 * @brief Get the duty cycle of a PWM rate
 *
 * @param dev        Pointer to the device descriptor
 * @param rate       PWM rate, 0 or 1
 * @param[out] pwm   Duty cycle, 0..255
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_ARG` if an argument is invalid
 */
esp_err_t pca953x_get_pwm(pca953x_t *dev, uint8_t rate, uint8_t *pwm);

/**
 * @brief Set the duty cycle of a PWM rate
 *
 * The duty cycle is `pwm / 256`. When the value is 0, the output is always
 * high and the LED is off. When the period is short enough, above roughly
 * 100 Hz, the LED looks dimmed instead of blinking.
 *
 * @param dev   Pointer to the device descriptor
 * @param rate  PWM rate, 0 or 1
 * @param pwm   Duty cycle, 0..255
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_ARG` if an argument is invalid
 */
esp_err_t pca953x_set_pwm(pca953x_t *dev, uint8_t rate, uint8_t pwm);

/**
 * @brief Set the period of a PWM rate in milliseconds
 *
 * The period is rounded up to the closest value the prescaler can produce,
 * between 7 ms and 1684 ms. The function changes the prescaler only, the
 * duty cycle is set with pca953x_set_pwm().
 *
 * @param dev        Pointer to the device descriptor
 * @param rate       PWM rate, 0 or 1
 * @param period_ms  Requested period, milliseconds
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_ARG` if an argument is invalid or the period is
 *         out of range
 */
esp_err_t pca953x_set_blink_period(pca953x_t *dev, uint8_t rate, uint32_t period_ms);

/**
 * @brief Get the period of a PWM rate in milliseconds
 *
 * @param dev              Pointer to the device descriptor
 * @param rate             PWM rate, 0 or 1
 * @param[out] period_ms   Period, milliseconds
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_ARG` if an argument is invalid
 */
esp_err_t pca953x_get_blink_period(pca953x_t *dev, uint8_t rate, uint32_t *period_ms);

/**
 * @brief Set the brightness of a LED
 *
 * This is the simple way to light a LED: the brightness goes from 0 to
 * `PCA953X_BRIGHTNESS_MAX`, that is, from dark to fully lit. The extremes are
 * absolute and switch the output to the steady modes, so 0 turns the LED off
 * and 256 turns it on, without any toggling. The values in between are the
 * duty cycle of the dimming generator, from 1/256 to 255/256 of the period.
 *
 * No prescaler and no generator has to be chosen: the driver uses the fast
 * generator and sets its period to the shortest one, about 6.6 ms or 152 Hz,
 * which is far above the flicker fusion threshold, so the LED looks steady
 * rather than blinking.
 *
 * The chip has two duty cycle registers for all its outputs, so the outputs
 * that are dimmed at the same time share the brightness: the last call wins
 * for all of them. Outputs that are switched on or off steadily do not depend
 * on the registers and can be used freely.
 *
 * @param dev         Pointer to the device descriptor
 * @param led         LED number, 0..1 for PCA9530, 0..3 for PCA9533
 * @param brightness  Brightness, 0 to `PCA953X_BRIGHTNESS_MAX`
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_ARG` if an argument is invalid
 */
esp_err_t pca953x_set_brightness(pca953x_t *dev, uint8_t led, uint16_t brightness);

/**
 * @brief Get the brightness of a LED
 *
 * A LED that is switched off or on steadily reports 0 or
 * `PCA953X_BRIGHTNESS_MAX`, a LED that blinks reports the brightness of the
 * pulses.
 *
 * @param dev             Pointer to the device descriptor
 * @param led             LED number, 0..1 for PCA9530, 0..3 for PCA9533
 * @param[out] brightness Brightness, 0 to `PCA953X_BRIGHTNESS_MAX`
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_ARG` if an argument is invalid
 */
esp_err_t pca953x_get_brightness(pca953x_t *dev, uint8_t led, uint16_t *brightness);

/**
 * @brief Blink a LED
 *
 * The function sets the period and the brightness of the pulses and switches
 * the LED to the slow generator, so one call is enough. The period is rounded
 * up to the closest value the prescaler can produce, between 7 ms and
 * 1684 ms.
 *
 * The brightness follows the same scale as pca953x_set_brightness(): 0 turns
 * the LED off and `PCA953X_BRIGHTNESS_MAX` turns it on steadily, because a
 * pulse that never goes low is not a pulse. The values in between blink.
 *
 * As with pca953x_set_brightness(), the blinking outputs share one period and
 * one brightness, because the chip has a single slow generator.
 *
 * @param dev         Pointer to the device descriptor
 * @param led         LED number, 0..1 for PCA9530, 0..3 for PCA9533
 * @param period_ms   Period of the blinking, 7 to 1684 ms
 * @param brightness  Brightness of the pulses, 0 to `PCA953X_BRIGHTNESS_MAX`
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_ARG` if an argument is invalid
 */
esp_err_t pca953x_blink(pca953x_t *dev, uint8_t led, uint32_t period_ms, uint16_t brightness);

/**
 * @brief Get the period and the brightness of a blinking LED
 *
 * @param dev              Pointer to the device descriptor
 * @param led              LED number, 0..1 for PCA9530, 0..3 for PCA9533
 * @param[out] period_ms   Period of the blinking, milliseconds
 * @param[out] brightness  Brightness of the pulses, 0 to `PCA953X_BRIGHTNESS_MAX`
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_STATE` if the LED is not blinking
 *         `ESP_ERR_INVALID_ARG` if an argument is invalid
 */
esp_err_t pca953x_get_blink(pca953x_t *dev, uint8_t led, uint32_t *period_ms, uint16_t *brightness);

/**
 * @brief Read the state of all LED pins
 *
 * The input register reflects the levels of the pins, so it is useful for
 * the pins that are set to `PCA953X_MODE_OFF` and used as GPIO inputs.
 *
 * @param dev           Pointer to the device descriptor
 * @param[out] levels   Pin levels, bit `n` is the level of LED `n`, 1 is high
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_ARG` if an argument is invalid
 */
esp_err_t pca953x_get_inputs(pca953x_t *dev, uint8_t *levels);

/**
 * @brief Read the state of a LED pin
 *
 * @param dev         Pointer to the device descriptor
 * @param led         LED number, 0..1 for PCA9530, 0..3 for PCA9533
 * @param[out] level  `true` if the pin is high
 *
 * @return `ESP_OK` on success
 *         `ESP_ERR_INVALID_ARG` if an argument is invalid
 */
esp_err_t pca953x_get_input(pca953x_t *dev, uint8_t led, bool *level);

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* __PCA953X_H__ */
