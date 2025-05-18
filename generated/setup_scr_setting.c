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

LV_IMG_DECLARE(wifi); // WIFI icon
LV_IMG_DECLARE(bluetooth); // 蓝牙 icon
LV_IMG_DECLARE(brightness); // 亮度 icon
LV_IMG_DECLARE(sound); // 声音 icon
LV_IMG_DECLARE(about); // 关于 icon

lv_obj_t *content_label = NULL; // 用于显示设置内容的标签
lv_obj_t *content_container = NULL; // 用于显示设置内容的容器
lv_obj_t *table = NULL; // 用于显示关于信息的表格

static lv_obj_t * g_kb_setting;
static void kb_setting_event_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *kb = lv_event_get_target(e);
	if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL){
		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
	}
}
__attribute__((unused)) static void ta_setting_event_cb(lv_event_t *e)
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

// // Event callback functions
// static void wifi_event_cb(lv_event_t *e) {
//     printf("WiFi clicked\n");
// }

// static void bluetooth_event_cb(lv_event_t *e) {
//     printf("Bluetooth clicked\n");
// }

// static void brightness_event_cb(lv_event_t *e) {
//     printf("Brightness clicked\n");
// }

// static void sound_event_cb(lv_event_t *e) {
//     printf("Sound clicked\n");
// }

// static void about_event_cb(lv_event_t *e) {
//     printf("About clicked\n");
// }

// Event callback for list items
void list_item_event_cb(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *list = lv_event_get_user_data(e);

    const char *text = lv_list_get_btn_text(list, btn);

    // 根据按钮文本更新右侧内容
    if (strcmp(text, "WiFi") == 0) {
        lv_label_set_text(content_label, "               WiFi \n网络名称: MyWiFi\n信号强度: 强");
    } else if (strcmp(text, "蓝牙") == 0) {
        lv_label_set_text(content_label, "               蓝牙 \n设备名称: MyDevice\n状态: 已连接");
    } else if (strcmp(text, "亮度") == 0) {
        lv_label_set_text(content_label, "               亮度 \n当前亮度: 80%\n自动调节: 关闭");
    } else if (strcmp(text, "声音") == 0) {
        lv_label_set_text(content_label, "               声音 \n音量: 50%\n静音模式: 关闭");
    } else if (strcmp(text, "关于") == 0) {
		lv_label_set_text(content_label, "               关于 \n版本: 1.0.0\n芯片型号: 全志H618\n性能参数: 2G+8G\n软件平台: Linux&LVGL V8.3\n作者: CY\n日期: 2025/05/02");
    }
}

