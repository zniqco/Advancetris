#include "scene_ingame.h"

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 28
#define BOARD_DRAW_X 85
#define BOARD_DRAW_Y 150
#define BOARD_VISIBLE_HEIGHT 20

#define ARR 1
#define DAS 12
#define DCD 2
#define SDF 40
#define LOCK_RESETABLE_FRAME(f) ((f) * 2 + 12) // LOCK_FRAME * 2 + 12

#define BLOCKS_OFFSET 32
#define NEXT_PIECES_OFFSET (BLOCKS_OFFSET + 10)
#define TEXTS_OFFSET 200
#define TEXTS_CLEAR_OFFSET (TEXTS_OFFSET + 4)
#define CLEAR_DISPLAY_FRAME 75

typedef enum {
    STATE_READY,
    STATE_PLAYING,
    STATE_END,
} STATE;

typedef enum {
    TSPIN_NONE,
    TSPIN_NORMAL,
    TSPIN_MINI,
} TSPIN;

static STATE state;
static s16 ready_frame;

static u8 board[BOARD_HEIGHT][BOARD_WIDTH];

static s8 next_blocks[16];
static u8 next_index;
static u8 next_count;

static u16 level;
static u32 score;
static u32 lines;
static u16 combo;
static u16 b2b;
static s8 hold;
static bool holdable;

static s8 tetrimino;
static u8 tetrimino_rotation;
static s8 tetrimino_x;
static s8 tetrimino_y;
static u8 tspin_state;

static u32 gravity; // 1/10000 scale
static u32 gravity_tick;
static s16 lock_frame_max;
static s16 lock_frame;
static s16 lock_resetable_y;
static s16 lock_resetable_frame;

static s16 softdrop_delay;
static s8 arr_direction;
static s16 arr_delay;

static s16 clear_remain_frame;
static TSPIN clear_tspin;
static s8 clear_lines;
static u16 clear_combo;
static bool clear_is_b2b;
static bool clear_is_all_clear;

static void set_level(u16 level);
static void set_tetrimino(s8 index);
static void set_tetrimino_next();
static bool test_tetrimino(s8 offset_x, s8 offset_y);
static void place_tetrimino();
static void rotate_tetrimino(bool is_ccw);
static u16 drop_offset_tetrimino();
static void set_block(s16 x, s16 y, u8 index);
static bool test_block(s16 x, s16 y);
static void write_u32_by_object(u16 x, u16 y, u16 palette, u32 value);
static void game_over();

static void IWRAM_CODE init() {
    REG_DISPCNT = MODE_1 | OBJ_ON | BG0_ON | BG1_ON | BG2_ON | OBJ_1D_MAP;

    // HACK: Align 8x8 to 7x7 for blocks
    REG_BG2PA = 0x0125;
    REG_BG2PD = 0x0125;
    REG_BG2X = 0x079B;
    REG_BG2Y = 0x04BC;

    // BG1: Ingame & Stats
    BG_OFFSET[1].x = 0;
    BG_OFFSET[1].y = 9;
    BGCTRL[1] = CHAR_BASE(1) | SCREEN_BASE(5) | BG_PRIORITY(2);

    // BG2: Blocks
    BG_OFFSET[2].x = 0;
    BG_OFFSET[2].y = 0;
    BGCTRL[2] = CHAR_BASE(0) | SCREEN_BASE(6) | BG_PRIORITY(1) | BG_SIZE_1;

    // Board
    memory_copy32(PATRAM4(1, 64), BOARD_TILES, BOARD_TILES_LENGTH);
    memory_copy32(PALETTE_BG(13), BOARD_PALETTE, BOARD_PALETTE_LENGTH);

    map_clear(5);
    put_sprite_batch(MAP_POSITION_W32(5, 6, 2), 13, BOARD_MAP, 64, 18, 19);

    // Block and next
    memory_copy32(PATRAM8(4, BLOCKS_OFFSET), BLOCKS_TILES, BLOCKS_TILES_LENGTH);
    memory_copy32(PATRAM8(4, NEXT_PIECES_OFFSET), NEXT_PIECES_TILES, NEXT_PIECES_TILES_LENGTH);
    memory_copy32(PALETTE_OBJ(8), BLOCKS_PALETTE, BLOCKS_PALETTE_LENGTH);

    // 8px Block (for Hack)
    memory_copy32(PATRAM8(0, 1), BLOCKS8_TILES, BLOCKS8_TILES_LENGTH);
    memory_copy32(PALETTE_BG(8), BLOCKS_PALETTE, BLOCKS_PALETTE_LENGTH);

    // Clear texts
    memory_copy32(PATRAM4(4, TEXTS_OFFSET), TEXTS_TILES, TEXTS_TILES_LENGTH);
    memory_copy32(PATRAM4(4, TEXTS_CLEAR_OFFSET), TEXTS_CLEAR_TILES, TEXTS_CLEAR_TILES_LENGTH);

    palette_copy(PALETTE_OBJ(7), TEXTS_PALETTE, 16, 0);

    // Transparent for ghost
    REG_BLDCNT = 0x2F40; // 101111 01 000000;
    REG_BLDALPHA = 0x2006; // 100000 00 000110;

    // Board and tile layer
    memory_fill32(board, 0, BOARD_WIDTH * BOARD_HEIGHT);
    memory_fill32(MAP_BASE_ADR(6), 0, 32 * 32);

    // Level setting
    set_level(1);

    background_set(marathon_level_background[level - 1]);

    // Stats
    state = STATE_READY;
    ready_frame = 0;

    next_index = 0;
    next_count = 0;    
    score = 0;
    lines = 0;
    combo = 0;
    b2b = 0;
    hold = -1;
    holdable = true;

    sqran(frame_count);
    set_tetrimino_next();
}

