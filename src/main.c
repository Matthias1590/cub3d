#include <MLX42.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "m.h"
#include "state.h"
#include "scene.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MINIMAP_SCALE 40
#define FOV_SCALE 0.66f

mlx_t *g_mlx;
state_t *g_state;
mlx_image_t *g_img;

void draw_rect(mlx_image_t *img, vec2i_t tl, vec2i_t size, uint32_t color)
{
	vec2i_t pos = tl;
	while (pos.y < tl.y + size.y)
	{
		pos.x = tl.x;
		while (pos.x < tl.x + size.x)
		{
			// TODO: Use memset per row? Could be a lot faster
			if (pos.x >= 0 && pos.x < SCREEN_WIDTH && pos.y >= 0 && pos.y < SCREEN_HEIGHT)
				mlx_put_pixel(img, pos.x, pos.y, color);
			pos.x++;
		}
		pos.y++;
	}
}

void draw_circle(mlx_image_t *img, vec2i_t center, int32_t radius, uint32_t color)
{
	for (int32_t y = -radius; y <= radius; y++)
	{
		for (int32_t x = -radius; x <= radius; x++)
		{
			if (x * x + y * y <= radius * radius)
			{
				int32_t px = center.x + x;
				int32_t py = center.y + y;
				if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT)
					mlx_put_pixel(img, px, py, color);
			}
		}
	}
}

void draw_line(mlx_image_t *img, vec2i_t start, vec2i_t end, uint32_t color)
{
	int32_t dx = end.x - start.x;
	int32_t dy = end.y - start.y;

	if (dx != 0)
	{
		float y_slope = (float)dy / (float)dx;

		if (start.x > end.x)
		{
			vec2i_t temp = start;
			start = end;
			end = temp;
		}
		vec2f_t pos = vec2f(start.x, start.y);
		while (pos.x < end.x)
		{
			if (roundf(pos.x) >= 0 && roundf(pos.x) < SCREEN_WIDTH && roundf(pos.y) >= 0 && roundf(pos.y) < SCREEN_HEIGHT)
				mlx_put_pixel(img, roundf(pos.x), roundf(pos.y), color);
			pos.y += y_slope;
			pos.x++;
		}
	}

	if (dy != 0)
	{
		float x_slope = (float)dx / (float)dy;

		if (start.y > end.y)
		{
			vec2i_t temp = start;
			start = end;
			end = temp;
		}
		vec2f_t pos = vec2f(start.x, start.y);
		while (pos.y < end.y)
		{
			if (roundf(pos.x) >= 0 && roundf(pos.x) < SCREEN_WIDTH && roundf(pos.y) >= 0 && roundf(pos.y) < SCREEN_HEIGHT)
				mlx_put_pixel(img, roundf(pos.x), roundf(pos.y), color);
			pos.x += x_slope;
			pos.y++;
		}
	}
}

void draw_map(mlx_image_t *img, map_t *map)
{
	for (int32_t y = 0; y < SCREEN_HEIGHT; y += MINIMAP_SCALE)
	{
		draw_line(img, vec2i(0, y), vec2i(SCREEN_WIDTH, y), 0xffffffff);
	}
	for (int32_t x = 0; x < SCREEN_WIDTH; x += MINIMAP_SCALE)
	{
		draw_line(img, vec2i(x, 0), vec2i(x, SCREEN_WIDTH), 0xffffffff);
	}

	for (int32_t y = 0; y < map->size.y; y++)
	{
		for (int32_t x = 0; x < map->size.x; x++)
		{
			if (map->tiles[y * map->size.x + x] == TILE_WALL)
			{
				draw_rect(img, vec2i(x * MINIMAP_SCALE, y * MINIMAP_SCALE), vec2i(MINIMAP_SCALE, MINIMAP_SCALE), 0xffffffff);
			}
			else if (map->tiles[y * map->size.x + x] == TILE_EMPTY)
			{
				draw_rect(img, vec2i(x * MINIMAP_SCALE, y * MINIMAP_SCALE), vec2i(MINIMAP_SCALE, MINIMAP_SCALE), 0x00ee00ff);
			}
		}
	}
}

