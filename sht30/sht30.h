#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHT30_I2C_ADDR_DEFAULT    0x44
#define SHT30_I2C_ADDR_ALT        0x45

typedef struct {
    i2c_master_dev_handle_t dev_handle;
} sht30_t;

/**
 * @brief SHT30 초기화 및 I2C 장치 등록
 *
 * @param bus_handle 이미 초기화된 I2C Master Bus
 * @param sht30      SHT30 객체
 * @param address    I2C 주소 (0x44 또는 0x45)
 */
esp_err_t sht30_init(
    i2c_master_bus_handle_t bus_handle,
    sht30_t *sht30,
    uint8_t address
);

/**
 * @brief 온도 및 습도 측정
 *
 * @param sht30       SHT30 객체
 * @param temperature 온도 (°C)
 * @param humidity    상대습도 (%RH)
 */
esp_err_t sht30_read(
    sht30_t *sht30,
    float *temperature,
    float *humidity
);

/**
 * @brief SHT30 장치 제거
 */
esp_err_t sht30_deinit(sht30_t *sht30);

#ifdef __cplusplus
}
#endif
