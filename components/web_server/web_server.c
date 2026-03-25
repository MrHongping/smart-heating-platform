#include "web_server.h"
#include "system_status.h"
#include "config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_http_server.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <cJSON.h>
#include <string.h>

static const char *TAG = "web_server";
static httpd_handle_t server = NULL;

// WebSocket 客户端列表
static httpd_handle_t ws_clients[10];
static int ws_client_count = 0;

// 系统状态JSON
static char *get_system_status_json(void)
{
    system_status_t status = system_status_get();
    
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "current_temp", status.current_temp);
    cJSON_AddNumberToObject(root, "target_temp", status.target_temp);
    cJSON_AddBoolToObject(root, "heating", status.heating);
    cJSON_AddNumberToObject(root, "pid_output", status.pid_output);
    cJSON_AddBoolToObject(root, "system_running", status.system_running);
    
    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    
    return json_str;
}

// 发送WebSocket消息给所有客户端
static void send_ws_message_to_all(const char *message)
{
    for (int i = 0; i < ws_client_count; i++) {
        if (ws_clients[i]) {
            httpd_ws_frame_t ws_pkt;
            memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
            ws_pkt.type = HTTPD_WS_TYPE_TEXT;
            ws_pkt.payload = (uint8_t *)message;
            ws_pkt.len = strlen(message);
            httpd_ws_send_frame(ws_clients[i], &ws_pkt);
        }
    }
}

// WebSocket连接处理
// static esp_err_t ws_open_handler(httpd_handle_t hd, int sockfd)
// {
//     if (ws_client_count < 10) {
//         ws_clients[ws_client_count++] = hd;
//         ESP_LOGI(TAG, "WebSocket client connected, count: %d", ws_client_count);
//     } else {
//         ESP_LOGE(TAG, "WebSocket client limit reached");
//     }
//     return ESP_OK;
// }

// // WebSocket断开连接处理
// static void ws_close_handler(httpd_handle_t hd)
// {
//     for (int i = 0; i < ws_client_count; i++) {
//         if (ws_clients[i] == hd) {
//             ws_clients[i] = NULL;
//             // 重新排列客户端列表
//             for (int j = i; j < ws_client_count - 1; j++) {
//                 ws_clients[j] = ws_clients[j + 1];
//             }
//             ws_client_count--;
//             ESP_LOGI(TAG, "WebSocket client disconnected, count: %d", ws_client_count);
//             break;
//         }
//     }
// }

// WebSocket事件处理
static esp_err_t ws_handler(httpd_req_t *req)
{
    // 处理WebSocket帧
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    
    // 接收WebSocket帧
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 处理WebSocket消息
    if (ws_pkt.len > 0) {
        ESP_LOGI(TAG, "Received WebSocket message: %.*s", ws_pkt.len, (char *)ws_pkt.payload);
    }
    
    // 发送系统状态作为响应
    char *json_str = get_system_status_json();
    ws_pkt.payload = (uint8_t *)json_str;
    ws_pkt.len = strlen(json_str);
    ret = httpd_ws_send_frame(req, &ws_pkt);
    free(json_str);
    
    return ret;
}



// API: 获取系统状态
static esp_err_t get_status_handler(httpd_req_t *req)
{
    char *json_str = get_system_status_json();
    httpd_resp_send(req, json_str, strlen(json_str));
    free(json_str);
    return ESP_OK;
}

// API: 设置目标温度
static esp_err_t set_temp_handler(httpd_req_t *req)
{
    char buf[100];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        return ESP_FAIL;
    }
    buf[ret] = '\0';
    
    cJSON *root = cJSON_Parse(buf);
    if (root) {
        cJSON *target_temp = cJSON_GetObjectItem(root, "target");
        if (cJSON_IsNumber(target_temp)) {
            float temp = (float)cJSON_GetNumberValue(target_temp);
            system_status_update_target_temp(temp);
            ESP_LOGI(TAG, "Set target temperature: %.1f", temp);
        }
        cJSON_Delete(root);
    }
    
    httpd_resp_send(req, "{\"status\": \"ok\"}", -1);
    return ESP_OK;
}

// API: 设置PID参数
static esp_err_t set_pid_handler(httpd_req_t *req)
{
    char buf[100];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        return ESP_FAIL;
    }
    buf[ret] = '\0';
    
    cJSON *root = cJSON_Parse(buf);
    if (root) {
        cJSON *kp = cJSON_GetObjectItem(root, "kp");
        cJSON *ki = cJSON_GetObjectItem(root, "ki");
        cJSON *kd = cJSON_GetObjectItem(root, "kd");
        
        if (cJSON_IsNumber(kp) && cJSON_IsNumber(ki) && cJSON_IsNumber(kd)) {
            float kp_val = (float)cJSON_GetNumberValue(kp);
            float ki_val = (float)cJSON_GetNumberValue(ki);
            float kd_val = (float)cJSON_GetNumberValue(kd);
            
            // 这里需要添加设置PID参数的代码
            ESP_LOGI(TAG, "Set PID parameters: Kp=%.2f, Ki=%.3f, Kd=%.1f", kp_val, ki_val, kd_val);
        }
        cJSON_Delete(root);
    }
    
    httpd_resp_send(req, "{\"status\": \"ok\"}", -1);
    return ESP_OK;
}

// 静态文件处理
static esp_err_t static_file_handler(httpd_req_t *req)
{
    char filepath[60];
    
    if (strcmp(req->uri, "/") == 0) {
        strcpy(filepath, "/index.html");
    } else {
        strcpy(filepath, req->uri);
    }
    
    char fullpath[100];
    sprintf(fullpath, "/spiffs%s", filepath);
    
    FILE *file = fopen(fullpath, "r");
    if (!file) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    
    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        httpd_resp_send_chunk(req, buffer, bytes_read);
    }
    fclose(file);
    
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

// 初始化Web服务器
void web_server_init(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    
    // 启动服务器
    if (httpd_start(&server, &config) == ESP_OK) {
        // 注册API处理程序
        httpd_uri_t status_uri = {
            .uri = "/api/status",
            .method = HTTP_GET,
            .handler = get_status_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &status_uri);
        
        httpd_uri_t set_temp_uri = {
            .uri = "/api/set_temp",
            .method = HTTP_POST,
            .handler = set_temp_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &set_temp_uri);
        
        httpd_uri_t set_pid_uri = {
            .uri = "/api/set_pid",
            .method = HTTP_POST,
            .handler = set_pid_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &set_pid_uri);
        
        // 注册WebSocket处理程序
        httpd_uri_t ws_uri = {
            .uri = "/ws",
            .method = HTTP_GET,
            .handler = ws_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &ws_uri);
        
        // 注册静态文件处理程序
        httpd_uri_t static_uri = {
            .uri = "/*",
            .method = HTTP_GET,
            .handler = static_file_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &static_uri);
        
        ESP_LOGI(TAG, "Web server initialized");
        ESP_LOGI(TAG, "API endpoints:");
        ESP_LOGI(TAG, "GET /api/status - Get system status");
        ESP_LOGI(TAG, "POST /api/set_temp - Set target temperature");
        ESP_LOGI(TAG, "POST /api/set_pid - Set PID parameters");
        ESP_LOGI(TAG, "WebSocket - Real-time temperature updates");
    } else {
        ESP_LOGE(TAG, "Failed to start web server");
    }
}

// Web服务器任务
void web_server_task(void *pvParameters)
{
    while (1) {
        // 每1秒发送一次温度数据到WebSocket客户端
        char *json_str = get_system_status_json();
        send_ws_message_to_all(json_str);
        free(json_str);
        
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1秒循环
    }
}
