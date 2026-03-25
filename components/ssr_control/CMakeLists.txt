idf_component_register(
    SRCS "ssr_control.c"
    INCLUDE_DIRS "include"
    REQUIRES system_status freertos config
)
