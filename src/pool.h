#pragma once

#include <stddef.h>

typedef struct {
	void **ptrs;
	size_t ptrs_count;
	size_t ptrs_capacity;
	int *fds;
	size_t fds_count;
	size_t fds_capacity;
} pool_t;

pool_t *pool_create(void);
void pool_destroy(pool_t *pool);

void *pool_alloc(pool_t *pool, size_t size);
int pool_open(pool_t *pool, const char *path, int flags);

void pool_free(pool_t *pool, void *ptr);
void *pool_move(pool_t *local_pool, void *value, pool_t *return_pool);
void *pool_return(pool_t *local_pool, void *return_value, pool_t *return_pool);
