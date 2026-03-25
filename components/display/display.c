#include "display.h"
#include "system_status.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

void display_init(void)
{
    // 初始化I2C OLED屏幕
    // 实际应用中需要使用I2C库和OLED驱动
    printf("Display initialized\n");
}

void display_update(void)
{
    system_status_t status = system_status_get();
    
    // 模拟显示输出（实际应用中需要使用OLED驱动库）
    printf("\n=== Heating System Status ===\n");
    printf("Current Temp: %.1f °C\n", status.current_temp);
    printf("Target Temp: %.1f °C\n", status.target_temp);
    printf("Heating: %s\n", status.heating ? "ON" : "OFF");
    printf("PID Output: %.1f%%\n", status.pid_output);
    printf("============================\n");
}

void display_task(void *pvParameters)
{
    while (1) {
        display_update();
        vTaskDelay(pdMS_TO_TICKS(500)); // 500ms刷新频率
    }
}
