/**
 * @file lv_demo_music_main.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_demo_music_main.h"
#if LV_USE_DEMO_MUSIC

#include "lv_demo_music_list.h"
#include "assets/spectrum_1.h"
#include "assets/spectrum_2.h"
#include "assets/spectrum_3.h"
#include "generated/audio_player.h"

/*********************
 *      DEFINES
 *********************/
#define ACTIVE_TRACK_CNT    4
#define INTRO_TIME          2000
#define BAR_COLOR1          lv_color_hex(0xe9dbfc)
#define BAR_COLOR2          lv_color_hex(0x6f8af6)
#define BAR_COLOR3          lv_color_hex(0xffffff)
#if LV_DEMO_MUSIC_LARGE
    #define BAR_COLOR1_STOP     160
    #define BAR_COLOR2_STOP     200
#else
    #define BAR_COLOR1_STOP     80
    #define BAR_COLOR2_STOP     100
#endif
#define BAR_COLOR3_STOP     (2 * LV_HOR_RES / 3)
#define BAR_CNT             20
#define DEG_STEP            (180/BAR_CNT)
#define BAND_CNT            4
#define BAR_PER_BAND_CNT    (BAR_CNT / BAND_CNT)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_obj_t * create_cont(lv_obj_t * parent);
static void create_wave_images(lv_obj_t * parent);
static lv_obj_t * create_title_box(lv_obj_t * parent);
static lv_obj_t * create_icon_box(lv_obj_t * parent);
static lv_obj_t * create_spectrum_obj(lv_obj_t * parent);
static lv_obj_t * create_ctrl_box(lv_obj_t * parent);
static lv_obj_t * create_handle(lv_obj_t * parent);

static void spectrum_anim_cb(void * a, int32_t v);
static void start_anim_cb(void * a, int32_t v);
static void spectrum_draw_event_cb(lv_event_t * e);
static lv_obj_t * album_img_create(lv_obj_t * parent);
static void album_gesture_event_cb(lv_event_t * e);
static void play_event_click_cb(lv_event_t * e);
static void prev_click_event_cb(lv_event_t * e);
static void next_click_event_cb(lv_event_t * e);
static void slider_event_cb(lv_event_t * e);
static void timer_cb(lv_timer_t * t);
static void track_load(uint32_t id);
static void stop_start_anim_timer_cb(lv_timer_t * t);
static void spectrum_end_cb(lv_anim_t * a);
static void album_fade_anim_cb(void * var, int32_t v);
static int32_t get_cos(int32_t deg, int32_t a);
static int32_t get_sin(int32_t deg, int32_t a);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t * main_cont;
static lv_obj_t * spectrum_obj;
static lv_obj_t * title_label;
static lv_obj_t * artist_label;
// static lv_obj_t * genre_label;
static lv_obj_t * time_obj;
static lv_obj_t * time_all_obj;
static lv_obj_t * album_img_obj;
static lv_obj_t * slider_obj;
static uint32_t spectrum_i = 0;
static uint32_t spectrum_i_pause = 0;
static uint32_t bar_ofs = 0;
static uint32_t spectrum_lane_ofs_start = 0;
static uint32_t bar_rot = 0;
static uint32_t time_act;
static lv_timer_t  * sec_counter_timer;
static lv_timer_t * stop_start_anim_timer;
static const lv_font_t * font_small;
static const lv_font_t * font_large;
static uint32_t track_id = 0;
static bool playing;
static bool start_anim;
static lv_coord_t start_anim_values[40];
static lv_obj_t * play_obj;
static lv_obj_t * prev_obj;
static const uint16_t (* spectrum)[4];
static uint32_t spectrum_len;
static const uint16_t rnd_array[30] = {994, 285, 553, 11, 792, 707, 966, 641, 852, 827, 44, 352, 146, 581, 490, 80, 729, 58, 695, 940, 724, 561, 124, 653, 27, 292, 557, 506, 382, 199};

/**********************
 *      MACROS
 **********************/
LV_IMG_DECLARE(img_lv_demo_music_btn_loop); //顺序播放
LV_IMG_DECLARE(img_lv_demo_music_btn_rnd);  //随机播放
LV_IMG_DECLARE(img_lv_demo_music_btn_sgc);  //单曲循环

uint8_t play_id = 1;
uint8_t player_pause_flag = 0;
uint32_t frame_step = 0;
uint32_t total_time = 0;
uint8_t slider_update_flag = 0;

typedef enum {
    PLAY_MODE_LOOP = 0,   // 顺序播放
    PLAY_MODE_SINGLE,     // 单曲循环
    PLAY_MODE_RANDOM      // 随机播放
} play_mode_t;

static play_mode_t play_mode = PLAY_MODE_LOOP;
static lv_obj_t * loop_icon_obj = NULL; // 保存循环图标对象

static void my_music_play();

void my_music_play_uart(void)
{
    _lv_demo_music_resume();
    if(player_pause_flag == 1) {
        audio_player_pause();
        player_pause_flag = 0;
    }
    else {
        audio_player_play();
    }
}

void my_music_pause(void)
{
    _lv_demo_music_pause();
    audio_player_pause();
    player_pause_flag = 1;
}

void my_music_next(void)
{
    if(play_mode == PLAY_MODE_SINGLE) {
        // 单曲循环，重播当前曲目
        _lv_demo_music_play(track_id);
        my_music_play();
        // 不切换 play_id
    } else if(play_mode == PLAY_MODE_RANDOM) {
        // 随机播放
        uint32_t new_id = lv_rand(0, ACTIVE_TRACK_CNT - 1);
        while(new_id == track_id && ACTIVE_TRACK_CNT > 1) {
            new_id = lv_rand(0, ACTIVE_TRACK_CNT - 1);
        }
        track_id = new_id + 1;
        if (track_id > 3) track_id = 0;
        play_id = track_id;
        if (play_id == 0) play_id = 4;
        _lv_demo_music_album_next(false);
        _lv_demo_music_resume();
        my_music_play();
    } else {
        // 顺序播放
        _lv_demo_music_album_next(true);
        _lv_demo_music_resume();
        play_id ++;
        if(play_id > 4) play_id = 1;
        my_music_play();
    } 
}

void my_music_prev(void)
{
    if(play_mode == PLAY_MODE_SINGLE) {
        // 单曲循环，重播当前曲目
        _lv_demo_music_play(track_id);
        my_music_play();
        // 不切换 play_id
    } else if(play_mode == PLAY_MODE_RANDOM) {
        // 随机播放
        uint32_t new_id = lv_rand(0, ACTIVE_TRACK_CNT - 1);
        while(new_id == track_id && ACTIVE_TRACK_CNT > 1) {
            new_id = lv_rand(0, ACTIVE_TRACK_CNT - 1);
        }
        track_id = new_id + 1;
        if (track_id > 3) track_id = 0;
        play_id = track_id;
        if (play_id == 0) play_id = 4;
        _lv_demo_music_album_next(false);
        _lv_demo_music_resume();
        my_music_play();
    } else {
        // 顺序播放
        _lv_demo_music_album_next(false);
        _lv_demo_music_resume();
        play_id --;
        if(play_id <= 0) play_id = 4;
        my_music_play();
    }
}




