#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <gba.h>

#define PALETTE_BG(m) (BG_COLORS + ((m) << 4))
#define PALETTE_OBJ(m) (OBJ_COLORS + ((m) << 4))
#define MAP_POSITION_W32(m,x,y) ((u16 *)MAP_BASE_ADR(m) + (x) + (y) * 32)

void memory_fill32(void *target, u32 value, u32 size);
void memory_copy32(void *target, const void *source, u32 size);
void palette_copy(void *target, const void *source, u32 size, s16 brightness);
void input_update();
u16 input_state();
u16 input_is_held(u16 key);
u16 input_is_down(u16 key);
u16 input_is_up(u16 key);
void map_clear(u16 index);
void put_text(void *target, u8 palette_id, const char *text);
void put_text_format(void *target, u8 palette_id, const char *format, ...);
void put_sprite_batch(void *target, u8 palette_id, const unsigned short *batch, u16 offset, u16 width, u16 height);
void object_reset();
void object_fetch(s16 x, s16 y, s16 character, u16 attr0, u16 attr1, u16 attr2);
s32 clamp(s32 value, s32 min, s32 max);
s32 inverse_lerp(s32 value, s32 min, s32 max);
void profile_start();
u32 profile_stop();