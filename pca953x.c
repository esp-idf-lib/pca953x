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
 * @file pca953x.c
 *
 * ESP-IDF driver for PCA9530/PCA9533 I2C-bus LED dimmers
 *
 * Copyright (c) 2026 Ruslan V. Uss <unclerus@gmail.com>
 *
 * BSD Licensed as described in the file LICENSE
 */

#include <esp_idf_lib_helpers.h>
#include "pca953x.h"

#define I2C_FREQ_HZ 400000

#define REG_INPUT 0x00
#define REG_PSC0  0x01
#define REG_PWM0  0x02
#define REG_PSC1  0x03
#define REG_PWM1  0x04
#define REG_LS    0x05

#define RATES_COUNT 2
#define MODE_BITS   2
#define MODE_MASK   0x03

// The period of a rate is (prescaler + 1) / 152 seconds
#define BLINK_BASE_HZ       152
#define BLINK_MIN_PERIOD_MS 7
#define BLINK_MAX_PERIOD_MS 1684

#define CHECK(x) do { esp_err_t __; if ((__ = x) != ESP_OK) return __; } while (0)
#define CHECK_ARG(VAL) do { if (!(VAL)) return ESP_ERR_INVALID_ARG; } while (0)

static uint8_t leds_count(pca953x_type_t type)
{
    return type == PCA953X_TYPE_PCA9533 ? 4 : 2;
}

static bool led_valid(const pca953x_t *dev, uint8_t led)
{
    return led < leds_count(dev->type);
}

static uint8_t rate_reg(uint8_t rate)
{
    return rate ? REG_PSC1 : REG_PSC0;
}

static uint8_t pwm_reg(uint8_t rate)
{
    return rate ? REG_PWM1 : REG_PWM0;
}

static esp_err_t read_reg(pca953x_t *dev, uint8_t reg, uint8_t *val)
{
    CHECK_ARG(dev && val);

    I2C_DEV_TAKE_MUTEX(&dev->i2c_dev);
    I2C_DEV_CHECK(&dev->i2c_dev, i2c_dev_read_reg(&dev->i2c_dev, reg, val, 1));
    I2C_DEV_GIVE_MUTEX(&dev->i2c_dev);

    return ESP_OK;
}

