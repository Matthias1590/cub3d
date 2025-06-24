import pygame
import math

pygame.init()

map = [
	[True, True, True, True, True, True, True],
	[True, False, False, False, False, False, True],
	[True, False, False, False, False, False, True],
	[True, False, False, False, False, False, True],
	[True, True, True, True, True, True, True],
]

player_x = 1.3
player_y = 1.5
player_rot = 0.9

eps = 0.0001  # Small epsilon to nudge in the right direction

display = pygame.display.set_mode((640, 480))
pygame.display.set_caption("Map Display")

def get_next_horizontal(x, y, dir_x, dir_y):
	y_slope = dir_y / dir_x
	if dir_x >= 0:
		next_x = math.ceil(x)
	else:
		next_x = math.floor(x)
	moved_x = (next_x - x)
	return (next_x, y + moved_x * y_slope)

def get_next_vertical(x, y, dir_x, dir_y):
	x_slope = dir_x / dir_y
	if dir_y >= 0:
		next_y = math.ceil(y)
	else:
		next_y = math.floor(y)
	moved_y = (next_y - y)
	return (x + moved_y * x_slope, next_y)

def dist(p1, p2):
	return math.sqrt((p1[0] - p2[0]) ** 2 + (p1[1] - p2[1]) ** 2)

while True:
	for event in pygame.event.get():
		if event.type == pygame.QUIT:
			pygame.quit()
			exit()
	
	display.fill((0, 0, 0))

	for y in range(100):
		pygame.draw.line(display, (255, 255, 255), (0, y * 64), (640, y * 64), 1)
	
	for x in range(100):
		pygame.draw.line(display, (255, 255, 255), (x * 64, 0), (x * 64, 480), 1)

	for y, row in enumerate(map):
		for x, cell in enumerate(row):
			if cell:
				pygame.draw.rect(display, (255, 255, 255), (x * 64, y * 64, 64, 64))

	pygame.draw.circle(display, (255, 0, 0), (int(player_x * 64), int(player_y * 64)), 10)
	
	pygame.draw.line(display, (255, 0, 0), (int(player_x * 64), int(player_y * 64)), 
		(int((player_x + math.cos(player_rot)) * 64), int((player_y + math.sin(player_rot)) * 64)), 2)
	
	## Do raycasting
	# Find first hit
	dir_x = math.cos(player_rot)
	dir_y = math.sin(player_rot)

	p = (player_x, player_y)
	for i in range(10):
		next_horizontal = get_next_horizontal(p[0], p[1], dir_x, dir_y)
		next_vertical = get_next_vertical(p[0], p[1], dir_x, dir_y)

		if dist(p, next_horizontal) < dist(p, next_vertical):
			p = next_horizontal
		else:
			p = next_vertical

		# to nudge in right direction
		p = (p[0] + dir_x * eps, p[1] + dir_y * eps)

		pygame.draw.circle(display, (0, 255, 0), (p[0] * 64, p[1] * 64), 5)

	pygame.display.flip()
	pygame.time.delay(100)
