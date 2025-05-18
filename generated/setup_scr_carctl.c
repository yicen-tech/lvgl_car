#include "lvgl/lvgl.h"
#include "gui_guider.h"
#include <stdio.h>
#include "events_init.h"
#include "serial_comm.h"
#include "camera_utils.h"
#include "custom/weather_parser.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

// 声明图标资源
LV_IMG_DECLARE(_D_alpha_50x50);
LV_IMG_DECLARE(_P_alpha_60x60);
LV_IMG_DECLARE(_R_alpha_50x50);
LV_IMG_DECLARE(_N_alpha_50x50);
LV_IMG_DECLARE(_car_body_alpha_300x150);
LV_IMG_DECLARE(_car_lamp_on_alpha_50x50);
LV_IMG_DECLARE(_car_lamp_off_alpha_50x50);
LV_IMG_DECLARE(_double_flash_alpha_100x100);
LV_IMG_DECLARE(_unlock_alpha_50x50);
LV_IMG_DECLARE(_lock_alpha_50x50);
LV_IMG_DECLARE(_ws_wiper_alpha_50x50);

static void carctl_swipe_event_cb(lv_event_t *e);
static void carctl_P_btn_event_cb(lv_event_t *e);
static void carctl_R_btn_event_cb(lv_event_t *e);
static void carctl_lamp_off_event_cb(lv_event_t *e);
static void carctl_double_flash_event_cb(lv_event_t *e);
static void carctl_lock_event_cb(lv_event_t *e);
static void carctl_ws_wiper_sub_event_cb(lv_event_t *e);
static void carctl_ws_wiper_add_event_cb(lv_event_t *e);

static int ws_wiper_level = 0; // 初始档位为0
static lv_obj_t *ws_wiper_label; // 用于显示档位的文本对象
static bool is_lamp_on = false; // 初始状态为关闭
static bool is_double_flash_on = false; 
static bool is_locked = true; // 初始状态为锁定
static lv_obj_t *extra_btn1 = NULL;
static lv_obj_t *extra_btn2 = NULL;
static lv_obj_t *extra_btn3 = NULL;
static bool is_left_door_on = false;
static bool is_right_door_on = false;
static bool is_trunk_on = false;
static lv_obj_t *camera_canvas = NULL;
static int camera_canvas_exist = 0; // 摄像头画布是否存在
static int camera_exit_flag = 0; // 摄像头退出标志

static lv_coord_t col_dsc[] = {80, 80, 80, LV_GRID_TEMPLATE_LAST}; // 一列，宽120像素
static lv_coord_t row_dsc[] = {40, 40, LV_GRID_TEMPLATE_LAST}; // 两行，每行40像素

static lv_timer_t *date_time_update_timer = NULL; // 定时器变量

static void update_new_time_cb(lv_timer_t *timer);

// 时钟更新回调函数
static void update_new_time_cb(lv_timer_t *timer)
{
    // printf("update_time_cb called\n");
    lv_ui *ui = (lv_ui *)timer->user_data;

    if (ui == NULL && ui->carctl == NULL) {
        printf("carctl is NULL\n");
        return;
    }
    
    time_t now;
    struct tm tm;

    // 获取当前本地时间
    now = time(NULL);
    localtime_r(&now, &tm);

    char time_str[9]; // 存放格式化后的时间字符串，HH:MM:SS
    char date_str[20]; // 存放格式化后的日期字符串，YYYY-MM-DD 

    // 将时间格式化为字符串，精确到秒
    snprintf(time_str, sizeof(time_str), "%02d:%02d", tm.tm_hour, tm.tm_min);
    // 将日期格式化为字符串，精确到日
    snprintf(date_str, sizeof(date_str), "%02d月%02d日", tm.tm_mon + 1, tm.tm_mday);

    // 更新UI元素
    if (lv_obj_is_valid(ui->carctl_time_label)) {
        lv_label_set_text(ui->carctl_time_label, time_str);
    }
    if (lv_obj_is_valid(ui->carctl_date_label)) {
        lv_label_set_text(ui->carctl_date_label, date_str);
    }
}

