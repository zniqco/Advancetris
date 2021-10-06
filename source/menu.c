#include "menu.h"

u16 x = 0;
u16 y = 0;

u16 current_index = 0;
u32 current_selections = 0;
u16 current_level = 0;

u16 drawing_index = 0;
u16 drawing_level = 0;
u32 drawing_selections = 0;
u16 drawing_count = 0;

void menu_begin();
bool menu_item(const char *caption);
void menu_sub_begin();
void menu_sub_end();
void menu_end();

void menu_init() {
    current_index = 0;
    current_selections = 0;
    current_level = 0;

    map_clear(5);
}

void menu_update() {
    menu_begin();

    if (menu_item("START")) {
        current_scene = SCENE_INGAME;
    }

    if (menu_item("OPTION")) {
        menu_sub_begin();

        if (menu_item("TEST 1")) {

        }

        if (menu_item("TEST 2")) {

        }

        menu_sub_end();
    }

    menu_end();
}

void menu_begin() {
    x = 0;
    y = 0;
    drawing_index = 0;
    drawing_selections = 0;
    drawing_level = 0;
    drawing_count = 0;
}

bool menu_item(const char *caption) {
    u16 index = drawing_index++;

    if (current_level == drawing_level) {
        put_text(MAP_POSITION_W32(5, x + 2, y + 10), (current_index == index) ? 14 : 15, caption);

        ++y;
        ++drawing_count;
    } else if (((current_selections >> ((current_level - drawing_level - 1) * 4)) & 0x0F) == index) {
        return true;
    }

    return false;
}

void menu_sub_begin() {
    drawing_selections = (drawing_selections << 4) | (drawing_index - 1);
    drawing_index = 0;

    ++drawing_level;
}

void menu_sub_end() {
    drawing_index = (drawing_selections & 0x0F) + 1;
    drawing_selections = drawing_selections >> 4;

    --drawing_level;
}

void menu_end() {
    BG_OFFSET[1].y = y * 4;

    if (input_is_down(KEY_A)) {
        if (drawing_count > 0) {
            current_selections = (current_selections << 4) | current_index;
            current_index = 0;

            ++current_level;
        }
    } else if (input_is_down(KEY_B)) {
        if (current_level > 0) {
            current_index = current_selections & 0x0F;
            current_selections = current_selections >> 4;

            --current_level;
        }
    } else if (input_is_down(KEY_UP)) {
        if (current_index > 0)
            --current_index;
    } else if (input_is_down(KEY_DOWN)) {
        if (current_index < drawing_count - 1)
            ++current_index;
    }
}