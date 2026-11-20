#ifndef OLED_H
#define OLED_H

#include "esp_err.h"
#include "driver/i2c_master.h"

#define OLED_I2C_ADDR       0x3C
#define OLED_WIDTH          128
#define OLED_HEIGHT         64

/**
 * @brief OLED 초기화
 *
 * @param i2c_bus 기존에 생성한 I2C 버스 핸들
 */
esp_err_t oled_init(i2c_master_bus_handle_t i2c_bus);

/**
 * @brief OLED 화면 전체 지우기
 */
esp_err_t oled_clear(void);

/**
 * @brief OLED 테스트 패턴 출력
 */
esp_err_t oled_test_pattern(void);

#endif
