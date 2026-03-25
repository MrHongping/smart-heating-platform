#include "temp_curve.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include <esp_log.h>

static const char *TAG = "temp_curve";

// 最大曲线点数量
#define MAX_CURVE_POINTS 50

// 当前运行的温度曲线
static temp_curve_t current_curve;

// 温度曲线存储路径
#define CURVE_FILE_PATH "/spiffs/curves/%s.json"

/**
 * @brief 初始化温度曲线模块
 * @details 初始化温度曲线模块，设置默认值
 */
void temp_curve_init(void)
{
    memset(&current_curve, 0, sizeof(temp_curve_t));
    current_curve.points = malloc(sizeof(temp_curve_point_t) * MAX_CURVE_POINTS);
    if (current_curve.points == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for curve points");
    }
    ESP_LOGI(TAG, "Temperature curve module initialized");
}

/**
 * @brief 加载温度曲线
 * @details 从文件系统加载指定名称的温度曲线
 * @param name 曲线名称
 * @param curve 曲线结构体指针
 * @return 是否加载成功
 */
bool temp_curve_load(const char *name, temp_curve_t *curve)
{
    char file_path[100];
    sprintf(file_path, CURVE_FILE_PATH, name);
    
    FILE *file = fopen(file_path, "r");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open curve file: %s", file_path);
        return false;
    }
    
    // 读取文件内容
    char buffer[1024];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, file);
    buffer[bytes_read] = '\0';
    fclose(file);
    
    // 解析JSON
    // 这里简化处理，实际项目中应使用cJSON库
    // 示例：{"name":"Reflow", "points":[{"time":0,"temp":25},{"time":60,"temp":150},...]}
    
    // 这里只是示例实现，实际项目中需要完整的JSON解析
    strcpy(curve->name, name);
    curve->point_count = 0;
    
    ESP_LOGI(TAG, "Loaded curve: %s", name);
    return true;
}

/**
 * @brief 保存温度曲线
 * @details 将温度曲线保存到文件系统
 * @param curve 曲线结构体指针
 * @return 是否保存成功
 */
bool temp_curve_save(const temp_curve_t *curve)
{
    char file_path[100];
    sprintf(file_path, CURVE_FILE_PATH, curve->name);
    
    FILE *file = fopen(file_path, "w");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open curve file for writing: %s", file_path);
        return false;
    }
    
    // 写入JSON格式
    // 这里简化处理，实际项目中应使用cJSON库
    fprintf(file, "{\"name\":\"%s\", \"points\":[", curve->name);
    
    for (int i = 0; i < curve->point_count; i++) {
        fprintf(file, "{\"time\":%u,\"temp\":%.1f}", 
                curve->points[i].time_seconds, 
                curve->points[i].target_temp);
        if (i < curve->point_count - 1) {
            fprintf(file, ",");
        }
    }
    
    fprintf(file, "]}");
    fclose(file);
    
    ESP_LOGI(TAG, "Saved curve: %s", curve->name);
    return true;
}

/**
 * @brief 开始执行温度曲线
 * @details 开始执行指定名称的温度曲线
 * @param name 曲线名称
 * @return 是否启动成功
 */
bool temp_curve_start(const char *name)
{
    // 加载曲线
    if (!temp_curve_load(name, &current_curve)) {
        return false;
    }
    
    // 初始化曲线执行状态
    current_curve.is_running = true;
    current_curve.start_time = esp_timer_get_time() / 1000; // 转换为毫秒
    current_curve.current_point_index = 0;
    
    ESP_LOGI(TAG, "Started curve: %s", name);
    return true;
}

/**
 * @brief 停止温度曲线
 * @details 停止当前正在执行的温度曲线
 */
void temp_curve_stop(void)
{
    current_curve.is_running = false;
    ESP_LOGI(TAG, "Stopped curve: %s", current_curve.name);
}

/**
 * @brief 检查温度曲线是否正在运行
 * @return 是否正在运行
 */
bool temp_curve_is_running(void)
{
    return current_curve.is_running;
}

/**
 * @brief 获取当前目标温度
 * @details 根据当前时间计算并返回目标温度
 * @return 当前目标温度
 */
float temp_curve_get_current_target(void)
{
    if (!current_curve.is_running || current_curve.point_count == 0) {
        return 0.0;
    }
    
    // 计算当前运行时间（秒）
    uint32_t current_time = (esp_timer_get_time() / 1000) - current_curve.start_time;
    current_time /= 1000; // 转换为秒
    
    // 找到当前时间点对应的曲线段
    for (int i = 0; i < current_curve.point_count - 1; i++) {
        if (current_time >= current_curve.points[i].time_seconds && 
            current_time < current_curve.points[i + 1].time_seconds) {
            
            // 线性插值计算当前目标温度
            float t1 = current_curve.points[i].time_seconds;
            float t2 = current_curve.points[i + 1].time_seconds;
            float temp1 = current_curve.points[i].target_temp;
            float temp2 = current_curve.points[i + 1].target_temp;
            
            float ratio = (current_time - t1) / (t2 - t1);
            float current_temp = temp1 + (temp2 - temp1) * ratio;
            
            return current_temp;
        }
    }
    
    // 如果时间超过曲线结束点，返回最后一个点的温度
    if (current_time >= current_curve.points[current_curve.point_count - 1].time_seconds) {
        return current_curve.points[current_curve.point_count - 1].target_temp;
    }
    
    return current_curve.points[0].target_temp;
}

/**
 * @brief 获取当前运行的温度曲线
 * @return 当前温度曲线指针
 */
temp_curve_t *temp_curve_get_current(void)
{
    if (current_curve.is_running) {
        return &current_curve;
    }
    return NULL;
}