static void my_update_date_time(void)
{
    // lv_ui *ui = (lv_ui *)timer->user_data;

    // time_t now;
    // struct tm tm;

    // // 获取当前本地时间
    // now = time(NULL);
    // localtime_r(&now, &tm);

    // char time_str[9]; // 存放格式化后的时间字符串，HH:MM:SS
    // char date_str[20]; // 存放格式化后的日期字符串，YYYY-MM-DD 

    // // 将时间格式化为字符串，精确到秒
    // snprintf(time_str, sizeof(time_str), "%02d:%02d", tm.tm_hour, tm.tm_min);
    // // 将日期格式化为字符串，精确到日
    // snprintf(date_str, sizeof(date_str), "%02d月%02d日", tm.tm_mon + 1, tm.tm_mday);

    // // 更新UI元素
    // if (lv_obj_is_valid(ui->carctl_time_label)) {
    //     lv_label_set_text(ui->carctl_time_label, time_str);
    // }
    // if (lv_obj_is_valid(ui->carctl_date_label)) {
    //     lv_label_set_text(ui->carctl_date_label, date_str);
    // }
}

// 更新雨刷档位文本
static void update_ws_wiper_label() {
    char buf[4];
    sprintf(buf, "%d", ws_wiper_level);
    lv_label_set_text(ws_wiper_label, buf);

    if(ws_wiper_level == 0){
        const char *msg1 = "e";
        serial_send(fd, msg1, strlen(msg1));
    }        
    else if(ws_wiper_level == 1){
        const char *msg2 = "b";
        serial_send(fd, msg2, strlen(msg2));
    }
    else if(ws_wiper_level == 2){
        const char *msg3 = "c";
        serial_send(fd, msg3, strlen(msg3));
    }    
    else if(ws_wiper_level == 3){
        const char *msg4 = "d";
        serial_send(fd, msg4, strlen(msg4));
    } 
}

// 实现右滑事件回调函数
static void carctl_swipe_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);

    if (code == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if (dir == LV_DIR_LEFT) {
			if(camera_exit_flag == 1){
				cleanup_back_up(ui);
			}
            if (date_time_update_timer != NULL) {
                lv_timer_del(date_time_update_timer);
                date_time_update_timer = NULL;
            }
            // // 左滑返回 home 界面
            // lv_scr_load_anim(ui->home, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);

			lv_obj_t * act_scr = lv_scr_act();
			lv_disp_t * d = lv_obj_get_disp(act_scr);
			if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr))
			{
				if (guider_ui.home_del == true)
					setup_scr_home(&guider_ui);
				lv_scr_load_anim(guider_ui.home, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, true);
				guider_ui.carctl_del = true;
			}

			camera_canvas_exist = 0;
        }
    }
}

static void carctl_P_btn_event_cb(lv_event_t *e) {
	lv_ui *ui = lv_event_get_user_data(e);

    // 如果画布存在，则隐藏画布
    if (camera_canvas_exist) {
		cleanup_back_up(ui);
        lv_obj_add_flag(camera_canvas, LV_OBJ_FLAG_HIDDEN);
        // printf("Camera canvas hidden\n");
    }
	camera_exit_flag = 0;
    // 显示原有控件（如果有需要，可以在这里添加逻辑）
    // printf("Original controls displayed\n");
}

