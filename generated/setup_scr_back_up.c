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
#include "camera_utils.h"

// 在文件开头初始化camera结构体
camera_t camera = {
    .fd = -1,
    .buffers = NULL,
    .n_buffers = 0,
    .img_obj = NULL,
    .img_data = NULL,
    .conv_buffer = NULL,
    .width = 0,
    .height = 0,
    .running = false,
    .need_conversion = false
};

// YUYV转换到RGB32(ARGB8888)的优化函数
void yuyv_to_rgb32_optimized(const uint8_t *yuyv, uint32_t *rgb, int width, int height)
{
    static int frame_count = 0;
    // // 只在第一帧和每100帧输出一次日志
    // if (frame_count == 0 || frame_count % 100 == 0) {
    //     printf("转换YUYV到RGB32: %dx%d (帧 #%d)\n", width, height, frame_count);
    // }
    frame_count++;

    if (!yuyv || !rgb) {
        // printf("错误：转换函数的输入或输出缓冲区为空\n");
        return;
    }
    
    // 预计算查找表
    static int initialized = 0;
    static int y_table[256];
    static int u_table[256][2];
    static int v_table[256][2];
    
    if (!initialized) {
        for (int i = 0; i < 256; i++) {
            y_table[i] = (i - 16) * 298;
            u_table[i][0] = 516 * (i - 128);
            u_table[i][1] = -100 * (i - 128);
            v_table[i][0] = 409 * (i - 128);
            v_table[i][1] = -208 * (i - 128);
        }
        initialized = 1;
    }
    
    for (int i = 0; i < width * height / 2; i++) {
        int y0 = yuyv[i * 4 + 0];
        int u  = yuyv[i * 4 + 1];
        int y1 = yuyv[i * 4 + 2];
        int v  = yuyv[i * 4 + 3];
        
        int r, g, b;
        int y_val, u0_val, u1_val, v0_val, v1_val;
        
        // 使用查找表加速计算
        y_val = y_table[y0];
        u0_val = u_table[u][0];
        u1_val = u_table[u][1];
        v0_val = v_table[v][0];
        v1_val = v_table[v][1];
        
        // 第一个像素
        b = (y_val + u0_val) >> 8;
        g = (y_val + u1_val + v1_val) >> 8;
        r = (y_val + v0_val) >> 8;
        
        // 限制RGB值在0-255范围内
        r = r < 0 ? 0 : (r > 255 ? 255 : r);
        g = g < 0 ? 0 : (g > 255 ? 255 : g);
        b = b < 0 ? 0 : (b > 255 ? 255 : b);
        
        // 转换为RGB32格式 (ARGB8888)
        // Alpha通道设为255（完全不透明）
        rgb[i * 2] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        
        // 第二个像素
        y_val = y_table[y1];
        
        b = (y_val + u0_val) >> 8;
        g = (y_val + u1_val + v1_val) >> 8;
        r = (y_val + v0_val) >> 8;
        
        // 限制RGB值在0-255范围内
        r = r < 0 ? 0 : (r > 255 ? 255 : r);
        g = g < 0 ? 0 : (g > 255 ? 255 : g);
        b = b < 0 ? 0 : (b > 255 ? 255 : b);
        
        // 转换为RGB32格式 (ARGB8888)
        rgb[i * 2 + 1] = (0xFF << 24) | (r << 16) | (g << 8) | b;
    }
    
    // // 只在第一帧和每100帧输出一次日志
    // if (frame_count == 1 || frame_count % 100 == 0) {
    //     printf("YUYV到RGB32转换完成 (帧 #%d)\n", frame_count-1);
    // }
}

