#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <pca953x.h>
#include <esp_err.h>
#include <esp_log.h>

static const char *TAG = "pca953x_example";

#ifdef CONFIG_EXAMPLE_TYPE_PCA9533
static const pca953x_type_t DEVICE_TYPE = PCA953X_TYPE_PCA9533;
static const uint8_t LEDS_COUNT = 4;
#else
static const pca953x_type_t DEVICE_TYPE = PCA953X_TYPE_PCA9530;
static const uint8_t LEDS_COUNT = 2;
#endif

static pca953x_t dev;

static const char *mode_name(pca953x_mode_t mode)
{
    switch (mode)
    {
        case PCA953X_MODE_OFF:
            return "off";
        case PCA953X_MODE_ON:
            return "on";
        case PCA953X_MODE_PWM0:
            return "PWM0";
        case PCA953X_MODE_PWM1:
            return "PWM1";
    }
    return "unknown";
}

void test(void *pvParameters)
{
    ESP_ERROR_CHECK(pca953x_init_desc(&dev, DEVICE_TYPE, CONFIG_EXAMPLE_I2C_ADDR, 0,
                                      CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));

    // Rate 0: a short period, about 152 Hz, so the output looks dimmed
    // instead of blinking. Rate 1: half a second period, so the output blinks.
    ESP_ERROR_CHECK(pca953x_set_blink_period(&dev, 0, 7));
    ESP_ERROR_CHECK(pca953x_set_pwm(&dev, 0, 64));  // 25% duty cycle

    ESP_ERROR_CHECK(pca953x_set_blink_period(&dev, 1, 500));
    ESP_ERROR_CHECK(pca953x_set_pwm(&dev, 1, 128)); // 50% duty cycle

    // The first output is dimmed, the second one blinks
    ESP_ERROR_CHECK(pca953x_set_mode(&dev, 0, PCA953X_MODE_PWM0));
    ESP_ERROR_CHECK(pca953x_set_mode(&dev, 1, PCA953X_MODE_PWM1));

    // The remaining outputs of the PCA9533 are driven steady
    if (LEDS_COUNT > 2)
    {
        ESP_ERROR_CHECK(pca953x_set_mode(&dev, 2, PCA953X_MODE_ON));
        ESP_ERROR_CHECK(pca953x_set_mode(&dev, 3, PCA953X_MODE_OFF));
    }

    while (1)
    {
        uint8_t levels = 0;
        esp_err_t res = pca953x_get_inputs(&dev, &levels);
        if (res != ESP_OK)
        {
            ESP_LOGE(TAG, "Could not read the inputs: %s", esp_err_to_name(res));
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        for (uint8_t led = 0; led < LEDS_COUNT; led++)
        {
            pca953x_mode_t mode;
            bool level = false;
            if (pca953x_get_mode(&dev, led, &mode) == ESP_OK && pca953x_get_input(&dev, led, &level) == ESP_OK)
                ESP_LOGI(TAG, "LED%u: %s, pin is %s", led, mode_name(mode), level ? "high" : "low");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main()
{
    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreate(test, "test", configMINIMAL_STACK_SIZE * 6, NULL, 5, NULL);
}
