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
#include "lvgl/demos/music/lv_demo_music.h"
#include "lvgl/demos/music/lv_demo_music_list.h"
#include "lvgl/demos/music/lv_demo_music_main.h"
#include "audio_player.h"
#include "lvgl/demos/music/lv_demo_music_main.h"

extern uint8_t play_id; // 播放器ID

static lv_obj_t * g_kb_music;
static lv_obj_t * ctrl; // 音乐控制界面
static lv_obj_t * list; // 音乐列表

static void kb_music_event_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *kb = lv_event_get_target(e);
	if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL){
		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
	}
}
__attribute__((unused)) static void ta_music_event_cb(lv_event_t *e)
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
static void music_music_return_btn_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		_lv_demo_music_pause();
        audio_player_pause();
		audio_player_cleanup(); // 清理音频播放器资源
		audio_player_stop(); // 停止音频播放
		audio_player_set_time(0); // 重置播放时间

		player_pause_flag = 0;

        if (list != NULL) {
			lv_obj_del(list);
			list = NULL;
		}
		if (ctrl != NULL) {
			lv_obj_del(ctrl);
			ctrl = NULL;
		}
		lv_obj_t * act_scr = lv_scr_act();
		lv_disp_t * d = lv_obj_get_disp(act_scr);
		if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr))
		{
			lv_obj_clean(act_scr);
			if (guider_ui.home_del == true)
				setup_scr_home(&guider_ui);
			lv_scr_load_anim(guider_ui.home, LV_SCR_LOAD_ANIM_OVER_RIGHT, 100, 100, true);
			guider_ui.music_del = true;
		}
	}
		break;
	default:
		break;
	}
}

void events_init_music(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->music_music_return_btn, music_music_return_btn_event_handler, LV_EVENT_ALL, ui);
}

void setup_scr_music(lv_ui *ui){

	//Write codes music
	ui->music = lv_obj_create(NULL);
	if (ui->music == NULL) {
		printf("Error: Failed to create ui->music\n");
		return;
	}
	lv_obj_set_scrollbar_mode(ui->music, LV_SCROLLBAR_MODE_OFF);

	// 设置界面背景色为音乐demo样式
	lv_obj_set_style_bg_color(ui->music, lv_color_hex(0x343247), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_color(ui->music, lv_color_hex(0x343247), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->music, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->music, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	// 创建键盘（保留原有功能）
	g_kb_music = lv_keyboard_create(ui->music);
	lv_obj_add_event_cb(g_kb_music, kb_music_event_cb, LV_EVENT_ALL, NULL);
	lv_obj_add_flag(g_kb_music, LV_OBJ_FLAG_HIDDEN);
	lv_obj_set_style_text_font(g_kb_music, &lv_font_simsun_18, LV_PART_MAIN|LV_STATE_DEFAULT);

	// 创建返回按钮
	ui->music_music_return_btn = lv_imgbtn_create(ui->music);
	lv_obj_set_pos(ui->music_music_return_btn, 920, 45);
	lv_obj_set_size(ui->music_music_return_btn, 50, 50);
	lv_obj_set_scrollbar_mode(ui->music_music_return_btn, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_style_img_recolor(ui->music_music_return_btn, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_imgbtn_set_src(ui->music_music_return_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_return_alpha_50x50, NULL);
	lv_obj_add_flag(ui->music_music_return_btn, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(ui->music_music_return_btn, music_music_return_btn_event_handler, LV_EVENT_CLICKED, NULL);
	lv_obj_move_foreground(ui->music_music_return_btn);  // 确保返回按钮在最前面

	// 初始化音乐组件
	if (list != NULL) {
		lv_obj_del(list);
		list = NULL;
	}
	if (ctrl != NULL) {
		lv_obj_del(ctrl);
		ctrl = NULL;
	}

	// 初始化音频播放器
    if (audio_player_init() < 0) {
        printf("音频播放器初始化失败\n");
    }
	switch (play_id)  /* Replace with actual expression */
	{
	case 1:
		audio_player_load("/home/kickpi/dowmload/lvgl_demo/app/music/music1.mp3");
		break;
	case 2:
		audio_player_load("/home/kickpi/dowmload/lvgl_demo/app/music/music2.mp3");
		break;            
	case 3:
		audio_player_load("/home/kickpi/dowmload/lvgl_demo/app/music/music3.mp3");
		break;
	case 4:
		audio_player_load("/home/kickpi/dowmload/lvgl_demo/app/music/music4.mp3");
		break;                    
	default:
		break;
	}
	#if defined(LV_USE_DEMO_MUSIC) && LV_USE_DEMO_MUSIC
	// 初始化音乐demo的组件
	list = _lv_demo_music_list_create(ui->music);
	if (list == NULL) {
		printf("Error: _lv_demo_music_list_create returned NULL\n");
		return;
	}
	
	ctrl = _lv_demo_music_main_create(ui->music);
	if (ctrl == NULL) {
		printf("Error: _lv_demo_music_main_create returned NULL\n");
		return;
	}

	// 调整组件位置以适应您的界面布局
	lv_obj_align(ctrl, LV_ALIGN_CENTER, 0, 0);
	lv_obj_align(list, LV_ALIGN_CENTER, 0, 0);
	
	lv_obj_move_foreground(ui->music_music_return_btn);  // 确保返回按钮在最前面

	// // 播放第一首歌曲作为默认状态
	// _lv_demo_music_play(0);
	#else
	// 如果未启用LVGL音乐demo，显示提示消息
	lv_obj_t *label = lv_label_create(ui->music);
	lv_label_set_text(label, "请在lv_conf.h中启用LV_USE_DEMO_MUSIC功能");
	lv_obj_set_style_text_font(label, &lv_font_simsun_26, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(label, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
	#endif

	// 初始化事件处理
	events_init_music(ui);
}