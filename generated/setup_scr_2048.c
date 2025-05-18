#include "lvgl/lvgl.h"
#include "gui_guider.h"
#include <stdlib.h>
#include <time.h>

#define GRID_SIZE 4
#define MAX_LEADERBOARD_SIZE 5

static int grid[GRID_SIZE][GRID_SIZE];
static lv_obj_t *tile_labels[GRID_SIZE][GRID_SIZE];

// 添加得分和排行榜相关的全局变量
static uint32_t current_score = 0;
static uint32_t leaderboard[MAX_LEADERBOARD_SIZE] = {0};
static lv_obj_t *score_label = NULL;
static lv_obj_t *leaderboard_labels[MAX_LEADERBOARD_SIZE] = {NULL};

// 添加触摸事件相关的变量
static lv_point_t touch_start_point;
static bool is_swiping = false;
static int swipe_direction = -1; // 0:左 1:右 2:上 3:下

static void init_game();
static void add_random_tile();
static void update_tiles(lv_ui *ui);
static bool move_tiles(int direction);
static bool can_move();
static void restart_game(lv_event_t *e);

// 添加更新得分显示的函数
static void update_score_display() {
    if (score_label) {
        char buf[32];
        snprintf(buf, sizeof(buf), "得分: %lu", current_score);
        lv_label_set_text(score_label, buf);
    }
}

// 添加更新排行榜的函数
static void update_leaderboard() {
    // 检查当前得分是否能进入排行榜
    int insert_pos = -1;
    for (int i = 0; i < MAX_LEADERBOARD_SIZE; i++) {
        if (current_score > leaderboard[i]) {
            insert_pos = i;
            break;
        }
    }
    
    if (insert_pos >= 0) {
        // 移动其他分数
        for (int i = MAX_LEADERBOARD_SIZE - 1; i > insert_pos; i--) {
            leaderboard[i] = leaderboard[i-1];
        }
        leaderboard[insert_pos] = current_score;
    }
    
    // 更新排行榜显示
    for (int i = 0; i < MAX_LEADERBOARD_SIZE; i++) {
        if (leaderboard_labels[i]) {
            char buf[32];
            if (leaderboard[i] > 0) {
                snprintf(buf, sizeof(buf), "%d. %lu", i+1, leaderboard[i]);
            } else {
                snprintf(buf, sizeof(buf), "%d. ---", i+1);
            }
            lv_label_set_text(leaderboard_labels[i], buf);
        }
    }
}

static void init_game() {
    srand(time(NULL));
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = 0;
        }
    }
    add_random_tile();
    add_random_tile();
}

static void add_random_tile() {
    int empty_tiles[GRID_SIZE * GRID_SIZE][2];
    int empty_count = 0;

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 0) {
                empty_tiles[empty_count][0] = i;
                empty_tiles[empty_count][1] = j;
                empty_count++;
            }
        }
    }

    if (empty_count > 0) {
        int random_index = rand() % empty_count;
        int x = empty_tiles[random_index][0];
        int y = empty_tiles[random_index][1];
        grid[x][y] = (rand() % 2 + 1) * 2;
    }
}

static void update_tiles(lv_ui *ui) {
    static const lv_color_t tile_colors[] = {
        LV_COLOR_MAKE(0xCC, 0xCC, 0xCC),  // 0
        LV_COLOR_MAKE(0xEE, 0xE4, 0xDA),  // 2
        LV_COLOR_MAKE(0xED, 0xE0, 0xC8),  // 4
        LV_COLOR_MAKE(0xF2, 0xB1, 0x79),  // 8
        LV_COLOR_MAKE(0xF5, 0x95, 0x63),  // 16
        LV_COLOR_MAKE(0xF6, 0x7C, 0x5F),  // 32
        LV_COLOR_MAKE(0xF6, 0x5E, 0x3B),  // 64
        LV_COLOR_MAKE(0xED, 0xCF, 0x72),  // 128
        LV_COLOR_MAKE(0xED, 0xCC, 0x61),  // 256
        LV_COLOR_MAKE(0xED, 0xC8, 0x50),  // 512
        LV_COLOR_MAKE(0xED, 0xC5, 0x3F),  // 1024
        LV_COLOR_MAKE(0xED, 0xC2, 0x2E),  // 2048
    };

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            lv_obj_t *tile = lv_obj_get_parent(tile_labels[i][j]);
            int value = grid[i][j];
            
            // 更新颜色
            int color_index = 0;
            if (value > 0) {
                color_index = (int)log2(value);
                if (color_index >= sizeof(tile_colors)/sizeof(tile_colors[0])) {
                    color_index = sizeof(tile_colors)/sizeof(tile_colors[0]) - 1;
                }
            }
            lv_obj_set_style_bg_color(tile, tile_colors[color_index], LV_PART_MAIN | LV_STATE_DEFAULT);

            // 更新文本
            if (value == 0) {
                lv_label_set_text(tile_labels[i][j], "");
            } else {
                char buf[8];
                snprintf(buf, sizeof(buf), "%d", value);
                lv_label_set_text(tile_labels[i][j], buf);
            }
        }
    }
}

