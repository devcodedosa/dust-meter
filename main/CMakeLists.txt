idf_component_register(
    SRCS 
        "main.c"
        "lib.c"
        "../i2c/i2c_scan.c"
        "../sht30/sht30.c"
        "../oled/oled.c"
        "../gpio/gpio_test.c"
                    
    INCLUDE_DIRS 
        "."
        "../i2c/"
        "../sht30/"
        "../oled/"
        "../gpio/"

    REQUIRES
        esp_driver_i2c
        esp_driver_gpio
        esp_driver_ledc
        
)
