#ifndef RENDER
#define RENDER

#include "game_types.h"

extern void overlay_message_ultimate(uint32_t *dest, const uint32_t *src, const unsigned int text_w, const unsigned int text_h, const signed int displacement_x, const signed int displacement_y);
extern void draw_message(message *message, const signed int displacement_x, const signed int displacement_y, const unsigned int scale);
extern void draw_score(const unsigned int score, uint32_t *pixel_buf, const signed int displacement_x, const signed int displacement_y, const unsigned int scale);
extern void render(const struct app_state *state);
extern message create_message(uint32_t *msg_buf, unsigned char *msg_content);

#endif