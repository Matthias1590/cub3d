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

bool cast_ray(map_t *map, vec2f_t origin, vec2f_t dir, vec2f_t *hit)
{
	dir = vec2f_normalize(dir);
	
	for (size_t i = 0; i < 500; i++)
	{
		if (map->tiles[(int)(origin.y) * map->size.x + (int)(origin.x)] == TILE_WALL)
		{
			*hit = origin;
			return true;
		}

		origin.x += dir.x * 0.01f;
		origin.y += dir.y * 0.01f;

		if (origin.x < 0 || origin.x >= map->size.x || origin.y < 0 || origin.y >= map->size.y)
		{
			return false;
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
		if (cast_ray(g_state->scene->map, origin, ray_dir, &hit))
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
		vec2f_t origin = scene->player.position;
		if (cast_ray(scene->map, origin, ray_dir, &hit))
		{
			float dist = vec2f_dist(scene->player.position, hit);
			int height = (int)(SCREEN_HEIGHT / dist);
			int start_y = (SCREEN_HEIGHT - height) / 2;
			int end_y = start_y + height;
			if (end_y > SCREEN_HEIGHT)
				end_y = SCREEN_HEIGHT;
			if (start_y < 0)
				start_y = 0;
			draw_line(img, vec2i(x, start_y), vec2i(x, end_y), 0xff0000ff);
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

	g_img = mlx_new_image(g_mlx, SCREEN_WIDTH, SCREEN_HEIGHT);
	mlx_image_to_window(g_mlx, g_img, 0, 0);

	mlx_loop_hook(g_mlx, (void (*)(void *))loop_hook, NULL);
	mlx_loop(g_mlx);

	state_destroy(g_state);
	return (0);
}
