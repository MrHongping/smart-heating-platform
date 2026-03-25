#include "potentiometer.h"
#include "system_status.h"
#include "config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

void potentiometer_init(void)
{
    // 获取配置
    config_t config = config_get();
    
    // 初始化ADC引脚
    // 实际应用中需要使用ESP32的ADC库
    printf("Potentiometer initialized on pin %d\n", config.potentiometer_pin);
}

float potentiometer_read(void)
{
    // 获取配置
    config_t config = config_get();
    
    // 模拟电位器读取（实际应用中需要使用ADC库）
    // 这里简单模拟电位器值
    static float pot_value = 0.0;
    static int direction = 1;
    
    pot_value += direction * 0.01;
    if (pot_value > 1.0) {
        direction = -1;
    } else if (pot_value < 0.0) {
        direction = 1;
    }
    
    // 将电位器值转换为温度
    float temp = config.min_temperature + (config.max_temperature - config.min_temperature) * pot_value;
    
    return temp;
}

void potentiometer_task(void *pvParameters)
{
    while (1) {
        float temp = potentiometer_read();
        system_status_update_target_temp(temp);
        vTaskDelay(pdMS_TO_TICKS(500)); // 500ms更新一次
    }
}
