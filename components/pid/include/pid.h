#ifndef PID_H
#define PID_H

// 函数声明
void pid_init(void);
float pid_calculate(float setpoint, float process_value);
void pid_task(void *pvParameters);

#endif // PID_H
