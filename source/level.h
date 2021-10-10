#pragma once

#include <gba.h>
#include "background.h"

#define MARATHON_MAX_LEVEL 20
#define MARATHON_LINES_PER_LEVEL 10

extern const u32 marathon_level_gravity[MARATHON_MAX_LEVEL];
extern const u16 marathon_lock_delay[MARATHON_MAX_LEVEL];
extern const background_data *marathon_level_background[MARATHON_MAX_LEVEL];