// 异步更新图像的回调函数
void img_update_cb(void *data)
{
    img_update_data_t *update_data = (img_update_data_t *)data;

    if (!camera.running || camera.fd == -1) {
        printf("摄像头已关闭，取消更新\n");
        free(data);
        return;
    }
    if (!update_data) {
        printf("错误：更新数据为空\n");
        return;
    }
    
    if (!update_data->img_obj) {
        printf("错误：图像对象为空\n");
        free(data);
        return;
    }
    
    if (!update_data->img_dsc) {
        printf("错误：图像描述符为空\n");
        free(data);
        return;
    }
    
    if (!update_data->img_dsc->data) {
        printf("错误：图像数据为空\n");
        free(data);
        return;
    }

    // 检查图像尺寸
    if (update_data->img_dsc->header.w <= 0 || update_data->img_dsc->header.h <= 0) {
        printf("错误：图像尺寸无效，宽=%d，高=%d\n", 
               update_data->img_dsc->header.w, update_data->img_dsc->header.h);
        free(data);
        return;
    }
  
    // 检查图像对象是否仍然有效
    if (lv_obj_is_valid(update_data->img_obj)) {
        // 检查图像数据的前几个字节，确认不是全0或无效数据
        // 修改这里：从uint16_t改为uint32_t以匹配RGB32格式
        uint32_t *img_data = (uint32_t *)update_data->img_dsc->data;

        // 确保图像对象可见
        lv_obj_clear_flag(update_data->img_obj, LV_OBJ_FLAG_HIDDEN);
        
        // 设置图像源
        lv_img_set_src(update_data->img_obj, update_data->img_dsc);
        // printf("图像源设置完成，等待LVGL渲染\n");
        
        // 强制刷新对象
        lv_obj_invalidate(update_data->img_obj);
        
        // 确保图像对象在Z轴上位于前面
        lv_obj_move_foreground(update_data->img_obj);
    } else {
        printf("错误：图像对象已失效\n");
    }
    
    // 释放在camera_thread中分配的内存
    free(data);
}
// 初始化摄像头前先检查设备是否存在
int check_camera_device(const char *dev_name)
{
    int fd = open(dev_name, O_RDWR);
    if (fd == -1) {
        return -1;
    }
    close(fd);
    return 0;
}

// 修改init_camera函数，增加更多错误检查
int init_camera(const char *dev_name, int width, int height)
{
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;
    enum v4l2_buf_type type;
    
    // 确保摄像头未初始化
    if (camera.fd != -1) {
        printf("关闭已存在的摄像头连接\n");
        close_camera();
    }
    
    // 初始化camera结构体
    memset(&camera, 0, sizeof(camera));
    camera.fd = -1;
    camera.buffers = NULL;
    camera.n_buffers = 0;
    camera.img_obj = NULL;
    camera.img_data = NULL;
    camera.conv_buffer = NULL;
    camera.width = 0;
    camera.height = 0;
    camera.running = false;
    camera.need_conversion = false;
    camera.frame_changed = false;
    clock_gettime(CLOCK_MONOTONIC, &camera.last_update_time);
    
    // 打开摄像头设备
    camera.fd = open(dev_name, O_RDWR);
    if (camera.fd == -1) {
        perror("无法打开摄像头设备");
        return -1;
    }
    
    // 查询设备功能
    if (ioctl(camera.fd, VIDIOC_QUERYCAP, &cap) == -1) {
        perror("VIDIOC_QUERYCAP失败");
        close(camera.fd);
        camera.fd = -1;
        return -1;
    }
    
    // 检查设备功能
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "设备不支持视频捕获\n");
        close(camera.fd);
        camera.fd = -1;
        return -1;
    }
    
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        fprintf(stderr, "设备不支持流媒体IO\n");
        close(camera.fd);
        camera.fd = -1;
        return -1;
    }
    
    // 设置视频格式
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; // 使用RGB565格式，与LVGL兼容
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    
    if (ioctl(camera.fd, VIDIOC_S_FMT, &fmt) == -1) {
        printf("VIDIOC_S_FMT失败: %s\n", strerror(errno));
        close(camera.fd);
        camera.fd = -1;
        return -1;
    }
    
    // 检查实际设置的格式
    if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV) {
        // printf("摄像头使用YUYV格式，需要转换为RGB565\n");
        camera.need_conversion = true;
    } else if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB565) {
        // printf("摄像头使用RGB565格式，无需转换\n");
        camera.need_conversion = false;
    } else {
        // printf("摄像头使用不支持的格式: %c%c%c%c\n",
            //    (fmt.fmt.pix.pixelformat & 0xFF),
            //    ((fmt.fmt.pix.pixelformat >> 8) & 0xFF),
            //    ((fmt.fmt.pix.pixelformat >> 16) & 0xFF),
            //    ((fmt.fmt.pix.pixelformat >> 24) & 0xFF));
        close(camera.fd);
        camera.fd = -1;
        return -1;
    }

    // 保存实际的宽度和高度
    camera.width = fmt.fmt.pix.width;
    camera.height = fmt.fmt.pix.height;
    
    // 请求缓冲区
    memset(&req, 0, sizeof(req));
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    
    if (ioctl(camera.fd, VIDIOC_REQBUFS, &req) == -1) {
        perror("VIDIOC_REQBUFS失败");
        close(camera.fd);
        camera.fd = -1;
        return -1;
    }
    
    // 分配缓冲区
    camera.buffers = calloc(req.count, sizeof(buffer_t));
    if (!camera.buffers) {
        perror("无法分配缓冲区内存");
        close(camera.fd);
        camera.fd = -1;
        return -1;
    }
    camera.n_buffers = req.count;
    
    // 映射缓冲区
    for (unsigned int i = 0; i < camera.n_buffers; ++i) {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        
        if (ioctl(camera.fd, VIDIOC_QUERYBUF, &buf) == -1) {
            perror("VIDIOC_QUERYBUF失败");
            close_camera();
            return -1;
        }
        
        camera.buffers[i].length = buf.length;
        camera.buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
                                      MAP_SHARED, camera.fd, buf.m.offset);
        
        if (camera.buffers[i].start == MAP_FAILED) {
            perror("mmap失败");
            close_camera();
            return -1;
        }
    }
    
    // 将缓冲区放入队列
    for (unsigned int i = 0; i < camera.n_buffers; ++i) {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        
        if (ioctl(camera.fd, VIDIOC_QBUF, &buf) == -1) {
            perror("VIDIOC_QBUF失败");
            close_camera();
            return -1;
        }
    }
    
    // 开始视频流
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(camera.fd, VIDIOC_STREAMON, &type) == -1) {
        perror("VIDIOC_STREAMON失败");
        close_camera();
        return -1;
    }
    
    // 为LVGL图像分配内存 (RGB32格式)
    size_t img_size = camera.width * camera.height * 4; // RGB32 = 4字节/像素
    camera.img_data = malloc(img_size);
    if (!camera.img_data) {
        perror("无法分配图像内存");
        close_camera();
        return -1;
    }
    
    // 为格式转换分配临时缓冲区 (如果需要)
    if (camera.need_conversion) {
        // YUYV格式也是每像素2字节
        camera.conv_buffer = malloc(img_size);
        if (!camera.conv_buffer) {
            perror("无法分配转换缓冲区内存");
            free(camera.img_data);
            camera.img_data = NULL;
            close_camera();
            return -1;
        }
    }

    // 初始化LVGL图像描述符
    camera.img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR; // 或 LV_IMG_CF_TRUE_COLOR_ALPHA，取决于LVGL版本
    camera.img_dsc.header.w = camera.width;
    camera.img_dsc.header.h = camera.height;
    camera.img_dsc.header.always_zero = 0;
    camera.img_dsc.data_size = camera.width * camera.height * 4; // RGB32每像素4字节
    camera.img_dsc.data = camera.img_data;
    
    // 初始化互斥锁
    if (pthread_mutex_init(&camera.mutex, NULL) != 0) {
        perror("无法初始化互斥锁");
        if (camera.img_data) {
            free(camera.img_data);
            camera.img_data = NULL;
        }
        close_camera();
        return -1;
    }
    
    return 0;
}

