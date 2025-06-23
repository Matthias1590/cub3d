#include "m.h"
#include <math.h>

vec2i_t vec2i(int32_t x, int32_t y)
{
	return (vec2i_t){
		.x = x,
		.y = y,
	};
}

vec2f_t vec2f(float x, float y)
{
	return (vec2f_t){
		.x = x,
		.y = y,
	};
}

float vec2f_len(vec2f_t vec)
{
	return (sqrtf(vec.x * vec.x + vec.y * vec.y));
}

vec2f_t vec2f_neg(vec2f_t vec)
{
	return (vec2f_t){
		.x = -vec.x,
		.y = -vec.y,
	};
}

vec2f_t vec2f_normalize(vec2f_t vec)
{
	float len = vec2f_len(vec);
	if (len == 0.0f)
		return (vec);
	return (vec2f_scale(vec, 1.0f / len));
}

vec2f_t vec2f_scale(vec2f_t vec, float scalar)
{
	return (vec2f_t){
		.x = vec.x * scalar,
		.y = vec.y * scalar,
	};
}

vec2f_t vec2f_add(vec2f_t v1, vec2f_t v2)
{
	return (vec2f_t){
		.x = v1.x + v2.x,
		.y = v1.y + v2.y,
	};
}

vec2f_t vec2f_from_rot(rot_t rot)
{
	return (vec2f_t){
		.x = cosf(rot.rads),
		.y = sinf(rot.rads),
	};
}

vec2f_t vec2f_rot90(vec2f_t vec)
{
	return (vec2f_t){
		.x = vec.y,
		.y = -vec.x,
	};
}

rot_t rad(float value)
{
	return (rot_t){
		.rads = value,
	};
}

rot_t deg(float value)
{
	return rad(value * (M_PI / 180.0f));
}
