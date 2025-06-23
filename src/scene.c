#include "scene.h"
#include "io.h"
#include "pool.h"
#include "map.h"
#include <stdbool.h>

static bool player_from_str(const char *str, player_t *player)
{
	vec2i_t pos = vec2i(0, 0);
	size_t i = 0;

	while (str[i])
	{
		pos.x++;
		if (str[i] == '\n')
		{
			pos.y++;
			pos.x = 0;
		}
		else if (str[i] == 'N')
		{
			*player = (player_t){
				.position = vec2f(pos.x - 0.5, pos.y + 0.5),
				.yaw = deg(-90),
			};
			return (true);
		}
		else if (str[i] == 'E')
		{
			*player = (player_t){
				.position = vec2f(pos.x - 0.5, pos.y + 0.5),
				.yaw = deg(0),
			};
			return (true);
		}
		else if (str[i] == 'S')
		{
			*player = (player_t){
				.position = vec2f(pos.x - 0.5, pos.y + 0.5),
				.yaw = deg(90),
			};
			return (true);
		}
		else if (str[i] == 'W')
		{
			*player = (player_t){
				.position = vec2f(pos.x - 0.5, pos.y + 0.5),
				.yaw = deg(180),
			};
			return (true);
		}
		i++;
	}
	return (false);
}

scene_t *scene_from_file(pool_t *pool, const char *path)
{
	pool_t *local_pool = pool_create();
	if (!local_pool)
		return (NULL);

	scene_t *scene = pool_alloc(local_pool, sizeof(scene_t));
	if (!scene)
		return (pool_destroy(local_pool), NULL);

	char *file_content = read_entire_file(local_pool, path);
	if (!file_content)
		return (pool_destroy(local_pool), NULL);
	
	scene->map = map_from_str(local_pool, file_content);
	if (!scene->map)
		return (pool_destroy(local_pool), NULL);

	if (!player_from_str(file_content, &scene->player))
		return (pool_destroy(local_pool), NULL);

	scene->floor_color = 0x00ff00ff;  // Default floor color
	scene->ceiling_color = 0x0000ffff;  // Default ceiling color

	pool_move(local_pool, scene->map->tiles, pool);  // TODO: Not checked
	pool_move(local_pool, scene->map, pool);  // TODO: Not checked
	return (pool_return(local_pool, scene, pool));
}
