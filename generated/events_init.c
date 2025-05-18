/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: MIT
 * The auto-generated can only be used on NXP devices
 */

#include "events_init.h"
#include <stdio.h>
#include "lvgl/lvgl.h"
#include "camera_utils.h"

extern void close_camera(void);  // 如果close_camera函数在其他文件中定义
extern void cleanup_back_up(lv_ui *ui);


void events_init(lv_ui *ui)
{
}

static void home_music_btn_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		lv_obj_t * act_scr = lv_scr_act();
		lv_disp_t * d = lv_obj_get_disp(act_scr);
		if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr))
		{
			lv_obj_clean(act_scr);
			if (guider_ui.music_del == true)
				setup_scr_music(&guider_ui);
			lv_scr_load_anim(guider_ui.music, LV_SCR_LOAD_ANIM_OVER_LEFT, 100, 100, true);
			guider_ui.home_del = true;
		}
	}
		break;
	default:
		break;
	}
}

void show_music_player(void) {
    lv_obj_t *act_scr = lv_scr_act();
    lv_obj_clean(act_scr);
    if (guider_ui.music_del == true)
        setup_scr_music(&guider_ui);
    lv_scr_load_anim(guider_ui.music, LV_SCR_LOAD_ANIM_OVER_LEFT, 100, 100, true);
    guider_ui.home_del = true;
}

static void home_setting_btn_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		lv_obj_t * act_scr = lv_scr_act();
		lv_disp_t * d = lv_obj_get_disp(act_scr);
		if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr))
		{
			lv_obj_clean(act_scr);
			if (guider_ui.setting_del == true)
				setup_scr_setting(&guider_ui);
			lv_scr_load_anim(guider_ui.setting, LV_SCR_LOAD_ANIM_OVER_LEFT, 100, 100, true);
			guider_ui.home_del = true;
		}
	}
		break;
	default:
		break;
	}
}

static void home_video_btn_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		lv_obj_t * act_scr = lv_scr_act();
		lv_disp_t * d = lv_obj_get_disp(act_scr);
		if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr))
		{
			if (guider_ui.video_del == true)
				setup_scr_video(&guider_ui);
			lv_scr_load_anim(guider_ui.video, LV_SCR_LOAD_ANIM_OVER_LEFT, 100, 100, true);
			guider_ui.home_del = true;
		}
	}
		break;
	default:
		break;
	}
}

void show_backup_uart(void) {
	lv_obj_t *act_scr = lv_scr_act();
	lv_obj_clean(act_scr);
	if (guider_ui.back_up_del == true)
		setup_scr_back_up(&guider_ui);
	lv_scr_load_anim(guider_ui.back_up, LV_SCR_LOAD_ANIM_OVER_LEFT, 100, 100, true);
	guider_ui.home_del = true;
}

static void home_backup_btn_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		lv_obj_t * act_scr = lv_scr_act();
		lv_disp_t * d = lv_obj_get_disp(act_scr);
		if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr))
		{
			if (guider_ui.back_up_del == true)
				setup_scr_back_up(&guider_ui);
			lv_scr_load_anim(guider_ui.back_up, LV_SCR_LOAD_ANIM_OVER_LEFT, 100, 100, true);
			guider_ui.home_del = true;
		}
	}
		break;
	default:
		break;
	}
}

void events_init_home(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->home_music_btn, home_music_btn_event_handler, LV_EVENT_CLICKED, ui);
	lv_obj_add_event_cb(ui->home_setting_btn, home_setting_btn_event_handler, LV_EVENT_CLICKED, ui);
	lv_obj_add_event_cb(ui->home_video_btn, home_video_btn_event_handler, LV_EVENT_CLICKED, ui);
	lv_obj_add_event_cb(ui->home_backup_btn, home_backup_btn_event_handler, LV_EVENT_CLICKED, ui);
}

void show_home_uart(void) {
	lv_ui *ui = &guider_ui; 

	cleanup_back_up(ui);
	setup_scr_home(ui);
	lv_scr_load(ui->home);

	lv_obj_t *act_scr = lv_scr_act();
	lv_obj_clean(act_scr);
	if (guider_ui.home_del == true)
		setup_scr_home(&guider_ui);
	lv_scr_load_anim(guider_ui.home, LV_SCR_LOAD_ANIM_OVER_RIGHT, 100, 100, true);
	guider_ui.back_up_del = true;
}

static void back_up_imgbtn_2_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_ui *ui = lv_event_get_user_data(e);
    
    if (code == LV_EVENT_CLICKED) {
        // 在切换到home界面前清理资源
        cleanup_back_up(ui);
        
        // 然后再切换到home界面
        setup_scr_home(ui);
        lv_scr_load(ui->home);
    }
    switch (code)
    {
    case LV_EVENT_CLICKED:
    {
        // 先加载home界面，再关闭摄像头
        lv_obj_t * act_scr = lv_scr_act();
        lv_disp_t * d = lv_obj_get_disp(act_scr);
        
        if (guider_ui.home_del == true) {
            setup_scr_home(&guider_ui);
        }
        lv_scr_load_anim(guider_ui.home, LV_SCR_LOAD_ANIM_OVER_RIGHT, 100, 100, true);
        guider_ui.back_up_del = true;
        
    }
        break;
    default:
        break;
    }
}

void events_init_back_up(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->back_up_imgbtn_2, back_up_imgbtn_2_event_handler, LV_EVENT_ALL, ui);
}

static void setting_setting_return_btn_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		lv_obj_t * act_scr = lv_scr_act();
		lv_disp_t * d = lv_obj_get_disp(act_scr);
		if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr))
		{
			lv_obj_clean(act_scr);
			if (guider_ui.home_del == true)
				setup_scr_home(&guider_ui);
			lv_scr_load_anim(guider_ui.home, LV_SCR_LOAD_ANIM_OVER_RIGHT, 100, 100, true);
			guider_ui.setting_del = true;
		}
	}
		break;
	default:
		break;
	}
}

void events_init_setting(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->setting_setting_return_btn, setting_setting_return_btn_event_handler, LV_EVENT_ALL, ui);
}