// def get_next_horizontal(x, y, dir_x, dir_y):
// 	y_slope = dir_y / dir_x
// 	if dir_x >= 0:
// 		next_x = math.ceil(x)
// 	else:
// 		next_x = math.floor(x)
// 	moved_x = (next_x - x)
// 	return (next_x, y + moved_x * y_slope)

vec2f_t get_next_horizontal(vec2f_t p, vec2f_t dir)
{
	float y_slope = dir.y / dir.x;
	float next_x = (dir.x >= 0) ? ceilf(p.x) : floorf(p.x);
	float moved_x = (next_x - p.x);
	return vec2f(next_x, p.y + moved_x * y_slope);
}

// def get_next_vertical(x, y, dir_x, dir_y):
// 	x_slope = dir_x / dir_y
// 	if dir_y >= 0:
// 		next_y = math.ceil(y)
// 	else:
// 		next_y = math.floor(y)
// 	moved_y = (next_y - y)
// 	return (x + moved_y * x_slope, next_y)

vec2f_t get_next_vertical(vec2f_t p, vec2f_t dir)
{
	float x_slope = dir.x / dir.y;
	float next_y = (dir.y >= 0) ? ceilf(p.y) : floorf(p.y);
	float moved_y = (next_y - p.y);
	return vec2f(p.x + moved_y * x_slope, next_y);
}

bool cast_ray(map_t *map, vec2f_t origin, vec2f_t dir, vec2f_t *hit, dir_t *hit_dir)
{
	dir = vec2f_normalize(dir);
	
	for (size_t i = 0; i < 500; i++)
	{
		vec2f_t p = origin;
		bool horizontal = false;
		for (size_t i = 0; i < 10; i++)
		{
			vec2f_t next_horizontal = get_next_horizontal(p, dir);
			vec2f_t next_vertical = get_next_vertical(p, dir);

			if (vec2f_dist(p, next_horizontal) < vec2f_dist(p, next_vertical))
			{
				p = next_horizontal;
				horizontal = true;
			}
			else
			{
				p = next_vertical;
				horizontal = false;
			}

			p = vec2f_add(p, vec2f_scale(dir, EPS));

			if (p.x < 0 || p.x >= map->size.x || p.y < 0 || p.y >= map->size.y)
				return false;
			if (map->tiles[(int)p.y * map->size.x + (int)p.x] == TILE_WALL)
			{
				if (hit)
					*hit = p;
				if (hit_dir)
				{
					if (horizontal)
					{
						if (dir.x > 0)
							*hit_dir = DIR_WEST;
						else
							*hit_dir = DIR_EAST;
					}
					else
					{
						if (dir.y > 0)
							*hit_dir = DIR_NORTH;
						else
							*hit_dir = DIR_SOUTH;
					}
				}
				return true;
			}
		}
	}

	return false;
}

