#include "display.h"
#include "system_status.h"
#include "config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "driver/i2c.h"

// OLED屏幕参数
#define OLED_I2C_ADDRESS 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// I2C配置
#define I2C_MASTER_NUM I2C_NUM_0  // I2C端口号
#define I2C_MASTER_FREQ_HZ 100000 // I2C时钟频率

// SSD1306命令
#define SSD1306_COMMAND_MODE 0x00
#define SSD1306_DATA_MODE 0x40
#define SSD1306_SET_CONTRAST 0x81
#define SSD1306_DISPLAY_ON 0xAF
#define SSD1306_DISPLAY_OFF 0xAE
#define SSD1306_SET_MEMORY_ADDR_MODE 0x20
#define SSD1306_SET_COLUMN_ADDR 0x21
#define SSD1306_SET_PAGE_ADDR 0x22
#define SSD1306_SET_START_LINE 0x40
#define SSD1306_SET_SEGMENT_REMAP 0xA0
#define SSD1306_SET_COM_SCAN_DIR 0xC0
#define SSD1306_SET_COM_PINS 0xDA
#define SSD1306_SET_DISPLAY_OFFSET 0xD3
#define SSD1306_SET_DISPLAY_CLOCK 0xD5
#define SSD1306_SET_PRECHARGE 0xD9
#define SSD1306_SET_VCOM_DETECT 0xDB

// I2C发送函数
static esp_err_t i2c_master_send(uint8_t address, uint8_t *data, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, data, size, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// 发送命令到OLED
static void oled_send_command(uint8_t cmd)
{
    uint8_t data[2] = {SSD1306_COMMAND_MODE, cmd};
    i2c_master_send(OLED_I2C_ADDRESS, data, 2);
}

// 发送数据到OLED
static void oled_send_data(uint8_t data)
{
    uint8_t buf[2] = {SSD1306_DATA_MODE, data};
    i2c_master_send(OLED_I2C_ADDRESS, buf, 2);
}

// 清空屏幕
static void oled_clear(void)
{
    for (int page = 0; page < 8; page++) {
        oled_send_command(0xB0 + page); // 设置页地址
        oled_send_command(0x00);       // 设置列地址低4位
        oled_send_command(0x10);       // 设置列地址高4位
        for (int col = 0; col < 128; col++) {
            oled_send_data(0x00);       // 发送空数据
        }
    }
}

// 显示字符串
static void oled_show_string(int x, int y, const char *str)
{
    // 简单的字符显示函数，实际应用中需要字库
    oled_send_command(0xB0 + y);       // 设置页地址
    oled_send_command(0x00 + (x & 0x0F)); // 设置列地址低4位
    oled_send_command(0x10 + (x >> 4));   // 设置列地址高4位
    
    while (*str) {
        // 发送字符数据（实际应用中需要从字库中获取）
        oled_send_data(0x00);
        oled_send_data(0xFF);
        oled_send_data(0x81);
        oled_send_data(0x81);
        oled_send_data(0x81);
        oled_send_data(0x81);
        oled_send_data(0xFF);
        oled_send_data(0x00);
        str++;
    }
}

void display_init(void)
{
    // 获取配置
    config_t config = config_get();
    
    // 初始化I2C
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = config.i2c_sda_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = config.i2c_scl_pin,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    
    // 初始化SSD1306
    oled_send_command(SSD1306_DISPLAY_OFF);
    oled_send_command(SSD1306_SET_DISPLAY_CLOCK);
    oled_send_command(0x80);
    oled_send_command(SSD1306_SET_MEMORY_ADDR_MODE);
    oled_send_command(0x00);
    oled_send_command(SSD1306_SET_START_LINE);
    oled_send_command(SSD1306_SET_SEGMENT_REMAP | 0x01);
    oled_send_command(SSD1306_SET_COM_SCAN_DIR | 0x08);
    oled_send_command(SSD1306_SET_COM_PINS);
    oled_send_command(0x12);
    oled_send_command(SSD1306_SET_CONTRAST);
    oled_send_command(0xCF);
    oled_send_command(SSD1306_SET_PRECHARGE);
    oled_send_command(0xF1);
    oled_send_command(SSD1306_SET_VCOM_DETECT);
    oled_send_command(0x40);
    oled_send_command(0xA4);
    oled_send_command(0xA6);
    oled_send_command(SSD1306_DISPLAY_ON);
    
    // 清空屏幕
    oled_clear();
    
    printf("OLED display initialized\n");
}

void display_update(void)
{
    system_status_t status = system_status_get();
    char buffer[32];
    
    // 清空屏幕
    oled_clear();
    
    // 显示当前温度
    snprintf(buffer, sizeof(buffer), "Current: %.1f°C", status.current_temp);
    oled_show_string(0, 0, buffer);
    
    // 显示目标温度
    snprintf(buffer, sizeof(buffer), "Target: %.1f°C", status.target_temp);
    oled_show_string(0, 1, buffer);
    
    // 显示加热状态
    snprintf(buffer, sizeof(buffer), "Heating: %s", status.heating ? "ON" : "OFF");
    oled_show_string(0, 2, buffer);
    
    // 显示PID输出
    snprintf(buffer, sizeof(buffer), "Power: %.1f%%", status.pid_output);
    oled_show_string(0, 3, buffer);
    
    // 同时输出到串口（调试用）
    printf("\n=== Heating System Status ===\n");
    printf("Current Temp: %.1f °C\n", status.current_temp);
    printf("Target Temp: %.1f °C\n", status.target_temp);
    printf("Heating: %s\n", status.heating ? "ON" : "OFF");
    printf("PID Output: %.1f%%\n", status.pid_output);
    printf("============================\n");
}

void display_task(void *pvParameters)
{
    config_t config = config_get();
    while (1) {
        display_update();
        vTaskDelay(pdMS_TO_TICKS(config.display_update_interval)); // 使用配置的刷新频率
    }
}
