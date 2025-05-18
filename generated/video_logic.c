#include "gui_guider.h"
#include <stdio.h>
#include "lvgl/lvgl.h"

#include "video_logic.h"
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>

// 全局变量定义
bool is_fullscreen = false;
bool is_video_playing = false;
pthread_mutex_t video_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ui_mutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t video_cond = PTHREAD_COND_INITIALIZER;
pthread_t video_thread;
bool video_thread_running = false;
char current_video_path[512] = {0};
int video_duration = 0;
int video_seek_time = 0;
bool video_seek_requested = false;

// 视频映射表
static const char* video_paths[] = {
    "/home/kickpi/dowmload/lvgl_demo/app/video/video1.mp4",  
    "/home/kickpi/dowmload/lvgl_demo/app/video/video2.mp4",  
    "/home/kickpi/dowmload/lvgl_demo/app/video/video3.mp4",   
    "/home/kickpi/dowmload/lvgl_demo/app/video/video4.mp4",  
    "/home/kickpi/dowmload/lvgl_demo/app/video/video5.mp4",     
	"/home/kickpi/dowmload/lvgl_demo/app/video/video6.mp4"   
};

static const char* video_titles[] = {
    "老人，海鸥，沙滩",
    "最长的电影",
    "美人鱼",
    "鹬",
    "Anime mix-cut1",    
	"Anime mix-cut2"
};
// 获取视频数量
static const int video_count = 6;

// 定义更新数据结构
typedef struct {
    lv_ui *ui;
    int duration;
    char duration_text[32];
} video_duration_update_t;

// 定义帧结构
typedef struct {
    uint8_t *data;  // 帧数据
    int width;      // 帧宽度
    int height;     // 帧高度
} Frame;

// 定义帧队列
#define FRAME_QUEUE_SIZE 10
typedef struct {
    Frame frames[FRAME_QUEUE_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} FrameQueue;

// 初始化帧队列
static FrameQueue frame_queue = {
    .head = 0,
    .tail = 0,
    .count = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER
};

static void delete_img_obj_cb(void *data);

static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);
static void *video_playback_thread(void *arg);

void start_video_playback(lv_ui *ui, const char *video_path);
void stop_video_playback(lv_ui *ui);
void play_video_by_index(lv_ui *ui, int index);
void cleanup_video_resources(void);


// 在程序启动时检查视频文件是否存在
static void check_video_files(void)
{
    // printf("检查视频文件:\n");
    for (int i = 0; i < video_count; i++) {
        FILE *file = fopen(video_paths[i], "r");
        if (file) {
            fclose(file);
            // printf("  [√] %s (%s)\n", video_titles[i], video_paths[i]);
        } else {
            // printf("  [×] %s (%s) - %s\n", video_titles[i], video_paths[i], strerror(errno));
        }
    }
}

// 异步回调函数，用于更新UI上的总时长
static void update_duration_cb(void *data)
{
    video_duration_update_t *update_data = (video_duration_update_t *)data;
    if (!update_data) return;
    
    lv_ui *ui = update_data->ui;
    
    // 设置进度条的范围
    lv_bar_set_range(ui->video_progress_bar, 0, update_data->duration); // 使用毫秒作为单位

    // 更新总时长标签
    lv_label_set_text(ui->video_total_time_label, update_data->duration_text);
    
    // 释放内存
    free(update_data);
}

// 定义进度更新数据结构
typedef struct {
    lv_ui *ui;
    int current_time;
    int duration;
    char time_text[32];
} video_progress_update_t;

// 异步回调函数，用于更新进度条和当前时间
static void update_progress_cb(void *data)
{
    video_progress_update_t *update_data = (video_progress_update_t *)data;
    if (!update_data) return;
    
    lv_ui *ui = update_data->ui;
    int current_time = update_data->current_time;
    int duration = update_data->duration;
    
    // 安全地更新UI
    pthread_mutex_lock(&ui_mutex);
    
    // 更新进度条
    if ((duration > 0) && (ui->video_progress_bar) && (lv_obj_is_valid(ui->video_progress_bar))) {
        lv_slider_set_value(ui->video_progress_bar, current_time, LV_ANIM_OFF);
    }
    
    // 更新当前时间标签
    if (ui->video_current_time_label && lv_obj_is_valid(ui->video_current_time_label)) {
        lv_label_set_text(ui->video_current_time_label, update_data->time_text);
    }
    
    pthread_mutex_unlock(&ui_mutex);
    
    // 释放内存
    free(update_data);
}

