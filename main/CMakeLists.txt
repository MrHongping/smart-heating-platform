idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES system_status temperature pid ssr_control display web_server safety
)
