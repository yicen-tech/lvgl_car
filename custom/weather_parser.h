#ifndef WEATHER_PARSER_H
#define WEATHER_PARSER_H

// 定义天气数据结构
typedef struct {
    char location[64];
    char weather[32];
    char temperature[16];
    char last_update[64];       
} WeatherData;

// 函数声明
WeatherData* parse_weather_json(const char* json_string);
WeatherData* get_weather_data();
void free_weather_data(WeatherData* weather);
void print_weather_data(const WeatherData* weather);

#endif /* WEATHER_PARSER_H */