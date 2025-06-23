#pragma once

#include "pool.h"
#include "scene.h"

typedef struct {
	pool_t *static_pool;
	scene_t *scene;
} state_t;

state_t *state_create(void);
void state_destroy(state_t *state);
