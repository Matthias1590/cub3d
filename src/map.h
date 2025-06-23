#pragma once

#include "m.h"
#include "pool.h"
#include "tile.h"

typedef struct {
	tile_t *tiles;
	vec2i_t size;
} map_t;

map_t *map_from_str(pool_t *pool, const char *str);
