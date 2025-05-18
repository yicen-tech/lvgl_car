// audio_player.c
#include "audio_player.h"
#include <stdio.h>
#include <string.h>

static AudioPlayer player = {0};

int audio_player_init(void) {
    // 初始化SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("SDL初始化失败: %s\n", SDL_GetError());
        return -1;
    }

    // 修改音频初始化参数
    // 采样率改为48000（或44100，根据实际音频文件调整）
    // 设置为立体声(2个通道)
    // 增加音频缓冲区大小
    if (Mix_OpenAudio(48000, AUDIO_S16SYS, 2, 4096) < 0) {
        printf("SDL_mixer初始化失败: %s\n", Mix_GetError());
        return -1;
    }

    // 设置混音通道数
    Mix_AllocateChannels(4);

    // 获取并打印实际的音频设备参数
    int frequency;
    Uint16 format;
    int channels;
    Mix_QuerySpec(&frequency, &format, &channels);
    // printf("音频设备参数：\n");
    // printf("采样率: %d Hz\n", frequency);
    // printf("格式: %d\n", format);
    // printf("通道数: %d\n", channels);

    player.state = PLAYER_STOPPED;
    return 0;
}

int audio_player_load(const char *file_path) {
    // 如果当前有音乐在播放，先释放
    if (player.music) {
        Mix_FreeMusic(player.music);
        player.music = NULL;
    }

    // 加载新的音乐文件
    player.music = Mix_LoadMUS(file_path);
    if (!player.music) {
        printf("无法加载音乐文件: %s\n", Mix_GetError());
        return -1;
    }

    //// 获取并打印音乐文件类型
    // Mix_MusicType type = Mix_GetMusicType(player.music);
    // printf("音乐文件类型: ");
    // switch(type) {
    //     case MUS_MP3: printf("MP3\n"); break;
    //     case MUS_OGG: printf("OGG\n"); break;
    //     case MUS_WAV: printf("WAV\n"); break;
    //     case MUS_MOD: printf("MOD\n"); break;
    //     case MUS_MID: printf("MIDI\n"); break;
    //     default: printf("未知\n"); break;
    // }

    strncpy(player.current_file, file_path, sizeof(player.current_file) - 1);
    player.state = PLAYER_STOPPED;
    return 0;
}

void audio_player_cleanup(void) {
    if (player.music) {
        Mix_FreeMusic(player.music);
        player.music = NULL;
    }
    Mix_CloseAudio();
    SDL_Quit();
}

void audio_player_play(void) {
    if (player.music) {
        if (player.state != PLAYER_PLAYING) {
            // 设置音乐播放的渐入效果（可选）
            // Mix_FadeInMusic(player.music, -1, 2000);  // 2000ms渐入
            Mix_PlayMusic(player.music, -1);
            player.state = PLAYER_PLAYING;
        }
    }
}

void audio_player_pause(void) {
    if (player.state == PLAYER_PLAYING) {
        Mix_PauseMusic();
        player.state = PLAYER_PAUSED;
    } else if (player.state == PLAYER_PAUSED) {
        Mix_ResumeMusic();
        player.state = PLAYER_PLAYING;
    }
}

void audio_player_stop(void) {
    Mix_HaltMusic();
    player.state = PLAYER_STOPPED;
}

void audio_player_set_volume(int volume) {
    int sdl_volume = (volume * 128) / 100;
    Mix_VolumeMusic(sdl_volume); // 音量范围0-128
}

void audio_player_set_time(uint32_t time_sec) {
    if (player.music) {
        
        // 设置音乐播放位置
        if (Mix_SetMusicPosition(time_sec) == -1) {
            // printf("无法设置音乐播放位置: %s\n", Mix_GetError());
            return;
        }

        // 如果当前音乐未播放，则开始播放
        if (player.state != PLAYER_PLAYING) {
            Mix_PlayMusic(player.music, -1); // 从指定位置开始播放
            player.state = PLAYER_PLAYING;
        }
    } else {
        // printf("没有加载音乐文件，无法设置播放位置。\n");
    }
}