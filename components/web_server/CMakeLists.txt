idf_component_register(
    SRCS "web_server.c"
    INCLUDE_DIRS "include"
    REQUIRES system_status freertos esp_http_server esp_wifi json 
)
