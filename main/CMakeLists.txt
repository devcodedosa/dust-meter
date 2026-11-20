idf_component_register(
    SRCS 
        "main.c"
        "../i2c/i2c_scan.c"
        "../sht30/sht30.c"
        "../oled/oled.c"
                    
    INCLUDE_DIRS 
        "."
        "../i2c/"
        "../sht30/"
        "../oled/"

    REQUIRES
        esp_driver_i2c
        
)
