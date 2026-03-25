#include "ssr_control.h"
#include "system_status.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

// SSR控制引脚（实际应用中需要根据硬件定义）
#define SSR_PIN 2

// 控制窗口大小（毫秒）
#define CONTROL_WINDOW_MS 1000

void ssr_control_init(void)
{
    // 初始化SSR控制引脚
    // 实际应用中需要使用ESP32的GPIO库
    printf("SSR control initialized\n");
}

void ssr_set_power(float power)
{
    // 计算导通时间
    int on_time_ms = (int)(power / 100.0 * CONTROL_WINDOW_MS);
    int off_time_ms = CONTROL_WINDOW_MS - on_time_ms;
    
    // 控制SSR
    if (on_time_ms > 0) {
        // 实际应用中：gpio_set_level(SSR_PIN, 1);
        system_status_update_heating(true);
        printf("SSR ON for %dms\n", on_time_ms);
        vTaskDelay(pdMS_TO_TICKS(on_time_ms));
    }
    
    if (off_time_ms > 0) {
        // 实际应用中：gpio_set_level(SSR_PIN, 0);
        system_status_update_heating(false);
        printf("SSR OFF for %dms\n", off_time_ms);
        vTaskDelay(pdMS_TO_TICKS(off_time_ms));
    }
}

void ssr_control_task(void *pvParameters)
{
    while (1) {
        system_status_t status = system_status_get();
        if (status.system_running) {
            ssr_set_power(status.pid_output);
        } else {
            // 系统停止时关闭SSR
            // 实际应用中：gpio_set_level(SSR_PIN, 0);
            system_status_update_heating(false);
            vTaskDelay(pdMS_TO_TICKS(CONTROL_WINDOW_MS));
        }
    }
}
