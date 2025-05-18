#include "lvgl/lvgl.h"
#include "gui_guider.h"
#include "stdio.h"
#include "serial_comm.h"

// 声明图标资源
LV_IMG_DECLARE(_music_icon_alpha_90x90);
LV_IMG_DECLARE(_video_icon_alpha_70x70);
LV_IMG_DECLARE(_backup_icon_alpha_70x70);
LV_IMG_DECLARE(_setting_icon_alpha_70x70);
LV_IMG_DECLARE(game_2048_icon);

static void menu_swipe_event_cb(lv_event_t *e);
static void menu_music_btn_event_cb(lv_event_t *e);
static void menu_video_btn_event_cb(lv_event_t *e);
static void menu_backup_btn_event_cb(lv_event_t *e);
static void menu_setting_btn_event_cb(lv_event_t *e);
static void menu_2048_btn_event_cb(lv_event_t *e);

// 实现右滑事件回调函数
static void menu_swipe_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);

    if (code == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (dir == LV_DIR_RIGHT) {
            // 右滑返回 home 界面
            lv_scr_load_anim(ui->home, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, false);
        }		
    }
}

// 音乐按钮点击事件
static void menu_music_btn_event_cb(lv_event_t *e) {
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
			guider_ui.menu_del = true;
		}
	}
		break;
	default:
		break;
	}
}

// 视频按钮点击事件
static void menu_video_btn_event_cb(lv_event_t *e) {
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
			guider_ui.menu_del = true;
		}
	}
		break;
	default:
		break;
	}
}

// 备份按钮点击事件
static void menu_backup_btn_event_cb(lv_event_t *e) {
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
			guider_ui.menu_del = true;
		}
	}
		break;
	default:
		break;
	}
}

// 设置按钮点击事件
static void menu_setting_btn_event_cb(lv_event_t *e) {
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
			guider_ui.menu_del = true;
		}
	}
		break;
	default:
		break;
	}
}

// 2048按钮点击事件
static void menu_2048_btn_event_cb(lv_event_t *e) {
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
			if (guider_ui.game_2048_del == true)
				setup_scr_2048(&guider_ui);
			lv_scr_load_anim(guider_ui.game_2048, LV_SCR_LOAD_ANIM_OVER_LEFT, 100, 100, true);
			guider_ui.menu_del = true;
		}
	}
		break;
	default:
		break;
	}
}

void setup_scr_menu(lv_ui *ui) {
    // 创建菜单界面
    ui->menu = lv_obj_create(NULL);
    lv_obj_set_scrollbar_mode(ui->menu, LV_SCROLLBAR_MODE_OFF);

    // 设置背景颜色
    lv_obj_set_style_bg_color(ui->menu, lv_color_make(0xFF, 0xFF, 0xFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->menu, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

	// 创建菜单文本
	lv_obj_t *menu_label = lv_label_create(ui->menu);
    lv_label_set_text(menu_label, "菜单");
	lv_obj_set_style_text_font(menu_label, &lv_font_simsun_26, LV_PART_MAIN | LV_STATE_DEFAULT); 	
	lv_obj_set_size(menu_label, 150, 150);
	lv_obj_set_pos(menu_label, 480, 30);

    // 创建音乐按钮
    lv_obj_t *menu_music_btn = lv_imgbtn_create(ui->menu);
    lv_obj_set_size(menu_music_btn, 90, 90);
    lv_obj_set_pos(menu_music_btn, 50, 90);
    lv_obj_set_scrollbar_mode(menu_music_btn, LV_SCROLLBAR_MODE_OFF);
    lv_imgbtn_set_src(menu_music_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_music_icon_alpha_90x90, NULL);
    lv_obj_add_event_cb(menu_music_btn, menu_music_btn_event_cb, LV_EVENT_CLICKED, ui);

	// 添加音乐按钮的文本
	lv_obj_t *music_label = lv_label_create(ui->menu);
	lv_label_set_text(music_label, "音乐");
	lv_obj_set_style_text_font(music_label, &lv_font_simsun_26, LV_PART_MAIN | LV_STATE_DEFAULT); 
	lv_obj_align_to(music_label, menu_music_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    // 创建视频按钮
    lv_obj_t *menu_video_btn = lv_imgbtn_create(ui->menu);
    lv_obj_set_size(menu_video_btn, 70, 70);
    lv_obj_set_pos(menu_video_btn, 150, 100);
    lv_obj_set_scrollbar_mode(menu_video_btn, LV_SCROLLBAR_MODE_OFF);
    lv_imgbtn_set_src(menu_video_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_video_icon_alpha_70x70, NULL);
    lv_obj_add_event_cb(menu_video_btn, menu_video_btn_event_cb, LV_EVENT_CLICKED, ui);

    // 添加视频按钮的文本
    lv_obj_t *video_label = lv_label_create(ui->menu);
    lv_label_set_text(video_label, "视频");
	lv_obj_set_style_text_font(video_label, &lv_font_simsun_26, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_align_to(video_label, menu_video_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 14);

    // 创建倒车影像按钮
    lv_obj_t *menu_backup_btn = lv_imgbtn_create(ui->menu);
    lv_imgbtn_set_src(menu_backup_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_backup_icon_alpha_70x70, NULL);
    lv_obj_set_size(menu_backup_btn, 70, 70);
    lv_obj_set_pos(menu_backup_btn, 250, 100);
    lv_obj_add_event_cb(menu_backup_btn, menu_backup_btn_event_cb, LV_EVENT_CLICKED, ui);

    // 添加倒车影像按钮的文本
    lv_obj_t *backup_label = lv_label_create(ui->menu);
    lv_label_set_text(backup_label, "倒车影像");
	lv_obj_set_style_text_font(backup_label, &lv_font_simsun_26, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_align_to(backup_label, menu_backup_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 14);

    // 创建设置按钮
    lv_obj_t *menu_setting_btn = lv_imgbtn_create(ui->menu);
    lv_imgbtn_set_src(menu_setting_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_setting_icon_alpha_70x70, NULL);
    lv_obj_set_size(menu_setting_btn, 70, 70);
    lv_obj_set_pos(menu_setting_btn, 350, 100);
    lv_obj_add_event_cb(menu_setting_btn, menu_setting_btn_event_cb, LV_EVENT_CLICKED, ui);

    // 添加设置按钮的文本
    lv_obj_t *setting_label = lv_label_create(ui->menu);
    lv_label_set_text(setting_label, "设置");
	lv_obj_set_style_text_font(setting_label, &lv_font_simsun_26, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_align_to(setting_label, menu_setting_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 14);

    // 创建2048按钮
    lv_obj_t *menu_2048_btn = lv_imgbtn_create(ui->menu);
    lv_imgbtn_set_src(menu_2048_btn, LV_IMGBTN_STATE_RELEASED, NULL, &game_2048_icon, NULL);
    lv_obj_set_size(menu_2048_btn, 70, 70);
    lv_obj_set_pos(menu_2048_btn, 450, 100);
    lv_obj_add_event_cb(menu_2048_btn, menu_2048_btn_event_cb, LV_EVENT_CLICKED, ui);	

    // 添加2048按钮的文本
    lv_obj_t *game_label = lv_label_create(ui->menu);
    lv_label_set_text(game_label, "2048");
	lv_obj_set_style_text_font(game_label, &lv_font_simsun_26, LV_PART_MAIN | LV_STATE_DEFAULT); 	
    lv_obj_align_to(game_label, menu_2048_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 14);

    // 为菜单界面添加右滑事件回调
    lv_obj_add_event_cb(ui->menu, menu_swipe_event_cb, LV_EVENT_GESTURE, ui);
}


