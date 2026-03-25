#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "temperature.h"
#include "pid.h"
#include "ssr_control.h"
#include "display.h"
#include "web_server.h"
#include "system_status.h"
#include "safety.h"

void app_main(void)
{
    // 初始化系统
    system_status_init();
    temperature_init();
    pid_init();
    ssr_control_init();
    display_init();
    web_server_init();
    safety_init();

    // 创建任务
    xTaskCreate(temperature_task, "temperature_task", 4096, NULL, 5, NULL);
    xTaskCreate(pid_task, "pid_task", 4096, NULL, 4, NULL);
    xTaskCreate(ssr_control_task, "ssr_control_task", 4096, NULL, 3, NULL);
    xTaskCreate(display_task, "display_task", 4096, NULL, 2, NULL);
    xTaskCreate(web_server_task, "web_server_task", 8192, NULL, 1, NULL);
    xTaskCreate(safety_task, "safety_task", 4096, NULL, 6, NULL);

    printf("Smart heating system initialized\n");
}