static void IWRAM_CODE cleanup() {
    map_clear(5);
    map_clear(6);
}

static void IWRAM_CODE update() {
    switch (state) {
        case STATE_READY:
            object_fetch(105, 77, TEXTS_OFFSET, OBJ_16_COLOR | OBJ_WIDE, OBJ_SIZE(1), OBJ_PALETTE(7));

            if (++ready_frame >= 90)
                state = STATE_PLAYING;

            break;

        case STATE_PLAYING:
            // Hard drop
            if (input_is_down(KEY_UP)) {
                u16 drop_offset = drop_offset_tetrimino();

                tetrimino_y -= drop_offset;
                score += drop_offset;

                place_tetrimino();
            }

            // Move
            s8 pressed = 0;

            if (input_is_down(KEY_LEFT))
                pressed = -1;
            else if (input_is_down(KEY_RIGHT))
                pressed = 1;

            if (pressed != 0) {
                if (test_tetrimino(pressed, 0)) {
                    tetrimino_x += pressed;
                    lock_frame = lock_frame_max;
                }
                    
                if (arr_direction != pressed) {
                    arr_direction = pressed;
                    arr_delay = DAS;
                }
            }

            if ((input_is_up(KEY_LEFT) && arr_direction == -1) || (input_is_up(KEY_RIGHT) && arr_direction == 1))
                arr_direction = 0;

            if (arr_direction != 0) {
                if (--arr_delay <= 0) {
                    if (test_tetrimino(arr_direction, 0)) {
                        tetrimino_x += arr_direction;
                        lock_frame = lock_frame_max;
                    }
                    
                    arr_delay = ARR;
                }
            }

            // Soft drop
            if (input_is_held(KEY_DOWN)) {
                if (--softdrop_delay <= 0) {
                    if (test_tetrimino(0, -1)) {
                        --tetrimino_y;
                        ++score;

                        softdrop_delay = 1;
                    }
                }
            } else {
                softdrop_delay = 0;
            }

            // Rotate
            if (input_is_down(KEY_A))
                rotate_tetrimino(true);

            if (input_is_down(KEY_B))
                rotate_tetrimino(false);
            
            if (holdable && input_is_down(KEY_L)) {
                if (hold >= 0) {
                    s8 temp = hold;

                    hold = tetrimino;
                    set_tetrimino(temp);
                } else {
                    hold = tetrimino;
                    set_tetrimino_next();
                }

                holdable = false;
            }

            // Gravity
            gravity_tick += gravity;

            while (gravity_tick >= 10000) {
                if (test_tetrimino(0, -1)) {
                    --tetrimino_y;
                    gravity_tick -= 10000;
                } else {
                    gravity_tick = 0;
                }
            }

            // Lock
            u16 drop_offset = drop_offset_tetrimino();

            if (drop_offset == 0) {
                if (lock_resetable_y > tetrimino_y) {
                    lock_resetable_y = tetrimino_y;
                    lock_resetable_frame = LOCK_RESETABLE_FRAME(lock_frame_max);
                }

                --lock_resetable_frame;
                --lock_frame;

                if (lock_frame <= 0 || lock_resetable_frame <= 0)
                    place_tetrimino();
            } else {
                --lock_resetable_frame;
            }

            // Current block
            if (tetrimino >= 0) {
                for (s16 j = 0; j < 4; ++j) {
                    for (s16 i = 0; i < 4; ++i) {
                        u8 current = tetrimino_shape[tetrimino][tetrimino_rotation][((3 - j) * 4) + i];

                        if (current != 0) {                
                            s16 x = BOARD_DRAW_X + (tetrimino_x + i) * 7;
                            s16 y = BOARD_DRAW_Y - (tetrimino_y + j + 1) * 7;
                            s16 character = (BLOCKS_OFFSET + current - 1) * 2;

                            // Normal
                            object_fetch(x, y, character, OBJ_256_COLOR | OBJ_SQUARE, OBJ_SIZE(0), 0);

                            // Ghost
                            if (drop_offset != 0 && tetrimino_y + j - drop_offset < BOARD_VISIBLE_HEIGHT)
                                object_fetch(x, y + drop_offset * 7, character, OBJ_256_COLOR | OBJ_SQUARE | OBJ_TRANSLUCENT, OBJ_SIZE(0), 0);
                        }
                    }
                }
            }
            break;

        case STATE_END:
            break;
    }

    // Next
    for (s16 i = 0; i < 5; ++i) {
        s16 next = (NEXT_PIECES_OFFSET + next_blocks[(next_index + i) & 0x0F] * 8) * 2;

        object_fetch(165, 21 + i * 22, next, OBJ_256_COLOR | OBJ_WIDE, OBJ_SIZE(2), 0);
    }

    // Hold
    if (hold >= 0)
        object_fetch(51, 21, (NEXT_PIECES_OFFSET + hold * 8) * 2, OBJ_256_COLOR | OBJ_WIDE | (holdable ? 0 : OBJ_TRANSLUCENT), OBJ_SIZE(2), 0);

    // Clear state
    if (clear_remain_frame > 0) {
        s16 clear_current_frame = CLEAR_DISPLAY_FRAME - clear_remain_frame;
        s16 offset_x_delta = 24 - clamp(clear_current_frame, 0, 24);
        s16 offset_x = (offset_x_delta * offset_x_delta * offset_x_delta * offset_x_delta) / 32000; // Ease out quartic?
        s16 y = 149;

        palette_copy(PALETTE_OBJ(7), TEXTS_PALETTE, 16, inverse_lerp(clear_current_frame, 18, 0));

        if (clear_combo >= 2) {
            object_fetch(46 - offset_x, y, TEXTS_CLEAR_OFFSET + 32, OBJ_16_COLOR | OBJ_WIDE, OBJ_SIZE(1), OBJ_PALETTE(7));

            u32 value = clear_combo;
            u32 x = 43 - offset_x;

            while (true) {
                u32 current = DivMod(value, 10);

                object_fetch(x, y, TEXTS_CLEAR_OFFSET + 36 + current, OBJ_16_COLOR | OBJ_SQUARE, OBJ_SIZE(0), OBJ_PALETTE(7));

                if (value < 10)
                    break;

                value = Div(value, 10);
                x -= 5;
            }

            y -= 6;
        }

        if (clear_lines >= 1) {
            object_fetch(46 - offset_x, y, TEXTS_CLEAR_OFFSET + (clear_lines - 1) * 4, OBJ_16_COLOR | OBJ_WIDE, OBJ_SIZE(1), OBJ_PALETTE(7));

            y -= 6;
        }

        if (clear_tspin == TSPIN_NORMAL) {
            object_fetch(46 - offset_x, y, TEXTS_CLEAR_OFFSET + 16, OBJ_16_COLOR | OBJ_WIDE, OBJ_SIZE(1), OBJ_PALETTE(7));

            y -= 6;
        } else if (clear_tspin == TSPIN_MINI) {
            object_fetch(24 - offset_x, y, TEXTS_CLEAR_OFFSET + 16, OBJ_16_COLOR | OBJ_WIDE, OBJ_SIZE(1), OBJ_PALETTE(7));
            object_fetch(46 - offset_x, y, TEXTS_CLEAR_OFFSET + 20, OBJ_16_COLOR | OBJ_WIDE, OBJ_SIZE(1), OBJ_PALETTE(7));

            y -= 6;
        }

        if (clear_is_b2b)
            object_fetch(62 - offset_x, y, TEXTS_CLEAR_OFFSET + 24, OBJ_16_COLOR | OBJ_WIDE, OBJ_SIZE(0), OBJ_PALETTE(7));

        if (clear_is_all_clear) {
            object_fetch(163 + offset_x, 149, TEXTS_CLEAR_OFFSET + 26, OBJ_16_COLOR | OBJ_WIDE, OBJ_SIZE(1), OBJ_PALETTE(7));
            object_fetch(195 + offset_x, 149, TEXTS_CLEAR_OFFSET + 30, OBJ_16_COLOR | OBJ_WIDE, OBJ_SIZE(0), OBJ_PALETTE(7));
        }

        --clear_remain_frame;
    }

    // Stats
    write_u32_by_object(70, 63, 14, score);
    write_u32_by_object(70, 63 + 24, 14, lines);
    write_u32_by_object(70, 63 + 48, 14, level);
}