static bool move_tiles(int direction) {
    bool moved = false;
    int temp[GRID_SIZE][GRID_SIZE];
    memcpy(temp, grid, sizeof(grid));  // 使用memcpy优化数组复制

    if (direction == 0 || direction == 1) {  // 水平移动
        for (int i = 0; i < GRID_SIZE; i++) {
            int merged[GRID_SIZE] = {0};
            int *row = grid[i];  // 获取行指针，优化访问
            
            if (direction == 0) {  // 左移
                int write_pos = 0;
                for (int read_pos = 0; read_pos < GRID_SIZE; read_pos++) {
                    if (row[read_pos] == 0) continue;
                    
                    if (write_pos > 0 && !merged[write_pos-1] && 
                        row[write_pos-1] == row[read_pos]) {
                        // 合并相同数字
                        row[write_pos-1] *= 2;
                        current_score += row[write_pos-1];  // 在合并时立即添加得分
                        row[read_pos] = 0;
                        merged[write_pos-1] = 1;
                        moved = true;
                        update_score_display();  // 更新得分显示
                    } else if (read_pos != write_pos) {
                        // 移动到空位
                        row[write_pos] = row[read_pos];
                        if (read_pos != write_pos) {
                            row[read_pos] = 0;
                            moved = true;
                        }
                        write_pos++;
                    } else {
                        write_pos++;
                    }
                }
            } else {  // 右移
                int write_pos = GRID_SIZE - 1;
                for (int read_pos = GRID_SIZE - 1; read_pos >= 0; read_pos--) {
                    if (row[read_pos] == 0) continue;
                    
                    if (write_pos < GRID_SIZE-1 && !merged[write_pos+1] && 
                        row[write_pos+1] == row[read_pos]) {
                        // 合并相同数字
                        row[write_pos+1] *= 2;
                        current_score += row[write_pos+1];  // 在合并时立即添加得分
                        row[read_pos] = 0;
                        merged[write_pos+1] = 1;
                        moved = true;
                        update_score_display();  // 更新得分显示
                    } else if (read_pos != write_pos) {
                        // 移动到空位
                        row[write_pos] = row[read_pos];
                        if (read_pos != write_pos) {
                            row[read_pos] = 0;
                            moved = true;
                        }
                        write_pos--;
                    } else {
                        write_pos--;
                    }
                }
            }
        }
    } else {  // 垂直移动
        for (int j = 0; j < GRID_SIZE; j++) {
            int merged[GRID_SIZE] = {0};
            
            if (direction == 2) {  // 向上移动
                int write_pos = 0;
                for (int read_pos = 0; read_pos < GRID_SIZE; read_pos++) {
                    if (grid[read_pos][j] == 0) continue;
                    
                    if (write_pos > 0 && !merged[write_pos-1] && 
                        grid[write_pos-1][j] == grid[read_pos][j]) {
                        // 合并相同数字
                        grid[write_pos-1][j] *= 2;
                        current_score += grid[write_pos-1][j];  // 在合并时立即添加得分
                        grid[read_pos][j] = 0;
                        merged[write_pos-1] = 1;
                        moved = true;
                        update_score_display();  // 更新得分显示
                    } else if (read_pos != write_pos) {
                        // 移动到空位
                        grid[write_pos][j] = grid[read_pos][j];
                        if (read_pos != write_pos) {
                            grid[read_pos][j] = 0;
                            moved = true;
                        }
                        write_pos++;
                    } else {
                        write_pos++;
                    }
                }
            } else {  // 向下移动 (direction == 3)
                int write_pos = GRID_SIZE - 1;
                for (int read_pos = GRID_SIZE - 1; read_pos >= 0; read_pos--) {
                    if (grid[read_pos][j] == 0) continue;
                    
                    if (write_pos < GRID_SIZE-1 && !merged[write_pos+1] && 
                        grid[write_pos+1][j] == grid[read_pos][j]) {
                        // 合并相同数字
                        grid[write_pos+1][j] *= 2;
                        current_score += grid[write_pos+1][j];  // 在合并时立即添加得分
                        grid[read_pos][j] = 0;
                        merged[write_pos+1] = 1;
                        moved = true;
                        update_score_display();  // 更新得分显示
                    } else if (read_pos != write_pos) {
                        // 移动到空位
                        grid[write_pos][j] = grid[read_pos][j];
                        if (read_pos != write_pos) {
                            grid[read_pos][j] = 0;
                            moved = true;
                        }
                        write_pos--;
                    } else {
                        write_pos--;
                    }
                }
            }
        }
    }
    return moved;
}

