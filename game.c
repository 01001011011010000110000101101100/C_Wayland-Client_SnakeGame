#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <sys/timerfd.h>
#include <sys/random.h>

#include "game_types.h"
#include "render.h"

#define PENALTY_SEC 2

static void update_timer_interval(int timer_fd, long interval_ms) {
	struct itimerspec new_value = {0};
	new_value.it_interval.tv_sec = interval_ms / 1000;
    new_value.it_interval.tv_nsec = (interval_ms % 1000) * 1000000L;
	new_value.it_value = new_value.it_interval;

	timerfd_settime(timer_fd, 0, &new_value, NULL);
}

static void spawn_food(struct app_state *app)
{
	bool occupied;
	unsigned int rand_value[2];
	do {
		occupied = false;
		getrandom(&rand_value, sizeof(rand_value), GRND_NONBLOCK);

		app->food.x = rand_value[0] % GRID_SIZE;
		app->food.y = GRID_OFFSET_Y + (rand_value[1] % GRID_SIZE);

		for (unsigned int i = 0; i < app->snake_len; i++) {
			if (app->snake[i].x == app->food.x && app->snake[i].y == app->food.y) {
				occupied = true;
				break;
			}
		}
	} while (occupied);
}

void reset_snake(struct app_state *app)
{
	spawn_food(app);
	app->snake[0].x = GRID_SIZE / 2; app->snake[0].y = GRID_OFFSET_Y + (GRID_SIZE / 2);
	app->snake_len = 1;
	app->session_score = 0;
	app->game_started = false;
	app->last_tick = (struct timespec){0};
	app->tick_interval = 180;
	if (app->timer_fd > 0) {
		update_timer_interval(app->timer_fd, app->tick_interval);
	}
	app->queue_len = 0;
	app->dir = DIR_UP;
	app->last_dir = DIR_UP;
}

static void trigger_game_over(struct app_state *app) {
	app->game_pause = false;
	if (app->session_score > app->high_score) {
		app->high_score = app->session_score;
	}
	reset_snake(app);
	app->game_over = true;
	app->refresh = true;
	
	clock_gettime(CLOCK_MONOTONIC, &app->game_over_until);
	app->game_over_until.tv_sec += PENALTY_SEC;
}

void update_game(struct app_state *app)
{	
	// Go forward
	for (unsigned int i = app->snake_len - 1; i > 0; i--) {
		app->snake[i] = app->snake[i - 1];
	}
	// Movement
	if (app->queue_len > 0) {
		app->dir = app->input_queue[0];
		app->input_queue[0] = app->input_queue[1];
		app->queue_len--;
	}
	switch (app->dir) {
		case DIR_UP:	app->snake[0].y--; break;
		case DIR_DOWN:	app->snake[0].y++; break;
		case DIR_LEFT:	app->snake[0].x--;break;
		case DIR_RIGHT: app->snake[0].x++; break;
	}
	app->last_dir = app->dir;
	// Collision with map boundaries
	if (app->snake[0].x < 0 || app->snake[0].x >= GRID_SIZE ||
		app->snake[0].y < GRID_OFFSET_Y || app->snake[0].y >= GRID_SIZE + GRID_OFFSET_Y)
	{
		trigger_game_over(app);
		return;
	}
	// Ate itself
	for (unsigned int i = 1; i < app->snake_len; i++) {
		if (app->snake[0].x == app->snake[i].x &&
			app->snake[0].y == app->snake[i].y)
		{
			trigger_game_over(app);
			return;
		}
	}

	// Ate The food
	if (app->snake[0].x == app->food.x &&
		app->snake[0].y == app->food.y)
	{
		if (app->snake_len < MAX_SNAKE) {
			app->snake[app->snake_len] = app->snake[app->snake_len - 1];
			app->snake_len++;
			app->session_score++;
			if (app->tick_interval > 60 && (app->snake_len % 5) == 0) {
				app->tick_interval -= 10;
				update_timer_interval(app->timer_fd, app->tick_interval);
			}
		}
		spawn_food(app);
	}
}