// 修改camera_thread函数，增加错误处理和安全检查
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
            printf("VIDIOC_DQBUF失败: %s\n", strerror(errno));
            break;
        }
        
        // 确保索引有效
        if (buf.index >= camera.n_buffers) {
             printf("缓冲区索引超出范围: %d >= %d\n", buf.index, camera.n_buffers);
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
                    printf("错误：无法分配更新数据内存\n");
                }
            }
            // 重置标志并更新时间
            camera.frame_changed = false;
            camera.last_update_time = current_time;
        }
        // 将缓冲区放回队列
        if (ioctl(camera.fd, VIDIOC_QBUF, &buf) == -1) {
            printf("VIDIOC_QBUF失败: %s\n", strerror(errno));
            break;
        }
        
        // 短暂延时，控制帧率
        usleep(30000); // 约30fps
    }
    // printf("摄像头线程结束\n");
    return NULL;
}

// 修改close_camera函数，增加安全检查
void close_camera(void)
{
    enum v4l2_buf_type type;
    
    // printf("开始关闭摄像头\n");
    
    // 检查摄像头是否已经关闭
    if (camera.fd == -1) {
        // printf("摄像头已经关闭\n");
        return;
    }
    
    // 安全地停止线程
    if (camera.running) {
        // printf("停止摄像头线程\n");
        camera.running = false;
        
        // 添加超时机制
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1; // 1秒超时
        
        int ret = pthread_timedjoin_np(camera.thread, NULL, &ts);
        if (ret != 0) {
            // printf("线程无法在超时内结束，强制取消\n");
            pthread_cancel(camera.thread);
            pthread_join(camera.thread, NULL);
        }
    }
    
    // 销毁互斥锁
    pthread_mutex_destroy(&camera.mutex);
    
    // 停止视频流
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(camera.fd, VIDIOC_STREAMOFF, &type);
    
    // 解除内存映射
    if (camera.buffers) {
        for (unsigned int i = 0; i < camera.n_buffers; ++i) {
            if (camera.buffers[i].start != MAP_FAILED && camera.buffers[i].start != NULL) {
                munmap(camera.buffers[i].start, camera.buffers[i].length);
            }
        }
        free(camera.buffers);
        camera.buffers = NULL;
    }
    
    // 释放图像数据
    if (camera.img_data) {
        free(camera.img_data);
        camera.img_data = NULL;
    }
    // 释放转换缓冲区
    if (camera.conv_buffer) {
        free(camera.conv_buffer);
        camera.conv_buffer = NULL;
    }    
    // 关闭设备
    // printf("关闭设备\n");
    close(camera.fd);
    camera.fd = -1;
    camera.n_buffers = 0;
    
    // 重置其他字段
    camera.img_obj = NULL;
    camera.width = 0;
    camera.height = 0;
    
    // printf("摄像头关闭完成\n");
}