static void *camera_thread(void *arg)
{
    // printf("摄像头线程开始运行\n");
    
    // 检查camera结构体是否正确初始化
    if (camera.fd == -1) {
        printf("错误：摄像头未打开\n");
        return NULL;
    }
    
    if (!camera.img_data) {
        printf("错误：图像数据缓冲区未分配\n");
        return NULL;
    }
    
    if (!camera.buffers) {
        printf("错误：摄像头缓冲区未分配\n");
        return NULL;
    }
    
    struct v4l2_buffer buf;
    // 初始化最后更新时间
    clock_gettime(CLOCK_MONOTONIC, &camera.last_update_time);    
    while (camera.running) {
        // 从队列中取出一个已填充的缓冲区
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        
        // 添加超时处理
        fd_set fds;
        struct timeval tv;
        int r;
        
        FD_ZERO(&fds);
        FD_SET(camera.fd, &fds);
        
        // 设置超时为1秒
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        
        r = select(camera.fd + 1, &fds, NULL, NULL, &tv);
        
        if (r == -1) {
            if (errno == EINTR) continue;
            printf("select错误: %s\n", strerror(errno));
            break;
        }
        
        if (r == 0) {
            // 超时，检查是否应该继续运行
            if (!camera.running) break;
            continue;
        }
        
        if (ioctl(camera.fd, VIDIOC_DQBUF, &buf) == -1) {
            if (errno == EAGAIN) continue;
            // printf("VIDIOC_DQBUF失败: %s\n", strerror(errno));
            break;
        }
        
        // 确保索引有效
        if (buf.index >= camera.n_buffers) {
            //  printf("缓冲区索引超出范围: %d >= %d\n", buf.index, camera.n_buffers);
            continue;
        }
        
        // 复制图像数据并进行格式转换
        pthread_mutex_lock(&camera.mutex);
        if (camera.img_data && camera.buffers[buf.index].start) {
            if (camera.need_conversion) {
                // 先复制原始YUYV数据到转换缓冲区
                memcpy(camera.conv_buffer, camera.buffers[buf.index].start, buf.bytesused);
                // 然后转换为RGB32格式
                yuyv_to_rgb32_optimized(camera.conv_buffer, (uint32_t *)camera.img_data, 
                            camera.width, camera.height);
                camera.frame_changed = true; // 标记帧已更新               
            } else {
                // 如果不需要转换，直接复制
                memcpy(camera.img_data, camera.buffers[buf.index].start, camera.img_dsc.data_size);
                camera.frame_changed = true; // 标记帧已更新
            }
        }
        pthread_mutex_unlock(&camera.mutex);
        // 获取当前时间
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);        
        // 计算自上次更新UI以来的时间（毫秒）
        long elapsed_ms = (current_time.tv_sec - camera.last_update_time.tv_sec) * 1000 + 
                         (current_time.tv_nsec - camera.last_update_time.tv_nsec) / 1000000;

        // 只有当帧发生变化且距离上次更新至少经过50ms时才更新UI
        if (camera.frame_changed && elapsed_ms >= 50) {
            // 通知LVGL更新图像
            if (camera.running && camera.img_obj) {
                img_update_data_t *update_data = malloc(sizeof(img_update_data_t));
                if (update_data) {
                    update_data->img_obj = camera.img_obj;
                    update_data->img_dsc = &camera.img_dsc;
                    lv_async_call(img_update_cb, update_data);
                } else {
                    // printf("错误：无法分配更新数据内存\n");
                }
            }
            // 重置标志并更新时间
            camera.frame_changed = false;
            camera.last_update_time = current_time;
        }
        // 将缓冲区放回队列
        if (ioctl(camera.fd, VIDIOC_QBUF, &buf) == -1) {
            // printf("VIDIOC_QBUF失败: %s\n", strerror(errno));
            break;
        }
        
        // 短暂延时，控制帧率
        usleep(30000); // 约30fps
    }
    // printf("摄像头线程结束\n");
    return NULL;
}

