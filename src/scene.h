#pragma once

#include "pool.h"
#include "player.h"
#include "map.h"

typedef struct {
	player_t player;
	map_t *map;
} scene_t;

scene_t *scene_from_file(pool_t *pool, const char *path);
