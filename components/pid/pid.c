#include "pid.h"
#include "system_status.h"
#include "config.h"
#include "temp_curve.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

// PID参数和状态
static float g_last_error = 0.0;
static float g_integral = 0.0;
static float g_integral_limit = 100.0;

void pid_init(void)
{
    g_last_error = 0.0;
    g_integral = 0.0;
    printf("PID controller initialized\n");
}

float pid_calculate(float setpoint, float process_value)
{
    // 获取最新PID参数
    config_t config = config_get();
    float g_kp = config.pid_kp;
    float g_ki = config.pid_ki;
    float g_kd = config.pid_kd;
    
    // 计算误差
    float error = setpoint - process_value;
    
    // 比例项
    float proportional = g_kp * error;
    
    // 积分项（带限制）
    g_integral += g_ki * error * 0.1; // 0.1是采样时间（100ms）
    if (g_integral > g_integral_limit) {
        g_integral = g_integral_limit;
    } else if (g_integral < -g_integral_limit) {
        g_integral = -g_integral_limit;
    }
    
    // 微分项
    float derivative = g_kd * (error - g_last_error) / 0.1;
    g_last_error = error;
    
    // 计算总输出
    float output = proportional + g_integral + derivative;
    
    // 限制输出范围
    if (output > 100.0) {
        output = 100.0;
    } else if (output < 0.0) {
        output = 0.0;
    }
    
    return output;
}

void pid_task(void *pvParameters)
{
    while (1) {
        system_status_t status = system_status_get();
        float target_temp = status.target_temp;
        
        // 如果温度曲线正在运行，使用曲线计算的目标温度
        if (temp_curve_is_running()) {
            target_temp = temp_curve_get_current_target();
        }
        
        float output = pid_calculate(target_temp, status.current_temp);
        system_status_update_pid_output(output);
        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms控制周期
    }
}