// 视频播放线程函数
static void *video_playback_thread(void *arg)
{
    // 设置线程为可取消状态
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    lv_ui *ui = (lv_ui *)arg;
    AVFormatContext *pFormatCtx = NULL;
    int videoStream = -1;
    AVCodecContext *pCodecCtx = NULL;
    const AVCodec *pCodec = NULL;
    static AVFrame *pFrame = NULL;
    static AVFrame *pFrameRGB = NULL;
    static AVPacket *packet = NULL;
    struct SwsContext *sws_ctx = NULL;
    uint8_t *buffer = NULL;
    lv_obj_t *img_obj = NULL;
    lv_img_dsc_t img_dsc;
    int target_width = 0;
	int target_height = 0;
    int ret = 0;

    // 当前播放时间(秒)
    int current_time = 0.0;
    int duration = 0.0;
    int64_t last_update_time = 0;

    // 初始化所有指针为NULL
    memset(&img_dsc, 0, sizeof(img_dsc));

    if (!pFrame) pFrame = av_frame_alloc();
    if (!pFrameRGB) pFrameRGB = av_frame_alloc();
    if (!packet) packet = av_packet_alloc();

    // 注册所有格式和编解码器
    #if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100)
        av_register_all();
    #endif
	// 在尝试打开视频前，先检查文件是否存在
	FILE *file = fopen(current_video_path, "r");
	if (file == NULL) {
		// printf("文件不存在或无法访问: %s (错误码: %d)\n", current_video_path, errno);
		perror("fopen");
		goto end;
	} else {
		fclose(file);
		// printf("文件存在且可访问: %s\n", current_video_path);
	}    
    // 打开视频文件
    if (avformat_open_input(&pFormatCtx, current_video_path, NULL, NULL) != 0) {
        // printf("无法打开视频文件: %s\n", current_video_path);
        goto end;
    }
    
    // 获取流信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        // printf("无法获取流信息\n");
        goto end;
    }

    // 获取视频总时长(微秒)，并转换为秒
    if (pFormatCtx->duration != AV_NOPTS_VALUE) {
        duration = pFormatCtx->duration / AV_TIME_BASE;
        // printf("视频总时长: %d 秒\n", duration);
        
        // 更新UI上的总时长显示
        char duration_text[32];
        int hours = (int)duration / 3600;
        int minutes = ((int)duration % 3600) / 60;
        int seconds = (int)duration % 60;
        
        if (hours > 0) {
            snprintf(duration_text, sizeof(duration_text), "%02d:%02d:%02d", hours, minutes, seconds);
        } else {
            snprintf(duration_text, sizeof(duration_text), "%02d:%02d", minutes, seconds);
        }
        
        // 使用LVGL的异步调用更新UI
        video_duration_update_t *update_data = malloc(sizeof(video_duration_update_t));
        if (update_data) {
            update_data->ui = ui;
            update_data->duration = duration;
            strncpy(update_data->duration_text, duration_text, sizeof(update_data->duration_text));
            lv_async_call(update_duration_cb, update_data);
        }        
    } else {
        printf("无法获取视频时长\n");
    }

    // 查找视频流
    videoStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pCodec, 0);
    if (videoStream < 0) {
        printf("无法找到视频流\n");
        goto end;
    }

    // 创建解码器上下文
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
        printf("无法分配解码器上下文\n");
        goto end;
    }
    
    // 从流中复制编解码器参数到解码器上下文
    if (avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStream]->codecpar) < 0) {
        printf("无法复制编解码器参数\n");
        goto end;
    }
    
    // 打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("无法打开解码器\n");
        goto end;
    }
    
    // printf("解码器名称: %s\n", pCodec->name);
    // printf("解码器长名称: %s\n", pCodec->long_name);
    // printf("视频像素格式: %s\n", av_get_pix_fmt_name(pCodecCtx->pix_fmt));

    // 分配视频帧
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    
    if (!pFrame || !pFrameRGB) {
        printf("无法分配视频帧\n");
        goto end;
    }
    
    // 确定目标尺寸
    target_width = 900;  // 根据您的UI调整
    target_height = 500; // 根据您的UI调整
    
    // 分配内存
    buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_BGRA, target_width, target_height, 1));
    if (!buffer) {
        printf("无法分配内存\n");
        goto end;
    }

    // 设置pFrameRGB的数据指针和行大小
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer,
        AV_PIX_FMT_BGRA, target_width, target_height, 1);

    // 初始化SWS上下文用于图像转换
    sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, 
            pCodecCtx->pix_fmt,
            target_width, target_height, AV_PIX_FMT_BGRA,
            SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if (!sws_ctx) {
        printf("无法初始化图像转换上下文\n");
        goto end;
    }

    // 创建LVGL图像描述符
    img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
    img_dsc.header.always_zero = 0;
    img_dsc.header.reserved = 0;
    img_dsc.header.w = target_width;
    img_dsc.header.h = target_height;
    img_dsc.data_size = target_width * target_height * 4;
    img_dsc.data = buffer;
    
    // 创建图像对象
    pthread_mutex_lock(&ui_mutex);
    img_obj = lv_img_create(ui->video_player);
    lv_img_set_src(img_obj, &img_dsc);
    lv_obj_center(img_obj);
    pthread_mutex_unlock(&ui_mutex);
    
    // 分配数据包
    packet = av_packet_alloc();
    if (!packet) {
        printf("无法分配数据包\n");
        goto end;
    }
    
    // printf("开始读取帧并显示\n");
    // 读取帧并显示
    while (av_read_frame(pFormatCtx, packet) >= 0 && video_thread_running) {
        // 添加取消点
        pthread_testcancel();        
        
        // 检查是否应该终止线程
        pthread_mutex_lock(&video_mutex);
        bool should_exit = !video_thread_running;
        pthread_mutex_unlock(&video_mutex);
        
        if (should_exit) {
            // printf("收到线程终止请求，正在退出视频播放线程...\n");
            av_packet_unref(packet);
            break;
        }                

        // 检查是否有跳转请求
        pthread_mutex_lock(&video_mutex);
        bool seek_requested = video_seek_requested;
        int seek_time = video_seek_time;
        if (seek_requested) {
            video_seek_requested = false;
        }
        pthread_mutex_unlock(&video_mutex);
        
        // 处理跳转请求
        if (seek_requested) {
            int64_t seek_target = (int64_t)(seek_time * AV_TIME_BASE);
            int ret = av_seek_frame(pFormatCtx, -1, seek_target, AVSEEK_FLAG_BACKWARD);
            
            if (ret < 0) {
                // printf("跳转失败: %s\n", av_err2str(ret));
            } else {
                // printf("成功跳转到 %d 秒\n", seek_time);
                
                // 清空解码器缓冲区
                avcodec_flush_buffers(pCodecCtx);
                
                // 更新当前时间
                current_time = seek_time;
                
                // 准备更新数据
                video_progress_update_t *update_data = malloc(sizeof(video_progress_update_t));
                if (update_data) {
                    update_data->ui = ui;
                    update_data->current_time = current_time;
                    update_data->duration = duration;
                    
                    // 格式化当前时间
                    int hours = (int)current_time / 3600;
                    int minutes = ((int)current_time % 3600) / 60;
                    int seconds = (int)current_time % 60;
                    
                    if (hours > 0 || duration > 3600) {
                        snprintf(update_data->time_text, sizeof(update_data->time_text), 
                                "%02d:%02d:%02d", hours, minutes, seconds);
                    } else {
                        snprintf(update_data->time_text, sizeof(update_data->time_text), 
                                "%02d:%02d", minutes, seconds);
                    }
                    
                    // 异步更新UI
                    lv_async_call(update_progress_cb, update_data);
                }
            }
        }

        pthread_mutex_lock(&video_mutex);
        if (!video_thread_running) {
            pthread_mutex_unlock(&video_mutex);
            break;
        }
        bool playing = is_video_playing;
        pthread_mutex_unlock(&video_mutex);

        if (!playing) {
            // 暂停状态
            pthread_mutex_lock(&video_mutex);
            while (is_video_playing == false && video_thread_running) {
                pthread_cond_wait(&video_cond, &video_mutex);
            }
            pthread_mutex_unlock(&video_mutex);
            
            // 如果线程被要求停止，跳出循环
            if (!video_thread_running) {
                av_packet_unref(packet);
                break;
            }
        }

        // 只处理视频流
        if (packet->stream_index != videoStream) {
            av_packet_unref(packet);
            continue;
        }
        // 计算当前播放时间
        if (packet->pts != AV_NOPTS_VALUE) {
            AVRational time_base = pFormatCtx->streams[videoStream]->time_base;
            current_time = packet->pts * av_q2d(time_base);
            
            // 每200毫秒更新一次UI，避免频繁更新
            int64_t current_ms = av_gettime() / 1000;
            if (current_ms - last_update_time > 500) {
                last_update_time = current_ms;
                
                // 准备更新数据
                video_progress_update_t *update_data = malloc(sizeof(video_progress_update_t));
                if (update_data) {
                    update_data->ui = ui;
                    update_data->current_time = current_time;
                    update_data->duration = duration;
                    
                    // 格式化当前时间
                    int hours = (int)current_time / 3600;
                    int minutes = ((int)current_time % 3600) / 60;
                    int seconds = (int)current_time % 60;
                    
                    if (hours > 0 || duration > 3600) {
                        snprintf(update_data->time_text, sizeof(update_data->time_text), 
                                "%02d:%02d:%02d", hours, minutes, seconds);
                    } else {
                        snprintf(update_data->time_text, sizeof(update_data->time_text), 
                                "%02d:%02d", minutes, seconds);
                    }
                    
                    // 异步更新UI
                    lv_async_call(update_progress_cb, update_data);
                }
            }
        }        

        // 发送数据包到解码器
        ret = avcodec_send_packet(pCodecCtx, packet);
        if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, AV_ERROR_MAX_STRING_SIZE);
            
            if (ret == AVERROR(EAGAIN)) {
                // printf("解码器缓冲区已满，需要先接收帧\n");
                // 接收帧以清空解码器缓冲区
                AVFrame *temp_frame = av_frame_alloc();
                if (temp_frame) {
                    avcodec_receive_frame(pCodecCtx, temp_frame);
                    av_frame_free(&temp_frame);
                    // 再次尝试发送数据包
                    ret = avcodec_send_packet(pCodecCtx, packet);
                }
            } else {
                printf("发送数据包到解码器失败: %s (错误码: %d)\n", errbuf, ret);
                av_packet_unref(packet);
                continue;
            }
            
            // 如果再次尝试仍然失败
            if (ret < 0) {
                av_packet_unref(packet);
                continue;
            }
        }
        
        // 接收解码后的帧
        while (ret >= 0) {
            ret = avcodec_receive_frame(pCodecCtx, pFrame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                // 需要更多数据包或已到达文件末尾
                break;
            } else if (ret < 0) {
                printf("从解码器接收帧失败\n");
                break;
            }
            
            // 帧解码成功，进行色彩空间转换
            if (!pFrameRGB->data[0] || !buffer) {
                printf("帧数据无效，跳过此帧\n");
                continue;
            }
            
            // 进行色彩空间转换
            int convert_ret = sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
                                      pFrame->linesize, 0, pCodecCtx->height,
                                      pFrameRGB->data, pFrameRGB->linesize);
            
            if (convert_ret <= 0) {
                printf("色彩空间转换失败\n");
                continue;
            }
 
            // // 将解码后的帧存储到队列中
            // Frame frame;
            // frame.width = target_width;
            // frame.height = target_height;
            // frame.data = malloc(target_width * target_height * 4);  // 假设使用 BGRA 格式
            // memcpy(frame.data, buffer, target_width * target_height * 4);
            // enqueue_frame(&frame);

            // 安全地更新UI
            pthread_mutex_lock(&ui_mutex);
            if (img_obj && lv_obj_is_valid(img_obj)) {
                lv_img_set_src(img_obj, &img_dsc);
                lv_refr_now(NULL);
            }
            pthread_mutex_unlock(&ui_mutex);
            // 在每次循环结束时再次检查终止标志
            pthread_mutex_lock(&video_mutex);
            should_exit = !video_thread_running;
            pthread_mutex_unlock(&video_mutex);
            
            if (should_exit) {
                // printf("收到线程终止请求，正在退出视频播放线程...\n");
                av_packet_unref(packet);
                break;
            }
            // 控制播放速度
            usleep(33000); // 约30fps
            // 在耗时操作后添加取消点
            pthread_testcancel();                       
        }
        // 释放数据包
        av_packet_unref(packet);
    }
