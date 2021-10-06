#pragma once

#include <gba.h>
#include "background.h"

#define MARATHON_MAX_LEVEL 20
#define MARATHON_LINES_PER_LEVEL 10

extern const u32 marathon_level_gravity[20];
extern const u16 marathon_lock_delay[20];
extern const background_data *marathon_level_background[20];