/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/*
 * Callback adapter function to convert parameter types to avoid compile-time
 * warning.
 */
static void _img_set_zoom_anim_cb(void * obj, int32_t zoom)
{
    lv_img_set_zoom((lv_obj_t *)obj, (uint16_t)zoom);
}

/*
 * Callback adapter function to convert parameter types to avoid compile-time
 * warning.
 */
static void _obj_set_x_anim_cb(void * obj, int32_t x)
{
    lv_obj_set_x((lv_obj_t *)obj, (lv_coord_t)x);
}

uint32_t _lv_demo_music_get_track_length_2(uint32_t track_id)
{
    switch(track_id) {
        case 0: return 60; 
        case 1: return 269; // 4:29
        case 2: return 60; 
        case 3: return 60; 
        default: return 0;
    }
}

lv_obj_t * _lv_demo_music_main_create(lv_obj_t * parent)
{
#if LV_DEMO_MUSIC_LARGE
    font_small = &lv_font_simsun_22;
    font_large = &lv_font_simsun_32;
#else
    font_small = &lv_font_montserrat_12;
    font_large = &lv_font_montserrat_16;
#endif

    /*Create the content of the music player*/
    lv_obj_t * cont = create_cont(parent);
    create_wave_images(cont);
    lv_obj_t * title_box = create_title_box(cont);
    lv_obj_t * icon_box = create_icon_box(cont);
    lv_obj_t * ctrl_box = create_ctrl_box(cont);
    spectrum_obj = create_spectrum_obj(cont);
    lv_obj_t * handle_box = create_handle(cont);

#if LV_DEMO_MUSIC_ROUND
    lv_obj_set_style_pad_hor(cont, LV_HOR_RES / 6, 0);
#endif

    /*Arrange the content into a grid*/
#if LV_DEMO_MUSIC_SQUARE || LV_DEMO_MUSIC_ROUND
    static const lv_coord_t grid_cols[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_rows[] = {LV_DEMO_MUSIC_HANDLE_SIZE,     /*Spacing*/
                                     0,   /*Spectrum obj, set later*/
                                     LV_GRID_CONTENT, /*Title box*/
                                     LV_GRID_FR(3),   /*Spacer*/
                                     LV_GRID_CONTENT, /*Icon box*/
                                     LV_GRID_FR(3),   /*Spacer*/
                                     LV_GRID_CONTENT, /*Control box*/
                                     LV_GRID_FR(3),   /*Spacer*/
                                     LV_GRID_CONTENT, /*Handle box*/
                                     LV_DEMO_MUSIC_HANDLE_SIZE,     /*Spacing*/
                                     LV_GRID_TEMPLATE_LAST
                                    };

    grid_rows[1] = LV_VER_RES;

    lv_obj_set_grid_dsc_array(cont, grid_cols, grid_rows);
    lv_obj_set_style_grid_row_align(cont, LV_GRID_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_grid_cell(spectrum_obj, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_set_grid_cell(title_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_ALIGN_CENTER, 2, 1);
    lv_obj_set_grid_cell(icon_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_ALIGN_CENTER, 4, 1);
    lv_obj_set_grid_cell(ctrl_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_ALIGN_CENTER, 6, 1);
    lv_obj_set_grid_cell(handle_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_ALIGN_CENTER, 8, 1);
#elif LV_DEMO_MUSIC_LANDSCAPE == 0
    static const lv_coord_t grid_cols[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t grid_rows[] = {LV_DEMO_MUSIC_HANDLE_SIZE,     /*Spacing*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Title box*/
                                           LV_GRID_FR(3),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Icon box*/
                                           LV_GRID_FR(3),   /*Spacer*/
# if LV_DEMO_MUSIC_LARGE == 0
                                           250,    /*Spectrum obj*/
# else
                                           480,   /*Spectrum obj*/
# endif
                                           LV_GRID_FR(3),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Control box*/
                                           LV_GRID_FR(3),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Handle box*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_DEMO_MUSIC_HANDLE_SIZE,     /*Spacing*/
                                           LV_GRID_TEMPLATE_LAST
                                          };

    lv_obj_set_grid_dsc_array(cont, grid_cols, grid_rows);
    lv_obj_set_style_grid_row_align(cont, LV_GRID_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_grid_cell(title_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    lv_obj_set_grid_cell(icon_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 4, 1);
    lv_obj_set_grid_cell(spectrum_obj, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 6, 1);
    lv_obj_set_grid_cell(ctrl_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 8, 1);
    lv_obj_set_grid_cell(handle_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 10, 1);
#else
    /*Arrange the content into a grid*/
    static const lv_coord_t grid_cols[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t grid_rows[] = {LV_DEMO_MUSIC_HANDLE_SIZE,     /*Spacing*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Title box*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Icon box*/
                                           LV_GRID_FR(3),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Control box*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Handle box*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_DEMO_MUSIC_HANDLE_SIZE,     /*Spacing*/
                                           LV_GRID_TEMPLATE_LAST
                                          };

    lv_obj_set_grid_dsc_array(cont, grid_cols, grid_rows);
    lv_obj_set_style_grid_row_align(cont, LV_GRID_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_grid_cell(title_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    lv_obj_set_grid_cell(icon_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 4, 1);
    lv_obj_set_grid_cell(ctrl_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 6, 1);
    lv_obj_set_grid_cell(handle_box, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_CENTER, 8, 1);
    lv_obj_set_grid_cell(spectrum_obj, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 1, 9);
#endif

    sec_counter_timer = lv_timer_create(timer_cb, 1000, NULL);
    lv_timer_pause(sec_counter_timer);

    /*Animate in the content after the intro time*/
    lv_anim_t a;

    start_anim = true;

    stop_start_anim_timer = lv_timer_create(stop_start_anim_timer_cb, INTRO_TIME + 6000, NULL);
    lv_timer_set_repeat_count(stop_start_anim_timer, 1);

    lv_anim_init(&a);
    lv_anim_set_path_cb(&a, lv_anim_path_bounce);

    uint32_t i;
    lv_anim_set_exec_cb(&a, start_anim_cb);
    for(i = 0; i < BAR_CNT; i++) {
        lv_anim_set_values(&a, LV_HOR_RES, 5);
        lv_anim_set_delay(&a, INTRO_TIME - 200 + rnd_array[i] % 200);
        lv_anim_set_time(&a, 2500 + rnd_array[i] % 500);
        lv_anim_set_var(&a, &start_anim_values[i]);
        lv_anim_start(&a);
    }

    lv_obj_fade_in(title_box, 1000, INTRO_TIME + 1000);
    lv_obj_fade_in(icon_box, 1000, INTRO_TIME + 1000);
    lv_obj_fade_in(ctrl_box, 1000, INTRO_TIME + 1000);
    lv_obj_fade_in(handle_box, 1000, INTRO_TIME + 1000);
    lv_obj_fade_in(album_img_obj, 800, INTRO_TIME + 1000);
    lv_obj_fade_in(spectrum_obj, 0, INTRO_TIME);

    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_var(&a, album_img_obj);
    lv_anim_set_time(&a, 1000);
    lv_anim_set_delay(&a, INTRO_TIME + 1000);
    lv_anim_set_values(&a, 1, LV_IMG_ZOOM_NONE);
    lv_anim_set_exec_cb(&a, _img_set_zoom_anim_cb);
    lv_anim_set_ready_cb(&a, NULL);
    lv_anim_start(&a);

    /* Create an intro from a logo + label */
    LV_IMG_DECLARE(img_lv_demo_music_logo);
    lv_obj_t * logo = lv_img_create(lv_scr_act());
    lv_img_set_src(logo, &img_lv_demo_music_logo);
    lv_obj_move_foreground(logo);

    lv_obj_t * title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "LVGL Demo\nMusic player");
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(title, font_large, 0);
    lv_obj_set_style_text_line_space(title, 8, 0);
    lv_obj_fade_out(title, 500, INTRO_TIME);
    lv_obj_align_to(logo, spectrum_obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align_to(title, logo, LV_ALIGN_OUT_LEFT_MID, -20, 0);

    lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_var(&a, logo);
    lv_anim_set_time(&a, 400);
    lv_anim_set_delay(&a, INTRO_TIME + 800);
    lv_anim_set_values(&a, LV_IMG_ZOOM_NONE, 10);
    lv_anim_set_ready_cb(&a, lv_obj_del_anim_ready_cb);
    lv_anim_start(&a);

    lv_obj_update_layout(main_cont);

    return main_cont;
}

void _lv_demo_music_main_close(void)
{
    if(stop_start_anim_timer) lv_timer_del(stop_start_anim_timer);
    lv_timer_del(sec_counter_timer);
}

void _lv_demo_music_album_next(bool next)
{
    uint32_t id = track_id;
    if(next) {
        id++;
        if(id >= ACTIVE_TRACK_CNT) id = 0;
    }
    else {
        if(id == 0) {
            id = ACTIVE_TRACK_CNT - 1;
        }
        else {
            id--;
        }
    }

    if(playing) {
        _lv_demo_music_play(id);
    }
    else {
        track_load(id);
    }
}

void _lv_demo_music_play(uint32_t id)
{
    track_load(id);

    _lv_demo_music_resume();
}

void _lv_demo_music_resume(void)
{
    playing = true;
    spectrum_i = spectrum_i_pause;
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_values(&a, spectrum_i, spectrum_len - 1);
    lv_anim_set_exec_cb(&a, spectrum_anim_cb);
    lv_anim_set_var(&a, spectrum_obj);
    lv_anim_set_time(&a, ((spectrum_len - spectrum_i) * 1000) / 30);
    lv_anim_set_playback_time(&a, 0);
    lv_anim_set_ready_cb(&a, spectrum_end_cb);
    lv_anim_start(&a);

    lv_timer_resume(sec_counter_timer);
    // lv_slider_set_range(slider_obj, 0, _lv_demo_music_get_track_length(track_id));
    lv_slider_set_range(slider_obj, 0, _lv_demo_music_get_track_length_2(track_id)); // 设置进度条范围为歌曲总时长

    lv_obj_add_state(play_obj, LV_STATE_CHECKED);

}

void _lv_demo_music_pause(void)
{
    playing = false;
    spectrum_i_pause = spectrum_i;
    spectrum_i = 0;
    lv_anim_del(spectrum_obj, spectrum_anim_cb);
    lv_obj_invalidate(spectrum_obj);
    lv_img_set_zoom(album_img_obj, LV_IMG_ZOOM_NONE);
    lv_timer_pause(sec_counter_timer);
    lv_obj_clear_state(play_obj, LV_STATE_CHECKED);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_obj_t * create_cont(lv_obj_t * parent)
{
    /*A transparent container in which the player section will be scrolled*/
    main_cont = lv_obj_create(parent);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_remove_style_all(main_cont);                            /*Make it transparent*/
    lv_obj_set_size(main_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_scroll_snap_y(main_cont, LV_SCROLL_SNAP_CENTER);    /*Snap the children to the center*/

    /*Create a container for the player*/
    lv_obj_t * player = lv_obj_create(main_cont);
    lv_obj_set_y(player, - LV_DEMO_MUSIC_HANDLE_SIZE);
#if LV_DEMO_MUSIC_SQUARE || LV_DEMO_MUSIC_ROUND
    lv_obj_set_size(player, LV_HOR_RES, 2 * LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE * 2);
#else
    lv_obj_set_size(player, LV_HOR_RES, LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE * 2);
#endif
    lv_obj_clear_flag(player, LV_OBJ_FLAG_SNAPABLE);

    lv_obj_set_style_bg_color(player, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_width(player, 0, 0);
    lv_obj_set_style_pad_all(player, 0, 0);
    lv_obj_set_scroll_dir(player, LV_DIR_VER);

    /* Transparent placeholders below the player container
     * It is used only to snap it to center.*/
    lv_obj_t * placeholder1 = lv_obj_create(main_cont);
    lv_obj_remove_style_all(placeholder1);
    lv_obj_clear_flag(placeholder1, LV_OBJ_FLAG_CLICKABLE);
    //    lv_obj_set_style_bg_color(placeholder1, lv_color_hex(0xff0000), 0);
    //    lv_obj_set_style_bg_opa(placeholder1, LV_OPA_50, 0);

    lv_obj_t * placeholder2 = lv_obj_create(main_cont);
    lv_obj_remove_style_all(placeholder2);
    lv_obj_clear_flag(placeholder2, LV_OBJ_FLAG_CLICKABLE);
    //    lv_obj_set_style_bg_color(placeholder2, lv_color_hex(0x00ff00), 0);
    //    lv_obj_set_style_bg_opa(placeholder2, LV_OPA_50, 0);

#if LV_DEMO_MUSIC_SQUARE || LV_DEMO_MUSIC_ROUND
    lv_obj_t * placeholder3 = lv_obj_create(main_cont);
    lv_obj_remove_style_all(placeholder3);
    lv_obj_clear_flag(placeholder3, LV_OBJ_FLAG_CLICKABLE);
    //    lv_obj_set_style_bg_color(placeholder3, lv_color_hex(0x0000ff), 0);
    //    lv_obj_set_style_bg_opa(placeholder3, LV_OPA_20, 0);

    lv_obj_set_size(placeholder1, lv_pct(100), LV_VER_RES);
    lv_obj_set_y(placeholder1, 0);

    lv_obj_set_size(placeholder2, lv_pct(100), LV_VER_RES);
    lv_obj_set_y(placeholder2, LV_VER_RES);

    lv_obj_set_size(placeholder3, lv_pct(100),  LV_VER_RES - 2 * LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_set_y(placeholder3, 2 * LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE);
#else
    lv_obj_set_size(placeholder1, lv_pct(100), LV_VER_RES);
    lv_obj_set_y(placeholder1, 0);

    lv_obj_set_size(placeholder2, lv_pct(100),  LV_VER_RES - 2 * LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_set_y(placeholder2, LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE);
#endif

    lv_obj_update_layout(main_cont);

    return player;
}

static void create_wave_images(lv_obj_t * parent)
{
    LV_IMG_DECLARE(img_lv_demo_music_wave_top);
    LV_IMG_DECLARE(img_lv_demo_music_wave_bottom);
    lv_obj_t * wave_top = lv_img_create(parent);
    lv_img_set_src(wave_top, &img_lv_demo_music_wave_top);
    lv_obj_set_width(wave_top, LV_HOR_RES);
    lv_obj_align(wave_top, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_flag(wave_top, LV_OBJ_FLAG_IGNORE_LAYOUT);

    lv_obj_t * wave_bottom = lv_img_create(parent);
    lv_img_set_src(wave_bottom, &img_lv_demo_music_wave_bottom);
    lv_obj_set_width(wave_bottom, LV_HOR_RES);
    lv_obj_align(wave_bottom, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(wave_bottom, LV_OBJ_FLAG_IGNORE_LAYOUT);

    LV_IMG_DECLARE(img_lv_demo_music_corner_left);
    LV_IMG_DECLARE(img_lv_demo_music_corner_right);
    lv_obj_t * wave_corner = lv_img_create(parent);
    lv_img_set_src(wave_corner, &img_lv_demo_music_corner_left);
#if LV_DEMO_MUSIC_ROUND == 0
    lv_obj_align(wave_corner, LV_ALIGN_BOTTOM_LEFT, 0, 0);
#else
    lv_obj_align(wave_corner, LV_ALIGN_BOTTOM_LEFT, -LV_HOR_RES / 6, 0);
#endif
    lv_obj_add_flag(wave_corner, LV_OBJ_FLAG_IGNORE_LAYOUT);

    wave_corner = lv_img_create(parent);
    lv_img_set_src(wave_corner, &img_lv_demo_music_corner_right);
#if LV_DEMO_MUSIC_ROUND == 0
    lv_obj_align(wave_corner, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
#else
    lv_obj_align(wave_corner, LV_ALIGN_BOTTOM_RIGHT, LV_HOR_RES / 6, 0);
#endif
    lv_obj_add_flag(wave_corner, LV_OBJ_FLAG_IGNORE_LAYOUT);
}

static lv_obj_t * create_title_box(lv_obj_t * parent)
{

    /*Create the titles*/
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_height(cont, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    title_label = lv_label_create(cont);
    lv_obj_set_style_text_font(title_label, font_large, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0x504d6d), 0);
    lv_label_set_text(title_label, _lv_demo_music_get_title(track_id));
    lv_obj_set_height(title_label, lv_font_get_line_height(font_large) * 3 / 2);

    artist_label = lv_label_create(cont);
    lv_obj_set_style_text_font(artist_label, font_small, 0);
    lv_obj_set_style_text_color(artist_label, lv_color_hex(0x504d6d), 0);
    lv_label_set_text(artist_label, _lv_demo_music_get_artist(track_id));

    // genre_label = lv_label_create(cont);
    // lv_obj_set_style_text_font(genre_label, font_small, 0);
    // lv_obj_set_style_text_color(genre_label, lv_color_hex(0x8a86b8), 0);
    // lv_label_set_text(genre_label, _lv_demo_music_get_genre(track_id));

    return cont;
}

static lv_obj_t * create_icon_box(lv_obj_t * parent)
{

    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_height(cont, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * icon;
    LV_IMG_DECLARE(img_lv_demo_music_icon_1);
    LV_IMG_DECLARE(img_lv_demo_music_icon_2);
    LV_IMG_DECLARE(img_lv_demo_music_icon_3);
    LV_IMG_DECLARE(img_lv_demo_music_icon_4);
    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_demo_music_icon_1);
    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_demo_music_icon_2);
    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_demo_music_icon_3);
    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_demo_music_icon_4);

    return cont;
}

static lv_obj_t * create_spectrum_obj(lv_obj_t * parent)
{
    /*Create the spectrum visualizer*/
    lv_obj_t * obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
#if LV_DEMO_MUSIC_LARGE
    lv_obj_set_height(obj, 500);
#else
    lv_obj_set_height(obj, 250);
#endif
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(obj, spectrum_draw_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_refresh_ext_draw_size(obj);
    album_img_obj = album_img_create(obj);
    return obj;
}

static void loop_icon_event_cb(lv_event_t * e)
{
    // 切换播放模式
    play_mode = (play_mode + 1) % 3;

    switch(play_mode) {
        case PLAY_MODE_LOOP:
            lv_img_set_src(loop_icon_obj, &img_lv_demo_music_btn_loop);
            // 这里可以添加顺序播放的逻辑
            break;
        case PLAY_MODE_SINGLE:
            lv_img_set_src(loop_icon_obj, &img_lv_demo_music_btn_sgc);
            // 这里可以添加单曲循环的逻辑
            break;
        case PLAY_MODE_RANDOM:
            lv_img_set_src(loop_icon_obj, &img_lv_demo_music_btn_rnd);
            // 这里可以添加随机播放的逻辑
            break;
    }
}

// 添加音量图标点击事件回调函数
static void volume_icon_event_cb(lv_event_t * e)
{
    lv_obj_t * volume_slider = lv_event_get_user_data(e); // 获取音量条对象
    // lv_obj_t * volume_label = lv_obj_get_user_data(volume_slider); // 获取音量值文本对象

    if (lv_obj_has_flag(volume_slider, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(volume_slider, LV_OBJ_FLAG_HIDDEN); // 显示音量条
        // lv_obj_clear_flag(volume_label, LV_OBJ_FLAG_HIDDEN);  // 显示音量值文本
    } else {
        lv_obj_add_flag(volume_slider, LV_OBJ_FLAG_HIDDEN); // 隐藏音量条
        // lv_obj_add_flag(volume_label, LV_OBJ_FLAG_HIDDEN);  // 隐藏音量值文本
    }
}

// 添加音量条回调函数
static void volume_slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);

    // 获取音量条的值
    uint32_t volume = lv_slider_get_value(slider);

    // 调整音频播放器的音量
    audio_player_set_volume(volume); // 假设 audio_player_set_volume 函数以 0-100 为范围

    // // 获取音量值文本对象
    // lv_obj_t * volume_label = lv_event_get_user_data(e);

    // // 更新音量值文本
    // lv_label_set_text_fmt(volume_label, "%d", volume);
}

static lv_obj_t * create_ctrl_box(lv_obj_t * parent)
{
    /*Create the control box*/
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_height(cont, LV_SIZE_CONTENT);
#if LV_DEMO_MUSIC_LARGE
    lv_obj_set_style_pad_bottom(cont, 17, 0);
#else
    lv_obj_set_style_pad_bottom(cont, 8, 0);
#endif
    // 修改网格布局，增加一行用于音量条和音量图标
    static const lv_coord_t grid_col[] = {LV_GRID_FR(2), LV_GRID_FR(3), LV_GRID_FR(5), LV_GRID_FR(5), LV_GRID_FR(5), LV_GRID_FR(3), LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t grid_row[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(cont, grid_col, grid_row);

    LV_IMG_DECLARE(img_lv_demo_music_btn_next);
    LV_IMG_DECLARE(img_lv_demo_music_btn_prev);
    LV_IMG_DECLARE(img_lv_demo_music_btn_play);
    LV_IMG_DECLARE(img_lv_demo_music_btn_pause);
    LV_IMG_DECLARE(img_lv_demo_music_slider_knob);

    // 添加音量图标
    LV_IMG_DECLARE(_img_lv_demo_music_volume_alpha_40x40);
    lv_obj_t * volume_icon = lv_img_create(cont);
    lv_img_set_src(volume_icon, &_img_lv_demo_music_volume_alpha_40x40);
    lv_obj_set_grid_cell(volume_icon, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_flag(volume_icon, LV_OBJ_FLAG_CLICKABLE);

    // 添加音量条
    lv_obj_t * volume_slider = lv_slider_create(cont);
    lv_obj_set_style_anim_time(volume_slider, 100, 0);
    lv_obj_add_flag(volume_slider, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_HIDDEN); // 默认隐藏音量条
    lv_obj_add_event_cb(volume_slider, volume_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL); // 添加事件回调

    lv_obj_set_height(volume_slider, 6);
    // lv_obj_set_width(volume_slider, 160); // 设置音量条的宽度
    lv_obj_set_grid_cell(volume_slider, LV_GRID_ALIGN_STRETCH, 2, 3, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_set_style_bg_img_src(volume_slider, &img_lv_demo_music_slider_knob, LV_PART_KNOB);
    lv_obj_set_style_bg_opa(volume_slider, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_pad_all(volume_slider, 20, LV_PART_KNOB);
    lv_obj_set_style_bg_grad_dir(volume_slider, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0x569af8), LV_PART_INDICATOR);
    lv_obj_set_style_bg_grad_color(volume_slider, lv_color_hex(0xa666f1), LV_PART_INDICATOR);
    lv_obj_set_style_outline_width(volume_slider, 0, 0);

    // 设置音量条初始值
    lv_slider_set_range(volume_slider, 0, 100); // 假设音量范围为 0-100
    lv_slider_set_value(volume_slider, 50, LV_ANIM_OFF); // 默认音量为 50%

    // // 添加音量值文本
    // lv_obj_t * volume_label = lv_label_create(cont);
    // lv_label_set_text(volume_label, "50"); // 初始值为 50
    // lv_obj_set_grid_cell(volume_label, LV_GRID_ALIGN_START, 6, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    // lv_obj_add_flag(volume_label, LV_OBJ_FLAG_HIDDEN); // 默认隐藏音量值文本

    // // 为音量条绑定事件回调，并传递音量值文本对象
    // lv_obj_add_event_cb(volume_slider, volume_slider_event_cb, LV_EVENT_VALUE_CHANGED, volume_label);

    // lv_obj_set_user_data(volume_slider, volume_label); // 将音量值文本绑定到音量条
    lv_obj_add_event_cb(volume_icon, volume_icon_event_cb, LV_EVENT_CLICKED, volume_slider);

    lv_obj_t * icon;
    // icon = lv_img_create(cont);
    // lv_img_set_src(icon, &img_lv_demo_music_btn_rnd);
    // lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    loop_icon_obj = lv_img_create(cont);
    lv_img_set_src(loop_icon_obj, &img_lv_demo_music_btn_loop); // 默认顺序播放
    lv_obj_set_grid_cell(loop_icon_obj, LV_GRID_ALIGN_END, 5, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_add_flag(loop_icon_obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(loop_icon_obj, loop_icon_event_cb, LV_EVENT_CLICKED, NULL);

    prev_obj = lv_img_create(cont);
    lv_img_set_src(prev_obj, &img_lv_demo_music_btn_prev);
    lv_obj_set_grid_cell(prev_obj, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_add_event_cb(prev_obj, prev_click_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(prev_obj, LV_OBJ_FLAG_CLICKABLE);

    play_obj = lv_imgbtn_create(cont);
    lv_imgbtn_set_src(play_obj, LV_IMGBTN_STATE_RELEASED, NULL, &img_lv_demo_music_btn_play, NULL);
    lv_imgbtn_set_src(play_obj, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &img_lv_demo_music_btn_pause, NULL);
    lv_obj_add_flag(play_obj, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_grid_cell(play_obj, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_add_event_cb(play_obj, play_event_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(play_obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_width(play_obj, img_lv_demo_music_btn_play.header.w);

    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_demo_music_btn_next);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_add_event_cb(icon, next_click_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);

    slider_obj = lv_slider_create(cont);
    lv_obj_set_style_anim_time(slider_obj, 100, 0);
    lv_obj_add_flag(slider_obj, LV_OBJ_FLAG_CLICKABLE); /*允许用户交互*/
    lv_obj_add_event_cb(slider_obj, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL); // 添加事件回调

#if LV_DEMO_MUSIC_LARGE == 0
    lv_obj_set_height(slider_obj, 3);
#else
    lv_obj_set_height(slider_obj, 6);
#endif
    lv_obj_set_grid_cell(slider_obj, LV_GRID_ALIGN_STRETCH, 2, 3, LV_GRID_ALIGN_CENTER, 2, 1);

    lv_obj_set_style_bg_img_src(slider_obj, &img_lv_demo_music_slider_knob, LV_PART_KNOB);
    lv_obj_set_style_bg_opa(slider_obj, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_pad_all(slider_obj, 20, LV_PART_KNOB);
    lv_obj_set_style_bg_grad_dir(slider_obj, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider_obj, lv_color_hex(0x569af8), LV_PART_INDICATOR);
    lv_obj_set_style_bg_grad_color(slider_obj, lv_color_hex(0xa666f1), LV_PART_INDICATOR);
    lv_obj_set_style_outline_width(slider_obj, 0, 0);

    time_obj = lv_label_create(cont);
    lv_obj_set_style_text_font(time_obj, font_small, 0);
    lv_obj_set_style_text_color(time_obj, lv_color_hex(0x8a86b8), 0);
    lv_label_set_text(time_obj, "0:00");
    lv_obj_set_grid_cell(time_obj, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);

    time_all_obj = lv_label_create(cont);
    lv_obj_set_style_text_font(time_all_obj, font_small, 0);
    lv_obj_set_style_text_color(time_all_obj, lv_color_hex(0x8a86b8), 0);
    lv_label_set_text(time_all_obj, "1:00");
    lv_obj_set_grid_cell(time_all_obj, LV_GRID_ALIGN_START, 5, 1, LV_GRID_ALIGN_CENTER, 2, 1);    

    total_time = _lv_demo_music_get_track_length_2(track_id); // 获取当前曲目的总时长（秒）
    lv_label_set_text_fmt(time_all_obj, "%"LV_PRIu32":%02"LV_PRIu32, total_time / 60, total_time % 60);
    
    time_act = 0;
    spectrum_i = 0;
    lv_slider_set_value(slider_obj, 0, 0);

    return cont;
}

static lv_obj_t * create_handle(lv_obj_t * parent)
{
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);

    lv_obj_set_size(cont, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cont, 8, 0);

    /*A handle to scroll to the track list*/
    lv_obj_t * handle_label = lv_label_create(cont);
    lv_label_set_text(handle_label, "ALL TRACKS");
    lv_obj_set_style_text_font(handle_label, font_small, 0);
    lv_obj_set_style_text_color(handle_label, lv_color_hex(0x8a86b8), 0);

    lv_obj_t * handle_rect = lv_obj_create(cont);
#if LV_DEMO_MUSIC_LARGE
    lv_obj_set_size(handle_rect, 40, 3);
#else
    lv_obj_set_size(handle_rect, 20, 2);
#endif

    lv_obj_set_style_bg_color(handle_rect, lv_color_hex(0x8a86b8), 0);
    lv_obj_set_style_border_width(handle_rect, 0, 0);

    return cont;
}

static void track_load(uint32_t id)
{
    spectrum_i = 0;
    time_act = 0;
    spectrum_i_pause = 0;

    lv_slider_set_value(slider_obj, 0, LV_ANIM_OFF);
    lv_label_set_text(time_obj, "0:00");

    if(id == track_id) return;
    bool next = false;
    if((track_id + 1) % ACTIVE_TRACK_CNT == id) next = true;

    _lv_demo_music_list_btn_check(track_id, false);

    track_id = id;

    _lv_demo_music_list_btn_check(id, true);

    lv_label_set_text(title_label, _lv_demo_music_get_title(track_id));
    lv_label_set_text(artist_label, _lv_demo_music_get_artist(track_id));
    // lv_label_set_text(genre_label, _lv_demo_music_get_genre(track_id));

    // 设置总时间
    total_time = _lv_demo_music_get_track_length_2(track_id); // 获取当前曲目的总时长（秒）
    lv_label_set_text_fmt(time_all_obj, "%"LV_PRIu32":%02"LV_PRIu32, total_time / 60, total_time % 60);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, album_img_obj);
    lv_anim_set_values(&a, lv_obj_get_style_img_opa(album_img_obj, 0), LV_OPA_TRANSP);
    lv_anim_set_exec_cb(&a, album_fade_anim_cb);
    lv_anim_set_time(&a, 500);
    lv_anim_start(&a);

    lv_anim_init(&a);
    lv_anim_set_var(&a, album_img_obj);
    lv_anim_set_time(&a, 500);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
#if LV_DEMO_MUSIC_LANDSCAPE
    if(next) {
        lv_anim_set_values(&a, 0, - LV_HOR_RES / 7);
    }
    else {
        lv_anim_set_values(&a, 0, LV_HOR_RES / 7);
    }
#else
    if(next) {
        lv_anim_set_values(&a, 0, - LV_HOR_RES / 2);
    }
    else {
        lv_anim_set_values(&a, 0, LV_HOR_RES / 2);
    }
#endif
    lv_anim_set_exec_cb(&a, _obj_set_x_anim_cb);
    lv_anim_set_ready_cb(&a, lv_obj_del_anim_ready_cb);
    lv_anim_start(&a);

    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_set_var(&a, album_img_obj);
    lv_anim_set_time(&a, 500);
    lv_anim_set_values(&a, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE / 2);
    lv_anim_set_exec_cb(&a, _img_set_zoom_anim_cb);
    lv_anim_set_ready_cb(&a, NULL);
    lv_anim_start(&a);

    album_img_obj = album_img_create(spectrum_obj);

    lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
    lv_anim_set_var(&a, album_img_obj);
    lv_anim_set_time(&a, 500);
    lv_anim_set_delay(&a, 100);
    lv_anim_set_values(&a, LV_IMG_ZOOM_NONE / 4, LV_IMG_ZOOM_NONE);
    lv_anim_set_exec_cb(&a, _img_set_zoom_anim_cb);
    lv_anim_set_ready_cb(&a, NULL);
    lv_anim_start(&a);

    lv_anim_init(&a);
    lv_anim_set_var(&a, album_img_obj);
    lv_anim_set_values(&a, 0, LV_OPA_COVER);
    lv_anim_set_exec_cb(&a, album_fade_anim_cb);
    lv_anim_set_time(&a, 500);
    lv_anim_set_delay(&a, 100);
    lv_anim_start(&a);
}

int32_t get_cos(int32_t deg, int32_t a)
{
    int32_t r = (lv_trigo_cos(deg) * a);

    r += LV_TRIGO_SIN_MAX / 2;
    return r >> LV_TRIGO_SHIFT;
}

int32_t get_sin(int32_t deg, int32_t a)
{
    int32_t r = lv_trigo_sin(deg) * a;

    r += LV_TRIGO_SIN_MAX / 2;
    return r >> LV_TRIGO_SHIFT;

}

lv_opa_t lv_obj_get_style_opa_recursive(const lv_obj_t * obj, uint32_t part)
{
    lv_opa_t opa = lv_obj_get_style_opa(obj, part);
    if(opa != LV_OPA_COVER) {
        const lv_obj_t * parent = lv_obj_get_parent(obj);
        while(parent) {
            lv_opa_t parent_opa = lv_obj_get_style_opa(parent, part);
            if(parent_opa != LV_OPA_COVER) {
                opa = (opa * parent_opa) / 255;
            }
            parent = lv_obj_get_parent(parent);
        }
    }
    return opa;
}

static void spectrum_draw_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
#if LV_DEMO_MUSIC_LANDSCAPE
        lv_event_set_ext_draw_size(e, LV_HOR_RES);
#else
        lv_event_set_ext_draw_size(e, LV_VER_RES);
#endif
    }
    else if(code == LV_EVENT_COVER_CHECK) {
        lv_event_set_cover_res(e, LV_COVER_RES_NOT_COVER);
    }
    else if(code == LV_EVENT_DRAW_POST) {
        lv_obj_t * obj = lv_event_get_target(e);
        lv_draw_ctx_t * draw_ctx = lv_event_get_draw_ctx(e);

        lv_opa_t opa = lv_obj_get_style_opa_recursive(obj, LV_PART_MAIN);
        if(opa < LV_OPA_MIN) return;

        lv_point_t poly[4];
        lv_point_t center;
        center.x = obj->coords.x1 + lv_obj_get_width(obj) / 2;
        center.y = obj->coords.y1 + lv_obj_get_height(obj) / 2;

        lv_draw_rect_dsc_t draw_dsc;
        lv_draw_rect_dsc_init(&draw_dsc);
        draw_dsc.bg_opa = LV_OPA_COVER;

        uint16_t r[64];
        uint32_t i;

        lv_coord_t min_a = 5;
#if LV_DEMO_MUSIC_LARGE == 0
        lv_coord_t r_in = 77;
#else
        lv_coord_t r_in = 160;
#endif
        r_in = (r_in * lv_img_get_zoom(album_img_obj)) >> 8;
        for(i = 0; i < BAR_CNT; i++) r[i] = r_in + min_a;

        uint32_t s;
        for(s = 0; s < 4; s++) {
            uint32_t f;
            uint32_t band_w = 0;    /*Real number of bars in this band.*/
            switch(s) {
                case 0:
                    band_w = 20;
                    break;
                case 1:
                    band_w = 8;
                    break;
                case 2:
                    band_w = 4;
                    break;
                case 3:
                    band_w = 2;
                    break;
            }

            /* Add "side bars" with cosine characteristic.*/
            for(f = 0; f < band_w; f++) {
                uint32_t ampl_main = spectrum[spectrum_i][s];
                int32_t ampl_mod = get_cos(f * 360 / band_w + 180, 180) + 180;
                int32_t t = BAR_PER_BAND_CNT * s - band_w / 2 + f;
                if(t < 0) t = BAR_CNT + t;
                if(t >= BAR_CNT) t = t - BAR_CNT;
                r[t] += (ampl_main * ampl_mod) >> 9;
            }
        }

        uint32_t amax = 20;
        int32_t animv = spectrum_i - spectrum_lane_ofs_start;
        if(animv > amax) animv = amax;
        for(i = 0; i < BAR_CNT; i++) {
            uint32_t deg_space = 1;
            uint32_t deg = i * DEG_STEP + 90;
            uint32_t j = (i + bar_rot + rnd_array[bar_ofs % 10]) % BAR_CNT;
            uint32_t k = (i + bar_rot + rnd_array[(bar_ofs + 1) % 10]) % BAR_CNT;

            uint32_t v = (r[k] * animv + r[j] * (amax - animv)) / amax;
            if(start_anim) {
                v = r_in + start_anim_values[i];
                deg_space = v >> 7;
                if(deg_space < 1) deg_space = 1;
            }

            if(v < BAR_COLOR1_STOP) draw_dsc.bg_color = BAR_COLOR1;
            else if(v > BAR_COLOR3_STOP) draw_dsc.bg_color = BAR_COLOR3;
            else if(v > BAR_COLOR2_STOP) draw_dsc.bg_color = lv_color_mix(BAR_COLOR3, BAR_COLOR2,
                                                                              ((v - BAR_COLOR2_STOP) * 255) / (BAR_COLOR3_STOP - BAR_COLOR2_STOP));
            else draw_dsc.bg_color = lv_color_mix(BAR_COLOR2, BAR_COLOR1,
                                                      ((v - BAR_COLOR1_STOP) * 255) / (BAR_COLOR2_STOP - BAR_COLOR1_STOP));

            uint32_t di = deg + deg_space;

            int32_t x1_out = get_cos(di, v);
            poly[0].x = center.x + x1_out;
            poly[0].y = center.y + get_sin(di, v);

            int32_t x1_in = get_cos(di, r_in);
            poly[1].x = center.x + x1_in;
            poly[1].y = center.y + get_sin(di, r_in);
            di += DEG_STEP - deg_space * 2;

            int32_t x2_in = get_cos(di, r_in);
            poly[2].x = center.x + x2_in;
            poly[2].y = center.y + get_sin(di, r_in);

            int32_t x2_out = get_cos(di, v);
            poly[3].x = center.x + x2_out;
            poly[3].y = center.y + get_sin(di, v);

            lv_draw_polygon(draw_ctx, &draw_dsc, poly, 4);

            poly[0].x = center.x - x1_out;
            poly[1].x = center.x - x1_in;
            poly[2].x = center.x - x2_in;
            poly[3].x = center.x - x2_out;
            lv_draw_polygon(draw_ctx, &draw_dsc, poly, 4);
        }
    }
}

static void spectrum_anim_cb(void * a, int32_t v)
{
    lv_obj_t * obj = a;
    if(start_anim) {
        lv_obj_invalidate(obj);
        return;
    }

    // 使用 frame_step 调整帧索引
    spectrum_i = v; 
    lv_obj_invalidate(obj);

    static uint32_t bass_cnt = 0;
    static int32_t last_bass = -1000;
    static int32_t dir = 1;
    if(spectrum[spectrum_i][0] > 12) {
        if(spectrum_i - last_bass > 5) {
            bass_cnt++;
            last_bass = spectrum_i;
            if(bass_cnt >= 2) {
                bass_cnt = 0;
                spectrum_lane_ofs_start = spectrum_i;
                bar_ofs++;
            }
        }
    }
    if(spectrum[spectrum_i][0] < 4) bar_rot += dir;

    lv_img_set_zoom(album_img_obj, LV_IMG_ZOOM_NONE + spectrum[spectrum_i][0]);
}

static void start_anim_cb(void * a, int32_t v)
{
    lv_coord_t * av = a;
    *av = v;
    lv_obj_invalidate(spectrum_obj);
}

static lv_obj_t * album_img_create(lv_obj_t * parent)
{
    LV_IMG_DECLARE(img_lv_demo_music_cover_1);
    LV_IMG_DECLARE(img_lv_demo_music_cover_2);
    LV_IMG_DECLARE(img_lv_demo_music_cover_3);
    LV_IMG_DECLARE(img_lv_demo_music_cover_4);

    lv_obj_t * img;
    img = lv_img_create(parent);

    switch(track_id) {
        case 3:
            lv_img_set_src(img, &img_lv_demo_music_cover_4);
            spectrum = spectrum_3;
            spectrum_len = sizeof(spectrum_3) / sizeof(spectrum_3[0]);
            break;        
        case 2:
            lv_img_set_src(img, &img_lv_demo_music_cover_3);
            spectrum = spectrum_3;
            spectrum_len = sizeof(spectrum_3) / sizeof(spectrum_3[0]);
            break;
        case 1:
            lv_img_set_src(img, &img_lv_demo_music_cover_2);
            spectrum = spectrum_2;
            spectrum_len = sizeof(spectrum_2) / sizeof(spectrum_2[0]);
            break;
        case 0:
            lv_img_set_src(img, &img_lv_demo_music_cover_1);
            spectrum = spectrum_1;
            spectrum_len = sizeof(spectrum_1) / sizeof(spectrum_1[0]);
            break;
    }
    lv_img_set_antialias(img, false);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(img, album_gesture_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(img, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);

    return img;

}

static void my_music_play()
{
    switch (play_id)  /* Replace with actual expression */
    {
        case 1:
            audio_player_stop();
            audio_player_load("/home/kickpi/dowmload/lvgl_demo/app/music/music1.mp3");
            audio_player_play();
            break;
        case 2:
            audio_player_stop();
            audio_player_load("/home/kickpi/dowmload/lvgl_demo/app/music/music2.mp3");
            audio_player_play();
            break;            
        case 3:
            audio_player_stop();
            audio_player_load("/home/kickpi/dowmload/lvgl_demo/app/music/music3.mp3");
            audio_player_play();
            break;
        case 4:
            audio_player_stop();
            audio_player_load("/home/kickpi/dowmload/lvgl_demo/app/music/music4.mp3");
            audio_player_play();
            break;                    
        default:
            break;
    }
}

static void play_event_click_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    if(lv_obj_has_state(obj, LV_STATE_CHECKED)) {
        _lv_demo_music_resume();
        if(player_pause_flag == 1) {
            audio_player_pause();
            player_pause_flag = 0;
        }
        else {
            audio_player_play();
        }
    }
    else {
        _lv_demo_music_pause();
        audio_player_pause();
        player_pause_flag = 1;
    }
}

static void prev_click_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        // _lv_demo_music_album_next(false);
        // _lv_demo_music_resume();

        if(play_mode == PLAY_MODE_SINGLE) {
            // 单曲循环，重播当前曲目
            _lv_demo_music_play(track_id);
            my_music_play();
            // 不切换 play_id
        } else if(play_mode == PLAY_MODE_RANDOM) {
            // 随机播放
            uint32_t new_id = lv_rand(0, ACTIVE_TRACK_CNT - 1);
            while(new_id == track_id && ACTIVE_TRACK_CNT > 1) {
                new_id = lv_rand(0, ACTIVE_TRACK_CNT - 1);
            }
            track_id = new_id + 1;
            if (track_id > 3) track_id = 0;
            play_id = track_id;
            if (play_id == 0) play_id = 4;
            _lv_demo_music_album_next(false);
            _lv_demo_music_resume();
            my_music_play();
        } else {
            // 顺序播放
            _lv_demo_music_album_next(false);
            _lv_demo_music_resume();
            play_id --;
            if(play_id <= 0) play_id = 4;
            my_music_play();
        }
    }    
}

static void next_click_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        // _lv_demo_music_album_next(true);
        // _lv_demo_music_resume();
        if(play_mode == PLAY_MODE_SINGLE) {
            // 单曲循环，重播当前曲目
            _lv_demo_music_play(track_id);
            my_music_play();
            // 不切换 play_id
        } else if(play_mode == PLAY_MODE_RANDOM) {
            // 随机播放
            uint32_t new_id = lv_rand(0, ACTIVE_TRACK_CNT - 1);
            while(new_id == track_id && ACTIVE_TRACK_CNT > 1) {
                new_id = lv_rand(0, ACTIVE_TRACK_CNT - 1);
            }
            track_id = new_id + 1;
            if (track_id > 3) track_id = 0;
            play_id = track_id;
            if (play_id == 0) play_id = 4;
            _lv_demo_music_album_next(false);
            _lv_demo_music_resume();
            my_music_play();
        } else {
            // 顺序播放
            _lv_demo_music_album_next(true);
            _lv_demo_music_resume();
            play_id ++;
            if(play_id > 4) play_id = 1;
            my_music_play();
        }
    }
}

static void album_gesture_event_cb(lv_event_t * e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if(dir == LV_DIR_LEFT) {
        if(play_mode == PLAY_MODE_SINGLE) {
            // 单曲循环，重播当前曲目
            _lv_demo_music_play(track_id);
            my_music_play();
            // 不切换 play_id
        } else if(play_mode == PLAY_MODE_RANDOM) {
            // 随机播放
            uint32_t new_id = lv_rand(0, ACTIVE_TRACK_CNT - 1);
            while(new_id == track_id && ACTIVE_TRACK_CNT > 1) {
                new_id = lv_rand(0, ACTIVE_TRACK_CNT - 1);
            }
            track_id = new_id + 1;
            if (track_id > 3) track_id = 0;
            play_id = track_id;
            if (play_id == 0) play_id = 4;
            _lv_demo_music_album_next(false);
            _lv_demo_music_resume();
            my_music_play();
        } else {
            // 顺序播放
            _lv_demo_music_album_next(true);
            _lv_demo_music_resume();
            play_id ++;
            if(play_id > 4) play_id = 1;
            my_music_play();
        }        
    }
    if(dir == LV_DIR_RIGHT) {
        if(play_mode == PLAY_MODE_SINGLE) {
            // 单曲循环，重播当前曲目
            _lv_demo_music_play(track_id);
            my_music_play();
            // 不切换 play_id
        } else if(play_mode == PLAY_MODE_RANDOM) {
            // 随机播放
            uint32_t new_id = lv_rand(0, ACTIVE_TRACK_CNT - 1);
            while(new_id == track_id && ACTIVE_TRACK_CNT > 1) {
                new_id = lv_rand(0, ACTIVE_TRACK_CNT - 1);
            }
            track_id = new_id + 1;
            if (track_id > 3) track_id = 0;
            play_id = track_id;
            if (play_id == 0) play_id = 4;
            _lv_demo_music_album_next(false);
            _lv_demo_music_resume();
            my_music_play();
        } else {
            // 顺序播放
            _lv_demo_music_album_next(false);
            _lv_demo_music_resume();
            play_id --;
            if(play_id <= 0) play_id = 4;
            my_music_play();
        }        
    }
}

static void slider_event_cb(lv_event_t * e)
{
    slider_update_flag = 1;
    lv_obj_t * slider = lv_event_get_target(e);

    // 获取进度条的值
    uint32_t new_time = lv_slider_get_value(slider);

    // 更新当前播放时间
    time_act = new_time;

    // 更新时间显示
    lv_label_set_text_fmt(time_obj, "%"LV_PRIu32":%02"LV_PRIu32, time_act / 60, time_act % 60);

    // 调整音频播放进度
    audio_player_set_time(new_time); // 假设音频播放器以秒为单位
    slider_update_flag = 0;
}

static void timer_cb(lv_timer_t * t)
{
    if(slider_update_flag == 0) {
        LV_UNUSED(t);
        time_act++;

        total_time = _lv_demo_music_get_track_length_2(track_id); // 获取当前曲目的总时长（秒）
        // 更新时间显示
        lv_label_set_text_fmt(time_obj, "%"LV_PRIu32":%02"LV_PRIu32, time_act / 60, time_act % 60);
        lv_slider_set_value(slider_obj, time_act, LV_ANIM_ON);

        // 如果当前时间达到总时间，停止计时器并播放下一首
        if (time_act >= total_time) {
            lv_timer_pause(sec_counter_timer); // 暂停计时器
            // _lv_demo_music_album_next(true);  // 播放下一首
            // _lv_demo_music_resume();         // 恢复播放
            // my_music_play();

            if(play_mode == PLAY_MODE_SINGLE) {
                // 单曲循环，重播当前曲目
                _lv_demo_music_play(track_id);
                my_music_play();
                // 不切换 play_id
            } else if(play_mode == PLAY_MODE_RANDOM) {
                // 随机播放
                uint32_t new_id = lv_rand(0, ACTIVE_TRACK_CNT - 1);
                while(new_id == track_id && ACTIVE_TRACK_CNT > 1) {
                    new_id = lv_rand(0, ACTIVE_TRACK_CNT - 1);
                }
                track_id = new_id + 1;
                if (track_id > 3) track_id = 0;
                play_id = track_id;
                if (play_id == 0) play_id = 4;
                _lv_demo_music_album_next(false);
                _lv_demo_music_resume();
                my_music_play();
            } else {
                // 顺序播放
                _lv_demo_music_album_next(true);
                _lv_demo_music_resume();
                play_id ++;
                if(play_id > 4) play_id = 1;
                my_music_play();
            }
        }
    }
}

static void spectrum_end_cb(lv_anim_t * a)
{
    LV_UNUSED(a);
    // _lv_demo_music_album_next(true);
}


static void stop_start_anim_timer_cb(lv_timer_t * t)
{
    LV_UNUSED(t);
    start_anim = false;
    stop_start_anim_timer = NULL;
    lv_obj_refresh_ext_draw_size(spectrum_obj);
}

static void album_fade_anim_cb(void * var, int32_t v)
{
    lv_obj_set_style_img_opa(var, v, 0);
}
#endif /*LV_USE_DEMO_MUSIC*/