static void carctl_R_btn_event_cb(lv_event_t *e) {
    lv_ui *ui = lv_event_get_user_data(e);

	camera_exit_flag = 1;
    // 如果画布不存在，则创建画布
    if (!camera_canvas_exist) {
        camera_canvas = lv_img_create(lv_scr_act());
        lv_obj_set_size(camera_canvas, 800, 600); // 设置画布大小
        lv_obj_set_pos(camera_canvas, 150, 0);   // 设置画布位置
        lv_obj_set_style_bg_color(camera_canvas, lv_color_make(0, 0, 0), LV_PART_MAIN | LV_STATE_DEFAULT); // 设置背景颜色为黑色
        lv_obj_set_style_bg_opa(camera_canvas, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT); // 设置背景不透明
		camera_canvas_exist = 1;
    }

    // 显示画布
    lv_obj_clear_flag(camera_canvas, LV_OBJ_FLAG_HIDDEN);

	// 检查摄像头设备是否存在
	const char *camera_dev = "/dev/video0";
	if (check_camera_device(camera_dev) != 0) {
		// 显示错误信息
		// printf("摄像头设备不存在\n");
        return;
		lv_obj_t *label = lv_label_create(ui->carctl);
		lv_label_set_text(label, "摄像头设备不存在");
		lv_obj_set_style_text_font(label, &lv_font_simsun_18, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_pos(label, 10, 100);
	}
	// 初始化摄像头
	if (init_camera(camera_dev, 800, 600) == 0) {
		// printf("初始化摄像头\n");
		// 创建一个初始图像
		memset(camera.img_data, 0, camera.width * camera.height * 2); // 清空图像数据
		
		// 设置图像对象
		camera.img_obj = camera_canvas;
		lv_img_set_src(camera.img_obj, &camera.img_dsc);
		
		// 创建摄像头线程
		camera.running = true; // 确保在创建线程前设置running为true
		// printf("创建摄像头线程\n");

		if (pthread_create(&camera.thread, NULL, camera_thread, NULL) != 0) {
			// printf("摄像头线程创建失败\n");
			close_camera();
			
			// 显示错误信息
			lv_obj_t *label = lv_label_create(ui->carctl);
			lv_label_set_text(label, "摄像头线程创建失败");
			lv_obj_set_pos(label, 50, 100);
		}
	} else {
		// 显示错误信息
		// printf("摄像头初始化失败\n");
	}
}

static void carctl_lamp_off_event_cb(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e); // 获取触发事件的按钮对象

    if (is_lamp_on) {
        const char *msg1 = "2";
        serial_send(fd, msg1, strlen(msg1));
        // 当前是灯光开启状态，切换为关闭状态
        lv_imgbtn_set_src(btn, LV_IMGBTN_STATE_RELEASED, NULL, &_car_lamp_off_alpha_50x50, NULL);
        // printf("Lamp turned OFF\n");
    } else {
        // 当前是灯光关闭状态，切换为开启状态
        const char *msg2 = "1";
        serial_send(fd, msg2, strlen(msg2));
        lv_imgbtn_set_src(btn, LV_IMGBTN_STATE_RELEASED, NULL, &_car_lamp_on_alpha_50x50, NULL);
        // printf("Lamp turned ON\n");
    }

    // 切换状态
    is_lamp_on = !is_lamp_on;
}

static void carctl_double_flash_event_cb(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e); // 获取触发事件的按钮对象

    if (is_double_flash_on) {
        const char *msg1 = "4";
        serial_send(fd, msg1, strlen(msg1));
    } else {
        const char *msg2 = "3";
        serial_send(fd, msg2, strlen(msg2));
    }

    // 切换状态
    is_double_flash_on = !is_double_flash_on;
}

// 额外按钮1的点击事件回调
static void extra_btn1_event_cb(lv_event_t *e) {
    
    lv_obj_t *btn = lv_event_get_target(e); // 获取触发事件的按钮对象

    if (is_right_door_on) {
        const char *msg1 = "8";
        serial_send(fd, msg1, strlen(msg1));
    } else {
        const char *msg2 = "7";
        serial_send(fd, msg2, strlen(msg2));
    }

    // 切换状态
    is_right_door_on = !is_right_door_on;
}

// 额外按钮2的点击事件回调
static void extra_btn2_event_cb(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e); // 获取触发事件的按钮对象

    if (is_left_door_on) {
        const char *msg1 = "6";
        serial_send(fd, msg1, strlen(msg1));
    } else {
        const char *msg2 = "5";
        serial_send(fd, msg2, strlen(msg2));
    }

    // 切换状态
    is_left_door_on = !is_left_door_on;
}

// 额外按钮3的点击事件回调
static void extra_btn3_event_cb(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e); // 获取触发事件的按钮对象

    if (is_trunk_on) {
        const char *msg1 = "a";
        serial_send(fd, msg1, strlen(msg1));
    } else {
        const char *msg2 = "9";
        serial_send(fd, msg2, strlen(msg2));
    }

    // 切换状态
    is_trunk_on = !is_trunk_on;
}

