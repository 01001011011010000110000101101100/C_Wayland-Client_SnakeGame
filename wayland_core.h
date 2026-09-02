#ifndef WAY_COR
#define WAY_COR

#include "game_types.h"

extern bool init_wayland(struct app_state *app);
extern void cleanup_wayland(struct app_state *app);
extern void frame_handler(void *data, struct wl_callback *callback, uint32_t time);

#endif