static void IWRAM_CODE set_level(u16 value) {
    level = value;

    gravity = marathon_level_gravity[value - 1];
    lock_frame_max = marathon_lock_delay[value - 1];

    background_set(marathon_level_background[value - 1]);
}

static void IWRAM_CODE set_tetrimino(s8 index) {
    tetrimino = index;
    tetrimino_rotation = 0;
    tetrimino_x = 3;
    tetrimino_y = 18;

    tspin_state = TSPIN_NONE;
    gravity_tick = 0;
    lock_frame = lock_frame_max;
    lock_resetable_y = BOARD_HEIGHT;
    lock_resetable_frame = 0;
}

static void IWRAM_CODE set_tetrimino_next() {
    while (next_count <= 5) {
        s8 blocks[7];

        for (s16 i = 0; i < 7; ++i)
            blocks[i] = i;
        
        for (s16 i = 6; i > 0; i--) {
            u16 j = DivMod(qran(), i + 1);
            s8 temp = blocks[i];

            blocks[i] = blocks[j];
            blocks[j] = temp;
        }

        for (s16 i = 0; i < 7; ++i)
            next_blocks[(next_index + next_count++) & 0x0F] = blocks[i];
    }

    set_tetrimino(next_blocks[next_index]);

    next_index = (next_index + 1) & 0x0F;
    --next_count;
}