static lv_obj_t * g_kb_back_up;
static void kb_back_up_event_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *kb = lv_event_get_target(e);
	if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL){
		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
	}
}
__attribute__((unused)) static void ta_back_up_event_cb(lv_event_t *e)
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

void setup_scr_back_up(lv_ui *ui){

	//Write codes back_up
	ui->back_up = lv_obj_create(NULL);

	//Create keyboard on back_up
	g_kb_back_up = lv_keyboard_create(ui->back_up);
	lv_obj_add_event_cb(g_kb_back_up, kb_back_up_event_cb, LV_EVENT_ALL, NULL);
	lv_obj_add_flag(g_kb_back_up, LV_OBJ_FLAG_HIDDEN);
	lv_obj_set_style_text_font(g_kb_back_up, &lv_font_simsun_18, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_scrollbar_mode(ui->back_up, LV_SCROLLBAR_MODE_OFF);

	//Set style for back_up. Part: LV_PART_MAIN, State: LV_STATE_DEFAULT
	lv_obj_set_style_bg_color(ui->back_up, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_color(ui->back_up, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->back_up, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->back_up, 242, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes back_up_imgbtn_2
	ui->back_up_imgbtn_2 = lv_imgbtn_create(ui->back_up);
	lv_obj_set_pos(ui->back_up_imgbtn_2, 930, 45);
	lv_obj_set_size(ui->back_up_imgbtn_2, 50, 50);
	lv_obj_set_scrollbar_mode(ui->back_up_imgbtn_2, LV_SCROLLBAR_MODE_OFF);

	//Set style for back_up_imgbtn_2. Part: LV_PART_MAIN, State: LV_STATE_DEFAULT
	lv_obj_set_style_shadow_width(ui->back_up_imgbtn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_color(ui->back_up_imgbtn_2, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_opa(ui->back_up_imgbtn_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_spread(ui->back_up_imgbtn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_x(ui->back_up_imgbtn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_ofs_y(ui->back_up_imgbtn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->back_up_imgbtn_2, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->back_up_imgbtn_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_recolor(ui->back_up_imgbtn_2, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_recolor_opa(ui->back_up_imgbtn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->back_up_imgbtn_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Set style for back_up_imgbtn_2. Part: LV_PART_MAIN, State: LV_STATE_PRESSED
	lv_obj_set_style_shadow_width(ui->back_up_imgbtn_2, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_color(ui->back_up_imgbtn_2, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_opa(ui->back_up_imgbtn_2, 255, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_spread(ui->back_up_imgbtn_2, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_ofs_x(ui->back_up_imgbtn_2, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_shadow_ofs_y(ui->back_up_imgbtn_2, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_text_color(ui->back_up_imgbtn_2, lv_color_make(0xFF, 0x33, 0xFF), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_recolor(ui->back_up_imgbtn_2, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_recolor_opa(ui->back_up_imgbtn_2, 0, LV_PART_MAIN|LV_STATE_PRESSED);
	lv_obj_set_style_img_opa(ui->back_up_imgbtn_2, 255, LV_PART_MAIN|LV_STATE_PRESSED);

	//Set style for back_up_imgbtn_2. Part: LV_PART_MAIN, State: LV_STATE_CHECKED
	lv_obj_set_style_shadow_width(ui->back_up_imgbtn_2, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_color(ui->back_up_imgbtn_2, lv_color_make(0x21, 0x95, 0xf6), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_opa(ui->back_up_imgbtn_2, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_spread(ui->back_up_imgbtn_2, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_ofs_x(ui->back_up_imgbtn_2, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_shadow_ofs_y(ui->back_up_imgbtn_2, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_text_color(ui->back_up_imgbtn_2, lv_color_make(0xFF, 0x33, 0xFF), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_recolor(ui->back_up_imgbtn_2, lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_recolor_opa(ui->back_up_imgbtn_2, 0, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_obj_set_style_img_opa(ui->back_up_imgbtn_2, 255, LV_PART_MAIN|LV_STATE_CHECKED);
	lv_imgbtn_set_src(ui->back_up_imgbtn_2, LV_IMGBTN_STATE_RELEASED, NULL, &_return_alpha_50x50, NULL);
	lv_obj_add_flag(ui->back_up_imgbtn_2, LV_OBJ_FLAG_CHECKABLE);

    // 在setup_scr_back_up函数中修改摄像头初始化部分
    // 创建图像对象用于显示摄像头画面
    ui->back_up_camera_img = lv_img_create(ui->back_up);
    lv_obj_set_pos(ui->back_up_camera_img, 105, 0);
    lv_obj_set_size(ui->back_up_camera_img, 800, 600);

    // 设置图像对象属性
    lv_obj_clear_flag(ui->back_up_camera_img, LV_OBJ_FLAG_HIDDEN);
    lv_img_set_pivot(ui->back_up_camera_img, 0, 0); // 设置旋转中心
    lv_img_set_angle(ui->back_up_camera_img, 0);    // 设置旋转角度
    lv_img_set_zoom(ui->back_up_camera_img, 256);   // 设置缩放 (256 = 100%)
    lv_img_set_antialias(ui->back_up_camera_img, false); // 关闭抗锯齿以提高性能

    // 设置图像对象样式
    lv_obj_set_style_bg_color(ui->back_up_camera_img, lv_color_make(0, 0, 0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->back_up_camera_img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->back_up_camera_img, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->back_up_camera_img, lv_color_make(255, 0, 0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->back_up_camera_img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 检查摄像头设备是否存在
    const char *camera_dev = "/dev/video0";
    if (check_camera_device(camera_dev) != 0) {
        // 显示错误信息
        // printf("摄像头设备不存在\n");
        lv_obj_t *label = lv_label_create(ui->back_up);
        lv_label_set_text(label, "摄像头设备不存在");
        lv_obj_set_style_text_font(label, &lv_font_simsun_18, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_pos(label, 10, 100);
        // return; // 如果设备不存在，直接返回
    }
    // 初始化摄像头
    if (init_camera(camera_dev, 800, 600) == 0) {
        // printf("初始化摄像头\n");
        // 创建一个初始图像
        memset(camera.img_data, 0, camera.width * camera.height * 2); // 清空图像数据
        
        // 设置图像对象
        camera.img_obj = ui->back_up_camera_img;
        lv_img_set_src(camera.img_obj, &camera.img_dsc);
        
        // 创建摄像头线程
        camera.running = true; // 确保在创建线程前设置running为true
        // printf("创建摄像头线程\n");

        if (pthread_create(&camera.thread, NULL, camera_thread, NULL) != 0) {
            perror("无法创建摄像头线程");
            close_camera();
            
            // 显示错误信息
            lv_obj_t *label = lv_label_create(ui->back_up);
            lv_label_set_text(label, "摄像头线程创建失败");
            lv_obj_set_pos(label, 50, 100);
        }
    } else {
        // 显示错误信息
        printf("摄像头初始化失败\n");
    }
    

	//Init events for screen
	events_init_back_up(ui);
}

// 添加一个清理函数，在离开back_up界面时调用
void cleanup_back_up(lv_ui *ui)
{
    // printf("清理back_up界面资源\n");
    
    // 安全地关闭摄像头
    if (camera.fd != -1) {
        // 停止摄像头线程
        if (camera.running) {
            // printf("停止摄像头线程\n");
            camera.running = false;
            
            // 等待线程结束，设置超时
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 2; // 2秒超时
            
            int ret = pthread_timedjoin_np(camera.thread, NULL, &ts);
            if (ret != 0) {
                // printf("线程无法在超时内结束，强制取消: %s\n", strerror(ret));
                pthread_cancel(camera.thread);
                pthread_join(camera.thread, NULL);
            }
        }
        
        // 关闭摄像头设备
        close_camera();
    }
    
    // 清除图像对象引用
    camera.img_obj = NULL;
    
    // printf("back_up界面资源清理完成\n");
}