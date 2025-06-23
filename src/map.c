#include "map.h"
#include "pool.h"
#include <stdbool.h>

static vec2i_t get_map_size(const char *str)
{
	vec2i_t size = vec2i(0, 0);
	int32_t x = 0;

	size_t i = 0;
	while (str[i])
	{
		if (str[i] == '\n')
		{
			size.y++;
			x = 0;
		}
		else
		{
			x++;
			if (x > size.x)
				size.x = x;
		}
		i++;
	}
	if (x > 0)
		size.y++;

	return (size);
}

static bool read_map(map_t *map, const char *str)
{
	vec2i_t pos = vec2i(0, 0);

	size_t i = 0;
	while (str[i])
	{
		pos.x++;
		if (str[i] == '\n')
		{
			pos.x = 0;
			pos.y++;
		}
		else if (str[i] == '0' || str[i] == 'N' || str[i] == 'E' || str[i] == 'S' || str[i] == 'W')
		{
			map->tiles[pos.y * map->size.x + pos.x - 1] = TILE_EMPTY;
		}
		else if (str[i] == '1')
		{
			map->tiles[pos.y * map->size.x + pos.x - 1] = TILE_WALL;
		}
		else if (str[i] == ' ')
		{
			map->tiles[pos.y * map->size.x + pos.x - 1] = TILE_VOID;
		}
		else
		{
			return (false);
		}
		i++;
	}
	return (true);
}

map_t *map_from_str(pool_t *pool, const char *str)
{
	pool_t *local_pool = pool_create();
	if (!local_pool)
		return (NULL);

	map_t *map = pool_alloc(local_pool, sizeof(map_t));
	if (!map)
		return (pool_destroy(local_pool), NULL);

	map->size = get_map_size(str);

	map->tiles = pool_alloc(local_pool, map->size.y * map->size.x * sizeof(tile_t));
	if (!map->tiles)
		return (pool_destroy(local_pool), NULL);

	if (!read_map(map, str))
		return (pool_destroy(local_pool), NULL);
	
	return (pool_return(local_pool, map, pool));
}