static bool IWRAM_CODE test_tetrimino(s8 offset_x, s8 offset_y) {
    for (s16 j = 0; j < 4; ++j) {
        for (s16 i = 0; i < 4; ++i) {
            u8 current = tetrimino_shape[tetrimino][tetrimino_rotation][((3 - j) * 4) + i];

            if (current != 0) {
                s16 x = tetrimino_x + offset_x + i;
                s16 y = tetrimino_y + offset_y + j;

                if (!test_block(x, y))
                    return false;
            }
        }
    }

    return true;
}

static void IWRAM_CODE place_tetrimino() {
    // Set blocks
    u16 highest_y = 0;

    for (s16 j = 0; j < 4; ++j) {
        for (s16 i = 0; i < 4; ++i) {
            u8 current = tetrimino_shape[tetrimino][tetrimino_rotation][((3 - j) * 4) + i];

            if (current != 0) {
                s16 x = tetrimino_x + i;
                s16 y = tetrimino_y + j;

                if (x < 0 || y < 0 || x >= BOARD_WIDTH)
                    continue;

                if (y < BOARD_HEIGHT)
                    set_block(x, y, current);

                if (highest_y < y)
                    highest_y = y;
            }
        }
    }

    // Check line clear
    u16 cleared = 0;
    bool all_clear = true;

    for (s16 j = 0; j < BOARD_HEIGHT; ++j) {
        bool is_cleared = true;
        bool is_blank_line = true;

        for (s16 i = 0; i < BOARD_WIDTH; ++i) {
            if (board[j][i] == 0)
                is_cleared = false;
            else
                is_blank_line = false;
        }

        if (is_cleared) {
            ++cleared;
        } else {
            if (!is_blank_line)
                all_clear = false;

            if (cleared > 0) {
                for (s16 i = 0; i < BOARD_WIDTH; ++i)
                    set_block(i, j - cleared, board[j][i]);

                if (j >= BOARD_HEIGHT - cleared) {
                    for (s16 i = 0; i < BOARD_WIDTH; ++i) 
                        set_block(i, j, 0);
                }
            }
        }
    }

    // Calculate points
    bool b2b_able = false;
    u32 points = 0;

    switch (tspin_state) {
        case TSPIN_NONE:
            switch (cleared) {
                case 1: points = (all_clear ? 800 : 100) * level; break;
                case 2: points = (all_clear ? 1200 : 300) * level; break;
                case 3: points = (all_clear ? 1800 : 500) * level; break;
                case 4: points = (all_clear ? 2000 : 800) * level; b2b_able = true; break;
            }

            break;

        case TSPIN_NORMAL:
            switch (cleared) {
                case 0: points = 400 * level; break;
                case 1: points = 800 * level; b2b_able = true; break;
                case 2: points = 1200 * level; b2b_able = true; break;
                case 3: points = 1600 * level; b2b_able = true; break;
            }

            break;

        case TSPIN_MINI:
            switch (cleared) {
                case 0: points = 100 * level; break;
                case 1: points = 200 * level; b2b_able = true; break;
                case 2: points = 400 * level; b2b_able = true; break;
            }

            break;
    }

    // Combo and B2B
    if (cleared > 0) {
        ++combo;

        if (b2b_able)
            ++b2b;
        else
            b2b = 0;

        if (b2b > 1)
            points = points * 3 / 2;
    } else {
        combo = 0;
    }

    // Clear
    if (cleared > 0 || tspin_state != TSPIN_NONE) {
        clear_lines = cleared;
        clear_combo = combo;
        clear_is_b2b = b2b_able && (b2b > 1);
        clear_tspin = tspin_state;
        clear_is_all_clear = all_clear;

        clear_remain_frame = CLEAR_DISPLAY_FRAME;
    }

    // Add score
    points += 50 * combo * level;
    score += points;

    // Lines
    lines += cleared;

    // States
    arr_delay = DCD;
    holdable = true;

    // Check placeable
    bool is_placeable = true;

    for (u16 j = 20; j < 22; ++j) {
        for (u16 i = BOARD_WIDTH / 2 - 2; i < BOARD_WIDTH / 2 + 2; ++i) {
            if (board[j][i] != 0) {
                is_placeable = false;
                break;
            }
        }

        if (!is_placeable)
            break;
    }

    if (highest_y >= 22 || !is_placeable) {
        game_over();
    } else {
        // Level
        u16 target_level = (lines / MARATHON_LINES_PER_LEVEL) + 1;

        if (target_level > MARATHON_MAX_LEVEL)
            target_level = MARATHON_MAX_LEVEL;

        if (target_level > level)
            set_level(target_level);

        set_tetrimino_next();
    }
}

