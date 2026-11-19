idf_component_register(
    SRCS 
        "main.c"
        "../i2c/i2c_scan.c"
                    
    INCLUDE_DIRS 
        "."
        "../i2c/"

    REQUIRES
        esp_driver_i2c
        
)
