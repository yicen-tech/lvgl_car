#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lvgl/demos/music/lv_demo_music.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "generated/gui_guider.h"
#include "generated/events_init.h"
// #include "generated/video_logic.h"
#include "custom/custom.h"
#include "custom/weather_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "generated/serial_comm.h"
#include "lvgl/demos/music/lv_demo_music_main.h"

#define DISP_BUF_SIZE (1024 * 600)

lv_ui guider_ui;

int fd = 0;

void uart_proc(void);


int main(void)
{
    lv_init();

    /*Linux frame buffer device init*/
    fbdev_init();

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf   = &disp_buf;
    disp_drv.flush_cb   = fbdev_flush;
    disp_drv.hor_res    = 1024;
    disp_drv.ver_res    = 600;
    lv_disp_drv_register(&disp_drv);

    /* Linux input device init */
    evdev_init();

    /* Initialize and register a display input driver */
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);      /*Basic initialization*/

    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;
    lv_indev_t * my_indev = lv_indev_drv_register(&indev_drv); 

    /*Create a Demo*/
    //lv_demo_widgets();

    // 启动音乐播放器演示
    // lv_demo_music();

    //gui_guide相关初始化
    setup_ui(&guider_ui);
    events_init(&guider_ui);

    //串口接收回调
    fd = serial_open(SERIAL_PORT);
    if (fd < 0) return -1;

    serial_set_rx_callback(my_rx_callback);
    serial_start_rx(fd);

    // serial_stop_rx();
    // serial_close(fd);

    /*Handle LVGL tasks*/
    while(1) {
        lv_timer_handler();
        uart_proc();
        usleep(5000);
        // if (video_thread_running) {
        //     update_ui(&guider_ui);
        // }        
    }

    return 0;
}

void uart_proc(void)
{
    if (goto_music) {
        show_music_player();
        goto_music = 0;
    }    
    if (goto_music_pause) {
        my_music_pause();
        goto_music_pause = 0;
    }
    if (goto_music_play) {
        my_music_play_uart();
        goto_music_play = 0;
    }
    if (goto_music_next) {
        my_music_next();
        goto_music_next = 0;
    }
    if (goto_music_prev) {
        my_music_prev();
        goto_music_prev = 0;
    }

    if (return_time_flag) {
        serial_send(fd, time_str_uart, strlen(time_str_uart));
        usleep(100000);
        return_time_flag = 0;
    }
    if (return_date_flag) {
        serial_send(fd, date_str_uart, strlen(date_str_uart));
        usleep(100000);
        return_date_flag = 0;
    }    
    if (return_week_flag) {
        serial_send(fd, week_str_uart, strlen(week_str_uart));
        usleep(100000);
        return_week_flag = 0;
    } 
    if (return_weather_flag) {
        serial_send(fd, weather_str_uart, strlen(weather_str_uart));
        usleep(100000);
        return_weather_flag = 0;
    } 
    if (goto_backup_flag) {
        show_backup_uart();
        goto_backup_flag = 0;
    }
    if (goto_home_flag) {
        show_home_uart();
        goto_home_flag = 0;
    }
}


/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
