
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_err.h"

#include "i2c_scan.h"

/* GPIO configuration */
#define I2C_MASTER_SCL_IO       9
#define I2C_MASTER_SDA_IO       8

#define I2C_MASTER_FREQ_HZ      100000

static const char *TAG = "I2C_SCAN";

void i2c_scan_test(i2c_master_bus_handle_t bus_handle)
{
    printf("\n");
    printf("================================\n");
    printf("          I2C SCANNER\n");
    printf("================================\n\n");


    printf("Scanning I2C bus...\n\n");

    esp_err_t ret;
    int found_count = 0;

    /* Scan I2C address range */
    for (uint8_t addr = 0x08; addr <= 0x77; addr++)
    {
        ret = i2c_master_probe(
            bus_handle,
            addr,
            100
        );

        if (ret == ESP_OK)
        {
            printf("[FOUND] Device address: 0x%02X\n", addr);
            found_count++;
        }

        int ch = getchar();

        if (ch == '\n' || ch == '\r')
        {
            break;
        }

        if (ch == EOF)
        {
            clearerr(stdin);
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        //vTaskDelay(pdMS_TO_TICKS(5));
    }

    printf("\n");
    printf("--------------------------------\n");
    printf("Scan completed.\n");
    printf("Devices found: %d\n", found_count);
    printf("--------------------------------\n");

    

    /* Release I2C bus */
#ifdef REMOVE_I2C
    ret = i2c_del_master_bus(bus_handle);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C bus release failed: %s",
                 esp_err_to_name(ret));
    }
#endif
}