void draw_player(mlx_image_t *img, player_t player)
{
	vec2f_t center = vec2f(player.position.x * MINIMAP_SCALE, player.position.y * MINIMAP_SCALE);
	vec2f_t ahead = vec2f_add(center, vec2f_scale(vec2f_from_rot(player.yaw), MINIMAP_SCALE * 100));

	draw_rect(img, vec2i(center.x - MINIMAP_SCALE/2, center.y - MINIMAP_SCALE/2), vec2i(MINIMAP_SCALE, MINIMAP_SCALE), 0xff0000ff);
	draw_line(img, vec2i(center.x, center.y), vec2i(ahead.x, ahead.y), 0xff0000ff);

	// https://lodev.org/cgtutor/raycasting.html
	vec2f_t dir = vec2f_from_rot(player.yaw);
	vec2f_t right = vec2f_rot90(dir);

	for (int x = 0; x < SCREEN_WIDTH; x++)
	{
		// [-1, 1] based on x
		float camera_x = 2 * x / (double)SCREEN_WIDTH - 1;
		vec2f_t ray_dir = vec2f_add(dir, vec2f_scale(right, FOV_SCALE * camera_x));

		vec2f_t t = vec2f_add(center, vec2f_scale(right, FOV_SCALE * camera_x * MINIMAP_SCALE));
		draw_line(img, vec2i(center.x, center.y), vec2i(t.x, t.y), 0xff0000ff);

		// draw_line(img, vec2i(center.x, center.y), vec2i(center.x + ray_dir.x * MINIMAP_SCALE * 10, center.y + ray_dir.y * MINIMAP_SCALE * 10), 0xff00ffff);

		vec2f_t hit;
		vec2f_t origin = player.position;
		if (cast_ray(g_state->scene->map, origin, ray_dir, &hit, NULL))
		{
			draw_circle(img, vec2i(hit.x * MINIMAP_SCALE, hit.y * MINIMAP_SCALE), 3, 0xff0000ff);
		}
	}
}

void draw_minimap(mlx_image_t *img, scene_t *scene)
{
	draw_map(img, scene->map);
	draw_player(img, scene->player);
}

mlx_image_t *g_wall_north;
mlx_image_t *g_wall_south;
mlx_image_t *g_wall_east;
mlx_image_t *g_wall_west;

float inverse_lerp(float a, float b, float value) {
    return (value - a) / (b - a);
}

void draw_texture_slice(mlx_image_t *img, mlx_image_t *texture, int x, int start_y, int end_y, float uv)
{
	uint32_t texture_x = (uint32_t)(uv * (texture->width - 1));
	assert(texture_x < texture->width);

	for (int y = start_y; y < end_y; y++)
	{
		if (y < 0 || y >= SCREEN_HEIGHT)
			continue;

		uint32_t texture_y = (uint32_t)(inverse_lerp(start_y, end_y, y) * texture->height);
		assert(texture_y < texture->height);

		uint32_t color = *(uint32_t *)(texture->pixels + (texture_y * texture->width + texture_x) * sizeof(uint32_t));
		mlx_put_pixel(img, x, y, color);
	}
}

float get_fract(float value)
{
	return value - floorf(value);
}

void draw_scene(mlx_image_t *img, scene_t *scene)
{
	draw_rect(img, vec2i(0, 0), vec2i(SCREEN_WIDTH, SCREEN_HEIGHT), scene->ceiling_color);
	draw_rect(img, vec2i(0, SCREEN_HEIGHT / 2), vec2i(SCREEN_WIDTH, SCREEN_HEIGHT / 2), scene->floor_color);

	vec2f_t dir = vec2f_from_rot(scene->player.yaw);
	vec2f_t right = vec2f_rot90(dir);

	for (int x = 0; x < SCREEN_WIDTH; x++)
	{
		// [-1, 1] based on x
		float camera_x = -2 * x / (double)SCREEN_WIDTH + 1;
		vec2f_t ray_dir = vec2f_normalize(vec2f_add(dir, vec2f_scale(right, FOV_SCALE * camera_x)));

		vec2f_t hit;
		dir_t hit_dir;
		vec2f_t origin = scene->player.position;
		if (cast_ray(scene->map, origin, ray_dir, &hit, &hit_dir))
		{
			float dist = vec2f_dist(scene->player.position, hit);
			float height = SCREEN_HEIGHT / dist;
			int start_y = (SCREEN_HEIGHT - height) / 2;
			int end_y = start_y + height;
			float uv = 0;
			mlx_image_t *texture;
			if (hit_dir == DIR_WEST)
			{
				uv = get_fract(hit.y);
				texture = g_wall_west;
			}
			else if (hit_dir == DIR_EAST)
			{
				uv = 1.0f - get_fract(hit.y);
				texture = g_wall_east;
			}
			else if (hit_dir == DIR_NORTH)
			{
				uv = 1.0f - get_fract(hit.x);
				texture = g_wall_north;
			}
			else // DIR_SOUTH
			{
				uv = get_fract(hit.x);
				texture = g_wall_south;
			}
			draw_texture_slice(img, texture, x, start_y, end_y, uv);
		}
	}
}

