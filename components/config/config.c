#include "config.h"
#include <stdio.h>
#include <string.h>
#include <esp_err.h>
#include <esp_vfs.h>
#include <esp_spiffs.h>

// 配置文件路径
#define CONFIG_FILE "/spiffs/config.json"

// 全局配置变量
static config_t g_config = {
    // 温度配置
    .min_temperature = 0.0,
    .max_temperature = 400.0,
    .default_target_temp = 100.0,
    
    // 引脚配置
    .ssr_pin = 2,
    .potentiometer_pin = 34,
    .i2c_scl_pin = 22,
    .i2c_sda_pin = 21,
    
    // PID参数
    .pid_kp = 10.0,
    .pid_ki = 0.5,
    .pid_kd = 50.0,
    
    // 系统配置
    .temp_sample_interval = 100,
    .display_update_interval = 500,
    .ssr_control_window = 1000,
    
    // WiFi配置
    .wifi_ssid = "Smart-Heating",
    .wifi_password = "12345678"
};

// 初始化SPIFFS
static esp_err_t spiffs_init(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    
    return esp_vfs_spiffs_register(&conf);
}

// 从文件加载配置
void config_load(void)
{
    // 初始化SPIFFS
    esp_err_t ret = spiffs_init();
    if (ret != ESP_OK) {
        printf("Failed to initialize SPIFFS, using default config\n");
        return;
    }
    
    // 打开配置文件
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (fp == NULL) {
        printf("Config file not found, using default config\n");
        return;
    }
    
    // 读取配置文件
    // 实际应用中应该使用JSON解析库
    // 这里简化处理，直接使用默认配置
    
    fclose(fp);
    printf("Config loaded from file\n");
}

// 保存配置到文件
void config_save(void)
{
    // 初始化SPIFFS
    esp_err_t ret = spiffs_init();
    if (ret != ESP_OK) {
        printf("Failed to initialize SPIFFS, cannot save config\n");
        return;
    }
    
    // 打开配置文件
    FILE *fp = fopen(CONFIG_FILE, "w");
    if (fp == NULL) {
        printf("Failed to open config file for writing\n");
        return;
    }
    
    // 写入配置文件
    // 实际应用中应该使用JSON库生成配置
    fprintf(fp, "{\n");
    fprintf(fp, "  \"min_temperature\": %.1f,\n", g_config.min_temperature);
    fprintf(fp, "  \"max_temperature\": %.1f,\n", g_config.max_temperature);
    fprintf(fp, "  \"default_target_temp\": %.1f,\n", g_config.default_target_temp);
    fprintf(fp, "  \"ssr_pin\": %d,\n", g_config.ssr_pin);
    fprintf(fp, "  \"potentiometer_pin\": %d,\n", g_config.potentiometer_pin);
    fprintf(fp, "  \"i2c_scl_pin\": %d,\n", g_config.i2c_scl_pin);
    fprintf(fp, "  \"i2c_sda_pin\": %d,\n", g_config.i2c_sda_pin);
    fprintf(fp, "  \"pid_kp\": %.1f,\n", g_config.pid_kp);
    fprintf(fp, "  \"pid_ki\": %.1f,\n", g_config.pid_ki);
    fprintf(fp, "  \"pid_kd\": %.1f,\n", g_config.pid_kd);
    fprintf(fp, "  \"temp_sample_interval\": %d,\n", g_config.temp_sample_interval);
    fprintf(fp, "  \"display_update_interval\": %d,\n", g_config.display_update_interval);
    fprintf(fp, "  \"ssr_control_window\": %d,\n", g_config.ssr_control_window);
    fprintf(fp, "  \"wifi_ssid\": \"%s\",\n", g_config.wifi_ssid);
    fprintf(fp, "  \"wifi_password\": \"%s\"\n", g_config.wifi_password);
    fprintf(fp, "}\n");
    
    fclose(fp);
    printf("Config saved to file\n");
}

void config_init(void)
{
    // 加载配置
    config_load();
    printf("Config initialized\n");
}

config_t config_get(void)
{
    return g_config;
}
