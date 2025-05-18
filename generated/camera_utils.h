#ifndef CAMERA_UTILS_H
#define CAMERA_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <pthread.h>
#include <errno.h>  // 添加这个头文件
#include <string.h> // 用于 memset, memcpy 等函数
#include <stdlib.h> // 用于 malloc, free 等函数


// 摄像头相关结构体和变量
typedef struct {
    void *start;
    size_t length;
} buffer_t;

typedef struct {
    int fd;
    buffer_t *buffers;
    unsigned int n_buffers;
    lv_obj_t *img_obj;
    lv_img_dsc_t img_dsc;
    uint8_t *img_data;       // RGB565格式的图像数据
    uint8_t *conv_buffer;    // 用于格式转换的临时缓冲区
    int width;
    int height;
    bool running;
    pthread_t thread;
    pthread_mutex_t mutex;
    bool need_conversion;    // 是否需要格式转换
    bool frame_changed;        // 标记帧是否已更新
    struct timespec last_update_time; // 上次更新UI的时间    
} camera_t;

extern camera_t camera;


// 定义用于传递图像更新数据的结构体
typedef struct {
    lv_obj_t *img_obj;     // 要更新的LVGL图像对象
    lv_img_dsc_t *img_dsc; // 指向图像描述符的指针
} img_update_data_t;


void yuyv_to_rgb32_optimized(const uint8_t *yuyv, uint32_t *rgb, int width, int height);
void img_update_cb(void *data);
int check_camera_device(const char *dev_name);
int init_camera(const char *dev_name, int width, int height);

// 关闭摄像头并释放资源
void close_camera(void);
void cleanup_back_up(lv_ui *ui);

#ifdef __cplusplus
}
#endif

#endif /* CAMERA_UTILS_H */