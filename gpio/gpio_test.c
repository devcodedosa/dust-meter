#include "gpio_test.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lib.h"

static const char *TAG = "GPIO_TEST";

/* LEDC 설정 */
#define BUZZER_LEDC_MODE       LEDC_LOW_SPEED_MODE
#define BUZZER_LEDC_CHANNEL    LEDC_CHANNEL_0
#define BUZZER_LEDC_TIMER      LEDC_TIMER_0
#define BUZZER_LEDC_DUTY_RES   LEDC_TIMER_10_BIT
#define BUZZER_LEDC_FREQ       2000

/* GPIO 초기화 */
esp_err_t gpio_test_init(void)
{
    gpio_config_t led_config = {
        .pin_bit_mask =
            (1ULL << GPIO_STATUS_LED) |
            (1ULL << GPIO_ERROR_LED),

        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t ret = gpio_config(&led_config);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LED GPIO initialization failed");
        return ret;
    }

    gpio_set_level(GPIO_STATUS_LED, 0);
    gpio_set_level(GPIO_ERROR_LED, 0);

    /* 부저 GPIO 초기화 */
    gpio_config_t buzzer_config = {
        .pin_bit_mask = (1ULL << GPIO_BUZZER),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ret = gpio_config(&buzzer_config);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Buzzer GPIO initialization failed");
        return ret;
    }

    /* LEDC 타이머 설정 */
    ledc_timer_config_t timer_config = {
        .speed_mode = BUZZER_LEDC_MODE,
        .duty_resolution = BUZZER_LEDC_DUTY_RES,
        .timer_num = BUZZER_LEDC_TIMER,
        .freq_hz = BUZZER_LEDC_FREQ,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ret = ledc_timer_config(&timer_config);

    if (ret != ESP_OK) {
        return ret;
    }

    /* LEDC 채널 설정 */
    ledc_channel_config_t channel_config = {
        .gpio_num = GPIO_BUZZER,
        .speed_mode = BUZZER_LEDC_MODE,
        .channel = BUZZER_LEDC_CHANNEL,
        .timer_sel = BUZZER_LEDC_TIMER,
        .duty = 0,
        .hpoint = 0,
    };

    ret = ledc_channel_config(&channel_config);

    if (ret != ESP_OK) {
        return ret;
    }

    ESP_LOGI(TAG, "GPIO test initialization completed");

    return ESP_OK;
}

/* LED 테스트 */
void gpio_test_led(void)
{
    ESP_LOGI(TAG, "LED TEST START");
    ESP_LOGI(TAG, "Press ENTER to exit");

    while (1) {

        /* 상태 LED ON */
        gpio_set_level(GPIO_STATUS_LED, 1);
        gpio_set_level(GPIO_ERROR_LED, 0);

        ESP_LOGI(TAG, "STATUS LED ON");

        if (wait_for_enter(1000)) {
            break;
        }

        /* 상태 LED OFF / 오류 LED ON */
        gpio_set_level(GPIO_STATUS_LED, 0);
        gpio_set_level(GPIO_ERROR_LED, 1);

        ESP_LOGI(TAG, "ERROR LED ON");

        if (wait_for_enter(1000)) {
            break;
        }

        /* 두 LED 모두 OFF */
        gpio_set_level(GPIO_ERROR_LED, 0);

        ESP_LOGI(TAG, "ALL LED OFF");

        if (wait_for_enter(1000)) {
            break;
        }
    }

    gpio_set_level(GPIO_STATUS_LED, 0);
    gpio_set_level(GPIO_ERROR_LED, 0);

    ESP_LOGI(TAG, "LED TEST END");
}

/* 부저 ON/OFF */
static esp_err_t buzzer_set_frequency(uint32_t frequency)
{
    if (frequency == 0) {
        return ledc_stop(
            BUZZER_LEDC_MODE,
            BUZZER_LEDC_CHANNEL,
            0
        );
    }

    esp_err_t ret = ledc_set_freq(
        BUZZER_LEDC_MODE,
        BUZZER_LEDC_TIMER,
        frequency
    );

    if (ret != ESP_OK) {
        return ret;
    }

    ret = ledc_set_duty(
        BUZZER_LEDC_MODE,
        BUZZER_LEDC_CHANNEL,
        512
    );

    if (ret != ESP_OK) {
        return ret;
    }

    return ledc_update_duty(
        BUZZER_LEDC_MODE,
        BUZZER_LEDC_CHANNEL
    );
}

/* 부저 테스트 */
void gpio_test_buzzer(void)
{
    ESP_LOGI(TAG, "BUZZER TEST START");
    ESP_LOGI(TAG, "Press ENTER to exit");

    const uint32_t frequencies[] = {
        1000,
        1500,
        2000,
        2500,
        3000
    };

    int index = 0;

    while (1) {

        uint32_t freq = frequencies[index];

        ESP_LOGI(TAG, "Buzzer frequency: %lu Hz",
                 (unsigned long)freq);

        buzzer_set_frequency(freq);

        if (wait_for_enter(1000)) {
            break;
        }

        buzzer_set_frequency(0);

        if (wait_for_enter(500)) {
            break;
        }

        index++;

        if (index >= sizeof(frequencies) / sizeof(frequencies[0])) {
            index = 0;
        }
    }

    buzzer_set_frequency(0);

    ESP_LOGI(TAG, "BUZZER TEST END");
}

/* 전체 테스트 */
void gpio_test_all(void)
{
    gpio_test_led();
    gpio_test_buzzer();
}
