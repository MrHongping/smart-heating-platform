idf_component_register(
    SRCS "safety.c"
    INCLUDE_DIRS "include"
    REQUIRES system_status freertos
)
