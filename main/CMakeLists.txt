idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS ".")

# SPIFFS 配置
spiffs_create_partition_image(spiffs ../web FLASH_IN_PROJECT)