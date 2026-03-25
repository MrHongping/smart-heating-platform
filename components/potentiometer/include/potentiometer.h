#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

// 函数声明
void potentiometer_init(void);
float potentiometer_read(void);
void potentiometer_task(void *pvParameters);

#endif // POTENTIOMETER_H
