#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <wayland-client.h>
#include <stdbool.h>
#include <string.h>
#include "unistd.h"

#include "xdg-shell-client-protocol.h"
#include "game_types.h"
#include "input.h"
#include "render.h"
#include "game.h"

// -----  SHAred memory
/**
 * The randname function generates a string of 6 random uppercase letters based on the current time in nanoseconds,
 * storing the result in the provided character buffer.
 * It uses the `clock_gettime` function to obtain the current time and manipulates the nanoseconds to produce the random letters.
 *
 * @param buf Pointer to a character buffer (must have space for at least 6 characters).
 */
static void randname(char *buf)
{

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts); // type: stop;
	unsigned long r = ts.tv_nsec;
	for (unsigned int i = 0; i < 6; ++i) {
		buf[i] = 'A'+(r&15)+(r&16)*2;
		r >>= 5;
	}
}

/**
 * Tries to creates a temporary file in the Random Access Memory (RAM). Following that, call `randname` function name the file a random name
 * `/wl_shm_XXXXXX`. * Use do-while to try create the file for 100 times.
 * @return `>0` - if can open the file will return the handle of it.
 * @return `<0` - if Can not open the file after 100 of tries.
 */
static int create_shm_file(void)
{
	int retries = 100;
	do {
		char name[] = "/wl_shm-XXXXXX";
		randname(name + sizeof(name) - 7);
		retries--;
		int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
		if (fd >= 0) {
			shm_unlink(name);
			return fd;
		}
	} while (retries > 0 && errno == EEXIST);
	return -1;
}

/**
 * 	The allocate_shm_file function allocates a shared memory file of a specified size by first creating the file and then resizing it using ftruncate.
 *	If the file creation or resizing fails, it returns `-1`; otherwise, it returns the file descriptor of the allocated shared memory file.
 * 	@param size a size of the target file.
 *  @return `fd` - file descriptor of the allocated shared memory file. Or `-1` if cannot open or create the file.
 */
static int allocate_shm_file(size_t size)
{
	int fd = create_shm_file();
	if (fd < 0)
		return -1;
	int ret;
	do {
		ret = ftruncate(fd, size);
	} while (ret < 0 && errno == EINTR);
	if (ret < 0) {
		close(fd);
		return -1;
	}
	return fd;
}

static void wl_buffer_release(void *data, struct wl_buffer *wl_buffer)
{
    struct app_state *state = data;
	for (unsigned int i = 0; i < NUM_OF_BUFFERS; i++) {
        if (state->buffers[i].wl_buf == wl_buffer) {
            state->buffers[i].is_busy = false;
            break;
        }
    }
}

static const struct wl_buffer_listener buffer_listener = {
	.release = wl_buffer_release,
};


//-------- Callback & Listener-------

static const struct wl_callback_listener frame_listener;

void frame_handler(void *data, struct wl_callback *callback, uint32_t time)
{
    struct app_state *app = data;

    if (callback) {
		wl_callback_destroy(callback);
		app->frame_callback = NULL;
	}

	if (!app->refresh) return;

    unsigned int next_buffer_idx = (NUM_OF_BUFFERS - 1) - app->current_buffer_idx;
    if (app->buffers[next_buffer_idx].is_busy) {

        app->frame_callback = wl_surface_frame(app->wl_surface);
        wl_callback_add_listener(app->frame_callback, &frame_listener, app);
        wl_surface_commit(app->wl_surface);

        return;
    }
    app->current_buffer_idx = next_buffer_idx;
    app->buffers[app->current_buffer_idx].is_busy = true;

	if (app->game_over) {
		memcpy(app->buffers[app->current_buffer_idx].pixels, app->end_msg.msg_buf, BUFFER_SIZE);
		app->refresh = false;
	}
	else if (app->game_started == false) {
		render(app);
		if (app->game_pause) {
			overlay_message_ultimate(app->buffers[app->current_buffer_idx].pixels, app->pause_msg.msg_buf, app->pause_msg.pix_wid, app->pause_msg.pix_hig, 0, 0);
		}
		else {
			overlay_message_ultimate(app->buffers[app->current_buffer_idx].pixels, app->start_msg.msg_buf, app->start_msg.pix_wid, app->start_msg.pix_hig, 0, 0);
		}
	}
	else {
		render(app);
	}

    wl_surface_attach(app->wl_surface, app->buffers[app->current_buffer_idx].wl_buf, 0, 0);
    wl_surface_damage(app->wl_surface, 0, 0, WIDTH, HEIGHT);

    if (app->refresh) {
        app->frame_callback = wl_surface_frame(app->wl_surface);
        wl_callback_add_listener(app->frame_callback, &frame_listener, app);
    }
	app->refresh = false;
    wl_surface_commit(app->wl_surface);
}

static const struct wl_callback_listener frame_listener = {
	.done = frame_handler
};

// ------------------- XDG -------------

