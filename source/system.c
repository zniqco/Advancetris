#include "system.h"

u32 frame_count = 0;
u16 key_previous_held_state = 0;
u16 key_held_state = 0;
u16 key_down_state = 0;
u16 key_up_state = 0;
u32 random_seed = 0x12345678;
OBJATTR *current_object_pointer = OAM;

void IWRAM_CODE memory_fill32(void *target, u32 value, u32 size) {
    u16 fast_size = size & ~0x1F;
    u16 normal_size = size & 0x1F;

    *(u32 *)target = value;

    CpuFastSet(target, target, (fast_size / 4) | FILL);

    if (normal_size) {
        *(u32 *)(target + fast_size) = value;

        CpuSet(target + fast_size, target + fast_size, (normal_size / 4) | FILL | COPY32);
    }
}

void IWRAM_CODE memory_copy32(void *target, const void *source, u32 size) {
    u16 fast_size = size & ~0x1F;
    u16 normal_size = size & 0x1F;

    CpuFastSet(source, target, (fast_size / 4));

    if (normal_size)
        CpuSet(source + fast_size, target + fast_size, (normal_size / 4) | COPY32);
}

void IWRAM_CODE palette_copy(void *target, const void *source, u32 size, s16 brightness) {
    if (brightness == 0) {
        memory_copy32(target, source, size * 2);
    } else {
        u16 *current_source = (u16 *)source;
        u16 *current_target = target;

        brightness = clamp(brightness * (0x1F * 10) / 1000, -0x1F, 0x1F);

        for (s16 i = 0; i < size; ++i) {
            u16 color = *(current_source++);
            s16 r = clamp((color & 0x1F) + brightness, 0, 0x1F);
            s16 g = clamp(((color >> 5) & 0x1F) + brightness, 0, 0x1F);
            s16 b = clamp(((color >> 10) & 0x1F) + brightness, 0, 0x1F);

            *(current_target++) = RGB5(r, g, b);
        }
    }
}

void IWRAM_CODE input_update() {
    key_previous_held_state = key_held_state;
    key_held_state = REG_KEYINPUT ^ 0x3FF;

    u16 difference = key_previous_held_state ^ key_held_state;

    key_down_state = key_held_state & difference;
    key_up_state = key_previous_held_state & difference;
}

u16 IWRAM_CODE input_state() {
    return key_held_state;
}

u16 IWRAM_CODE input_is_held(u16 key) {
    return key_held_state & key;
}

u16 IWRAM_CODE input_is_down(u16 key) {
    return key_down_state & key;
}

u16 IWRAM_CODE input_is_up(u16 key) {
    return key_up_state & key;
}

void IWRAM_CODE map_clear(u16 index) {
    memory_fill32(MAP_BASE_ADR(index), 0, 2048);
}

void IWRAM_CODE put_text(void *target, u8 palette_id, const char *text) {
    u32 index = 0;
    u16 *temp = (u16 *)target;
    u16 palette_value = palette_id << 12;

    while (true) {
        switch (text[index]) {
            case 0:
                return;

            default:
                *temp++ = (text[index] - 32) | palette_value;

                break;
        }

        ++index;
    }
}

void IWRAM_CODE put_text_format(void *target, u8 palette_id, const char *format, ...) {
    char buffer[128];
    va_list args;

    va_start(args, format);
    vsiprintf(buffer, format, args);
    va_end(args);

    put_text(target, palette_id, buffer);
}

void IWRAM_CODE put_sprite_batch(void *target, u8 palette_id, const unsigned short *batch, u16 offset, u16 width, u16 height) {
    u16 index = 0;
    u16 *temp = (u16 *)target;
    u16 palette_value = palette_id << 12;

    for (u16 j = 0; j < height; ++j) {
        for (u16 i = 0; i < width; ++i)
            *(temp + j * 32 + i) = (offset + batch[index++]) | palette_value;
    }
}

void IWRAM_CODE object_reset() {
    memory_fill32(OAM, 0, (u32)current_object_pointer - (u32)OAM);

    current_object_pointer = OAM;
}

void IWRAM_CODE object_fetch(s16 x, s16 y, s16 character, u16 attr0, u16 attr1, u16 attr2) {
    OBJATTR *object = current_object_pointer++;

    object->attr0 = OBJ_Y(y) | attr0;
    object->attr1 = OBJ_X(x) | attr1;
    object->attr2 = OBJ_CHAR(character) | attr2;
}

void IWRAM_CODE sqran(u32 seed) {
    random_seed = seed;
}

s16 IWRAM_CODE qran() {
    random_seed = 1664525 * random_seed + 1013904223;

    return (random_seed >> 16) & 0x7FFF;
}

s32 IWRAM_CODE clamp(s32 value, s32 min, s32 max) {
    if (value < min)
        return min;
    
    if (value > max)
        return max;
    
    return value;
}

s32 IWRAM_CODE inverse_lerp(s32 value, s32 min, s32 max) {
    if (min == max)
        return 0;
    else
        return clamp(((value - min) * 100) / (max - min), 0, 100);
}

inline void profile_start() {
    REG_TM2CNT_H = 0;
    REG_TM3CNT_H = 0;
    REG_TM2CNT_L = 0;
    REG_TM3CNT_L = 0;
    REG_TM3CNT_H = 0x0080 /* TM_ENABLE */ | 0x0004 /* TM_CASCADE */;
    REG_TM2CNT_H = 0x0080 /* TM_ENABLE */;
}

inline u32 profile_stop() {
    REG_TM2CNT_H = 0;

    return (REG_TM3CNT_L << 16) | REG_TM2CNT_L;
}