static bool can_move() {
    // 检查是否有空格
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 0) return true;
        }
    }
    
    // 检查是否有相邻的相同数字
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (j < GRID_SIZE-1 && grid[i][j] == grid[i][j+1]) return true;
            if (i < GRID_SIZE-1 && grid[i][j] == grid[i+1][j]) return true;
        }
    }
    return false;
}

static void restart_game(lv_event_t *e) {
    lv_ui *ui = lv_event_get_user_data(e);
    if (ui == NULL) {
        return;
    }    
    // 更新排行榜
    update_leaderboard();
    // 重置当前得分
    current_score = 0;
    update_score_display();

    init_game();
    update_tiles(ui);
}

static void gesture_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = lv_event_get_user_data(e);

    if (code == LV_EVENT_GESTURE) {
        lv_dir_t gesture_dir = lv_indev_get_gesture_dir(lv_indev_get_act());

        switch (gesture_dir) {
            case LV_DIR_LEFT:  // 向左滑动
                if (move_tiles(0)) {  // 0 表示左移
                    add_random_tile();
                    update_tiles(ui);
                    if (!can_move()) {
                        update_leaderboard();
                        lv_obj_t *mbox = lv_msgbox_create(ui->game_2048, "Game Over", "No more moves available!", NULL, true);
                        if (mbox) {
                            lv_obj_center(mbox);
                        }
                    }
                }
                break;

            case LV_DIR_RIGHT:  // 向右滑动
                if (move_tiles(1)) {  // 1 表示右移
                    add_random_tile();
                    update_tiles(ui);
                    if (!can_move()) {
                        update_leaderboard();
                        lv_obj_t *mbox = lv_msgbox_create(ui->game_2048, "Game Over", "No more moves available!", NULL, true);
                        if (mbox) {
                            lv_obj_center(mbox);
                        }
                    }
                }
                break;

            case LV_DIR_TOP:  // 向上滑动
                if (move_tiles(2)) {  // 2 表示上移
                    add_random_tile();
                    update_tiles(ui);
                    if (!can_move()) {
                        update_leaderboard();
                        lv_obj_t *mbox = lv_msgbox_create(ui->game_2048, "Game Over", "No more moves available!", NULL, true);
                        if (mbox) {
                            lv_obj_center(mbox);
                        }
                    }
                }
                break;

            case LV_DIR_BOTTOM:  // 向下滑动
                if (move_tiles(3)) {  // 3 表示下移
                    add_random_tile();
                    update_tiles(ui);
                    if (!can_move()) {
                        update_leaderboard();
                        lv_obj_t *mbox = lv_msgbox_create(ui->game_2048, "Game Over", "No more moves available!", NULL, true);
                        if (mbox) {
                            lv_obj_center(mbox);
                        }
                    }
                }
                break;

            default:
                break;
        }
    }
}

static void game_2048_return_btn_event_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code)
	{
	case LV_EVENT_CLICKED:
	{
		lv_obj_t * act_scr = lv_scr_act();
		lv_disp_t * d = lv_obj_get_disp(act_scr);
		if (d->prev_scr == NULL && (d->scr_to_load == NULL || d->scr_to_load == act_scr))
		{
			lv_obj_clean(act_scr);
			if (guider_ui.menu_del == true)
				setup_scr_menu(&guider_ui);
			lv_scr_load_anim(guider_ui.menu, LV_SCR_LOAD_ANIM_OVER_RIGHT, 100, 100, true);
			guider_ui.game_2048_del = true;
		}
	}
		break;
	default:
		break;
	}
}

