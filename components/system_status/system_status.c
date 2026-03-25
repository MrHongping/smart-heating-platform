#include "system_status.h"

// 全局状态变量
static system_status_t g_system_status = {
    .current_temp = 0.0,
    .target_temp = 100.0,
    .pid_output = 0.0,
    .heating = false,
    .system_running = true,
    .pid_kp = 10.0,
    .pid_ki = 0.5,
    .pid_kd = 50.0
};

// 互斥锁
static SemaphoreHandle_t g_status_mutex = NULL;

void system_status_init(void)
{
    // 创建互斥锁
    g_status_mutex = xSemaphoreCreateMutex();
    if (g_status_mutex == NULL) {
        printf("Failed to create status mutex\n");
    }
}

system_status_t system_status_get(void)
{
    system_status_t status = {0};
    
    if (xSemaphoreTake(g_status_mutex, portMAX_DELAY)) {
        status = g_system_status;
        xSemaphoreGive(g_status_mutex);
    }
    
    return status;
}

void system_status_update_current_temp(float temp)
{
    if (xSemaphoreTake(g_status_mutex, portMAX_DELAY)) {
        g_system_status.current_temp = temp;
        xSemaphoreGive(g_status_mutex);
    }
}

void system_status_update_target_temp(float temp)
{
    if (xSemaphoreTake(g_status_mutex, portMAX_DELAY)) {
        g_system_status.target_temp = temp;
        xSemaphoreGive(g_status_mutex);
    }
}

void system_status_update_pid_output(float output)
{
    if (xSemaphoreTake(g_status_mutex, portMAX_DELAY)) {
        g_system_status.pid_output = output;
        xSemaphoreGive(g_status_mutex);
    }
}

void system_status_update_heating(bool heating)
{
    if (xSemaphoreTake(g_status_mutex, portMAX_DELAY)) {
        g_system_status.heating = heating;
        xSemaphoreGive(g_status_mutex);
    }
}

void system_status_update_pid_params(float kp, float ki, float kd)
{
    if (xSemaphoreTake(g_status_mutex, portMAX_DELAY)) {
        g_system_status.pid_kp = kp;
        g_system_status.pid_ki = ki;
        g_system_status.pid_kd = kd;
        xSemaphoreGive(g_status_mutex);
    }
}

bool system_status_is_running(void)
{
    bool running = false;
    
    if (xSemaphoreTake(g_status_mutex, portMAX_DELAY)) {
        running = g_system_status.system_running;
        xSemaphoreGive(g_status_mutex);
    }
    
    return running;
}

void system_status_set_running(bool running)
{
    if (xSemaphoreTake(g_status_mutex, portMAX_DELAY)) {
        g_system_status.system_running = running;
        xSemaphoreGive(g_status_mutex);
    }
}
