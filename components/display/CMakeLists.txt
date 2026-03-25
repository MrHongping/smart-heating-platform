idf_component_register(
    SRCS "display.c"
    INCLUDE_DIRS "include"
    REQUIRES system_status freertos driver config
)