void setup_scr_2048(lv_ui *ui) {
    if (!ui) return;

    ui->game_2048 = lv_obj_create(NULL);
    if (!ui->game_2048) return;

    // 只设置背景色，不禁用滑动
    lv_obj_set_style_bg_color(ui->game_2048, lv_color_make(0xFF, 0xFF, 0xFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *grid_container = lv_obj_create(ui->game_2048);
    if (!grid_container) return;

    // 设置网格容器大小和位置
    lv_obj_set_size(grid_container, 450, 450);
    lv_obj_center(grid_container);
    lv_obj_set_layout(grid_container, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_all(grid_container, 8, 0);

    // 允许接收手势事件
    lv_obj_add_flag(grid_container, LV_OBJ_FLAG_GESTURE_BUBBLE);

    // 绑定手势事件
    lv_obj_add_event_cb(grid_container, gesture_event_cb, LV_EVENT_GESTURE, ui);

    // 网格描述数组
    static lv_coord_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    
    lv_obj_set_grid_dsc_array(grid_container, col_dsc, row_dsc);

    // 初始化标签数组
    memset(tile_labels, 0, sizeof(tile_labels));

    // 创建网格单元格
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            lv_obj_t *tile = lv_obj_create(grid_container);
            if (!tile) continue;

            lv_obj_set_grid_cell(tile, LV_GRID_ALIGN_STRETCH, j, 1, LV_GRID_ALIGN_STRETCH, i, 1);
            lv_obj_set_style_bg_color(tile, lv_color_make(0xCC, 0xCC, 0xCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(tile, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

            tile_labels[i][j] = lv_label_create(tile);
            if (!tile_labels[i][j]) {
                lv_obj_del(tile);
                continue;
            }

            lv_label_set_text(tile_labels[i][j], "");
            lv_obj_center(tile_labels[i][j]);
        }
    }

    // 创建重启按钮
    lv_obj_t *restart_btn = lv_btn_create(ui->game_2048);
    if (!restart_btn) return;

    lv_obj_set_size(restart_btn, 100, 40);
    lv_obj_align(restart_btn, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    lv_obj_t *label = lv_label_create(restart_btn);
    if (!label) return;

    lv_label_set_text(label, "Restart");
    lv_obj_center(label);
    lv_obj_add_event_cb(restart_btn, restart_game, LV_EVENT_CLICKED, ui);

    // 创建返回按钮
	ui->game_2048_return_btn = lv_imgbtn_create(ui->game_2048);
	lv_obj_set_pos(ui->game_2048_return_btn, 920, 45);
	lv_obj_set_size(ui->game_2048_return_btn, 50, 50);
	lv_obj_set_scrollbar_mode(ui->game_2048_return_btn, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_style_img_recolor(ui->game_2048_return_btn, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_imgbtn_set_src(ui->game_2048_return_btn, LV_IMGBTN_STATE_RELEASED, NULL, &_return_alpha_50x50, NULL);
	lv_obj_add_flag(ui->game_2048_return_btn, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(ui->game_2048_return_btn, game_2048_return_btn_event_handler, LV_EVENT_CLICKED, NULL);
	lv_obj_move_foreground(ui->game_2048_return_btn);  // 确保返回按钮在最前面

    // 创建得分显示
    score_label = lv_label_create(ui->game_2048);
    if (score_label) {
        lv_obj_align(score_label, LV_ALIGN_TOP_RIGHT, -160, 250);
        lv_obj_set_style_text_font(score_label, &lv_font_simsun_18, LV_PART_MAIN | LV_STATE_DEFAULT);
        update_score_display();
    }

    // 创建排行榜标题
    lv_obj_t *leaderboard_title = lv_label_create(ui->game_2048);
    if (leaderboard_title) {
        lv_label_set_text(leaderboard_title, "排行榜");
        lv_obj_align(leaderboard_title, LV_ALIGN_TOP_LEFT, 80, 100);
        lv_obj_set_style_text_font(leaderboard_title, &lv_font_simsun_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    // 创建排行榜项目
    for (int i = 0; i < MAX_LEADERBOARD_SIZE; i++) {
        leaderboard_labels[i] = lv_label_create(ui->game_2048);
        if (leaderboard_labels[i]) {
            lv_obj_align(leaderboard_labels[i], LV_ALIGN_TOP_LEFT, 80, 130 + i * 30);
            lv_obj_set_style_text_font(leaderboard_labels[i], &lv_font_simsun_18, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    update_leaderboard();

    // 初始化游戏
    init_game();
    update_tiles(ui);

    lv_obj_add_event_cb(ui->game_2048_return_btn, game_2048_return_btn_event_handler, LV_EVENT_ALL, ui);

}

