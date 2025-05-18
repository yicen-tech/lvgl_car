/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: MIT
 * The auto-generated can only be used on NXP devices
 */

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"
#include "guider_fonts/guider_fonts.h"

typedef struct
{
	lv_obj_t *home;
	bool home_del;
	lv_obj_t *home_music_btn;
	lv_obj_t *home_music_btn_label;
	lv_obj_t *home_system_name;
	lv_obj_t *home_setting_btn;
	lv_obj_t *home_setting_btn_label;
	lv_obj_t *home_digital_clock_1;
	lv_obj_t *home_datetext_1;
	lv_obj_t *home_school_img;
	lv_obj_t *home_video_btn;
	lv_obj_t *home_video_btn_label;
	lv_obj_t *home_backup_btn;
	lv_obj_t *home_backup_btn_label;
	lv_obj_t *home_car_img;
	lv_obj_t *home_spangroup_1;
	lv_obj_t *music;
	bool music_del;
	lv_obj_t *music_music_return_btn;
	lv_obj_t *music_music_return_btn_label;
	lv_obj_t *video;
	bool video_del;
	lv_obj_t *video_imgbtn_1;
	lv_obj_t *video_imgbtn_1_label;
	// 视频播放器组件
    lv_obj_t *video_player;
    lv_obj_t *video_ctrl_panel;
    lv_obj_t *video_play_btn;
    lv_obj_t *video_play_label;
    lv_obj_t *video_progress_bar;
    lv_obj_t *video_current_time_label;
	lv_obj_t *video_total_time_label;
    lv_obj_t *video_volume_btn;
    lv_obj_t *video_volume_label;
    lv_obj_t *video_volume_slider;
    lv_obj_t *video_fullscreen_btn;
    lv_obj_t *video_fullscreen_label;
    lv_obj_t *video_playlist_btn;
    lv_obj_t *video_playlist_label;
    lv_obj_t *video_playlist;
    lv_obj_t *video_playlist_list;
    lv_obj_t *video_item1;
    lv_obj_t *video_item2;
    lv_obj_t *video_item3;
	lv_obj_t *video_item4;
    lv_obj_t *video_item5;
	lv_obj_t *video_item6;	
	lv_obj_t *back_up;
	bool back_up_del;
	lv_obj_t *back_up_imgbtn_2;
	lv_obj_t *back_up_camera_img;
	lv_obj_t *back_up_imgbtn_2_label;
	lv_obj_t *setting;
	bool setting_del;
	lv_obj_t *setting_setting_return_btn;
	lv_obj_t *setting_setting_return_btn_label;
	lv_obj_t *setting_menu_1;
	lv_obj_t *system;
	lv_obj_t *time_date;
	lv_obj_t *brightness;
	lv_obj_t *setting_slider_1;
	lv_obj_t *setting_textprogress_1;
	lv_obj_t *setting_menu_1_main_page;
	lv_obj_t *setting_menu_1_sub_page0;
	lv_obj_t *setting_menu_1_menu_cont0;
	lv_obj_t *setting_menu_1_menu_label0;
	lv_obj_t *setting_menu_1_sub_page1;
	lv_obj_t *setting_menu_1_menu_cont1;
	lv_obj_t *setting_menu_1_menu_label1;
	lv_obj_t *setting_menu_1_sub_page2;
	lv_obj_t *setting_menu_1_menu_cont2;
	lv_obj_t *setting_menu_1_menu_label2;
	lv_obj_t *caro_3;
	lv_obj_t *caro_2;
	lv_obj_t *menu;
	bool menu_del;
	lv_obj_t *menu_music_btn;
	lv_obj_t *menu_video_btn;
	lv_obj_t *menu_backup_btn;
	lv_obj_t *menu_setting_btn;
	lv_obj_t *menu_2048_btn;
	// lv_obj_t *menu_weather_btn;
	lv_obj_t *game_2048;
	bool game_2048_del;
	lv_obj_t *game_2048_return_btn;
	//car_contrtoller
	lv_obj_t *carctl;
	bool carctl_del;
	lv_obj_t *carctl_info_grid;
	lv_obj_t *carctl_date_label;
	lv_obj_t *carctl_time_label;
	lv_obj_t *carctl_D_btn;
	lv_obj_t *carctl_P_btn;
	lv_obj_t *carctl_R_btn;
	lv_obj_t *carctl_N_btn;
	lv_obj_t *carctl_carbody_btn;
	lv_obj_t *carctl_double_flash_btn;	
	lv_obj_t *carctl_unlock_btn;
	lv_obj_t *carctl_lock_btn;
	lv_obj_t *carctl_ws_wiper_sub_btn;
	lv_obj_t *carctl_ws_wiper_add_btn;
	lv_obj_t *carctl_lamp_on_btn;
	lv_obj_t *carctl_lamp_off_btn;
}lv_ui;

void ui_init_style(lv_style_t * style);
void init_scr_del_flag(lv_ui *ui);
void setup_ui(lv_ui *ui);
extern lv_ui guider_ui;
void setup_scr_home(lv_ui *ui);
void setup_scr_music(lv_ui *ui);
void setup_scr_video(lv_ui *ui);
void setup_scr_back_up(lv_ui *ui);
void setup_scr_setting(lv_ui *ui);
void setup_scr_menu(lv_ui *ui);
void setup_scr_2048(lv_ui *ui);
void setup_scr_carctl(lv_ui *ui);
LV_IMG_DECLARE(_setting_alpha_70x70);
LV_IMG_DECLARE(_school_32bit_alpha_150x150);
LV_IMG_DECLARE(_Astern_image_1_alpha_70x70);
LV_IMG_DECLARE(_music_app_alpha_70x70);
LV_IMG_DECLARE(_car_alpha_659x322);
LV_IMG_DECLARE(_return_alpha_50x50);
LV_IMG_DECLARE(_video_alpha_70x70);

LV_IMG_DECLARE(big_rain); // 这行通常放在文件顶部或函数外部
LV_IMG_DECLARE(moderate_rain);
LV_IMG_DECLARE(small_rain);
LV_IMG_DECLARE(cloudy);
LV_IMG_DECLARE(sunny);
LV_IMG_DECLARE(yin_cloudy);
LV_IMG_DECLARE(snow);
LV_IMG_DECLARE(main_bg);
LV_IMG_DECLARE(fog);
LV_IMG_DECLARE(haze);
LV_IMG_DECLARE(shower_rain);

#ifdef __cplusplus
}
#endif
#endif