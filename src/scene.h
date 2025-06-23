#pragma once

#include "pool.h"
#include "player.h"
#include "map.h"

typedef struct {
	player_t player;
	map_t *map;
	uint32_t floor_color;
	uint32_t ceiling_color;
} scene_t;

scene_t *scene_from_file(pool_t *pool, const char *path);