void setup_scr_setting(lv_ui *ui){

	//Write codes setting
	ui->setting = lv_obj_create(NULL);

	//Create keyboard on setting
	g_kb_setting = lv_keyboard_create(ui->setting);
	lv_obj_add_event_cb(g_kb_setting, kb_setting_event_cb, LV_EVENT_ALL, NULL);
	lv_obj_add_flag(g_kb_setting, LV_OBJ_FLAG_HIDDEN);
	lv_obj_set_style_text_font(g_kb_setting, &lv_font_simsun_18, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_scrollbar_mode(ui->setting, LV_SCROLLBAR_MODE_OFF);

	//Set style for setting. Part: LV_PART_MAIN, State: LV_STATE_DEFAULT
	lv_obj_set_style_bg_color(ui->setting, lv_color_make(0xff, 0xf5, 0xf5), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_color(ui->setting, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->setting, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->setting, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes setting_setting_return_btn
	ui->setting_setting_return_btn = lv_imgbtn_create(ui->setting);
	lv_obj_set_pos(ui->setting_setting_return_btn, 940, 45);
	lv_obj_set_size(ui->setting_setting_return_btn, 50, 50);
	lv_obj_set_scrollbar_mode(ui->setting_setting_return_btn, LV_SCROLLBAR_MODE_OFF);

	//Set style for setting_setting_return_btn. Part: LV_PART_MAIN, State: LV_STATE_DEFAULT
	lv_obj_set_style_shadow_width(ui->setting_setting_return_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(ui->setting_setting_return_btn, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(ui->setting_setting_return_btn, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(ui->setting_setting_return_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(ui->setting_setting_return_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(ui->setting_setting_return_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->setting_setting_return_btn, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->setting_setting_return_btn, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_recolor(ui->setting_setting_return_btn, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_recolor_opa(ui->setting_setting_return_btn, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->setting_setting_return_btn, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Set style for setting_setting_return_btn. Part: LV_PART_MAIN, State: LV_STATE_PRESSED
	lv_obj_set_style_shadow_width(ui->setting_setting_return_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_color(ui->setting_setting_return_btn, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_opa(ui->setting_setting_return_btn, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_spread(ui->setting_setting_return_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_ofs_x(ui->setting_setting_return_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_ofs_y(ui->setting_setting_return_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_color(ui->setting_setting_return_btn, lv_color_make(0xFF, 0x33, 0xFF), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_recolor(ui->setting_setting_return_btn, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_recolor_opa(ui->setting_setting_return_btn, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_opa(ui->setting_setting_return_btn, 255, LV_PART_MAIN|LV_STATE_PRESSED);

	//Set style for setting_setting_return_btn. Part: LV_PART_MAIN, State: LV_STATE_CHECKED
	lv_obj_set_style_shadow_width(ui->setting_setting_return_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_color(ui->setting_setting_return_btn, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_opa(ui->setting_setting_return_btn, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_spread(ui->setting_setting_return_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_ofs_x(ui->setting_setting_return_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_ofs_y(ui->setting_setting_return_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->setting_setting_return_btn, lv_color_make(0xFF, 0x33, 0xFF), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_recolor(ui->setting_setting_return_btn, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_recolor_opa(ui->setting_setting_return_btn, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_opa(ui->setting_setting_return_btn, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_imgbtn_set_src(ui->setting_setting_return_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_return_alpha_50x50, NULL);
	lv_obj_add_flag(ui->setting_setting_return_btn, LV_OBJ_FLAG_CHECKABLE);

    // Create a horizontal container for the list and content
    lv_obj_t *main_container = lv_obj_create(ui->setting);
    lv_obj_set_size(main_container, 800, 400);
    lv_obj_align(main_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_flex_flow(main_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(main_container, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(main_container, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(main_container, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(main_container, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Create the list for the settings items
    lv_obj_t *list = lv_list_create(main_container);
    lv_obj_set_size(list, 300, 400);
	lv_obj_set_style_text_font(list, &lv_font_simsun_26, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(list, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Create the content container for the right side
    content_container = lv_obj_create(main_container);
    lv_obj_set_size(content_container, 480, 400);
    lv_obj_set_style_pad_all(content_container, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(content_container, lv_color_make(0xf0, 0xf0, 0xf0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(content_container, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

	// 创建多行标签用于显示内容
	content_label = lv_label_create(content_container);
	lv_label_set_text(content_label, "请选择一个选项");
	lv_label_set_long_mode(content_label, LV_LABEL_LONG_WRAP); // 设置为多行模式
	lv_obj_set_width(content_label, lv_pct(100)); // 设置标签宽度以支持换行
	lv_obj_align(content_label, LV_ALIGN_TOP_LEFT, 0, 0); // 左上角对齐
	lv_obj_set_style_text_font(content_label, &lv_font_simsun_26, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add items to the list
    lv_obj_t *btn;
    btn = lv_list_add_btn(list, &wifi, "WiFi");
    lv_obj_add_event_cb(btn, list_item_event_cb, LV_EVENT_CLICKED, list);

    btn = lv_list_add_btn(list, &bluetooth, "蓝牙");
    lv_obj_add_event_cb(btn, list_item_event_cb, LV_EVENT_CLICKED, list);

    btn = lv_list_add_btn(list, &brightness, "亮度");
    lv_obj_add_event_cb(btn, list_item_event_cb, LV_EVENT_CLICKED, list);

    btn = lv_list_add_btn(list, &sound, "声音");
    lv_obj_add_event_cb(btn, list_item_event_cb, LV_EVENT_CLICKED, list);

    btn = lv_list_add_btn(list, &about, "关于");
    lv_obj_add_event_cb(btn, list_item_event_cb, LV_EVENT_CLICKED, list);

	//Init events for screen
	events_init_setting(ui);
}