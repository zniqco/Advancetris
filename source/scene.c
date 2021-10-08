#include "scene.h"

static scene_t current = {
	.init = nothing,
	.cleanup = nothing,
	.update = nothing,
};

static scene_t next = {
	.init = nothing,
	.cleanup = nothing,
	.update = nothing,
};

static bool is_changed = false;

void scene_set(scene_t scene) {
    next = scene;
    is_changed = true;
}

void scene_update() {
    if (is_changed) {
        current.cleanup();
        current = next;
        current.init();

        is_changed = false;
    }

    current.update();
}

void nothing() {
}