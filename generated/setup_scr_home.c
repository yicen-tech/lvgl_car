/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: MIT
 * The auto-generated can only be used on NXP devices
 */
#include "lvgl/lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "custom/custom.h"
#include "custom/weather_parser.h"
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "serial_comm.h"


static int home_digital_clock_1_hour_value = 11;
static int home_digital_clock_1_min_value = 25;
static int home_digital_clock_1_sec_value = 50;

static lv_timer_t *g_weather_update_timer = NULL; // 全局定时器变量
static lv_timer_t *g_time_update_timer = NULL;

static void home_swipe_event_cb(lv_event_t *e);
static void update_weather_cb(lv_timer_t *timer); // 定时器回调函数声明

char time_str_uart[9] = {0};
char date_str_uart[20] = {0};
char week_str_uart[10] = {0};
char weather_str_uart[100] = {0};


// 定义心知天气API密钥和城市
#define SENIVERSE_KEY "SAvyIdLOMRyCSkvNa"
#define CITY "Guilin"  // 城市名称，可以是中文或拼音

WeatherData* get_weather_data();
void display_weather_on_lvgl(lv_obj_t *parent, WeatherData *weather);

// 用于存储HTTP响应的结构体
typedef struct {
    char *data;
    size_t size;
} HttpResponse;

// 回调函数，用于接收HTTP响应数据
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t real_size = size * nmemb;
    HttpResponse *resp = (HttpResponse *)userp;
    
    // 重新分配内存以存储新数据
    char *ptr = realloc(resp->data, resp->size + real_size + 1);
    if (!ptr) {
        printf("内存分配失败\n");
        return 0;
    }
    
    resp->data = ptr;
    memcpy(&(resp->data[resp->size]), contents, real_size);
    resp->size += real_size;
    resp->data[resp->size] = 0;
    
    return real_size;
}

// 获取天气数据的函数
WeatherData* get_weather_data() {
    CURL *curl;
    CURLcode res;
    HttpResponse resp;
    WeatherData *weather = NULL;
    
    // 初始化响应结构体
    resp.data = malloc(1);
    resp.size = 0;
    
    // 初始化libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if (curl) {
        char url[256];
        // 构建API URL
        snprintf(url, sizeof(url), 
                "https://api.seniverse.com/v3/weather/now.json?key=%s&location=%s&language=zh-Hans&unit=c",
                SENIVERSE_KEY, CITY);
        
        // 设置CURL选项
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback); 
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&resp);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // 10秒超时
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // 不验证SSL证书，在生产环境中应该启用
        
        // 执行请求
        res = curl_easy_perform(curl);
        
        // 检查请求是否成功
        if (res == CURLE_OK) {
            // printf("天气数据获取成功，JSON长度: %zu\n", resp.size);
            // 解析JSON数据
            weather = parse_weather_json(resp.data);
            // print_weather_data(weather);
        } else {
            printf("天气数据获取失败: %s\n", curl_easy_strerror(res));
        }
        
        // 清理
        curl_easy_cleanup(curl);
    }
    
    // 释放响应数据
    free(resp.data);
    curl_global_cleanup();
    
    return weather;
}

static void update_time_cb(lv_timer_t *timer);

