#ifndef SYSTEM_STATUS_H
#define SYSTEM_STATUS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// 系统状态结构
typedef struct {
    float current_temp;     // 当前温度
    float target_temp;      // 目标温度
    float pid_output;       // PID输出（0-100%）
    bool heating;           // 加热状态
    bool system_running;    // 系统运行状态
    float pid_kp;           // PID参数
    float pid_ki;           // PID参数
    float pid_kd;           // PID参数
} system_status_t;

// 函数声明
void system_status_init(void);
system_status_t system_status_get(void);
void system_status_update_current_temp(float temp);
void system_status_update_target_temp(float temp);
void system_status_update_pid_output(float output);
void system_status_update_heating(bool heating);
void system_status_update_pid_params(float kp, float ki, float kd);
bool system_status_is_running(void);
void system_status_set_running(bool running);

#endif // SYSTEM_STATUS_H