end:
    // 清理资源
    if (buffer) {
        av_free(buffer);
        buffer = NULL;
    }
    if (pFrameRGB) {
        av_frame_free(&pFrameRGB);
        pFrameRGB = NULL;
    }
    if (pFrame) {
        av_frame_free(&pFrame);
        pFrame = NULL;
    }
    if (pCodecCtx) {
        avcodec_free_context(&pCodecCtx);
        pCodecCtx = NULL;
    }
    if (pFormatCtx) {
        avformat_close_input(&pFormatCtx);
        pFormatCtx = NULL;
    }
    if (packet) {
        av_packet_free(&packet);
        packet = NULL;
    }
    if (sws_ctx) {
        sws_freeContext(sws_ctx);
        sws_ctx = NULL;
    }
    if (img_obj) {
        // 使用LVGL的异步调用删除UI对象
        lv_async_call(delete_img_obj_cb, img_obj);
        img_obj = NULL;
    }

    pthread_mutex_lock(&video_mutex);
    video_thread_running = false;
    pthread_mutex_unlock(&video_mutex);
    
    // printf("视频播放线程已结束\n");
    if (ui->video_play_label && lv_obj_is_valid(ui->video_play_label)) {
        lv_label_set_text(ui->video_play_label, LV_SYMBOL_PLAY);
    }
    
    return NULL;
}