// Functions :
static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {	
	struct app_state *app = data;
	xdg_surface_ack_configure(xdg_surface, serial);

	if (app->initialized) return;

	int fd = allocate_shm_file(SHM_POOL_SIZE);
	if (fd < 0) return;
	app->shm_data = mmap(NULL, SHM_POOL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	struct wl_shm_pool *pool = wl_shm_create_pool(app->wl_shm, fd, SHM_POOL_SIZE);

	for (unsigned int i = 0; i < NUM_OF_BUFFERS; i++) {
		app->buffers[i].is_busy = false;
		app->buffers[i].pixels = app->shm_data + (WIDTH * HEIGHT * i);
		app->buffers[i].wl_buf = wl_shm_pool_create_buffer(pool, i * BUFFER_SIZE, WIDTH, HEIGHT, STRIDE, WL_SHM_FORMAT_XRGB8888);
		wl_buffer_add_listener(app->buffers[i].wl_buf, &buffer_listener, app);
	}

	wl_shm_pool_destroy(pool);
	close(fd);

	app->initialized = true;
	frame_handler(app, NULL, 0);
}

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
	xdg_wm_base_pong(xdg_wm_base, serial);
}

static void xdg_toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel) {
	struct app_state *app = data;
	app->running = false;
}
// to avoid passes null pointer value
static void xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states) {/*NULL*/}

// Structures :

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_configure,
};

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
	.ping = xdg_wm_base_ping,
};

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
	.configure = xdg_toplevel_configure,
	.close = xdg_toplevel_close,
};

// ---- Registry : ----

// functions:
static void registry_handle_global(void *data, struct wl_registry *wl_registry, uint32_t name, const char *interface, uint32_t version)
{
	struct app_state *app = data;

	// Bind with Compositor Interface
	if (strcmp(interface, wl_compositor_interface.name) == 0)
		app->wl_compositor = wl_registry_bind(wl_registry, name, &wl_compositor_interface, VR(4, version));

	// Bind with Shared memory Interface
	else if (strcmp(interface, wl_shm_interface.name) == 0)
		app->wl_shm = wl_registry_bind(wl_registry, name, &wl_shm_interface, VR(1, version));

	// Bind with Seat Interface
	else if (strcmp(interface, wl_seat_interface.name) == 0) {
		app->wl_seat = wl_registry_bind(wl_registry, name, &wl_seat_interface, VR(1, version));
		wl_seat_add_listener(app->wl_seat, &seat_listener, app);
	}

	// Bind with X Desktop Group Windows Manager Base Interface
	else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		app->xdg_wm_base = wl_registry_bind(wl_registry, name, &xdg_wm_base_interface, VR(1, version));
		xdg_wm_base_add_listener(app->xdg_wm_base, &xdg_wm_base_listener, app);
	}
}

static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {/*NULL*/}

// structures:
static const struct wl_registry_listener registry_listener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
};

// ------------- INITIALIZING Wayland ----------
bool init_wayland(struct app_state *app) {

    app->wl_display = wl_display_connect(NULL);
	if (!app->wl_display) return false;

	app->wl_registry = wl_display_get_registry(app->wl_display);
	wl_registry_add_listener(app->wl_registry, &registry_listener, app);

	// Start
	wl_display_roundtrip(app->wl_display);
	if (!app->wl_shm || !app->wl_compositor ||  !app->xdg_wm_base) return false;

	app->wl_surface = wl_compositor_create_surface(app->wl_compositor);
	app->xdg_surface = xdg_wm_base_get_xdg_surface(app->xdg_wm_base, app->wl_surface);

	if (!app->wl_surface || !app->xdg_surface) return false;
	xdg_surface_add_listener(app->xdg_surface, &xdg_surface_listener, app);

	// toplevel
	app->xdg_toplevel = xdg_surface_get_toplevel(app->xdg_surface);
	xdg_toplevel_add_listener(app->xdg_toplevel, &xdg_toplevel_listener, app);
	xdg_toplevel_set_title(app->xdg_toplevel, TITLE);
	wl_surface_commit(app->wl_surface);

    return true;
}

void cleanup_wayland(struct app_state *app) {
	
	if (app->frame_callback) {
        wl_callback_destroy(app->frame_callback);
        app->frame_callback = NULL;
    }

    for (unsigned int i = 0; i < NUM_OF_BUFFERS; i++) {
        if (app->buffers[i].wl_buf)
			wl_buffer_destroy(app->buffers[i].wl_buf);
    }

    if (app->shm_data) munmap(app->shm_data, SHM_POOL_SIZE);

    if (app->xdg_toplevel) xdg_toplevel_destroy(app->xdg_toplevel);
    if (app->xdg_surface) xdg_surface_destroy(app->xdg_surface);
    if (app->xdg_wm_base) xdg_wm_base_destroy(app->xdg_wm_base);

    if (app->keyboard) wl_keyboard_destroy(app->keyboard);
    if (app->wl_seat) wl_seat_destroy(app->wl_seat);
    if (app->wl_surface) wl_surface_destroy(app->wl_surface);
    if (app->wl_shm) wl_shm_destroy(app->wl_shm);
    if (app->wl_compositor) wl_compositor_destroy(app->wl_compositor);
    if (app->wl_registry) wl_registry_destroy(app->wl_registry);

    if (app->wl_display) {
        wl_display_disconnect(app->wl_display);
    }
}