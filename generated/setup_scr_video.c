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
#include "video_logic.h" 

//变量声明
// static bool is_video_playing = false;
// static bool is_fullscreen = false;
static lv_obj_t *fullscreen_container = NULL;  // 全局变量，用于跟踪全屏容器



// 添加函数声明
static void exit_fullscreen_event_cb(lv_event_t *e);


static void video_imgbtn_1_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
        stop_video_playback(&guider_ui);
        is_video_playing = false;
        is_fullscreen = false;
        cleanup_video_resources();
        // 添加一个短暂延迟，确保线程有时间响应
        usleep(100000); // 100毫秒

		lv_obj_t * act_scr = lv_scr_act();
		lv_disp_t * d = lv_obj_get_disp(act_scr);
		if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr))
		{
			if (guider_ui.home_del == true)
				setup_scr_home(&guider_ui);
			lv_scr_load_anim(guider_ui.home, LV_SCR_LOAD_ANIM_OVER_RIGHT, 100, 100, true);
			guider_ui.video_del = true;
		}
	}
		break;
	default:
		break;
	}
}

static void video_play_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    
    if(code == LV_EVENT_CLICKED) {
        // 检查是否有视频在播放
        if (video_thread_running) {
            // 如果视频正在播放,则暂停/继续播放
            pause_resume_video(ui);
            if(is_video_playing == 0) {
                if (ui->video_play_label && lv_obj_is_valid(ui->video_play_label)) {
                    lv_label_set_text(ui->video_play_label, LV_SYMBOL_PLAY);
                }
            }
            else {
                if (ui->video_play_label && lv_obj_is_valid(ui->video_play_label)) {
                    lv_label_set_text(ui->video_play_label, LV_SYMBOL_PAUSE);
                }               
            }
        } else {
            // 如果没有视频在播放,则开始播放第一个视频
            play_video_by_index(ui, 0);
        }
    }
}

