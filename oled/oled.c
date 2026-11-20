#include "oled.h"

#include <string.h>
#include "esp_log.h"

static const char *TAG = "OLED";

static i2c_master_dev_handle_t oled_dev;

/* OLED 명령 전송 */
static esp_err_t oled_write_command(const uint8_t *commands, size_t length)
{
    uint8_t buffer[32];

    if (length + 1 > sizeof(buffer)) {
        return ESP_ERR_INVALID_SIZE;
    }

    buffer[0] = 0x00;  // Command control byte
    memcpy(&buffer[1], commands, length);

    return i2c_master_transmit(oled_dev, buffer, length + 1, 1000);
}

/* OLED 데이터 전송 */
static esp_err_t oled_write_data(const uint8_t *data, size_t length)
{
    uint8_t buffer[129];

    if (length > 128) {
        return ESP_ERR_INVALID_SIZE;
    }

    buffer[0] = 0x40;  // Data control byte
    memcpy(&buffer[1], data, length);

    return i2c_master_transmit(oled_dev, buffer, length + 1, 1000);
}

/* OLED 초기화 */
esp_err_t oled_init(i2c_master_bus_handle_t i2c_bus)
{
    esp_err_t ret;

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = OLED_I2C_ADDR,
        .scl_speed_hz = 400000,
    };

    ret = i2c_master_bus_add_device(i2c_bus, &dev_config, &oled_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add I2C device: %s",
                 esp_err_to_name(ret));
        return ret;
    }

    /* SSD1306 초기화 명령 */
    const uint8_t init_commands[] = {
        0xAE,       // Display OFF
        0xD5, 0x80, // Clock divide
        0xA8, 0x3F, // Multiplex ratio: 64
        0xD3, 0x00, // Display offset
        0x40,       // Start line
        0x8D, 0x14, // Charge pump ON
        0x20, 0x00, // Horizontal addressing mode
        0xA1,       // Segment remap
        0xC8,       // COM scan direction
        0xDA, 0x12, // COM pins
        0x81, 0x7F, // Contrast
        0xD9, 0xF1, // Pre-charge period
        0xDB, 0x40, // VCOMH
        0xA4,       // Display follows RAM
        0xA6,       // Normal display
        0xAF        // Display ON
    };

    ret = oled_write_command(init_commands, sizeof(init_commands));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "OLED initialization failed");
        return ret;
    }

    ESP_LOGI(TAG, "OLED initialized (0x%02X)", OLED_I2C_ADDR);

    return oled_clear();
}

/* 화면 전체 지우기 */
esp_err_t oled_clear(void)
{
    const uint8_t zero_data[OLED_WIDTH] = {0};

    /* Horizontal addressing mode */
    const uint8_t set_address[] = {
        0x21, 0x00, OLED_WIDTH - 1,
        0x22, 0x00, (OLED_HEIGHT / 8) - 1
    };

    esp_err_t ret = oled_write_command(set_address, sizeof(set_address));
    if (ret != ESP_OK) {
        return ret;
    }

    for (int page = 0; page < OLED_HEIGHT / 8; page++) {
        ret = oled_write_data(zero_data, sizeof(zero_data));
        if (ret != ESP_OK) {
            return ret;
        }
    }

    return ESP_OK;
}

/* 화면 테스트 패턴 */
esp_err_t oled_test_pattern(void)
{
    uint8_t data[OLED_WIDTH];

    const uint8_t set_address[] = {
        0x21, 0x00, OLED_WIDTH - 1,
        0x22, 0x00, (OLED_HEIGHT / 8) - 1
    };

    esp_err_t ret = oled_write_command(set_address, sizeof(set_address));
    if (ret != ESP_OK) {
        return ret;
    }

    for (int page = 0; page < OLED_HEIGHT / 8; page++) {

        for (int x = 0; x < OLED_WIDTH; x++) {
            if (page % 2 == 0) {
                data[x] = (x % 2 == 0) ? 0xAA : 0x55;
            } else {
                data[x] = 0xFF;
            }
        }

        ret = oled_write_data(data, sizeof(data));
        if (ret != ESP_OK) {
            return ret;
        }
    }

    ESP_LOGI(TAG, "OLED test pattern displayed");

    return ESP_OK;
}