void draw_state(mlx_image_t *img, state_t *state)
{
	draw_rect(img, vec2i(0, 0), vec2i(SCREEN_WIDTH, SCREEN_HEIGHT), 0x000000ff);

	// draw_minimap(img, state->scene);
	draw_scene(img, state->scene);
}

void loop_hook(void)
{
	if (mlx_is_key_down(g_mlx, MLX_KEY_LEFT))
	{
		g_state->scene->player.yaw.rads -= 0.05f;
		g_state->scene->player.yaw.rads = fmodf(g_state->scene->player.yaw.rads, 2.0f * M_PI);
		if (g_state->scene->player.yaw.rads < 0) g_state->scene->player.yaw.rads += 2.0f * M_PI; // keep it positive
	}
	if (mlx_is_key_down(g_mlx, MLX_KEY_RIGHT))
	{
		g_state->scene->player.yaw.rads += 0.05f;
		g_state->scene->player.yaw.rads = fmodf(g_state->scene->player.yaw.rads, 2.0f * M_PI);
		if (g_state->scene->player.yaw.rads < 0) g_state->scene->player.yaw.rads += 2.0f * M_PI; // keep it positive
	}

	vec2f_t player_forward = vec2f_from_rot(g_state->scene->player.yaw);
	vec2f_t player_left = vec2f_rot90(player_forward);
	vec2f_t move = vec2f(0, 0);

	if (mlx_is_key_down(g_mlx, MLX_KEY_W))
	{
		move = vec2f_add(move, player_forward);
	}
	if (mlx_is_key_down(g_mlx, MLX_KEY_A))
	{
		move = vec2f_add(move, player_left);
	}
	if (mlx_is_key_down(g_mlx, MLX_KEY_S))
	{
		move = vec2f_add(move, vec2f_neg(player_forward));
	}
	if (mlx_is_key_down(g_mlx, MLX_KEY_D))
	{
		move = vec2f_add(move, vec2f_neg(player_left));
	}

	move = vec2f_normalize(move);
	move = vec2f_scale(move, 0.1f);
	g_state->scene->player.position = vec2f_add(g_state->scene->player.position, move);

	draw_state(g_img, g_state);
}

int main(void)
{
	g_state = state_create();
	if (!g_state)
		return (1);

	g_mlx = mlx_init(SCREEN_WIDTH, SCREEN_HEIGHT, "cub3d", false);
	if (!g_mlx)
		return (1);

	g_state->scene = scene_from_file(g_state->static_pool, "./scenes/test.cub");
	if (!g_state->scene)
		return (1);
	
	mlx_texture_t *north_tex = mlx_load_png("./assets/wall.png");
	g_wall_north = mlx_texture_to_image(g_mlx, north_tex);
	mlx_texture_t *south_tex = mlx_load_png("./assets/wall.png");
	g_wall_south = mlx_texture_to_image(g_mlx, south_tex);
	mlx_texture_t *east_tex = mlx_load_png("./assets/wall.png");
	g_wall_east = mlx_texture_to_image(g_mlx, east_tex);
	mlx_texture_t *west_tex = mlx_load_png("./assets/wall.png");
	g_wall_west = mlx_texture_to_image(g_mlx, west_tex);

	g_img = mlx_new_image(g_mlx, SCREEN_WIDTH, SCREEN_HEIGHT);
	mlx_image_to_window(g_mlx, g_img, 0, 0);

	mlx_loop_hook(g_mlx, (void (*)(void *))loop_hook, NULL);
	mlx_loop(g_mlx);

	state_destroy(g_state);
	return (0);
}
