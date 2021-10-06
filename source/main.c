#include "main.h"

u16 previous_scene = SCENE_NONE;
u16 current_scene = SCENE_TITLE;
u32 frame_count = 0;

void IWRAM_CODE write_u32_by_object(u16 x, u16 y, u16 palette, u32 value) {
    while (true) {
        u32 current = DivMod(value, 10);

        object_fetch(x, y, current + 0x10, OBJ_16_COLOR | OBJ_SQUARE, OBJ_SIZE(0), OBJ_PALETTE(palette));

        if (value < 10)
            break;

        value = Div(value, 10);
        x -= 8;
    }
}

void init() {
    // IRQ
    irqInit();
    irqEnable(IRQ_VBLANK);

    // Registers
    REG_IME = 1;
    REG_DISPCNT = MODE_1 | OBJ_ON | BG0_ON | BG1_ON | OBJ_1D_MAP;

    // BG0: 8bpp Background
    BG_OFFSET[0].x = 0;
    BG_OFFSET[0].y = 0;
    BGCTRL[0] = CHAR_BASE(1) | SCREEN_BASE(4) | BG_PRIORITY(3) | BG_256_COLOR;

    // BG1: Text
    BG_OFFSET[1].x = 0;
    BG_OFFSET[1].y = 0;
    BGCTRL[1] = CHAR_BASE(1) | SCREEN_BASE(5) | BG_PRIORITY(2);

    // System font (BG)
    memory_copy32(PATRAM4(1, 0), FONT_TILES, FONT_TILES_LENGTH);
    memory_copy32(PALETTE_BG(14), FONT_PALETTE, FONT_PALETTE_LENGTH);

    // System font (OBJ)
    memory_copy32(PATRAM4(4, 0), FONT_TILES, FONT_TILES_LENGTH);
    memory_copy32(PALETTE_OBJ(14), FONT_PALETTE, FONT_PALETTE_LENGTH);

    // Init
    background_init();
}

int main() {
    init();

    // Debug
    u32 profile_max = 0;

    while (true) {
        VBlankIntrWait();

        // Debug
        profile_start();

        object_reset();
        input_update();
        background_update();

        // Scene
        if (previous_scene != current_scene) {
            switch (previous_scene) {
                case SCENE_TITLE:
                    break;

                case SCENE_MENU:
                    break;

                case SCENE_INGAME:
                    ingame_cleanup();
                    break;
            }

            switch (current_scene) {
                case SCENE_TITLE:
                    title_init();
                    break;

                case SCENE_MENU:
                    menu_init();
                    break;

                case SCENE_INGAME:
                    ingame_init();
                    break;
            }

            previous_scene = current_scene;
        }

        switch (current_scene) {
            case SCENE_TITLE:
                title_update();
                break;

            case SCENE_MENU:
                menu_update();
                break;

            case SCENE_INGAME:
                ingame_update();
                break;
        }

        frame_count = (frame_count + 1) & 0x7FFFFFFF;

        // -- Debug
        u32 profile_result = profile_stop();

        if (profile_max < profile_result)
            profile_max = profile_result;

        write_u32_by_object(232, 144, 14, profile_result);
        write_u32_by_object(232, 152, 14, profile_max);
        // -- Debug
    }
}