// 添加帧到队列
void enqueue_frame(Frame *frame) {
    pthread_mutex_lock(&frame_queue.mutex);
    while (frame_queue.count == FRAME_QUEUE_SIZE) {
        pthread_cond_wait(&frame_queue.cond, &frame_queue.mutex);
    }
    frame_queue.frames[frame_queue.tail] = *frame;
    frame_queue.tail = (frame_queue.tail + 1) % FRAME_QUEUE_SIZE;
    frame_queue.count++;
    pthread_cond_signal(&frame_queue.cond);
    pthread_mutex_unlock(&frame_queue.mutex);
}

// 从队列中取出帧
bool dequeue_frame(Frame *frame) {
    pthread_mutex_lock(&frame_queue.mutex);
    while (frame_queue.count == 0) {
        pthread_cond_wait(&frame_queue.cond, &frame_queue.mutex);
    }
    *frame = frame_queue.frames[frame_queue.head];
    frame_queue.head = (frame_queue.head + 1) % FRAME_QUEUE_SIZE;
    frame_queue.count--;
    pthread_cond_signal(&frame_queue.cond);
    pthread_mutex_unlock(&frame_queue.mutex);
    return true;
}

void update_ui(lv_ui *ui) {
    Frame frame;
    if (dequeue_frame(&frame)) {
        // 更新 LVGL 图像对象
        lv_img_dsc_t *img_dsc = malloc(sizeof(lv_img_dsc_t));
        img_dsc->header.always_zero = 0;
        img_dsc->header.w = frame.width;
        img_dsc->header.h = frame.height;
        img_dsc->header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
        img_dsc->data_size = frame.width * frame.height * 4;
        img_dsc->data = frame.data;

        pthread_mutex_lock(&ui_mutex);
        if (ui->video_player && lv_obj_is_valid(ui->video_player)) {
            lv_img_set_src(ui->video_player, img_dsc);
            lv_refr_now(NULL);
        }
        pthread_mutex_unlock(&ui_mutex);

        // 释放帧数据
        free(frame.data);
    }
}

