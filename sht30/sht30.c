#include "sht30.h"

#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SHT30";

#define SHT30_CMD_MEASURE_HIGH_REP    0x2400
#define SHT30_MEASURE_DELAY_MS        20
#define SHT30_TIMEOUT_MS              1000

/**
 * @brief CRC-8 계산
 *
 * Polynomial: 0x31
 * Initial value: 0xFF
 */
static uint8_t sht30_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFF;

    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];

        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

esp_err_t sht30_init(
    i2c_master_bus_handle_t bus_handle,
    sht30_t *sht30,
    uint8_t address)
{
    if (bus_handle == NULL || sht30 == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (address != SHT30_I2C_ADDR_DEFAULT &&
        address != SHT30_I2C_ADDR_ALT) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(sht30, 0, sizeof(sht30_t));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = 100000,
    };

    esp_err_t ret = i2c_master_bus_add_device(
        bus_handle,
        &dev_cfg,
        &sht30->dev_handle
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add I2C device: %s",
                 esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "SHT30 initialized (address: 0x%02X)", address);

    return ESP_OK;
}

esp_err_t sht30_read(
    sht30_t *sht30,
    float *temperature,
    float *humidity)
{
    if (sht30 == NULL ||
        sht30->dev_handle == NULL ||
        temperature == NULL ||
        humidity == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // 1. 측정 명령 전송
    uint8_t command[2] = {
        0x24,
        0x00
    };

    esp_err_t ret = i2c_master_transmit(
        sht30->dev_handle,
        command,
        sizeof(command),
        SHT30_TIMEOUT_MS
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Measurement command failed: %s",
                 esp_err_to_name(ret));
        return ret;
    }

    // 2. 센서 측정 대기
    vTaskDelay(pdMS_TO_TICKS(SHT30_MEASURE_DELAY_MS));

    // 3. 6바이트 데이터 수신
    uint8_t data[6] = {0};

    ret = i2c_master_receive(
        sht30->dev_handle,
        data,
        sizeof(data),
        SHT30_TIMEOUT_MS
    );

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Data receive failed: %s",
                 esp_err_to_name(ret));
        return ret;
    }

    // 4. 온도 CRC 검사
    uint8_t temp_crc = sht30_crc8(data, 2);

    if (temp_crc != data[2]) {
        ESP_LOGE(TAG, "Temperature CRC error");
        return ESP_FAIL;
    }

    // 5. 습도 CRC 검사
    uint8_t humidity_crc = sht30_crc8(&data[3], 2);

    if (humidity_crc != data[5]) {
        ESP_LOGE(TAG, "Humidity CRC error");
        return ESP_FAIL;
    }

    // 6. Raw 데이터 변환
    uint16_t raw_temp =
        ((uint16_t)data[0] << 8) | data[1];

    uint16_t raw_humidity =
        ((uint16_t)data[3] << 8) | data[4];

    
    // Debug Codes
    ESP_LOGI(TAG,
         "RAW DATA: T=0x%04X (%u), H=0x%04X (%u)",
         raw_temp,
         raw_temp,
         raw_humidity,
         raw_humidity);

    ESP_LOG_BUFFER_HEX(TAG, data, sizeof(data));


    // 7. 실제 측정값 계산
    *temperature =
        -45.0f + (175.0f * raw_temp / 65535.0f);

    *humidity =
        100.0f * raw_humidity / 65535.0f;

    ESP_LOGI(TAG,
             "Temperature: %.2f C, Humidity: %.2f %%RH",
             *temperature,
             *humidity);

    return ESP_OK;
}

esp_err_t sht30_deinit(sht30_t *sht30)
{
    if (sht30 == NULL || sht30->dev_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = i2c_master_bus_rm_device(
        sht30->dev_handle
    );

    if (ret == ESP_OK) {
        sht30->dev_handle = NULL;
        ESP_LOGI(TAG, "SHT30 device removed");
    }

    return ret;
}
