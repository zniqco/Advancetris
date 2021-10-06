#pragma once

#include "system.h"
#include "tetrimino.h"
#include "level.h"
#include "main.h"
#include "sprites/font.h"
#include "sprites/board.h"
#include "sprites/blocks.h"
#include "sprites/blocks8.h"
#include "sprites/next_pieces.h"
#include "sprites/texts.h"

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 28
#define BOARD_DRAW_X 85
#define BOARD_DRAW_Y 150
#define BOARD_VISIBLE_HEIGHT 20

#define TILE_POSITION_BLOCKS 32
#define TILE_POSITION_NEXT (TILE_POSITION_BLOCKS + 8)
#define TILE_POSITION_BOARD_BUFFER (TILE_POSITION_NEXT + 56)

#define ARR 1
#define DAS 12
#define DCD 2
#define SDF 40
#define LOCK_RESETABLE_FRAME(f) ((f) * 5 / 2) // LOCK_FRAME * 2.5

#define TEXTS_OFFSET 192
#define CLEAR_DISPLAY_FRAME 75

enum TSPIN {
    TSPIN_NONE,
    TSPIN_NORMAL,
    TSPIN_MINI,
};

enum LINES {
    LINES_SINGLE,
    LINES_DOUBLE,
    LINES_TRIPLE,
    LINES_TETRIS,
};

void ingame_init();
void ingame_cleanup();
void ingame_update();