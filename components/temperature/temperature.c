#include "temperature.h"
#include "system_status.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

// 滑动平均滤波参数
#define FILTER_SIZE 5
static float g_temp_buffer[FILTER_SIZE];
static int g_buffer_index = 0;

// 模拟温度值（实际应用中替换为真实传感器读取）
static float g_simulated_temp = 25.0;

void temperature_init(void)
{
    // 初始化温度缓冲区
    for (int i = 0; i < FILTER_SIZE; i++) {
        g_temp_buffer[i] = 25.0;
    }
    
    printf("Temperature sensor initialized\n");
}

float temperature_read(void)
{
    // 模拟温度读取（实际应用中替换为真实传感器读取）
    // 这里简单模拟温度变化
    static float temp_offset = 0.0;
    static int direction = 1;
    
    temp_offset += direction * 0.1;
    if (temp_offset > 5.0) {
        direction = -1;
    } else if (temp_offset < -5.0) {
        direction = 1;
    }
    
    float raw_temp = g_simulated_temp + temp_offset;
    
    // 滑动平均滤波
    g_temp_buffer[g_buffer_index] = raw_temp;
    g_buffer_index = (g_buffer_index + 1) % FILTER_SIZE;
    
    float filtered_temp = 0.0;
    for (int i = 0; i < FILTER_SIZE; i++) {
        filtered_temp += g_temp_buffer[i];
    }
    filtered_temp /= FILTER_SIZE;
    
    return filtered_temp;
}

void temperature_task(void *pvParameters)
{
    while (1) {
        float temp = temperature_read();
        system_status_update_current_temp(temp);
        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms采样周期
    }
}
