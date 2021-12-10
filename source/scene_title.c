#include "scene_title.h"

static void IWRAM_CODE init() {
    memory_copy32(PATRAM4(4, 64), TITLE_LEFT_TILES, TITLE_LEFT_TILES_LENGTH);
    memory_copy32(PALETTE_OBJ(0), TITLE_LEFT_PALETTE, TITLE_LEFT_PALETTE_LENGTH);

    memory_copy32(PATRAM4(4, 112), TITLE_RIGHT_TILES, TITLE_RIGHT_TILES_LENGTH);
    memory_copy32(PALETTE_OBJ(1), TITLE_RIGHT_PALETTE, TITLE_RIGHT_PALETTE_LENGTH);

    memory_copy32(PATRAM4(4, 136), TITLE_BACKGROUND_TILES, TITLE_BACKGROUND_TILES_LENGTH);
    memory_copy32(PALETTE_OBJ(2), TITLE_BACKGROUND_PALETTE, TITLE_BACKGROUND_PALETTE_LENGTH);
}

static void IWRAM_CODE update() {
    s16 x = 120;
    s16 y = 80;

    for (s16 i = 0; i < 6; ++i)
        object_fetch(x - 72 + i * 16, y - 16, 64 + i * 8, OBJ_16_COLOR | OBJ_TALL, OBJ_SIZE(2), OBJ_PALETTE(0));
    
    for (s16 i = 0; i < 3; ++i)
        object_fetch(x + 24 + i * 16, y - 16, 112 + i * 8, OBJ_16_COLOR | OBJ_TALL, OBJ_SIZE(2), OBJ_PALETTE(1));
    
    object_fetch(x - 48, y - 32, 136, OBJ_16_COLOR | OBJ_SQUARE, OBJ_SIZE(2), OBJ_PALETTE(2));
    object_fetch(x - 16, y - 32, 152, OBJ_16_COLOR | OBJ_SQUARE, OBJ_SIZE(2), OBJ_PALETTE(2));
    object_fetch(x + 16, y - 32, 168, OBJ_16_COLOR | OBJ_SQUARE, OBJ_SIZE(2), OBJ_PALETTE(2));
    object_fetch(x - 16, y, 184, OBJ_16_COLOR | OBJ_SQUARE, OBJ_SIZE(2), OBJ_PALETTE(2));

    object_fetch(x - 47, y - 31, 136, OBJ_16_COLOR | OBJ_SQUARE, OBJ_SIZE(2), OBJ_PALETTE(3));
    object_fetch(x - 15, y - 31, 152, OBJ_16_COLOR | OBJ_SQUARE, OBJ_SIZE(2), OBJ_PALETTE(3));
    object_fetch(x + 17, y - 31, 168, OBJ_16_COLOR | OBJ_SQUARE, OBJ_SIZE(2), OBJ_PALETTE(3));
    object_fetch(x - 15, y + 1, 184, OBJ_16_COLOR | OBJ_SQUARE, OBJ_SIZE(2), OBJ_PALETTE(3));

    if (input_is_down(KEY_A))
        scene_set(scene_ingame);
}

const scene_t scene_title = {
    .init = init,
    .cleanup = nothing,
    .update = update,
};