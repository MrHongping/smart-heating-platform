#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <stdint.h>

// 温度传感器类型
typedef enum {
    SENSOR_PT100,    // PT100 + MAX31865
    SENSOR_NTC       // NTC
} sensor_type_t;

// 函数声明
void temperature_init(void);
float temperature_read(void);
void temperature_task(void *pvParameters);

#endif // TEMPERATURE_H
