#include "scene_menu.h"

static u16 x = 0;
static u16 y = 0;

static u16 current_index = 0;
static u32 current_selections = 0;
static u16 current_level = 0;

static u16 drawing_index = 0;
static u16 drawing_level = 0;
static u32 drawing_selections = 0;
static u16 drawing_count = 0;

static void menu_begin();
static bool menu_item(const char *caption);
static void menu_sub_begin();
static void menu_sub_end();
static void menu_end();

static void init() {
    current_index = 0;
    current_selections = 0;
    current_level = 0;

    map_clear(5);
}

static void update() {
    menu_begin();

    if (menu_item("START")) {
        scene_set(scene_ingame);
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

static void menu_begin() {
    x = 0;
    y = 0;
    drawing_index = 0;
    drawing_selections = 0;
    drawing_level = 0;
    drawing_count = 0;
}

static bool menu_item(const char *caption) {
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

static void menu_sub_begin() {
    drawing_selections = (drawing_selections << 4) | (drawing_index - 1);
    drawing_index = 0;

    ++drawing_level;
}

static void menu_sub_end() {
    drawing_index = (drawing_selections & 0x0F) + 1;
    drawing_selections = drawing_selections >> 4;

    --drawing_level;
}

static void menu_end() {
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

const scene_t scene_menu = {
    .init = init,
    .cleanup = nothing,
    .update = update,
};