idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES temp_curve temperature pid ssr_control display web_server system_status safety potentiometer config)

# SPIFFS 配置
spiffs_create_partition_image(spiffs ../web FLASH_IN_PROJECT)