// 回调函数定义
static void delete_img_obj_cb(void *data)
{
    lv_obj_t *img_obj = (lv_obj_t *)data;
    if (img_obj && lv_obj_is_valid(img_obj)) {
        lv_obj_del(img_obj);
    }
}

// 暂停/继续视频播放
void pause_resume_video(lv_ui *ui)
{
    pthread_mutex_lock(&video_mutex);
    is_video_playing = !is_video_playing;
    if (is_video_playing) {
        pthread_cond_signal(&video_cond); // 通知线程继续
    }
    pthread_mutex_unlock(&video_mutex);
}

// 停止视频播放
void stop_video_playback(lv_ui *ui)
{
    // printf("停止视频播放\n");
    if (!video_thread_running) {
        // printf("视频线程已经停止\n");
        return;
    }
    
    // 设置终止标志
    pthread_mutex_lock(&video_mutex);
    video_thread_running = false;
    pthread_cond_signal(&video_cond); // 通知线程结束
    pthread_mutex_unlock(&video_mutex);
    if (pthread_join(video_thread, NULL) != 0) {
        // printf("无法等待视频线程结束\n");
    }   
    // 使用更可靠的方式等待线程结束
    
    // 创建一个超时时间
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 2; // 2秒超时
    
    // 尝试在超时前连接线程
    int join_result = pthread_timedjoin_np(video_thread, NULL, &ts);
    
    if (join_result == 0) {
        // printf("视频线程已正常结束\n");
    } else if (join_result == ETIMEDOUT) {
        // 如果超时，将线程分离而不是继续等待
        pthread_detach(video_thread);
    } else {
        // printf("pthread_timedjoin_np错误: %d\n", join_result);
        // 以防万一，尝试分离线程
        pthread_detach(video_thread);
    }
    
    // 重置状态
    is_video_playing = false;
    
    // 重置UI
    if (ui && ui->video_play_label) {
        lv_label_set_text(ui->video_play_label, LV_SYMBOL_PLAY);
    }
    
}

