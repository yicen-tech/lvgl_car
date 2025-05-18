#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

// 定义天气数据结构
typedef struct {
    char location[64];
    char weather[32];
    char temperature[16];
    char last_update[64];    
} WeatherData;

// 解析心知天气API返回的JSON数据
WeatherData* parse_weather_json(const char* json_string) {
    WeatherData* weather = (WeatherData*)malloc(sizeof(WeatherData));
    if (!weather) {
        return NULL;
    }
    memset(weather, 0, sizeof(WeatherData));
    
    cJSON* json = cJSON_Parse(json_string);
    if (!json) {
        printf("JSON解析错误: %s\n", cJSON_GetErrorPtr());
        free(weather);
        return NULL;
    }
    
    // 解析位置信息
    cJSON* results = cJSON_GetObjectItem(json, "results");
    if (results && cJSON_IsArray(results) && cJSON_GetArraySize(results) > 0) {
        cJSON* location_data = cJSON_GetArrayItem(results, 0);
        cJSON* location = cJSON_GetObjectItem(location_data, "location");
        
        if (location) {
            cJSON* name = cJSON_GetObjectItem(location, "name");
            if (name && cJSON_IsString(name)) {
                strncpy(weather->location, name->valuestring, sizeof(weather->location) - 1);
            }
        }
        
        // 解析天气数据
        cJSON* now = cJSON_GetObjectItem(location_data, "now");
        if (now) {
            cJSON* text = cJSON_GetObjectItem(now, "text");
            if (text && cJSON_IsString(text)) {
                strncpy(weather->weather, text->valuestring, sizeof(weather->weather) - 1);
            }
            
            cJSON* temp = cJSON_GetObjectItem(now, "temperature");
            if (temp && cJSON_IsString(temp)) {
                strncpy(weather->temperature, temp->valuestring, sizeof(weather->temperature) - 1);
            }        
        }

        // 解析最后更新时间
        cJSON* last_update = cJSON_GetObjectItem(location_data, "last_update");
        if (last_update) {
            strncpy(weather->last_update, last_update->valuestring, sizeof(weather->last_update) - 1);
        }        
    }
    
    cJSON_Delete(json);
    return weather;
}

// 释放天气数据结构
void free_weather_data(WeatherData* weather) {
    if (weather) {
        free(weather);
    }
}

// 打印天气数据
void print_weather_data(const WeatherData* weather) {
    if (!weather) return;
    
    printf("位置: %s\n", weather->location);
    printf("天气: %s\n", weather->weather);
    printf("温度: %s°C\n", weather->temperature);
    printf("最后更新: %s\n", weather->last_update);    
}