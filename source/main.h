#pragma once

#include "system.h"
#include "background.h"
#include "title.h"
#include "menu.h"
#include "ingame.h"

enum SCENE {
    SCENE_NONE,
    SCENE_TITLE,
    SCENE_MENU,
    SCENE_INGAME,
};

extern u16 current_scene;
extern u32 frame_count;