// 开始播放视频
void start_video_playback(lv_ui *ui, const char *video_path)
{
    if (!ui) {
        printf("错误: UI结构体为NULL\n");
        return;
    }
    if (is_video_playing) {
        // printf("视频已经在播放中，先停止当前视频\n");
        stop_video_playback(ui);
    }
  
    // 如果提供了视频路径，则使用它；否则使用当前路径
    if (video_path != NULL) {
        strncpy(current_video_path, video_path, sizeof(current_video_path) - 1);
        current_video_path[sizeof(current_video_path) - 1] = '\0';
    } else if (current_video_path[0] == '\0') {
        printf("没有指定视频路径\n");
        return;
    }
    
    // printf("开始播放视频: %s\n", current_video_path);
    
    // 设置播放状态
    is_video_playing = true;
    pthread_mutex_lock(&video_mutex);
    video_thread_running = true;
    pthread_mutex_unlock(&video_mutex);

    if (ui->video_play_label && lv_obj_is_valid(ui->video_play_label)) {
        lv_label_set_text(ui->video_play_label, LV_SYMBOL_PAUSE);
    } 
    // 重置进度条和时间标签
    if (ui->video_progress_bar && lv_obj_is_valid(ui->video_progress_bar)) {
        lv_slider_set_value(ui->video_progress_bar, 0, LV_ANIM_OFF);
    }
    
    if (ui->video_current_time_label && lv_obj_is_valid(ui->video_current_time_label)) {
        lv_label_set_text(ui->video_current_time_label, "00:00");
    }
    
    if (ui->video_total_time_label && lv_obj_is_valid(ui->video_total_time_label)) {
        lv_label_set_text(ui->video_total_time_label, "00:00");
    }

    // 创建视频播放线程
    if (pthread_create(&video_thread, NULL, video_playback_thread, ui) != 0) {
        // printf("创建视频播放线程失败\n");
        is_video_playing = false;

        pthread_mutex_lock(&video_mutex);
        video_thread_running = false;
        pthread_mutex_unlock(&video_mutex);
        return;
    }
    
    // printf("视频播放线程已创建\n");
}

// 使用索引播放视频
void play_video_by_index(lv_ui *ui, int index) {
    if (index >= 0 && index < video_count) {
        // 开始播放视频
        start_video_playback(ui, video_paths[index]);
    } else {
        // printf("无效的视频索引: %d\n", index);
    }
}
// 在程序退出函数中
void cleanup_video_resources(void)
{
    // printf("开始清理视频资源...\n");
    
    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&video_mutex);
    pthread_mutex_destroy(&ui_mutex);
    pthread_cond_destroy(&video_cond);
    
    // printf("视频资源已清理\n");
}

