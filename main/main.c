#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "i2c_scan.h"
#include "sht30.h"
#include "oled.h"

static const char *TAG = "BOARD_TEST";

#define I2C_SDA_GPIO   8
#define I2C_SCL_GPIO   9

/* Test function declarations */
static void test_gpio(void);
static void test_i2c(void);
static void test_oled(void);
static void test_sht30(void);
static void test_d9(void);

static i2c_master_bus_handle_t i2c_bus;
sht30_t sensor;

static esp_err_t i2c_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_SDA_GPIO,
        .scl_io_num = I2C_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false,
    };

    return i2c_new_master_bus(&bus_config, &i2c_bus);
}

/*
 * 2초 동안 키 입력 확인
 * Enter 입력 시 true 반환
 * 입력이 없으면 false 반환
 */
static bool wait_for_enter(uint32_t timeout_ms)
{
    uint32_t elapsed = 0;

    while (elapsed < timeout_ms) {

        int ch = getchar();

        if (ch != EOF) {
            if (ch == '\r' || ch == '\n') {
                return true;
            }
        } else {
            /* STDIN EOF 상태 초기화 */
            clearerr(stdin);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
        elapsed += 10;
    }

    return false;
}

/* Wait for USB Serial/JTAG input */
static int console_getchar(void)
{
    while (1)
    {
        int ch = getchar();

        if (ch != EOF)
        {
            return ch;
        }

        /* Clear EOF state after non-blocking read */
        clearerr(stdin);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* Display main menu */
static void show_menu(void)
{
    printf("\n");
    printf("======================================\n");
    printf("    ESP32-S3 Dust Meter BOARD TEST\n");
    printf("======================================\n");
    printf("\n");

    printf("  1. GPIO TEST\n");
    printf("  2. I2C SCAN\n");
    printf("  3. OLED TEST\n");
    printf("  4. SHT30 TEST\n");
    printf("  5. D9 SENSOR TEST\n");
    printf("\n");

    printf("  0. EXIT / MAIN MENU\n");
    printf("\n");

    printf("Select Menu: ");
    fflush(stdout);
}

/* GPIO test */
static void test_gpio(void)
{
    ESP_LOGI(TAG, "GPIO TEST");
    printf("GPIO test is not implemented yet.\n");
}

/* I2C test */
static void test_i2c(void)
{
    ESP_LOGI(TAG, "I2C SCAN");
    i2c_scan_test(i2c_bus);
    //printf("I2C scan is not implemented yet.\n");
}

/* OLED test */
static void test_oled(void)
{
    ESP_LOGI(TAG, "OLED TEST");
    //printf("OLED test is not implemented yet.\n");
         
    /* OLED 테스트 패턴 출력 */
    ESP_ERROR_CHECK(oled_test_pattern());

}

/* SHT30 test */
static void test_sht30(void)
{
    ESP_LOGI(TAG, "SHT30 TEST");
    ESP_LOGI(TAG, "Press ENTER to exit.");

    esp_err_t ret; 

    while (1) {

        float temperature = 0.0f;
        float humidity = 0.0f;

        ret = sht30_read(
            &sensor,
            &temperature,
            &humidity
        );

        if (ret == ESP_OK) {
            ESP_LOGI(TAG,
                     "T: %.2f C / H: %.2f %%RH",
                     temperature,
                     humidity);
        } else {
            ESP_LOGW(TAG, "SHT30 read failed");
        }

        /* 2초 동안 대기하면서 Enter 입력 확인 */
        if (wait_for_enter(2000)) {
            ESP_LOGI(TAG, "Leaving SHT30 test");
            break;
        }
    }
}

/* D9 sensor test */
static void test_d9(void)
{
    ESP_LOGI(TAG, "D9 SENSOR TEST");
    printf("D9 sensor test is not implemented yet.\n");
}

/* Menu handler */
static void process_menu(int selection)
{
    switch (selection)
    {
        case 1:
            test_gpio();
            break;

        case 2:
            test_i2c();
            break;

        case 3:
            test_oled();
            break;

        case 4:
            test_sht30();
            break;

        case 5:
            test_d9();
            break;

        case 0:
            printf("Returning to main menu...\n");
            break;

        default:
            printf("Invalid selection.\n");
            break;
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Board Test Firmware Started");

    /* I2C 초기화 */
    ESP_ERROR_CHECK(i2c_init());

    /* OLED 초기화 */
    ESP_ERROR_CHECK(oled_init(i2c_bus));

    /* SHT30 초기화 */
    ESP_ERROR_CHECK(sht30_init(
        i2c_bus,
        &sensor,
        SHT30_I2C_ADDR_DEFAULT
    ));

    /*
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SHT30 initialization failed");
        return;
    }*/
    

    while (1)
    {
        show_menu();

        /* Wait for user input */
        int ch = console_getchar();

        /* Ignore Enter */
        if (ch == '\r' || ch == '\n')
        {
            break;
        }

        printf("%c\n", ch);

        if (ch >= '0' && ch <= '5')
        {
            process_menu(ch - '0');
        }
        else
        {
            printf("Invalid input. Please select 0~5.\n");
        }

        vTaskDelay(pdMS_TO_TICKS(300));
    }
}