// 时钟更新回调函数
static update_time_cb(lv_timer_t *timer) 
{
	lv_ui *ui = (lv_ui *)timer->user_data;
    if (ui == NULL || !lv_obj_is_valid(ui->home)) {
        // 如果UI无效，停止定时器
        lv_timer_del(timer);
        g_time_update_timer = NULL;
        return;
    }	
    time_t now;
    struct tm tm;

    // 获取当前本地时间
    now = time(NULL);
    localtime_r(&now, &tm);

    static time_t start_time = 0;//初始时长为0
    if (start_time == 0) {start_time = now;}

    char time_str[9]; // 存放格式化后的时间字符串，HH:MM:SS
    char date_str[20]; // 存放格式化后的日期字符串，YYYY-MM-DD 
	char week_str[10]; // 存放星期几的字符串

    // 将时间格式化为字符串，精确到秒
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
    // 将日期格式化为字符串，精确到日
    snprintf(date_str, sizeof(date_str), "%04d/%02d/%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    // 获取并格式化星期几
    const char *weekdays[] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};
    snprintf(week_str, sizeof(week_str), "%s", weekdays[tm.tm_wday]);

	// 串口保存数据
	snprintf(time_str_uart, sizeof(time_str_uart), "1%02d%02d", tm.tm_hour, tm.tm_min);
	snprintf(date_str_uart, sizeof(date_str_uart), "0%02d%02d", tm.tm_mon + 1, tm.tm_mday);
	snprintf(week_str_uart, sizeof(week_str_uart), "2%01d", tm.tm_wday);

    // 安全地更新UI元素
    if (lv_obj_is_valid(ui->home_digital_clock_1)) {
    	lv_dclock_set_text(ui->home_digital_clock_1, time_str); // 更新时间标签的文本
	}
	if (lv_obj_is_valid(ui->home_datetext_1)) {
    	lv_label_set_text(ui->home_datetext_1, date_str); // 更新日期标签的文本
	}
	if (lv_obj_is_valid(ui->home_spangroup_1)) {
    	lv_span_set_text(lv_spangroup_get_child(ui->home_spangroup_1, 0), week_str); // 更新星期几文本
    	lv_spangroup_refr_mode(ui->home_spangroup_1); // 刷新span group以显示更新
	}
}

// 实现滑动事件回调函数
static void home_swipe_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_ui *ui = lv_event_get_user_data(e);

    if (code == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (dir == LV_DIR_LEFT) {
            // 左滑切换到 menu 界面
            setup_scr_menu(ui); // 假设 menu 界面初始化函数为 setup_scr_menu
            lv_scr_load_anim(ui->menu, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
        }
		else if (dir == LV_DIR_RIGHT) {
			if (g_time_update_timer != NULL) {
				lv_timer_del(g_time_update_timer);
				g_time_update_timer = NULL;
			}
			// // 右滑切换到 carctl 界面
			// setup_scr_carctl(ui); // 假设 carctl 界面初始化函数为 setup_scr_carctl
			// lv_scr_load_anim(ui->carctl, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, false);

			lv_obj_t * act_scr = lv_scr_act();
			lv_disp_t * d = lv_obj_get_disp(act_scr);
			if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr))
			{
				if (guider_ui.carctl_del == true)
					setup_scr_carctl(&guider_ui);
				lv_scr_load_anim(guider_ui.carctl, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, true);
				guider_ui.home_del = true;
			}
		}		
    }
}

// static void update_weather_cb(lv_timer_t *timer) {
//     lv_ui *ui = (lv_ui *)timer->user_data;
//     if (ui == NULL || !lv_obj_is_valid(ui->home)) {
//         // 如果UI无效，停止定时器
//         lv_timer_del(timer);
//         g_weather_update_timer = NULL;
//         return;
//     }
//     // 获取新的天气数据
//     WeatherData *weather = get_weather_data();
//     if (weather != NULL) {
//         // 更新位置标签
//         lv_label_set_text_fmt(ui->location_label, "%s", weather->location);

//         // 更新温度标签
//         lv_label_set_text_fmt(ui->temp_label, "%s", weather->temperature);

//         // 更新天气描述标签
//         lv_label_set_text_fmt(ui->weather_label, "%s", weather->weather);

