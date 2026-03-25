idf_component_register(
    SRCS "potentiometer.c"
    INCLUDE_DIRS "include"
    REQUIRES system_status freertos config
)
