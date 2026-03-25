#ifndef SSR_CONTROL_H
#define SSR_CONTROL_H

#include <stdbool.h>

// 函数声明
void ssr_control_init(void);
void ssr_set_power(float power);
void ssr_control_task(void *pvParameters);

#endif // SSR_CONTROL_H
