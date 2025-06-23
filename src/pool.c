#include "pool.h"
#include "mem.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void da_init(size_t elem_size, void **elems, size_t *count, size_t *capacity)
{
	*count = 0;
	*capacity = 1;
	*elems = malloc(*capacity * elem_size);
}

pool_t *pool_create(void)
{
	pool_t *pool = malloc(sizeof(pool_t));
	if (!pool)
		return (NULL);
	
	da_init(sizeof(*pool->fds), (void **)&pool->fds, &pool->fds_count, &pool->fds_capacity);
	da_init(sizeof(*pool->ptrs), (void **)&pool->ptrs, &pool->ptrs_count, &pool->ptrs_capacity);

	return (pool);
}

void pool_destroy(pool_t *pool)
{
	size_t i;

	i = 0;
	while (i < pool->fds_count)
	{
		close(pool->fds[i]);
		i++;
	}
	free(pool->fds);

	i = 0;
	while (i < pool->ptrs_count)
	{
		free(pool->ptrs[i]);
		i++;
	}
	free(pool->ptrs);

	free(pool);
}

void *pool_alloc(pool_t *pool, size_t size)
{
	void *ptr = malloc(size);
	if (!ptr)
		return (NULL);
	
	if (pool->ptrs_count == pool->ptrs_capacity)
	{
		pool->ptrs = mem_realloc(pool->ptrs, pool->ptrs_capacity * sizeof(*pool->ptrs), pool->ptrs_capacity * 2 * sizeof(*pool->ptrs));
		if (!pool->ptrs)
			return (NULL);
		pool->ptrs_capacity *= 2;
	}

	pool->ptrs[pool->ptrs_count++] = ptr;

	return (ptr);
}

int pool_open(pool_t *pool, const char *path, int flags)
{
	int fd = open(path, flags);
	if (fd < 0)
		return (fd);

	if (pool->fds_count == pool->fds_capacity)
	{
		pool->fds = mem_realloc(pool->fds, pool->fds_capacity * sizeof(*pool->fds), pool->fds_capacity * 2 * sizeof(*pool->fds));
		if (!pool->fds)
			return (-1);
		pool->fds_capacity *= 2;
	}

	pool->fds[pool->fds_count++] = fd;

	return (fd);
}

void pool_free(pool_t *pool, void *ptr)
{
	size_t i;

	i = 0;
	while (i < pool->ptrs_count)
	{
		if (pool->ptrs[i] == ptr)
		{
			mem_swap(&pool->ptrs[i], &pool->ptrs[--pool->ptrs_count], sizeof(*pool->ptrs));
			break ;
		}
		i++;
	}
}

void *pool_move(pool_t *local_pool, void *value, pool_t *return_pool)
{
	size_t i;

	i = 0;
	while (i < local_pool->ptrs_count)
	{
		if (local_pool->ptrs[i] == value)
		{
			if (return_pool->ptrs_count == return_pool->ptrs_capacity)
			{
				return_pool->ptrs = mem_realloc(return_pool->ptrs, return_pool->ptrs_capacity * sizeof(*return_pool->ptrs), return_pool->ptrs_capacity * 2 * sizeof(*return_pool->ptrs));
				if (!return_pool->ptrs)
					return (NULL);
				return_pool->ptrs_capacity *= 2;
			}

			return_pool->ptrs[return_pool->ptrs_count++] = local_pool->ptrs[i];
			
			mem_swap(&local_pool->ptrs[i], &local_pool->ptrs[--local_pool->ptrs_count], sizeof(*local_pool->ptrs));
			break ;
		}
		i++;
	}

	return (value);
}

void *pool_return(pool_t *local_pool, void *return_value, pool_t *return_pool)
{
	size_t i;

	i = 0;
	while (i < local_pool->ptrs_count)
	{
		if (local_pool->ptrs[i] == return_value)
		{
			if (return_pool->ptrs_count == return_pool->ptrs_capacity)
			{
				return_pool->ptrs = mem_realloc(return_pool->ptrs, return_pool->ptrs_capacity * sizeof(*return_pool->ptrs), return_pool->ptrs_capacity * 2 * sizeof(*return_pool->ptrs));
				if (!return_pool->ptrs)
					return (NULL);
				return_pool->ptrs_capacity *= 2;
			}

			return_pool->ptrs[return_pool->ptrs_count++] = local_pool->ptrs[i];
			
			mem_swap(&local_pool->ptrs[i], &local_pool->ptrs[--local_pool->ptrs_count], sizeof(*local_pool->ptrs));
			break ;
		}
		i++;
	}

	pool_destroy(local_pool);
	return (return_value);
}
