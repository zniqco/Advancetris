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

const background_data *background_image = NULL;
const background_data *background_image_next = NULL;
s16 background_brightness;
s16 background_brightness_latest;

void background_set_brightness(s16 brightness);

void IWRAM_CODE background_init() {
    background_image = NULL;
    background_image_next = NULL;
    background_brightness = 0;
    background_brightness_latest = 0;

    u16 *temp = (u16 *)MAP_BASE_ADR(4);
    u16 index = 168;

    for (s16 j = 0; j < 20; ++j) {
        for (s16 i = 0; i < 30; ++i)
            *(temp + j * 32 + i) = index++;
    }

    memory_fill32(PALETTE_BG(0), 0, 96 * 2);
    background_set_next(&backgrounds[0]);
}

void IWRAM_CODE background_set(const background_data *data) {
    background_image = data;

    memory_copy32(PATRAM8(1, 168), background_image->image, 30 * 20 * 64);
    background_set_brightness(background_brightness_latest);
}

void IWRAM_CODE background_set_next(const background_data *data) {
    if (background_image != data) {
        if (background_image != NULL)
            background_image_next = data;
        else
            background_set(data);
    }
}

void IWRAM_CODE background_set_brightness(s16 brightness) {
    brightness = clamp(brightness, 0, 100);

    if (background_brightness_latest != brightness) {
        palette_copy(PALETTE_BG(0), background_image->palette, 96, brightness - 100);

        background_brightness_latest = brightness;
    }
}

void IWRAM_CODE background_update() {
    if (background_image != NULL) {
        if (background_image_next != NULL) {
            background_brightness -= 8;

            if (background_brightness > 0) {
                background_set_brightness(background_brightness);
            } else {
                background_set(background_image_next);
                background_image_next = NULL;
                background_brightness = 0;
            }
        } else {
            background_brightness += 8;

            if (background_brightness > 100)
                background_brightness = 100;
            
            background_set_brightness(background_brightness);
        }
    }
}