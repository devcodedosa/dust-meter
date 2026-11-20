#ifndef GPIO_TEST_H
#define GPIO_TEST_H

#include "esp_err.h"

/* GPIO 핀 정의 */
#define GPIO_STATUS_LED    1
#define GPIO_ERROR_LED     2
#define GPIO_BUZZER        3

/* 초기화 */
esp_err_t gpio_test_init(void);

/* LED 테스트 */
void gpio_test_led(void);

/* 부저 테스트 */
void gpio_test_buzzer(void);

/* 전체 테스트 */
void gpio_test_all(void);

#endif
