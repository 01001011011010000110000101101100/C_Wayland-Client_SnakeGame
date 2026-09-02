#include <stdbool.h>
#include <unistd.h>

#include <linux/input-event-codes.h>
#include <wayland-client.h>
#include "game_types.h"

static void input_queue_handler(struct app_state *app, Direction new_dir) {
	Direction last_pendin = (app->queue_len > 0) ? app->input_queue[app->queue_len - 1] : app->last_dir;	
	bool is_opposite = (
						(new_dir == DIR_UP && last_pendin == DIR_DOWN) ||
						(new_dir == DIR_DOWN && last_pendin == DIR_UP) || 
						(new_dir == DIR_LEFT && last_pendin == DIR_RIGHT) ||
						(new_dir == DIR_RIGHT && last_pendin == DIR_LEFT)
					);
	if (is_opposite || new_dir == last_pendin) return;
	else if (app->queue_len >= 2) app->input_queue[app->queue_len - 1] = new_dir;
	else {
		app->input_queue[app->queue_len] = new_dir;
		app->queue_len++;
	}
}

static void kbd_keymap(void *data, struct wl_keyboard *kb, uint32_t format, int32_t fd, uint32_t size) {close(fd);} // For Keyboard map
static void kbd_enter(void *data, struct wl_keyboard *kb, uint32_t serial, struct wl_surface *surf, struct wl_array *keys) {}
static void kbd_leave(void *data, struct wl_keyboard *kb, uint32_t serial, struct wl_surface *surf) {struct app_state *app = data; app->game_pause = true; app->refresh = true; app->game_started = false;}
static void kbd_modifiers(void *data, struct wl_keyboard *kb, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {}
static void kbd_repeat_info(void *data, struct wl_keyboard *kb, int32_t rate, int32_t delay) {}

// Handle with keyboard input
static void kbd_key(void *data, struct wl_keyboard *kb, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
	struct app_state *app = data;
	if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {

		if (!app->game_started && !app->game_over) {
            if (key == KEY_UP || key == KEY_DOWN || key == KEY_LEFT || key == KEY_RIGHT ||
                key == KEY_W  || key == KEY_S    || key == KEY_A    || key == KEY_D) {
                app->game_started = true;
				app->refresh = true;
			}
			else if (key == KEY_1) {
				app->snake_shape = WORM_SHAPE;
				app->refresh = true;
			}
			else if (key == KEY_2) {
				app->snake_shape = SNAKE_SHAPE;
				app->refresh = true;
			}
			else if (key == KEY_3) {
				app->snake_shape = SQUARE_SHAPE;
				app->refresh = true;
			}
		}
		// Go up
		else if ((key == KEY_UP || key == KEY_W) && app->last_dir != DIR_DOWN) input_queue_handler(app, DIR_UP);
		// Go down
		else if ((key == KEY_DOWN || key == KEY_S) && app->last_dir != DIR_UP) input_queue_handler(app, DIR_DOWN);
		// Go left
		else if ((key == KEY_LEFT || key == KEY_A) && app->last_dir != DIR_RIGHT) input_queue_handler(app, DIR_LEFT);
		// Go Right
		else if ((key == KEY_RIGHT || key == KEY_D) && app->last_dir != DIR_LEFT) input_queue_handler(app, DIR_RIGHT);
		// Exit
		if (key == KEY_ESC) app->running = false;
		else if (key == KEY_P && app->game_started) {
			app->game_started = false;
			app->game_pause = true;
			app->refresh = true;
		}
	}
}

static const struct wl_keyboard_listener keyboard_listener = {
	.keymap = kbd_keymap,
	.enter = kbd_enter,
	.leave = kbd_leave,
	.key = kbd_key,
	.modifiers = kbd_modifiers,
	.repeat_info = kbd_repeat_info 
};
// define the seats
static void seat_capabilities(void *data, struct wl_seat *seat, uint32_t caps)
{
	struct app_state *app = data;
    if (caps & WL_SEAT_CAPABILITY_KEYBOARD) {
        app->keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(app->keyboard, &keyboard_listener, app);
    }
}
// To avoid passes null pointer
static void seat_name(void *data, struct wl_seat *seat, const char *name) {}
// define the structure
const struct wl_seat_listener seat_listener = {
	.capabilities = seat_capabilities,
	.name = seat_name 
};