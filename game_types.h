#ifndef TYPES
#define TYPES

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
// Application
#define TITLE 			"Snake Game"
// Messages
#define START_MESSAGE	"Start Moving.."
#define END_MESSAGE		"Game Over"
#define SCORE_TEXT		"Score: "
#define HTH_SCORE_TEXT	"High Score: "
#define PAUSE_TEXT  	"Pause"

#define SCORE_MSG_X		(-(WIDTH / 2) + CELL_SIZE * 3)
#define HIG_SCR_MSG_X	(HIG_SCR_INT_X - (5 * CELL_SIZE))
#define SCORE_INT_X		(SCORE_MSG_X + CELL_SIZE * 3)
#define HIG_SCR_INT_X	((WIDTH / 2) -  CELL_SIZE * 2)

#define FONT_BASE_SIZE  8
#define TEXT_SCALE      2
#define CHAR_WIDTH      (FONT_BASE_SIZE * TEXT_SCALE)
#define CHAR_HEIGHT     (FONT_BASE_SIZE * TEXT_SCALE)

// Screen Size
#define GRID_SIZE 		20
#define CELL_SIZE 		25
#define BYTE_PER_PIXEL 	4
#define WIDTH 			(GRID_SIZE * CELL_SIZE)
#define HEIGHT		 	((GRID_SIZE * CELL_SIZE) + SCORE_AREA_H)
#define GRID_OFFSET_Y 	2
#define SCORE_AREA_H	(CELL_SIZE * GRID_OFFSET_Y)
#define MIDDLE_SCR_ARE  (-(HEIGHT / 2) + (SCORE_AREA_H / 2))
#define MAX_SNAKE 		(GRID_SIZE * GRID_SIZE)
// Buffer Micros
#define STRIDE      	(WIDTH * BYTE_PER_PIXEL)
#define BUFFER_SIZE  	(HEIGHT * STRIDE)
#define SHM_POOL_SIZE 	(BUFFER_SIZE * 2)
#define NUM_OF_BUFFERS	2
// Database
#define RELATIVE_PATH	".local/share/Snake_Game/score_recorder.dat"
// Compositor Micros :
#define VR(a, b) ((a) < (b) ? (a) : (b))

/* ------------   Micros End   ----------- */

/* -------  Initializing The Main Structures : ------*/


// Diraction Operators
typedef enum {
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT
} Direction;

typedef enum {
	SNAKE_SHAPE,
	WORM_SHAPE,
	SQUARE_SHAPE,
} Shape;

typedef struct {
    int x, y;
} Point;

typedef struct {
	uint32_t *msg_buf;
	const unsigned char *string;
	unsigned int length;
	unsigned int pix_wid;
	unsigned int pix_hig;
} message;

// Client Structures 
struct app_state {
// Wayland
	struct wl_display 		*wl_display;
	struct wl_registry		*wl_registry;
	struct wl_shm 			*wl_shm;
	struct wl_compositor	*wl_compositor;
	struct wl_surface 		*wl_surface;
	struct wl_keyboard		*keyboard;
	struct wl_seat 			*wl_seat;
	struct wl_callback 		*frame_callback;
// X Desktop Group
	struct xdg_wm_base		*xdg_wm_base;
	struct xdg_surface 		*xdg_surface;
	struct xdg_toplevel		*xdg_toplevel;
// Buffers
	struct {
        struct wl_buffer *wl_buf;
        uint32_t		 *pixels;
        bool 			 is_busy;
    } buffers[NUM_OF_BUFFERS];

// Messages
	message start_msg;
	message end_msg;
	message score_msg;
	message high_score_msg;
	message pause_msg;

    unsigned int current_buffer_idx; // 0 or 1
	uint32_t *shm_data;
// Game :
	// snake & food
	Shape snake_shape;
    Point snake[MAX_SNAKE];
    unsigned int snake_len;
    Point 	food;
	// diraction
	Direction input_queue[2];
	Direction last_dir;
	Direction dir;
	unsigned int queue_len;
	// status
    bool running;
	bool initialized;
	// Messages & Display
	bool game_started;
	bool game_pause;
	bool game_over;
	bool refresh;
	struct timespec game_over_until;
	// Speed
	unsigned long tick_interval;
	int timer_fd;
	// Score
	unsigned int high_score;
	unsigned int session_score;
};

#endif