static u16 IWRAM_CODE drop_offset_tetrimino() {
    u16 min_offset = BOARD_HEIGHT;

    for (s16 i = 0; i < 4; ++i) {
        for (s16 j = 0; j < 4; ++j) {
            u8 current = tetrimino_shape[tetrimino][tetrimino_rotation][((3 - j) * 4) + i];

            if (current != 0) {
                s16 x = tetrimino_x + i;

                if (x < 0 || x >= BOARD_WIDTH)
                    break;

                s16 offset = 0;

                for (s16 y = tetrimino_y + j - 1; y >= 0; --y) {
                    if (board[y][x] == 0)
                        ++offset;
                    else
                        break;
                }

                if (min_offset > offset)
                    min_offset = offset;

                break;
            }
        }
    }

    return min_offset;
}

static void IWRAM_CODE rotate_tetrimino(bool is_clockwise) {
    u8 previous_rotation = tetrimino_rotation;
    s16 kick_base_index;

    if (is_clockwise) {
        if (tetrimino_rotation == 3)
            tetrimino_rotation = 0;
        else
            ++tetrimino_rotation;
        
        kick_base_index = previous_rotation * 10;
    } else {
        if (tetrimino_rotation == 0)
            tetrimino_rotation = 3;
        else
            --tetrimino_rotation;
        
        kick_base_index = tetrimino_rotation * 10 + 5;
    }

    bool success = false;

    for (s16 i = 0; i < 5; ++i) {
        s16 kick_index = kick_base_index + i;
        s8 kick_x = tetrimino_kick[tetrimino][kick_index][0];
        s8 kick_y = tetrimino_kick[tetrimino][kick_index][1];

        if (test_tetrimino(kick_x, kick_y)) {
            tetrimino_x += kick_x;
            tetrimino_y += kick_y;
            success = true;

            break;
        }
    }

    if (!success) {
        tetrimino_rotation = previous_rotation;
    } else {
        lock_frame = lock_frame_max;

        // T-spin
        if (tetrimino == 6) {
            u8 corners[4] = {
                test_block(tetrimino_x, tetrimino_y + 3) ? 0 : 1, // LU
                test_block(tetrimino_x + 2, tetrimino_y + 3) ? 0 : 1, // RU
                test_block(tetrimino_x + 2, tetrimino_y + 1) ? 0 : 1, // RD
                test_block(tetrimino_x, tetrimino_y + 1) ? 0 : 1, // LD
            };

            if (corners[0] + corners[1] + corners[2] + corners[3] >= 3) {
                s16 front0 = (0 + tetrimino_rotation) & 0x03;
                s16 front1 = (1 + tetrimino_rotation) & 0x03;

                if (corners[front0] && corners[front1])
                    tspin_state = TSPIN_NORMAL;
                else
                    tspin_state = TSPIN_MINI;
            } else {
                tspin_state = TSPIN_NONE;
            }            
        }
    }
}

