#ifndef TEMP_CURVE_H
#define TEMP_CURVE_H

#include <stdint.h>
#include <stdbool.h>

// 温度曲线点结构体
typedef struct {
    uint32_t time_seconds;  // 时间点（秒）
    float target_temp;      // 目标温度（℃）
} temp_curve_point_t;

// 温度曲线结构体
typedef struct {
    char name[50];                  // 曲线名称
    uint16_t point_count;           // 曲线点数量
    temp_curve_point_t *points;     // 曲线点数组
    bool is_running;                // 是否正在运行
    uint32_t start_time;            // 开始时间（系统时间戳）
    uint16_t current_point_index;   // 当前执行的曲线点索引
} temp_curve_t;

// 函数声明
void temp_curve_init(void);
bool temp_curve_load(const char *name, temp_curve_t *curve);
bool temp_curve_save(const temp_curve_t *curve);
bool temp_curve_start(const char *name);
void temp_curve_stop(void);
bool temp_curve_is_running(void);
float temp_curve_get_current_target(void);
temp_curve_t *temp_curve_get_current(void);

#endif // TEMP_CURVE_H