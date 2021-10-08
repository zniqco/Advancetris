#include "background.h"

const background_data backgrounds[10] = {
    { BACKGROUND_1_TILES, BACKGROUND_1_PALETTE },
    { BACKGROUND_2_TILES, BACKGROUND_2_PALETTE },
    { BACKGROUND_3_TILES, BACKGROUND_3_PALETTE },
    { BACKGROUND_4_TILES, BACKGROUND_4_PALETTE },
    { BACKGROUND_5_TILES, BACKGROUND_5_PALETTE },
    { BACKGROUND_6_TILES, BACKGROUND_6_PALETTE },
    { BACKGROUND_7_TILES, BACKGROUND_7_PALETTE },
    { BACKGROUND_8_TILES, BACKGROUND_8_PALETTE },
    { BACKGROUND_9_TILES, BACKGROUND_9_PALETTE },
    { BACKGROUND_10_TILES, BACKGROUND_10_PALETTE },
};

static const background_data *current;
static const background_data *next;
static s16 brightness;
static s16 brightness_latest;
static s16 loading_progress;

static void set_current(const background_data *data);
static void set_brightness(s16 brightness);

void IWRAM_CODE background_init() {
    current = NULL;
    next = NULL;
    brightness = 0;
    brightness_latest = 0;
    loading_progress = 0;

    u16 *temp = (u16 *)MAP_BASE_ADR(4);
    u16 index = 168;

    for (s16 j = 0; j < 20; ++j) {
        for (s16 i = 0; i < 30; ++i)
            *(temp + j * 32 + i) = index++;
    }

    memory_fill32(PALETTE_BG(0), 0, 96 * 2);
    background_set(&backgrounds[0]);
}

void IWRAM_CODE background_set(const background_data *data) {
    if (current != data) {
        if (current != NULL)
            next = data;
        else
            set_current(data);
    }
}

void IWRAM_CODE background_update() {
    if (current != NULL) {
        if (next != NULL) {
            brightness -= 8;

            if (brightness > 0) {
                set_brightness(brightness);
            } else {
                set_current(next);
                next = NULL;
                brightness = 0;
            }
        } else {
            if (loading_progress < BACKGROUND_LOADING_FRAMES) {
                u16 offset = loading_progress++ * (30 * 20 / BACKGROUND_LOADING_FRAMES);
                
                memory_copy32(PATRAM8(1, 168 + offset), current->image + offset * 16, (30 * 20 / BACKGROUND_LOADING_FRAMES) * 64);
            } else {
                brightness += 8;

                if (brightness > 100)
                    brightness = 100;
                
                set_brightness(brightness);
            }
        }
    }
}

static void IWRAM_CODE set_current(const background_data *data) {
    current = data;
    loading_progress = 0;

    set_brightness(brightness_latest);
}

static void IWRAM_CODE set_brightness(s16 brightness) {
    brightness = clamp(brightness, 0, 100);

    if (brightness_latest != brightness) {
        palette_copy(PALETTE_BG(0), current->palette, 96, brightness - 100);

        brightness_latest = brightness;
    }
}