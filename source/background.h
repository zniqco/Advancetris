#pragma once

#include <gba.h>
#include "system.h"
#include "sprites/background_1.h"
#include "sprites/background_2.h"
#include "sprites/background_3.h"
#include "sprites/background_4.h"
#include "sprites/background_5.h"
#include "sprites/background_6.h"
#include "sprites/background_7.h"
#include "sprites/background_8.h"
#include "sprites/background_9.h"
#include "sprites/background_10.h"

typedef struct {
    const unsigned int *image;
    const unsigned short *palette;
} background_data;

extern const background_data backgrounds[10];

void background_init();
void background_set(const background_data *data);
void background_set_next(const background_data *data);
void background_update();