static esp_err_t write_reg(pca953x_t *dev, uint8_t reg, uint8_t val)
{
    CHECK_ARG(dev);

    I2C_DEV_TAKE_MUTEX(&dev->i2c_dev);
    I2C_DEV_CHECK(&dev->i2c_dev, i2c_dev_write_reg(&dev->i2c_dev, reg, &val, 1));
    I2C_DEV_GIVE_MUTEX(&dev->i2c_dev);

    return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////

esp_err_t pca953x_init_desc(pca953x_t *dev, pca953x_type_t type, uint8_t addr, i2c_port_t port,
                            gpio_num_t sda_gpio, gpio_num_t scl_gpio)
{
    CHECK_ARG(dev && (type == PCA953X_TYPE_PCA9530 || type == PCA953X_TYPE_PCA9533)
              && (addr & 0x7c) == 0x60);

    dev->type = type;
    dev->i2c_dev.port = port;
    dev->i2c_dev.addr = addr;
    dev->i2c_dev.cfg.sda_io_num = sda_gpio;
    dev->i2c_dev.cfg.scl_io_num = scl_gpio;
#if HELPER_TARGET_IS_ESP32
    dev->i2c_dev.cfg.master.clk_speed = I2C_FREQ_HZ;
#endif

    return i2c_dev_create_mutex(&dev->i2c_dev);
}

esp_err_t pca953x_free_desc(pca953x_t *dev)
{
    CHECK_ARG(dev);

    return i2c_dev_delete_mutex(&dev->i2c_dev);
}

esp_err_t pca953x_get_mode(pca953x_t *dev, uint8_t led, pca953x_mode_t *mode)
{
    CHECK_ARG(dev && led_valid(dev, led) && mode);

    uint8_t ls;
    CHECK(read_reg(dev, REG_LS, &ls));
    *mode = (ls >> (led * MODE_BITS)) & MODE_MASK;

    return ESP_OK;
}

esp_err_t pca953x_set_mode(pca953x_t *dev, uint8_t led, pca953x_mode_t mode)
{
    CHECK_ARG(dev && led_valid(dev, led) && mode >= PCA953X_MODE_OFF && mode <= PCA953X_MODE_PWM1);

    uint8_t ls;

    I2C_DEV_TAKE_MUTEX(&dev->i2c_dev);
    I2C_DEV_CHECK(&dev->i2c_dev, i2c_dev_read_reg(&dev->i2c_dev, REG_LS, &ls, 1));
    ls = (ls & ~(MODE_MASK << (led * MODE_BITS))) | ((uint8_t)mode << (led * MODE_BITS));
    I2C_DEV_CHECK(&dev->i2c_dev, i2c_dev_write_reg(&dev->i2c_dev, REG_LS, &ls, 1));
    I2C_DEV_GIVE_MUTEX(&dev->i2c_dev);

    return ESP_OK;
}

esp_err_t pca953x_get_modes(pca953x_t *dev, uint8_t *ls)
{
    return read_reg(dev, REG_LS, ls);
}

esp_err_t pca953x_set_modes(pca953x_t *dev, uint8_t ls)
{
    return write_reg(dev, REG_LS, ls);
}

esp_err_t pca953x_get_prescaler(pca953x_t *dev, uint8_t rate, uint8_t *psc)
{
    CHECK_ARG(dev && rate < RATES_COUNT);

    return read_reg(dev, rate_reg(rate), psc);
}

esp_err_t pca953x_set_prescaler(pca953x_t *dev, uint8_t rate, uint8_t psc)
{
    CHECK_ARG(dev && rate < RATES_COUNT);

    return write_reg(dev, rate_reg(rate), psc);
}

esp_err_t pca953x_get_pwm(pca953x_t *dev, uint8_t rate, uint8_t *pwm)
{
    CHECK_ARG(dev && rate < RATES_COUNT);

    return read_reg(dev, pwm_reg(rate), pwm);
}

esp_err_t pca953x_set_pwm(pca953x_t *dev, uint8_t rate, uint8_t pwm)
{
    CHECK_ARG(dev && rate < RATES_COUNT);

    return write_reg(dev, pwm_reg(rate), pwm);
}

esp_err_t pca953x_set_blink_period(pca953x_t *dev, uint8_t rate, uint32_t period_ms)
{
    CHECK_ARG(dev && rate < RATES_COUNT);
    if (period_ms < BLINK_MIN_PERIOD_MS || period_ms > BLINK_MAX_PERIOD_MS)
        return ESP_ERR_INVALID_ARG;

    // Round the period up so that the blink is not faster than requested
    uint32_t psc = (period_ms * BLINK_BASE_HZ + 999) / 1000 - 1;

    return write_reg(dev, rate_reg(rate), psc);
}

esp_err_t pca953x_get_blink_period(pca953x_t *dev, uint8_t rate, uint32_t *period_ms)
{
    CHECK_ARG(dev && rate < RATES_COUNT && period_ms);

    uint8_t psc;
    CHECK(read_reg(dev, rate_reg(rate), &psc));
    *period_ms = ((uint32_t)psc + 1) * 1000 / BLINK_BASE_HZ;

    return ESP_OK;
}

esp_err_t pca953x_get_inputs(pca953x_t *dev, uint8_t *levels)
{
    return read_reg(dev, REG_INPUT, levels);
}

esp_err_t pca953x_get_input(pca953x_t *dev, uint8_t led, bool *level)
{
    CHECK_ARG(dev && led_valid(dev, led) && level);

    uint8_t inputs;
    CHECK(read_reg(dev, REG_INPUT, &inputs));
    *level = (inputs & BIT(led)) != 0;

    return ESP_OK;
}