// 		const lv_img_dsc_t *weather_icon = NULL;
// 		// 判断天气类型并选择对应图片
// 		if(strcmp(weather->weather, "多云") == 0) {
// 			weather_icon = &cloudy;
// 		} else if(strcmp(weather->weather, "晴") == 0) {
// 			weather_icon = &sunny;
// 		} else if(strcmp(weather->weather, "小雨") == 0) {
// 			weather_icon = &small_rain;
// 		} else if(strcmp(weather->weather, "中雨") == 0 ) {
// 			weather_icon = &moderate_rain;
// 		} else if(strcmp(weather->weather, "大雨") == 0) {
// 			weather_icon = &big_rain;		
// 		} else if(strcmp(weather->weather, "雾") == 0) {
// 			weather_icon = &fog;	
// 		} else if(strcmp(weather->weather, "霾") == 0) {
// 			weather_icon = &haze;				
// 		} else if(strstr(weather->weather, "雪") != NULL) {
// 			weather_icon = &snow;
// 		} else if(strstr(weather->weather, "阴") != NULL) {
// 			weather_icon = &yin_cloudy;
// 		} else if(strstr(weather->weather, "阵雨") != NULL) {
// 			weather_icon = &shower_rain;		
// 		} else {
// 			// 默认图标，当没有匹配的天气类型时使用
// 			weather_icon = &cloudy;
// 		}

// 		// 设置图片源
// 		lv_img_set_src(ui->weather_img, weather_icon);
// 		lv_img_set_zoom(ui->weather_img, 80);  // 设置为原始大小的31.25%
// 		lv_obj_align(ui->weather_img, LV_ALIGN_TOP_LEFT, 200, -48);
// 		lv_img_set_antialias(ui->weather_img, true);
// 		lv_obj_invalidate(ui->weather_img);

// 		free_weather_data(weather);
//     }
// }