static void carctl_lock_event_cb(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e); // 获取触发事件的按钮对象

    if (is_locked) {
        // 当前是锁定状态，切换为解锁状态
        lv_imgbtn_set_src(btn, LV_IMGBTN_STATE_RELEASED, NULL, &_unlock_alpha_50x50, NULL);
        // printf("Unlocked\n");

        // 创建3个空白按钮
        extra_btn1 = lv_btn_create(lv_scr_act());
        lv_obj_set_size(extra_btn1, 35, 35);
        lv_obj_set_pos(extra_btn1, 475, 300);
		lv_obj_set_style_bg_color(extra_btn1, lv_color_make(0x00, 0xBF, 0xFF), LV_PART_MAIN | LV_STATE_DEFAULT); // 蓝色背景
		lv_obj_set_style_bg_opa(extra_btn1, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT); // 设置背景不透明
        lv_obj_add_event_cb(extra_btn1, extra_btn1_event_cb, LV_EVENT_CLICKED, NULL); // 添加点击事件

        extra_btn2 = lv_btn_create(lv_scr_act());
        lv_obj_set_size(extra_btn2, 35, 35);
        lv_obj_set_pos(extra_btn2, 475, 440);
		lv_obj_set_style_bg_color(extra_btn2, lv_color_make(0x00, 0xBF, 0xFF), LV_PART_MAIN | LV_STATE_DEFAULT); // 蓝色背景
		lv_obj_set_style_bg_opa(extra_btn2, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT); // 设置背景不透明
        lv_obj_add_event_cb(extra_btn2, extra_btn2_event_cb, LV_EVENT_CLICKED, NULL); // 添加点击事件

        extra_btn3 = lv_btn_create(lv_scr_act());
        lv_obj_set_size(extra_btn3, 35, 35);
        lv_obj_set_pos(extra_btn3, 660, 370);
		lv_obj_set_style_bg_color(extra_btn3, lv_color_make(0x00, 0xBF, 0xFF), LV_PART_MAIN | LV_STATE_DEFAULT); // 蓝色背景
		lv_obj_set_style_bg_opa(extra_btn3, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT); // 设置背景不透明
        lv_obj_add_event_cb(extra_btn3, extra_btn3_event_cb, LV_EVENT_CLICKED, NULL); // 添加点击事件		
    } else {
        // 当前是解锁状态，切换为锁定状态
        lv_imgbtn_set_src(btn, LV_IMGBTN_STATE_RELEASED, NULL, &_lock_alpha_50x50, NULL);

        // 删除3个空白按钮
        if (extra_btn1) {
            lv_obj_del(extra_btn1);
            extra_btn1 = NULL;
        }
        if (extra_btn2) {
            lv_obj_del(extra_btn2);
            extra_btn2 = NULL;
        }
        if (extra_btn3) {
            lv_obj_del(extra_btn3);
            extra_btn3 = NULL;
        }		
    }

    // 切换状态
    is_locked = !is_locked;
}

static void carctl_ws_wiper_sub_event_cb(lv_event_t *e) {

	if (ws_wiper_level > 0) {
        ws_wiper_level--;
        update_ws_wiper_label();
    }
}
static void carctl_ws_wiper_add_event_cb(lv_event_t *e) {

    if (ws_wiper_level < 3) {
        ws_wiper_level++;
        update_ws_wiper_label();   
    }
}


