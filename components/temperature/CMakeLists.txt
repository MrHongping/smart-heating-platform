idf_component_register(
    SRCS "temperature.c"
    INCLUDE_DIRS "include"
    REQUIRES system_status freertos
)