void setup_scr_home(lv_ui *ui){

	//Write codes home
	ui->home = lv_obj_create(NULL);
	lv_obj_set_scrollbar_mode(ui->home, LV_SCROLLBAR_MODE_OFF);

	//创建背景图片对象
	lv_obj_t * bg_img = lv_img_create(ui->home);
	lv_img_set_src(bg_img, &main_bg);  // 设置背景图片源
	lv_obj_set_size(bg_img, LV_PCT(100), LV_PCT(100));  // 设置大小为父对象的100%
	lv_obj_set_style_img_recolor_opa(bg_img, 0, 0);  // 不重新着色
	lv_obj_align(bg_img, LV_ALIGN_CENTER, 0, 0);  // 居中对齐
	lv_obj_add_flag(bg_img, LV_OBJ_FLAG_FLOATING);  // 设置为浮动，防止影响其他控件
	lv_obj_clear_flag(bg_img, LV_OBJ_FLAG_CLICKABLE);  // 清除可点击标志
	lv_obj_move_background(bg_img);  // 移动到最底层
	lv_obj_set_style_bg_opa(ui->home, 0, LV_PART_MAIN|LV_STATE_DEFAULT);  // 设置背景透明

    // 如果已经存在定时器，先删除它
    if (g_time_update_timer != NULL) {
        lv_timer_del(g_time_update_timer);
        g_time_update_timer = NULL;
    }
	// if (g_weather_update_timer != NULL) {
	// 	lv_timer_del(g_weather_update_timer);
	// 	g_weather_update_timer = NULL;
	// }
	//Write codes home_music_btn
	ui->home_music_btn = lv_imgbtn_create(ui->home);
	lv_obj_set_pos(ui->home_music_btn, 310, 519);
	lv_obj_set_size(ui->home_music_btn, 70, 70);
	lv_obj_set_scrollbar_mode(ui->home_music_btn, LV_SCROLLBAR_MODE_OFF);

	//Set style for home_music_btn. Part: LV_PART_MAIN, State: LV_STATE_DEFAULT
	lv_obj_set_style_shadow_width(ui->home_music_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(ui->home_music_btn, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(ui->home_music_btn, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(ui->home_music_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(ui->home_music_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(ui->home_music_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->home_music_btn, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->home_music_btn, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_recolor(ui->home_music_btn, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_recolor_opa(ui->home_music_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->home_music_btn, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Set style for home_music_btn. Part: LV_PART_MAIN, State: LV_STATE_PRESSED
	lv_obj_set_style_shadow_width(ui->home_music_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_color(ui->home_music_btn, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_opa(ui->home_music_btn, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_spread(ui->home_music_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_ofs_x(ui->home_music_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_ofs_y(ui->home_music_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_color(ui->home_music_btn, lv_color_make(0xFF, 0x33, 0xFF), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_recolor(ui->home_music_btn, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_recolor_opa(ui->home_music_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_opa(ui->home_music_btn, 255, LV_PART_MAIN|LV_STATE_PRESSED);

	//Set style for home_music_btn. Part: LV_PART_MAIN, State: LV_STATE_CHECKED
	lv_obj_set_style_shadow_width(ui->home_music_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_color(ui->home_music_btn, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_opa(ui->home_music_btn, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_spread(ui->home_music_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_ofs_x(ui->home_music_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_ofs_y(ui->home_music_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->home_music_btn, lv_color_make(0xFF, 0x33, 0xFF), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_recolor(ui->home_music_btn, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_recolor_opa(ui->home_music_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_opa(ui->home_music_btn, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_imgbtn_set_src(ui->home_music_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_music_app_alpha_70x70, NULL);
	lv_obj_add_flag(ui->home_music_btn, LV_OBJ_FLAG_CHECKABLE);

	//Write codes home_system_name
	ui->home_system_name = lv_spangroup_create(ui->home);
	lv_obj_set_pos(ui->home_system_name, 429, 31);
	lv_obj_set_size(ui->home_system_name, 166, 26);
	lv_obj_set_scrollbar_mode(ui->home_system_name, LV_SCROLLBAR_MODE_OFF);
	lv_spangroup_set_align(ui->home_system_name, LV_TEXT_ALIGN_LEFT);
	lv_spangroup_set_overflow(ui->home_system_name, LV_SPAN_OVERFLOW_CLIP);
	lv_spangroup_set_mode(ui->home_system_name, LV_SPAN_MODE_BREAK);

	//Set style for home_system_name. Part: LV_PART_MAIN, State: LV_STATE_DEFAULT
	lv_obj_set_style_radius(ui->home_system_name, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->home_system_name, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_color(ui->home_system_name, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->home_system_name, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->home_system_name, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->home_system_name, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(ui->home_system_name, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(ui->home_system_name, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(ui->home_system_name, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(ui->home_system_name, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(ui->home_system_name, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->home_system_name, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->home_system_name, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->home_system_name, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->home_system_name, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->home_system_name, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->home_system_name, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->home_system_name, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//create spans
	lv_span_t *home_system_name_span;

	//create a new span
	home_system_name_span = lv_spangroup_new_span(ui->home_system_name);
	lv_span_set_text(home_system_name_span, "车载终端系统");
	lv_style_set_text_color(&home_system_name_span->style, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_decor(&home_system_name_span->style, LV_TEXT_DECOR_NONE);
	lv_style_set_text_font(&home_system_name_span->style, &lv_font_simsun_26);
	lv_spangroup_refr_mode(ui->home_system_name);

	//Write codes home_setting_btn
	ui->home_setting_btn = lv_imgbtn_create(ui->home);
	lv_obj_set_pos(ui->home_setting_btn, 656, 520);
	lv_obj_set_size(ui->home_setting_btn, 70, 70);
	lv_obj_set_scrollbar_mode(ui->home_setting_btn, LV_SCROLLBAR_MODE_OFF);

	//Set style for home_setting_btn. Part: LV_PART_MAIN, State: LV_STATE_DEFAULT
	lv_obj_set_style_shadow_width(ui->home_setting_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(ui->home_setting_btn, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(ui->home_setting_btn, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(ui->home_setting_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(ui->home_setting_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(ui->home_setting_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->home_setting_btn, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->home_setting_btn, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_recolor(ui->home_setting_btn, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_recolor_opa(ui->home_setting_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->home_setting_btn, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Set style for home_setting_btn. Part: LV_PART_MAIN, State: LV_STATE_PRESSED
	lv_obj_set_style_shadow_width(ui->home_setting_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_color(ui->home_setting_btn, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_opa(ui->home_setting_btn, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_spread(ui->home_setting_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_ofs_x(ui->home_setting_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_ofs_y(ui->home_setting_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_color(ui->home_setting_btn, lv_color_make(0xFF, 0x33, 0xFF), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_recolor(ui->home_setting_btn, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_recolor_opa(ui->home_setting_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_opa(ui->home_setting_btn, 255, LV_PART_MAIN|LV_STATE_PRESSED);

	//Set style for home_setting_btn. Part: LV_PART_MAIN, State: LV_STATE_CHECKED
	lv_obj_set_style_shadow_width(ui->home_setting_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_color(ui->home_setting_btn, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_opa(ui->home_setting_btn, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_spread(ui->home_setting_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_ofs_x(ui->home_setting_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_ofs_y(ui->home_setting_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->home_setting_btn, lv_color_make(0xFF, 0x33, 0xFF), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_recolor(ui->home_setting_btn, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_recolor_opa(ui->home_setting_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_opa(ui->home_setting_btn, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_imgbtn_set_src(ui->home_setting_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_setting_alpha_70x70, NULL);
	lv_obj_add_flag(ui->home_setting_btn, LV_OBJ_FLAG_CHECKABLE);
	static bool home_digital_clock_1_timer_enabled = false;

	//Write codes home_digital_clock_1
	ui->home_digital_clock_1 = lv_dclock_create(ui->home,"11:25:50");
	lv_obj_set_style_text_align(ui->home_digital_clock_1, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_pos(ui->home_digital_clock_1, 17, 66);
	lv_obj_set_size(ui->home_digital_clock_1, 132, 47);

	// 4. 设置时钟更新定时器（如果需要实时更新）
	g_time_update_timer = lv_timer_create(update_time_cb, 1000, ui);

	//Set style for home_digital_clock_1. Part: LV_PART_MAIN, State: LV_STATE_DEFAULT
	lv_obj_set_style_radius(ui->home_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->home_digital_clock_1, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_color(ui->home_digital_clock_1, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->home_digital_clock_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->home_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->home_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(ui->home_digital_clock_1, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(ui->home_digital_clock_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(ui->home_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(ui->home_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(ui->home_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->home_digital_clock_1, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->home_digital_clock_1, &lv_font_montserratMedium_29, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->home_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->home_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->home_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->home_digital_clock_1, 7, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->home_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write datetext home_datetext_1
	ui->home_datetext_1 = lv_label_create(ui->home);
	lv_label_set_text(ui->home_datetext_1, "2025/03/28");
	lv_obj_set_style_text_align(ui->home_datetext_1, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_pos(ui->home_datetext_1, 19, 12);
	lv_obj_set_size(ui->home_datetext_1, 130, 36);

	//write datetext event
	// lv_obj_add_flag(ui->home_datetext_1, LV_OBJ_FLAG_CLICKABLE);
	// lv_obj_add_event_cb(ui->home_datetext_1, home_datetext_1_event, LV_EVENT_ALL, NULL);

	//Set style for home_datetext_1. Part: LV_PART_MAIN, State: LV_STATE_DEFAULT
	lv_obj_set_style_radius(ui->home_datetext_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->home_datetext_1, lv_color_make(0xfb, 0xb6, 0xb6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_color(ui->home_datetext_1, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->home_datetext_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->home_datetext_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->home_datetext_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(ui->home_datetext_1, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(ui->home_datetext_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(ui->home_datetext_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(ui->home_datetext_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(ui->home_datetext_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->home_datetext_1, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->home_datetext_1, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->home_datetext_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->home_datetext_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->home_datetext_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->home_datetext_1, 7, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes home_school_img
	ui->home_school_img = lv_img_create(ui->home);
	lv_obj_set_pos(ui->home_school_img, 874, 0);
	lv_obj_set_size(ui->home_school_img, 150, 150);
	lv_obj_set_scrollbar_mode(ui->home_school_img, LV_SCROLLBAR_MODE_OFF);

	//Set style for home_school_img. Part: LV_PART_MAIN, State: LV_STATE_DEFAULT
	lv_obj_set_style_img_recolor(ui->home_school_img, lv_color_make(0x00, 0x01, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_recolor_opa(ui->home_school_img, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->home_school_img, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_add_flag(ui->home_school_img, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->home_school_img,&_school_32bit_alpha_150x150);
	lv_img_set_pivot(ui->home_school_img, 50,50);
	lv_img_set_angle(ui->home_school_img, 0);

	//Write codes home_video_btn
	ui->home_video_btn = lv_imgbtn_create(ui->home);
	lv_obj_set_pos(ui->home_video_btn, 418, 519);
	lv_obj_set_size(ui->home_video_btn, 70, 70);
	lv_obj_set_scrollbar_mode(ui->home_video_btn, LV_SCROLLBAR_MODE_OFF);

	//Set style for home_video_btn. Part: LV_PART_MAIN, State: LV_STATE_DEFAULT
	lv_obj_set_style_shadow_width(ui->home_video_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(ui->home_video_btn, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(ui->home_video_btn, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(ui->home_video_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(ui->home_video_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(ui->home_video_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->home_video_btn, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->home_video_btn, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_recolor(ui->home_video_btn, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_recolor_opa(ui->home_video_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->home_video_btn, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Set style for home_video_btn. Part: LV_PART_MAIN, State: LV_STATE_PRESSED
	lv_obj_set_style_shadow_width(ui->home_video_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_color(ui->home_video_btn, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_opa(ui->home_video_btn, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_spread(ui->home_video_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_ofs_x(ui->home_video_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_ofs_y(ui->home_video_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_color(ui->home_video_btn, lv_color_make(0xFF, 0x33, 0xFF), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_recolor(ui->home_video_btn, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_recolor_opa(ui->home_video_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_opa(ui->home_video_btn, 255, LV_PART_MAIN|LV_STATE_PRESSED);

	//Set style for home_video_btn. Part: LV_PART_MAIN, State: LV_STATE_CHECKED
	lv_obj_set_style_shadow_width(ui->home_video_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_color(ui->home_video_btn, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_opa(ui->home_video_btn, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_spread(ui->home_video_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_ofs_x(ui->home_video_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_ofs_y(ui->home_video_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->home_video_btn, lv_color_make(0xFF, 0x33, 0xFF), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_recolor(ui->home_video_btn, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_recolor_opa(ui->home_video_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_opa(ui->home_video_btn, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_imgbtn_set_src(ui->home_video_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_video_alpha_70x70, NULL);
	lv_obj_add_flag(ui->home_video_btn, LV_OBJ_FLAG_CHECKABLE);

	//Write codes home_backup_btn
	ui->home_backup_btn = lv_imgbtn_create(ui->home);
	lv_obj_set_pos(ui->home_backup_btn, 538, 520);
	lv_obj_set_size(ui->home_backup_btn, 70, 70);
	lv_obj_set_scrollbar_mode(ui->home_backup_btn, LV_SCROLLBAR_MODE_OFF);

	//Set style for home_backup_btn. Part: LV_PART_MAIN, State: LV_STATE_DEFAULT
	lv_obj_set_style_shadow_width(ui->home_backup_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(ui->home_backup_btn, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(ui->home_backup_btn, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(ui->home_backup_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(ui->home_backup_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(ui->home_backup_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->home_backup_btn, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->home_backup_btn, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_recolor(ui->home_backup_btn, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_recolor_opa(ui->home_backup_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->home_backup_btn, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Set style for home_backup_btn. Part: LV_PART_MAIN, State: LV_STATE_PRESSED
	lv_obj_set_style_shadow_width(ui->home_backup_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_color(ui->home_backup_btn, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_opa(ui->home_backup_btn, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_spread(ui->home_backup_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_ofs_x(ui->home_backup_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_ofs_y(ui->home_backup_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_color(ui->home_backup_btn, lv_color_make(0xFF, 0x33, 0xFF), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_recolor(ui->home_backup_btn, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_recolor_opa(ui->home_backup_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_opa(ui->home_backup_btn, 255, LV_PART_MAIN|LV_STATE_PRESSED);

	//Set style for home_backup_btn. Part: LV_PART_MAIN, State: LV_STATE_CHECKED
	lv_obj_set_style_shadow_width(ui->home_backup_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_color(ui->home_backup_btn, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_opa(ui->home_backup_btn, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_spread(ui->home_backup_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_ofs_x(ui->home_backup_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_ofs_y(ui->home_backup_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->home_backup_btn, lv_color_make(0xFF, 0x33, 0xFF), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_recolor(ui->home_backup_btn, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_recolor_opa(ui->home_backup_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_opa(ui->home_backup_btn, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_imgbtn_set_src(ui->home_backup_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_Astern_image_1_alpha_70x70, NULL);
	lv_obj_add_flag(ui->home_backup_btn, LV_OBJ_FLAG_CHECKABLE);

	//Write codes home_spangroup_1
	ui->home_spangroup_1 = lv_spangroup_create(ui->home);
	lv_obj_set_pos(ui->home_spangroup_1, 48, 42);
	lv_obj_set_size(ui->home_spangroup_1, 86, 20);
	lv_obj_set_scrollbar_mode(ui->home_spangroup_1, LV_SCROLLBAR_MODE_OFF);
	lv_spangroup_set_align(ui->home_spangroup_1, LV_TEXT_ALIGN_LEFT);
	lv_spangroup_set_overflow(ui->home_spangroup_1, LV_SPAN_OVERFLOW_ELLIPSIS);
	lv_spangroup_set_mode(ui->home_spangroup_1, LV_SPAN_MODE_BREAK);

	//Set style for home_spangroup_1. Part: LV_PART_MAIN, State: LV_STATE_DEFAULT
	lv_obj_set_style_radius(ui->home_spangroup_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->home_spangroup_1, lv_color_make(0xfb, 0xb6, 0xb6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_color(ui->home_spangroup_1, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->home_spangroup_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->home_spangroup_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->home_spangroup_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(ui->home_spangroup_1, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(ui->home_spangroup_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(ui->home_spangroup_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(ui->home_spangroup_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(ui->home_spangroup_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(ui->home_spangroup_1, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->home_spangroup_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(ui->home_spangroup_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->home_spangroup_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->home_spangroup_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->home_spangroup_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->home_spangroup_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_add_event_cb(ui->home, home_swipe_event_cb, LV_EVENT_GESTURE, ui);
	
	//create spans
	lv_span_t *home_spangroup_1_span;

	//create a new span
	home_spangroup_1_span = lv_spangroup_new_span(ui->home_spangroup_1);
	lv_span_set_text(home_spangroup_1_span, "星期四");
	lv_style_set_text_color(&home_spangroup_1_span->style, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_decor(&home_spangroup_1_span->style, LV_TEXT_DECOR_NONE);
	lv_style_set_text_font(&home_spangroup_1_span->style, &lv_font_simsun_20);
	lv_spangroup_refr_mode(ui->home_spangroup_1);

	WeatherData *weather = get_weather_data();
	if (weather != NULL) {  // 添加空指针检查
    // 创建标签显示位置
    lv_obj_t *location_label = lv_label_create(ui->home);
    lv_label_set_text_fmt(location_label, "%s", weather->location);
    lv_obj_align(location_label, LV_ALIGN_TOP_LEFT, 200, 12);
    lv_obj_set_style_text_font(location_label, &lv_font_simsun_26, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(location_label, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);

    // 创建标签显示温度
    lv_obj_t *temp_label = lv_label_create(ui->home);
    lv_label_set_text_fmt(temp_label, "%s", weather->temperature);
    lv_obj_align(temp_label, LV_ALIGN_TOP_LEFT, 160, 42);
    lv_obj_set_style_text_font(temp_label, &lv_font_simsun_40, LV_PART_MAIN | LV_STATE_DEFAULT);	
	lv_obj_set_style_text_color(temp_label, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);

    lv_obj_t *temp_danwei_label = lv_label_create(ui->home);
    lv_label_set_text_fmt(temp_danwei_label, "℃");
    lv_obj_align(temp_danwei_label, LV_ALIGN_TOP_LEFT, 200, 44);
    lv_obj_set_style_text_font(temp_danwei_label, &lv_font_simsun_16, LV_PART_MAIN | LV_STATE_DEFAULT);	
	lv_obj_set_style_text_color(temp_danwei_label, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);

	lv_obj_t *weather_label = lv_label_create(ui->home);
	lv_label_set_text_fmt(weather_label, "%s", weather->weather);
	lv_obj_align(weather_label, LV_ALIGN_TOP_LEFT, 215, 55);
	lv_obj_set_style_text_font(weather_label, &lv_font_simsun_26, LV_PART_MAIN | LV_STATE_DEFAULT); 
	lv_obj_set_style_text_color(weather_label, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);  

	// 创建图片天气UI
	lv_obj_t *weather_img = lv_img_create(ui->home);

	// 根据weather->weather的值选择对应的天气图片
	const lv_img_dsc_t *weather_icon = NULL;
	int weather_code = 0; // 默认值为 0（多云）
	
	// 判断天气类型并选择对应图片和数字代码
	if (strcmp(weather->weather, "多云") == 0) {
		weather_icon = &cloudy;
		weather_code = 0;
	} else if (strcmp(weather->weather, "晴") == 0) {
		weather_icon = &sunny;
		weather_code = 1;
	} else if (strcmp(weather->weather, "小雨") == 0) {
		weather_icon = &small_rain;
		weather_code = 2;
	} else if (strcmp(weather->weather, "中雨") == 0) {
		weather_icon = &moderate_rain;
		weather_code = 3;
	} else if (strcmp(weather->weather, "大雨") == 0) {
		weather_icon = &big_rain;
		weather_code = 4;
	} else if (strcmp(weather->weather, "雾") == 0) {
		weather_icon = &fog;
		weather_code = 5;
	} else if (strcmp(weather->weather, "霾") == 0) {
		weather_icon = &haze;
		weather_code = 6;
	} else if (strstr(weather->weather, "雪") != NULL) {
		weather_icon = &snow;
		weather_code = 7;
	} else if (strstr(weather->weather, "阴") != NULL) {
		weather_icon = &yin_cloudy;
		weather_code = 8;
	} else if (strstr(weather->weather, "阵雨") != NULL) {
		weather_icon = &shower_rain;
		weather_code = 9;
	} else {
		// 默认图标，当没有匹配的天气类型时使用
		weather_icon = &cloudy;
		weather_code = 0;
	}

	snprintf(weather_str_uart, sizeof(weather_str_uart), "3%01d%s", weather_code, weather->temperature);

	// 设置图片源
	lv_img_set_src(weather_img, weather_icon);
	lv_img_set_zoom(weather_img, 80);  // 设置为原始大小的31.25%
	lv_obj_align(weather_img, LV_ALIGN_TOP_LEFT, 200, -48);
	lv_img_set_antialias(weather_img, true);
	lv_obj_invalidate(weather_img);

	free_weather_data(weather);
	}

	// // 初始化天气更新定时器（10 分钟）
	// g_weather_update_timer = lv_timer_create(update_weather_cb, 600000, ui);

	//Init events for screen
	events_init_home(ui);
}