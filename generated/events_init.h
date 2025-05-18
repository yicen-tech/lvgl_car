/*
 * Copyright 2025 NXP
 * SPDX-License-Identifier: MIT
 * The auto-generated can only be used on NXP devices
 */


#ifndef EVENTS_INIT_H_
#define EVENTS_INIT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"
#include "camera_utils.h"

void events_init(lv_ui *ui);
void events_init_home(lv_ui *ui);
void events_init_music(lv_ui *ui);
void events_init_video(lv_ui *ui);
void events_init_back_up(lv_ui *ui);
void events_init_setting(lv_ui *ui);

void show_music_player(void);
void show_backup_uart(void);
void show_home_uart(void);


#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */