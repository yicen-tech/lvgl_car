// audio_player.h
#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

// 音频播放器状态
typedef enum {
    PLAYER_STOPPED,
    PLAYER_PLAYING,
    PLAYER_PAUSED
} PlayerState;

// 音频播放器结构体
typedef struct {
    Mix_Music *music;
    PlayerState state;
    char current_file[256];
} AudioPlayer;

// 函数声明
int audio_player_init(void);
void audio_player_cleanup(void);
int audio_player_load(const char *file_path);
void audio_player_play(void);
void audio_player_pause(void);
void audio_player_stop(void);
void audio_player_set_volume(int volume); // 0-128
void audio_player_set_time(uint32_t time_sec); // 设置播放时间（秒）

#endif