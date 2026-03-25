#include "safety.h"
#include "system_status.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

// 安全参数
#define MAX_TEMPERATURE 250.0 // 最高温度限制
#define MIN_TEMPERATURE -20.0 // 最低温度限制

void safety_init(void)
{
    // 初始化安全保护机制
    printf("Safety system initialized\n");
}

void safety_task(void *pvParameters)
{
    while (1) {
        system_status_t status = system_status_get();
        
        // 过温保护
        if (status.current_temp > MAX_TEMPERATURE) {
            printf("⚠️  Over temperature detected: %.1f °C\n", status.current_temp);
            system_status_set_running(false);
            printf("🔒 System stopped for safety\n");
        }
        
        // 传感器异常检测
        if (status.current_temp < MIN_TEMPERATURE || status.current_temp > 300.0) {
            printf("⚠️  Sensor error detected: %.1f °C\n", status.current_temp);
            system_status_set_running(false);
            printf("🔒 System stopped for safety\n");
        }
        
        // 看门狗机制（模拟）
        static int watchdog_counter = 0;
        watchdog_counter++;
        if (watchdog_counter >= 10) {
            watchdog_counter = 0;
            printf("🐶 Watchdog reset\n");
        }
        
        vTaskDelay(pdMS_TO_TICKS(500)); // 500ms检查一次
    }
}