static void IWRAM_CODE set_block(s16 x, s16 y, u8 index) {
    if (board[y][x] != index) {
        board[y][x] = index;

        if (y < 22) {
            s16 grid_x = 13 + x;
            s16 grid_y = 21 - y;

            u8 *pointer = (u8 *)MAP_BASE_ADR(6) + (grid_y * 32 + grid_x);
            u16 *aligned_pointer = (u16 *)((u32)pointer & ~0x01);
            s16 offset = ((u32)pointer & 0x01) * 8;

            *aligned_pointer = (*aligned_pointer & ~(0xFF << offset)) | (index << offset);
        }
    }
}

static bool IWRAM_CODE test_block(s16 x, s16 y) {
    if (x < 0 || x >= BOARD_WIDTH || y < 0)
        return false;

    if (y < BOARD_HEIGHT && board[y][x] != 0)
        return false;
    
    return true;
}

static void IWRAM_CODE game_over() {
    for (s16 j = 0; j < BOARD_HEIGHT; ++j) {
        for (s16 i = 0; i < BOARD_WIDTH; ++i) {
            if (board[j][i] != 0)
                set_block(i, j, 9);
        }
    }

    state = STATE_END;
}

static void IWRAM_CODE write_u32_by_object(u16 x, u16 y, u16 palette, u32 value) {
    while (true) {
        u32 current = DivMod(value, 10);

        object_fetch(x, y, current + 0x10, OBJ_16_COLOR | OBJ_SQUARE, OBJ_SIZE(0), OBJ_PALETTE(palette));

        if (value < 10)
            break;

        value = Div(value, 10);
        x -= 8;
    }
}

const scene_t scene_ingame = {
	.init = init,
    .cleanup = cleanup,
	.update = update,
};