static void video_volume_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    
    if(code == LV_EVENT_CLICKED) {
        if(lv_obj_has_flag(ui->video_volume_slider, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(ui->video_volume_slider, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(ui->video_volume_slider, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void video_volume_slider_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    
    if(code == LV_EVENT_VALUE_CHANGED) {
        int32_t value = lv_slider_get_value(ui->video_volume_slider);
        // 设置音量
        if(value == 0) {
            lv_label_set_text(ui->video_volume_label, LV_SYMBOL_MUTE);
        } else if(value < 50) {
            lv_label_set_text(ui->video_volume_label, LV_SYMBOL_VOLUME_MID);
        } else {
            lv_label_set_text(ui->video_volume_label, LV_SYMBOL_VOLUME_MAX);
        }
    }
}

static void video_fullscreen_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    
    if(code == LV_EVENT_CLICKED) {
        if(is_fullscreen) {
            // 退出全屏 - 删除全屏容器
            if(fullscreen_container != NULL) {
                lv_obj_del(fullscreen_container);
                fullscreen_container = NULL;
            }
            
            // 显示控制面板
            lv_obj_clear_flag(ui->video_ctrl_panel, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(ui->video_fullscreen_label, LV_SYMBOL_PLUS);
            is_fullscreen = false;
        } else {
            // 进入全屏 - 创建新的全屏容器
            fullscreen_container = lv_obj_create(lv_layer_top());
            lv_obj_set_size(fullscreen_container, LV_HOR_RES, LV_VER_RES);
            lv_obj_set_pos(fullscreen_container, 0, 0);
            lv_obj_set_style_bg_color(fullscreen_container, lv_color_black(), 0);
            lv_obj_set_style_bg_opa(fullscreen_container, 255, 0);
            lv_obj_clear_flag(fullscreen_container, LV_OBJ_FLAG_SCROLLABLE);
            
            // 在这里添加视频内容到全屏容器
            // 例如，可以创建一个图像对象来显示视频帧
            lv_obj_t *video_content = lv_img_create(fullscreen_container);
            lv_obj_set_size(video_content, LV_HOR_RES, LV_VER_RES);
            lv_obj_center(video_content);
            // 设置视频内容...
            
            // 创建退出全屏按钮
            lv_obj_t *exit_fullscreen_btn = lv_btn_create(fullscreen_container);
            lv_obj_set_pos(exit_fullscreen_btn, LV_HOR_RES - 110, 10);
            lv_obj_set_size(exit_fullscreen_btn, 50, 50);
            lv_obj_set_style_radius(exit_fullscreen_btn, 25, 0);
            lv_obj_set_style_bg_color(exit_fullscreen_btn, lv_color_make(0x30, 0x30, 0x30), 0);
            lv_obj_set_style_bg_opa(exit_fullscreen_btn, 150, 0);
            
            lv_obj_t *exit_label = lv_label_create(exit_fullscreen_btn);
            lv_label_set_text(exit_label, LV_SYMBOL_CLOSE);
            lv_obj_center(exit_label);
            
            // 为退出按钮添加事件
            lv_obj_add_event_cb(exit_fullscreen_btn, exit_fullscreen_event_cb, LV_EVENT_CLICKED, ui);
            
            // 隐藏控制面板
            lv_obj_add_flag(ui->video_ctrl_panel, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(ui->video_fullscreen_label, LV_SYMBOL_MINUS);
            is_fullscreen = true;
        }
    }
}

static void exit_fullscreen_event_cb(lv_event_t *e)
{
    lv_ui *ui = lv_event_get_user_data(e);
    
    // 退出全屏 - 删除全屏容器
    if(fullscreen_container != NULL) {
        lv_obj_del(fullscreen_container);
        fullscreen_container = NULL;
    }
    
    // 显示控制面板
    lv_obj_clear_flag(ui->video_ctrl_panel, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(ui->video_fullscreen_label, LV_SYMBOL_PLUS);
    is_fullscreen = false;
}
static void video_playlist_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    
    if(code == LV_EVENT_CLICKED) {
        // 切换播放列表的显示/隐藏状态
        if(lv_obj_has_flag(ui->video_playlist, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(ui->video_playlist, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(ui->video_playlist, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void video_playlist_item_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);
    
    if(code == LV_EVENT_CLICKED) {
        lv_obj_t *obj = lv_event_get_target(e);
        const char *txt = lv_list_get_btn_text(ui->video_playlist_list, obj);

        // 根据 txt 判断点击了哪个视频文件
        if (strcmp(txt, "老人,海鸥,沙滩.mp4") == 0) {
            play_video_by_index(ui, 0);
        } else if (strcmp(txt, "最长的电影.mp4") == 0) {
            play_video_by_index(ui, 1);
        } else if (strcmp(txt, "美人鱼.mp4") == 0) {
            play_video_by_index(ui, 2);
        } else if (strcmp(txt, "鹬.mp4") == 0) {
            play_video_by_index(ui, 3);      
        } else if (strcmp(txt, "Anime mix-cut1.mp4") == 0) {
            play_video_by_index(ui, 4);
        } else if (strcmp(txt, "Anime mix-cut2.mp4") == 0) {
            play_video_by_index(ui, 5);                     
        } else {
            // 未知视频文件
            // printf("未知视频文件: %s\n", txt);
        }        
        // 切换视频
        lv_label_set_text(ui->video_play_label, LV_SYMBOL_PAUSE);
        // is_video_playing = true;
        lv_obj_add_flag(ui->video_playlist, LV_OBJ_FLAG_HIDDEN);
        
        // 这里添加实际加载并播放视频的代码
    }
}

// 进度条点击事件回调函数
static void video_progress_event_cb(lv_event_t *e)
{
    lv_ui *ui = lv_event_get_user_data(e);
    lv_obj_t *obj = lv_event_get_target(e);

    // 获取进度条的当前值
    int slider_value = lv_slider_get_value(obj);

    // 计算目标时间位置(秒)
    int target_time = slider_value;

    // printf("跳转到目标时间: %d 秒\n", target_time);

    // 发送跳转命令
    pthread_mutex_lock(&video_mutex);
    video_seek_time = target_time;
    video_seek_requested = true;
    pthread_mutex_unlock(&video_mutex);
}

void events_init_video(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->video_imgbtn_1, video_imgbtn_1_event_handler, LV_EVENT_ALL, ui);

    // ... existing code ...
    lv_obj_add_event_cb(ui->video_play_btn, video_play_btn_event_cb, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->video_progress_bar, video_progress_event_cb, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->video_volume_btn, video_volume_btn_event_cb, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->video_volume_slider, video_volume_slider_event_cb, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->video_fullscreen_btn, video_fullscreen_btn_event_cb, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->video_playlist_btn, video_playlist_btn_event_cb, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->video_item1, video_playlist_item_event_cb, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->video_item2, video_playlist_item_event_cb, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->video_item3, video_playlist_item_event_cb, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->video_item4, video_playlist_item_event_cb, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->video_item5, video_playlist_item_event_cb, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->video_item6, video_playlist_item_event_cb, LV_EVENT_CLICKED, ui);    
}


static lv_obj_t * g_kb_video;
static void kb_video_event_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *kb = lv_event_get_target(e);
	if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL){
		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
	}
}
__attribute__((unused)) static void ta_video_event_cb(lv_event_t *e)
{

	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *ta = lv_event_get_target(e);
	lv_obj_t *kb = lv_event_get_user_data(e);
	if (code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED)
	{
		lv_keyboard_set_textarea(kb, ta);
		lv_obj_move_foreground(kb);
		lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
	}
	if (code == LV_EVENT_CANCEL || code == LV_EVENT_DEFOCUSED)
	{
		lv_keyboard_set_textarea(kb, NULL);
		lv_obj_move_background(kb);
		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
	}
}

void setup_scr_video(lv_ui *ui){

	//Write codes video
	ui->video = lv_obj_create(NULL);

	//Create keyboard on video
	g_kb_video = lv_keyboard_create(ui->video);
	lv_obj_add_event_cb(g_kb_video, kb_video_event_cb, LV_EVENT_ALL, NULL);
	lv_obj_add_flag(g_kb_video, LV_OBJ_FLAG_HIDDEN);
	lv_obj_set_style_text_font(g_kb_video, &lv_font_simsun_18, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_scrollbar_mode(ui->video, LV_SCROLLBAR_MODE_OFF);

	//Set style for video. Part: LV_PART_MAIN, State: LV_STATE_DEFAULT
	lv_obj_set_style_bg_color(ui->video, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_color(ui->video, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->video, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->video, 235, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes video_imgbtn_1
	ui->video_imgbtn_1 = lv_imgbtn_create(ui->video);
	lv_obj_set_pos(ui->video_imgbtn_1, 964, 40);
	lv_obj_set_size(ui->video_imgbtn_1, 50, 50);
	lv_obj_set_scrollbar_mode(ui->video_imgbtn_1, LV_SCROLLBAR_MODE_OFF);

	//Set style for video_imgbtn_1. Part: LV_PART_MAIN, State: LV_STATE_DEFAULT
	lv_obj_set_style_shadow_width(ui->video_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(ui->video_imgbtn_1, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(ui->video_imgbtn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(ui->video_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(ui->video_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(ui->video_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->video_imgbtn_1, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->video_imgbtn_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_recolor(ui->video_imgbtn_1, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_recolor_opa(ui->video_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->video_imgbtn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Set style for video_imgbtn_1. Part: LV_PART_MAIN, State: LV_STATE_PRESSED
	lv_obj_set_style_shadow_width(ui->video_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_color(ui->video_imgbtn_1, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_opa(ui->video_imgbtn_1, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_spread(ui->video_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_ofs_x(ui->video_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_ofs_y(ui->video_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_color(ui->video_imgbtn_1, lv_color_make(0xFF, 0x33, 0xFF), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_recolor(ui->video_imgbtn_1, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_recolor_opa(ui->video_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_opa(ui->video_imgbtn_1, 255, LV_PART_MAIN|LV_STATE_PRESSED);

	//Set style for video_imgbtn_1. Part: LV_PART_MAIN, State: LV_STATE_CHECKED
	lv_obj_set_style_shadow_width(ui->video_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_color(ui->video_imgbtn_1, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_opa(ui->video_imgbtn_1, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_spread(ui->video_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_ofs_x(ui->video_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_ofs_y(ui->video_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->video_imgbtn_1, lv_color_make(0xFF, 0x33, 0xFF), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_recolor(ui->video_imgbtn_1, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_recolor_opa(ui->video_imgbtn_1, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_opa(ui->video_imgbtn_1, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_imgbtn_set_src(ui->video_imgbtn_1, LV_IMGBTN_STATE_RELEASED, NULL, &_return_alpha_50x50, NULL);
	lv_obj_add_flag(ui->video_imgbtn_1, LV_OBJ_FLAG_CHECKABLE);

    //创建视频播放区域
    ui->video_player = lv_obj_create(ui->video);
    lv_obj_set_pos(ui->video_player, 50, 10);
    lv_obj_set_size(ui->video_player, 900, 500);
    lv_obj_set_style_bg_color(ui->video_player, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->video_player, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui->video_player, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(ui->video_player, LV_SCROLLBAR_MODE_OFF);

    //创建控制面板
    ui->video_ctrl_panel = lv_obj_create(ui->video);
    lv_obj_set_pos(ui->video_ctrl_panel, 50, 520);
    lv_obj_set_size(ui->video_ctrl_panel, 900, 80);
    lv_obj_set_style_bg_color(ui->video_ctrl_panel, lv_color_make(0x30, 0x30, 0x30), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->video_ctrl_panel, 200, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->video_ctrl_panel, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    // 禁用滚动功能
    lv_obj_clear_flag(ui->video_ctrl_panel, LV_OBJ_FLAG_SCROLLABLE); // 禁用滚动
    lv_obj_set_scrollbar_mode(ui->video_ctrl_panel, LV_SCROLLBAR_MODE_OFF); // 关闭滚动条

    //创建播放/暂停按钮
    ui->video_play_btn = lv_btn_create(ui->video_ctrl_panel);
    lv_obj_set_pos(ui->video_play_btn, 10, -10);
    lv_obj_set_size(ui->video_play_btn, 50, 50);
    lv_obj_set_style_radius(ui->video_play_btn, 25, LV_PART_MAIN|LV_STATE_DEFAULT);  
    lv_obj_clear_flag(ui->video_play_btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(ui->video_play_btn, LV_SCROLLBAR_MODE_OFF);

    ui->video_play_label = lv_label_create(ui->video_play_btn);
    lv_label_set_text(ui->video_play_label, LV_SYMBOL_PLAY);
    lv_obj_center(ui->video_play_label);
        
	// 创建进度条
	ui->video_progress_bar = lv_slider_create(ui->video_ctrl_panel);  // 使用slider而不是bar以支持拖动
	lv_obj_set_pos(ui->video_progress_bar, 135, 5);
	lv_obj_set_size(ui->video_progress_bar, 500, 20);
	lv_slider_set_range(ui->video_progress_bar, 0, 1000);  // 使用0-1000范围表示进度百分比
	lv_obj_set_style_bg_color(ui->video_progress_bar, lv_color_make(0x80, 0x80, 0x80), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->video_progress_bar, 100, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->video_progress_bar, lv_color_make(0x00, 0xa0, 0xff), LV_PART_INDICATOR|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->video_progress_bar, 255, LV_PART_INDICATOR|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->video_progress_bar, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui->video_progress_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(ui->video_progress_bar, LV_SCROLLBAR_MODE_OFF);

	// 创建当前时间标签
	ui->video_current_time_label = lv_label_create(ui->video_ctrl_panel);
	lv_obj_set_pos(ui->video_current_time_label, 70, 5);
	lv_label_set_text(ui->video_current_time_label, "00:00");
	lv_obj_set_style_text_color(ui->video_current_time_label, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->video_current_time_label, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);

	// 创建总时长标签
	ui->video_total_time_label = lv_label_create(ui->video_ctrl_panel);
	lv_obj_set_pos(ui->video_total_time_label, 645, 5);
	lv_label_set_text(ui->video_total_time_label, "00:00");
	lv_obj_set_style_text_color(ui->video_total_time_label, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->video_total_time_label, &lv_font_simsun_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    
    //创建音量按钮
    ui->video_volume_btn = lv_btn_create(ui->video_ctrl_panel);
    lv_obj_set_pos(ui->video_volume_btn, 690, -10);
    lv_obj_set_size(ui->video_volume_btn, 50, 50);
    lv_obj_set_style_radius(ui->video_volume_btn, 25, LV_PART_MAIN|LV_STATE_DEFAULT);
    
    ui->video_volume_label = lv_label_create(ui->video_volume_btn);
    lv_label_set_text(ui->video_volume_label, LV_SYMBOL_VOLUME_MAX);
    lv_obj_center(ui->video_volume_label);
    lv_obj_clear_flag(ui->video_volume_label, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(ui->video_volume_label, LV_SCROLLBAR_MODE_OFF);

	//创建音量滑块（默认隐藏）
	ui->video_volume_slider = lv_slider_create(ui->video);  // 改为创建在video根对象上，而不是控制面板上
	lv_obj_set_pos(ui->video_volume_slider, 775, 340);  // 位置调整到音量按钮上方
	lv_obj_set_size(ui->video_volume_slider, 20, 180);
	lv_slider_set_range(ui->video_volume_slider, 0, 100);
	lv_slider_set_value(ui->video_volume_slider, 50, LV_ANIM_OFF);
	lv_obj_set_style_bg_color(ui->video_volume_slider, lv_color_make(0x30, 0x30, 0x30), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->video_volume_slider, 200, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->video_volume_slider, 5, LV_PART_MAIN|LV_STATE_DEFAULT);  // 添加圆角效果
	lv_obj_add_flag(ui->video_volume_slider, LV_OBJ_FLAG_HIDDEN);
    
	//创建全屏按钮
	ui->video_fullscreen_btn = lv_btn_create(ui->video_ctrl_panel);
	lv_obj_set_pos(ui->video_fullscreen_btn, 750, -10);
	lv_obj_set_size(ui->video_fullscreen_btn, 50, 50);
	lv_obj_set_style_radius(ui->video_fullscreen_btn, 25, LV_PART_MAIN|LV_STATE_DEFAULT);

	ui->video_fullscreen_label = lv_label_create(ui->video_fullscreen_btn);
	lv_label_set_text(ui->video_fullscreen_label, LV_SYMBOL_PLUS);
	lv_obj_center(ui->video_fullscreen_label);
    lv_obj_clear_flag(ui->video_fullscreen_label, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(ui->video_fullscreen_label, LV_SCROLLBAR_MODE_OFF);

    //创建播放列表按钮
    ui->video_playlist_btn = lv_btn_create(ui->video_ctrl_panel);
    lv_obj_set_pos(ui->video_playlist_btn, 810, -10);
    lv_obj_set_size(ui->video_playlist_btn, 50, 50);
    lv_obj_set_style_radius(ui->video_playlist_btn, 25, LV_PART_MAIN|LV_STATE_DEFAULT);

    ui->video_playlist_label = lv_label_create(ui->video_playlist_btn);
    lv_label_set_text(ui->video_playlist_label, LV_SYMBOL_LIST);
    lv_obj_center(ui->video_playlist_label);
    
    //创建播放列表弹窗（默认隐藏）
    ui->video_playlist = lv_obj_create(ui->video);
    lv_obj_set_size(ui->video_playlist, 300, 400);
    lv_obj_set_pos(ui->video_playlist, 650, 110);
    lv_obj_set_style_bg_color(ui->video_playlist, lv_color_make(0x30, 0x30, 0x30), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->video_playlist, 230, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->video_playlist, 10, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_add_flag(ui->video_playlist, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_scroll_dir(ui->video_playlist, LV_DIR_VER);
    
	//播放列表内容
	ui->video_playlist_list = lv_list_create(ui->video_playlist);
	lv_obj_set_pos(ui->video_playlist_list, -20, -20);
	lv_obj_set_size(ui->video_playlist_list, 300, 400);
	lv_obj_set_style_text_font(ui->video_playlist_list, &lv_font_simsun_18, LV_PART_MAIN|LV_STATE_DEFAULT);

	//添加示例视频
	lv_list_add_text(ui->video_playlist_list, "视频文件");
	ui->video_item1 = lv_list_add_btn(ui->video_playlist_list, NULL, "老人,海鸥,沙滩.mp4");
	ui->video_item2 = lv_list_add_btn(ui->video_playlist_list, NULL, "最长的电影.mp4");
	ui->video_item3 = lv_list_add_btn(ui->video_playlist_list, NULL, "美人鱼.mp4");
    ui->video_item4 = lv_list_add_btn(ui->video_playlist_list, NULL, "鹬.mp4");   
	ui->video_item5 = lv_list_add_btn(ui->video_playlist_list, NULL, "Anime mix-cut1.mp4");
    ui->video_item6 = lv_list_add_btn(ui->video_playlist_list, NULL, "Anime mix-cut2.mp4");     

	//Init events for screen
	events_init_video(ui);
}

