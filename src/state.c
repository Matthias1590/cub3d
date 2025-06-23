#include "state.h"
#include "pool.h"
#include <stdlib.h>

state_t *state_create(void)
{
	state_t *state = malloc(sizeof(state_t));
	if (!state)
		return (NULL);

	state->static_pool = pool_create();

	return (state);
}

void state_destroy(state_t *state)
{
	pool_destroy(state->static_pool);

	free(state);
}
