#pragma once

#include "system.h"

typedef struct {
    void (*init)();
    void (*cleanup)();
    void (*update)();
} scene_t;

void scene_set(scene_t scene);
void scene_update();
void nothing();