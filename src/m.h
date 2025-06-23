#pragma once

#include <stdint.h>

typedef struct {
	float rads;
} rot_t;

rot_t rad(float value);
rot_t deg(float value);

typedef struct {
	int32_t x, y;
} vec2i_t;

vec2i_t vec2i(int32_t x, int32_t y);

typedef struct {
	float x, y;
} vec2f_t;

vec2f_t vec2f(float x, float y);
float vec2f_len(vec2f_t vec);
vec2f_t vec2f_neg(vec2f_t vec);
vec2f_t vec2f_normalize(vec2f_t vec);
vec2f_t vec2f_scale(vec2f_t vec, float scalar);
vec2f_t vec2f_add(vec2f_t v1, vec2f_t v2);
vec2f_t vec2f_from_rot(rot_t rot);
vec2f_t vec2f_rot90(vec2f_t vec);