void setup_scr_carctl(lv_ui *ui) {
    // 如果已经存在定时器，先删除它
    if (date_time_update_timer != NULL) {
        lv_timer_del(date_time_update_timer);
        date_time_update_timer = NULL;
    }
    // 创建菜单界面
    ui->carctl = lv_obj_create(NULL);
    lv_obj_set_scrollbar_mode(ui->carctl, LV_SCROLLBAR_MODE_OFF);

    // 设置背景颜色
    lv_obj_set_style_bg_color(ui->carctl, lv_color_make(0xFF, 0xFF, 0xFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->carctl, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 2. 创建左上角的网格容器
    ui->carctl_info_grid = lv_obj_create(ui->carctl);
    lv_obj_set_size(ui->carctl_info_grid, 320, 130); // 总高度80，宽120
    lv_obj_set_pos(ui->carctl_info_grid, 150, 40);   // 屏幕左上角
    lv_obj_set_layout(ui->carctl_info_grid, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(ui->carctl_info_grid, col_dsc, row_dsc);
    lv_obj_set_scrollbar_mode(ui->carctl_info_grid, LV_SCROLLBAR_MODE_OFF); // 禁用滑动条
    lv_obj_clear_flag(ui->carctl_info_grid, LV_OBJ_FLAG_SCROLLABLE);       // 禁用滑动

    // 设置网格容器的背景颜色和透明度
    lv_obj_set_style_bg_color(ui->carctl_info_grid, lv_color_make(0xF0, 0xF0, 0xF0), LV_PART_MAIN | LV_STATE_DEFAULT); // 浅灰色背景
    lv_obj_set_style_bg_opa(ui->carctl_info_grid, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT); // 不透明

    lv_obj_set_style_radius(ui->carctl_info_grid, 10, LV_PART_MAIN | LV_STATE_DEFAULT); // 圆角半径为10
    lv_obj_set_style_border_color(ui->carctl_info_grid, lv_color_make(0xA0, 0xA0, 0xA0), LV_PART_MAIN | LV_STATE_DEFAULT); // 灰色边框
    lv_obj_set_style_border_width(ui->carctl_info_grid, 2, LV_PART_MAIN | LV_STATE_DEFAULT); // 边框宽度为2
    lv_obj_set_style_border_opa(ui->carctl_info_grid, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT); // 半透明边框

    lv_obj_set_style_shadow_color(ui->carctl_info_grid, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN | LV_STATE_DEFAULT); // 黑色阴影
    lv_obj_set_style_shadow_width(ui->carctl_info_grid, 10, LV_PART_MAIN | LV_STATE_DEFAULT); // 阴影宽度为10
    lv_obj_set_style_shadow_opa(ui->carctl_info_grid, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT); // 阴影透明度为30%
    lv_obj_set_style_shadow_ofs_x(ui->carctl_info_grid, 5, LV_PART_MAIN | LV_STATE_DEFAULT); // 阴影水平偏移
    lv_obj_set_style_shadow_ofs_y(ui->carctl_info_grid, 5, LV_PART_MAIN | LV_STATE_DEFAULT); // 阴影垂直偏移

    lv_obj_set_style_bg_color(ui->carctl_info_grid, lv_color_make(0xFF, 0xFF, 0xFF), LV_PART_MAIN | LV_STATE_DEFAULT); // 渐变起始颜色
    lv_obj_set_style_bg_grad_color(ui->carctl_info_grid, lv_color_make(0xE0, 0xE0, 0xE0), LV_PART_MAIN | LV_STATE_DEFAULT); // 渐变结束颜色
    lv_obj_set_style_bg_grad_dir(ui->carctl_info_grid, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT); // 渐变方向为垂直

    // 3. 创建日期Label
    ui->carctl_date_label = lv_label_create(ui->carctl_info_grid);
    lv_label_set_text(ui->carctl_date_label, "5月1日"); // 这里可以后续动态更新
    lv_obj_set_style_text_font(ui->carctl_date_label, &lv_font_simsun_26, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_set_grid_cell(ui->carctl_date_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);

    // 3. 创建时间Label
    ui->carctl_time_label = lv_label_create(ui->carctl_info_grid);
    lv_label_set_text(ui->carctl_time_label, "12:00"); // 这里可以后续动态更新
    lv_obj_set_style_text_font(ui->carctl_time_label, &lv_font_simsun_26, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_set_grid_cell(ui->carctl_time_label, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 1);

    time_t now;
    struct tm tm;

    // 获取当前本地时间
    now = time(NULL);
    localtime_r(&now, &tm);

    char time_str[9]; // 存放格式化后的时间字符串，HH:MM:SS
    char date_str[20]; // 存放格式化后的日期字符串，YYYY-MM-DD 

    // 将时间格式化为字符串，精确到秒
    snprintf(time_str, sizeof(time_str), "%02d:%02d", tm.tm_hour, tm.tm_min);
    // 将日期格式化为字符串，精确到日
    snprintf(date_str, sizeof(date_str), "%d月%d日", tm.tm_mon + 1, tm.tm_mday);

    // 更新UI元素
    if (lv_obj_is_valid(ui->carctl_time_label)) {
        lv_label_set_text(ui->carctl_time_label, time_str);
    }
    if (lv_obj_is_valid(ui->carctl_date_label)) {
        lv_label_set_text(ui->carctl_date_label, date_str);
    }

    // // 创建定时器
    // date_time_update_timer = lv_timer_create(update_new_time_cb, 2000, ui);

    WeatherData *weather = get_weather_data();

    // 4. 创建位置Label
    lv_obj_t *location_label = lv_label_create(ui->carctl_info_grid);
    lv_label_set_text(location_label, "桂林"); // 这里可以后续动态更新
    lv_obj_set_style_text_font(location_label, &lv_font_simsun_26, LV_PART_MAIN | LV_STATE_DEFAULT); 
    lv_obj_set_grid_cell(location_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 0, 1);

    if (weather != NULL) {
        // 4. 创建天气Label
        lv_obj_t *weather_label = lv_label_create(ui->carctl_info_grid);
        lv_label_set_text_fmt(weather_label, "%s %s℃",weather->weather,weather->temperature); // 这里可以后续动态更新
        lv_obj_set_style_text_font(weather_label, &lv_font_simsun_26, LV_PART_MAIN | LV_STATE_DEFAULT); 
        lv_obj_set_grid_cell(weather_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 1, 1);

        lv_obj_t *weather_img = lv_img_create(ui->carctl_info_grid);
        const lv_img_dsc_t *weather_icon = NULL;
        if(strcmp(weather->weather, "多云") == 0) {
            weather_icon = &cloudy;
        } else if(strcmp(weather->weather, "晴") == 0) {
            weather_icon = &sunny;
        } else if(strcmp(weather->weather, "小雨") == 0) {
            weather_icon = &small_rain;
        } else if(strcmp(weather->weather, "中雨") == 0 ) {
            weather_icon = &moderate_rain;
        } else if(strcmp(weather->weather, "大雨") == 0) {
            weather_icon = &big_rain;		
        } else if(strcmp(weather->weather, "雾") == 0) {
            weather_icon = &fog;	
        } else if(strcmp(weather->weather, "霾") == 0) {
            weather_icon = &haze;				
        } else if(strstr(weather->weather, "雪") != NULL) {
            weather_icon = &snow;
        } else if(strstr(weather->weather, "阴") != NULL) {
            weather_icon = &yin_cloudy;
        } else if(strstr(weather->weather, "阵雨") != NULL) {
            weather_icon = &shower_rain;		
        } else {
            // 默认图标，当没有匹配的天气类型时使用
            weather_icon = &cloudy;
        }

        lv_img_set_src(weather_img, weather_icon); // 设置天气图标
        lv_img_set_zoom(weather_img, 80);  // 设置为原始大小的31.25%
        lv_img_set_antialias(weather_img, true);
        // lv_obj_invalidate(weather_img);
        lv_obj_set_grid_cell(weather_img, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 2);

        free_weather_data(weather);
    }

    // 创建车身按钮
    lv_obj_t *carctl_carbody_btn = lv_imgbtn_create(ui->carctl);
	lv_obj_set_size(carctl_carbody_btn, 300, 150);
	lv_obj_set_pos(carctl_carbody_btn, 380, 310);
    lv_obj_set_scrollbar_mode(carctl_carbody_btn, LV_SCROLLBAR_MODE_OFF);
    lv_imgbtn_set_src(carctl_carbody_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_car_body_alpha_300x150, NULL);

    // 创建D按钮
    lv_obj_t *carctl_D_btn = lv_imgbtn_create(ui->carctl);
    lv_obj_set_size(carctl_D_btn, 50, 50);
    lv_obj_set_pos(carctl_D_btn, 31, 45);
    lv_obj_set_scrollbar_mode(carctl_D_btn, LV_SCROLLBAR_MODE_OFF);
    lv_imgbtn_set_src(carctl_D_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_D_alpha_50x50, NULL);

    // 创建P按钮
    lv_obj_t *carctl_P_btn = lv_imgbtn_create(ui->carctl);
    lv_obj_set_size(carctl_P_btn, 60, 60);
    lv_obj_set_pos(carctl_P_btn, 26, 181);
    lv_obj_set_scrollbar_mode(carctl_P_btn, LV_SCROLLBAR_MODE_OFF);
    lv_imgbtn_set_src(carctl_P_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_P_alpha_60x60, NULL);
    lv_obj_add_event_cb(carctl_P_btn, carctl_P_btn_event_cb, LV_EVENT_CLICKED, ui);

    // 创建R按钮
    lv_obj_t *carctl_R_btn = lv_imgbtn_create(ui->carctl);
    lv_obj_set_size(carctl_R_btn, 50, 50);
    lv_obj_set_pos(carctl_R_btn, 31, 315);
    lv_obj_set_scrollbar_mode(carctl_R_btn, LV_SCROLLBAR_MODE_OFF);
    lv_imgbtn_set_src(carctl_R_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_R_alpha_50x50, NULL);
    lv_obj_add_event_cb(carctl_R_btn, carctl_R_btn_event_cb, LV_EVENT_CLICKED, ui);

	// 创建N按钮
	lv_obj_t *carctl_N_btn = lv_imgbtn_create(ui->carctl);
	lv_obj_set_size(carctl_N_btn, 50, 50);
	lv_obj_set_pos(carctl_N_btn, 31, 455);
	lv_obj_set_scrollbar_mode(carctl_N_btn, LV_SCROLLBAR_MODE_OFF);
	lv_imgbtn_set_src(carctl_N_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_N_alpha_50x50, NULL);

    // 创建车灯开启按钮
    lv_obj_t *carctl_lamp_off_btn = lv_imgbtn_create(ui->carctl);
    lv_obj_set_size(carctl_lamp_off_btn, 50, 50);
    lv_obj_set_pos(carctl_lamp_off_btn, 590, 70);
    lv_imgbtn_set_src(carctl_lamp_off_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_car_lamp_off_alpha_50x50, NULL);
    lv_obj_add_event_cb(carctl_lamp_off_btn, carctl_lamp_off_event_cb, LV_EVENT_CLICKED, NULL);

    // 创建双闪灯按钮
    lv_obj_t *carctl_double_flash_btn = lv_imgbtn_create(ui->carctl);
    lv_obj_set_size(carctl_double_flash_btn, 100, 100);
    lv_obj_set_pos(carctl_double_flash_btn, 652, 45);
    lv_imgbtn_set_src(carctl_double_flash_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_double_flash_alpha_100x100, NULL);
    lv_obj_add_event_cb(carctl_double_flash_btn, carctl_double_flash_event_cb, LV_EVENT_CLICKED, NULL);

    // 创建锁车按钮
    lv_obj_t *carctl_lock_btn = lv_imgbtn_create(ui->carctl);
    lv_obj_set_size(carctl_lock_btn, 50, 50);
    lv_obj_set_pos(carctl_lock_btn, 323, 288);
    lv_imgbtn_set_src(carctl_lock_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_lock_alpha_50x50, NULL);
    lv_obj_add_event_cb(carctl_lock_btn, carctl_lock_event_cb, LV_EVENT_CLICKED, NULL);

    // 创建雨刷加减档按钮
    ws_wiper_label = lv_label_create(ui->carctl);
	lv_obj_set_size(ws_wiper_label, 60, 35); // 设置宽度为100，高度为50
    lv_obj_set_pos(ws_wiper_label, 670, 180); // 设置文本位置
    lv_label_set_text(ws_wiper_label, "0"); // 初始值为0
	lv_obj_set_style_bg_color(ws_wiper_label, lv_color_make(0xAD, 0xD8, 0xE6), LV_PART_MAIN | LV_STATE_DEFAULT); // 蓝色背景
	lv_obj_set_style_bg_opa(ws_wiper_label, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT); // 设置背景不透明
	lv_obj_set_style_text_font(ws_wiper_label, &lv_font_simsun_30, LV_PART_MAIN | LV_STATE_DEFAULT); // 设置字体大小为20
	lv_obj_set_style_text_align(ws_wiper_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *carctl_ws_wiper_sub_btn = lv_imgbtn_create(ui->carctl);
    lv_obj_set_size(carctl_ws_wiper_sub_btn, 50, 50);
    lv_obj_set_pos(carctl_ws_wiper_sub_btn, 600, 170);
    lv_imgbtn_set_src(carctl_ws_wiper_sub_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_ws_wiper_alpha_50x50, NULL);
    lv_obj_add_event_cb(carctl_ws_wiper_sub_btn, carctl_ws_wiper_sub_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *carctl_ws_wiper_add_btn = lv_imgbtn_create(ui->carctl);
    lv_obj_set_size(carctl_ws_wiper_add_btn, 50, 50);
    lv_obj_set_pos(carctl_ws_wiper_add_btn, 750, 170);
    lv_imgbtn_set_src(carctl_ws_wiper_add_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_ws_wiper_alpha_50x50, NULL);
    lv_obj_add_event_cb(carctl_ws_wiper_add_btn, carctl_ws_wiper_add_event_cb, LV_EVENT_CLICKED, NULL);


    // 为菜单界面添加右滑事件回调
    lv_obj_add_event_cb(ui->carctl, carctl_swipe_event_cb, LV_EVENT_GESTURE, ui);
}


