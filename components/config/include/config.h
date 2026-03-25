#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

// 配置结构
typedef struct {
    // 温度配置
    float min_temperature;      // 最低温度
    float max_temperature;      // 最高温度
    float default_target_temp;  // 默认目标温度
    
    // 引脚配置
    int ssr_pin;               // SSR控制引脚
    int potentiometer_pin;      // 电位器ADC引脚
    int i2c_scl_pin;           // I2C SCL引脚
    int i2c_sda_pin;           // I2C SDA引脚
    
    // PID参数
    float pid_kp;              // PID比例系数
    float pid_ki;              // PID积分系数
    float pid_kd;              // PID微分系数
    
    // 系统配置
    int temp_sample_interval;   // 温度采样间隔（ms）
    int display_update_interval; // 显示更新间隔（ms）
    int ssr_control_window;     // SSR控制窗口（ms）
    
    // WiFi配置
    char wifi_ssid[32];        // WiFi SSID
    char wifi_password[32];     // WiFi密码
} config_t;

// 函数声明
void config_init(void);
config_t config_get(void);
void config_load(void);
void config_save(void);

#endif // CONFIG_H
