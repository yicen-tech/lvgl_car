#ifndef VIDEO_LOGIC_H
#define VIDEO_LOGIC_H

#include <stdbool.h>
#include <pthread.h>

// // 确保包含用户界面结构
// typedef struct _lv_ui lv_ui;

// 全局变量声明
extern bool is_fullscreen;
extern bool is_video_playing;
extern pthread_mutex_t video_mutex;
extern pthread_mutex_t ui_mutex;
extern pthread_cond_t video_cond;
extern pthread_t video_thread;
extern bool video_thread_running;
extern char current_video_path[512];
extern int video_duration;
extern int video_seek_time;
extern bool video_seek_requested;


void update_ui(lv_ui *ui);
void pause_resume_video(lv_ui *ui);
void start_video_playback(lv_ui *ui, const char *video_path);
void stop_video_playback(lv_ui *ui);
void play_video_by_index(lv_ui *ui, int index);

void cleanup_video_resources(void);


#endif /* VIDEO_LOGIC_H */