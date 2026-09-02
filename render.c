#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "game_types.h"

#define COLOR_BG    	0xFF1E1E2E
#define CR_SNAKE_HEAD 	0xFF27AE60
#define CR_SNAKE_BODY 	0xFF66BB6A

#define _               0x00000000
#define R               0xFFE74C3C
#define G   /*GREEN*/   0xFF2ECC71
#define B   /*Brown*/   0xFF795548 
#define BL              0xFF000000
#define GR /*DG*/       0xFFB0B0B0
#define DG              0xFF808080
#define PI              0xFFFFB6C1 
#define D               0xFF0A1F14
#define S               0xFF134E35
#define M               0xFF10B981
#define H               0xFF34D399
#define K               0xFFD97706
#define E               0xFFFBBF24
#define T               0xFFEF4444

#define ICON_SIZE		25

static const uint32_t WORM_UP[ICON_SIZE][ICON_SIZE];
static const uint32_t WORM_DOWN[ICON_SIZE][ICON_SIZE];
static const uint32_t WORM_RIGHT[ICON_SIZE][ICON_SIZE];
static const uint32_t WORM_LEFT[ICON_SIZE][ICON_SIZE];
static const uint32_t APPLE_SPRITE[ICON_SIZE][ICON_SIZE];

static const uint32_t SNAKE_UP[ICON_SIZE][ICON_SIZE];
static const uint32_t SNAKE_DOWN[ICON_SIZE][ICON_SIZE];
static const uint32_t SNAKE_LEFT[ICON_SIZE][ICON_SIZE];
static const uint32_t SNAKE_RIGHT[ICON_SIZE][ICON_SIZE];
static const uint32_t MOUSE_SPRITE[ICON_SIZE][ICON_SIZE];

static const uint8_t FONT[128][8];

static void draw_rect(uint32_t *pixels ,unsigned int x, unsigned int y, unsigned int width, unsigned int hight, uint32_t color)
{
	for (unsigned int j = y; j < (y + hight); j++) {
		for (unsigned int i = x; i < (x + width); i++) {
			if (i < WIDTH && j < HEIGHT)
				pixels[(j * WIDTH) + i] = color;
		}
	}
}

static void draw_circle(uint32_t *pixels,  int center_x,  int center_y,  int radius, uint32_t color)
{
    unsigned int r2 = radius * radius;
    for (signed int j = -radius; j <= radius; j++) {
        for (signed int i = -radius; i <= radius; i++) {
            if ((i * i + j * j) <= r2) {
                int px = center_x + i;
                int py = center_y + j;
                if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                    pixels[py * WIDTH + px] = color;
                }
            }
        }
    }
}

static void draw_sprite(uint32_t *pixels, unsigned int x, unsigned int y, unsigned int sprite_w, unsigned int sprite_h, const uint32_t *sprite)
{
	for (unsigned int j = 0; j < sprite_h; j++) {
		for (unsigned int i = 0; i < sprite_w; i++) {
			uint32_t color = sprite[j * sprite_w + i];
			if (color == _) // None
                continue;
			unsigned int pixel_x = x + i;
			unsigned int pixel_y = y + j;

			if (pixel_x < WIDTH && pixel_y < HEIGHT) 
                pixels[pixel_y * WIDTH + pixel_x] = color;
		}
	}
}

void draw_message(message *message, const signed int displacement_x, const signed int displacement_y, const unsigned int scale) {
        /*Less than*/                                                           /*More than*/
    if (displacement_x < -(WIDTH / 2) || displacement_y < -(HEIGHT / 2) || displacement_x > (WIDTH / 2) || displacement_y > (HEIGHT / 2)) 
    {
        fprintf(stderr ,"The Displacement is out of range: `displacement_x`=%i, `displacement_y`=%i. From `draw_message` function", displacement_x, displacement_y);
        return;
    }

    unsigned int scaled_char_width = FONT_BASE_SIZE * scale;
    unsigned int scaled_char_height = FONT_BASE_SIZE * scale;
    message->pix_wid = message->length * scaled_char_width;
    message->pix_hig = scaled_char_height;

    unsigned int start_x = ((WIDTH - message->pix_wid) / 2) + displacement_x;
    unsigned int start_y = ((HEIGHT - message->pix_hig) / 2) + displacement_y;

    for (unsigned int y = start_y; y < start_y + scaled_char_height; y++) {
        if (y < HEIGHT && start_x < WIDTH)
            memset(&message->msg_buf[y * WIDTH + start_x], 0x00000000, message->pix_wid * sizeof(uint32_t));
    }
    
    for (unsigned int letter = 0; letter < message->length; letter++) {
        unsigned char c = message->string[letter];
        unsigned int char_offset_x = start_x + (letter * scaled_char_width);

        for (unsigned int byte_index = 0; byte_index < FONT_BASE_SIZE; byte_index++) {
            uint8_t byte_value = FONT[c][byte_index];

            for (int bit_index = 0; bit_index < 8; bit_index++) {
                if (byte_value & (1 << (7 - bit_index))) {
                    for (unsigned int sy = 0; sy < scale; sy++) {
                        for (unsigned int sx = 0; sx < scale; sx++) {

                            unsigned int px = char_offset_x + (scale * bit_index) + sx;
                            unsigned int py = start_y + (byte_index * scale) + sy;

                            if (px < WIDTH && py < HEIGHT)
                                message->msg_buf[py * WIDTH + px] = 0xFFFFFFFF;
                        }
                    }
                }
            }
        }
    }
}

message create_message(uint32_t *msg_buf, unsigned char *msg_content) {
    message msg;
    msg.string = msg_content;
    msg.length = (unsigned int) strlen(msg.string);
    msg.msg_buf = msg_buf;
    return msg;
}

void draw_score(const unsigned int score, uint32_t *pixel_buf, const signed int displacement_x, const signed int displacement_y, const unsigned int scale) {
            /*Less than*/                                                           /*More than*/
    if (displacement_x < -(WIDTH / 2) || displacement_y < -(HEIGHT / 2) || displacement_x > (WIDTH / 2) || displacement_y > (HEIGHT / 2)) 
    {
        fprintf(stderr ,"The Displacement is out of range: `displacement_x`=%i, `displacement_y`=%i. From `draw_score` function", displacement_x, displacement_y);
        return;
    }
    unsigned char score_str[16];
    snprintf(score_str, sizeof(score_str), "%u", score);

    message msg;
    msg.string = score_str;
    msg.length = strlen(score_str);
    msg.pix_wid = msg.length * CHAR_WIDTH;
    
    unsigned int start_x = ((WIDTH - msg.pix_wid) / 2) + displacement_x;
    unsigned int start_y = (HEIGHT - CHAR_HEIGHT) / 2 + displacement_y;

    for (unsigned int letter_index = 0; letter_index < msg.length; letter_index++) {
        unsigned char c = msg.string[letter_index];
        unsigned int x_offset = start_x + (CHAR_WIDTH * letter_index);

        for (unsigned int byte_index = 0; byte_index < FONT_BASE_SIZE; byte_index++) {
            uint8_t byte_value = FONT[c][byte_index];
            for (unsigned int bit = 0; bit < 8; bit++) {
                if (byte_value & (1 << (7 - bit))) {
                    for (unsigned int sy = 0; sy < scale; sy++) {
                        for (unsigned int sx = 0; sx < scale; sx++) {

                            unsigned int px = x_offset + (scale * bit) + sx;
                            unsigned int py = start_y + (scale * byte_index) + sy;
                            if (px < WIDTH && py < HEIGHT) {
                                memset(&pixel_buf[py * WIDTH + px], 0, sizeof(uint32_t));
                                pixel_buf[py * WIDTH + px] = 0xFFFFFFFF;
                            }
                        }
                    }
                }
            }
        }
    }
}

void overlay_message_ultimate(uint32_t *dest, const uint32_t *src, const unsigned int text_w, const unsigned int text_h, const signed int displacement_x, const signed int displacement_y) {
        /*Less than*/                                                           /*More than*/
    if (displacement_x < -(WIDTH / 2) || displacement_y < -(HEIGHT / 2) || displacement_x > (WIDTH / 2) || displacement_y > (HEIGHT / 2)) 
    {
        fprintf(stderr ,"The Displacement is out of range: `displacement_x`=%i, `displacement_y`=%i. From: `overlay_message_ultimate` function", displacement_x, displacement_y);
        return;
    }

    unsigned int start_x = ((WIDTH - text_w) / 2) + displacement_x;
    unsigned int start_y = ((HEIGHT - text_h) / 2) + displacement_y;

    for (unsigned int y = 0; y < text_h; y++){
        unsigned int raw_offset = (start_y + y) * WIDTH + start_x;
        unsigned int src_offset = (start_y + y) * WIDTH + start_x;
        for (unsigned int x = 0; x < text_w; x++) {
            uint32_t p_src = src[src_offset + x];
            uint32_t p_dst = dest[raw_offset + x];

            uint32_t mask = (p_src!=0) ? 0xFFFFFFFF : 0x00000000;
            dest[raw_offset + x] = (p_dst & ~mask) | (p_src & mask);
        }
    }
}

void render(const struct app_state *app) {

	uint32_t *activate_pixels = app->buffers[app->current_buffer_idx].pixels;
    draw_rect(activate_pixels, 0, 0, WIDTH, HEIGHT, COLOR_BG); // Draw the Back ground
    draw_rect(activate_pixels, 0, 0, WIDTH, SCORE_AREA_H, K); // Clear Score bar

    if (app->snake_shape == WORM_SHAPE) {
        // Draw Food
        draw_sprite(activate_pixels, app->food.x * CELL_SIZE, app->food.y * CELL_SIZE, ICON_SIZE, ICON_SIZE, (const uint32_t *)APPLE_SPRITE); // Draw the apple
        // Draw Head
        switch (app->dir) {
            case DIR_UP: 	draw_sprite(activate_pixels, app->snake[0].x * CELL_SIZE, app->snake[0].y * CELL_SIZE, ICON_SIZE, ICON_SIZE, (const uint32_t *)WORM_UP); break;
            case DIR_DOWN:	draw_sprite(activate_pixels, app->snake[0].x * CELL_SIZE, app->snake[0].y * CELL_SIZE, ICON_SIZE, ICON_SIZE, (const uint32_t *)WORM_DOWN); break;
            case DIR_RIGHT: draw_sprite(activate_pixels, app->snake[0].x * CELL_SIZE, app->snake[0].y * CELL_SIZE, ICON_SIZE, ICON_SIZE, (const uint32_t *)WORM_RIGHT); break;
            case DIR_LEFT: 	draw_sprite(activate_pixels, app->snake[0].x * CELL_SIZE, app->snake[0].y * CELL_SIZE, ICON_SIZE, ICON_SIZE, (const uint32_t *)WORM_LEFT); break;
        }
        // Draw Body
        for (unsigned int i = 1; i < app->snake_len; i++) {     
                unsigned int center_x = app->snake[i].x * CELL_SIZE + (CELL_SIZE / 2);
                unsigned int center_y = app->snake[i].y * CELL_SIZE + (CELL_SIZE / 2);
                unsigned int radius = (CELL_SIZE / 2) - 1;
                draw_circle(activate_pixels, center_x, center_y, radius, G);
        }
    }
    else if (app->snake_shape == SNAKE_SHAPE) {
        // Draw food
        draw_sprite(activate_pixels, app->food.x * CELL_SIZE, app->food.y * CELL_SIZE, ICON_SIZE, ICON_SIZE, (const uint32_t *)MOUSE_SPRITE);
        // Draw Head
        switch (app->dir)
                {
                    case DIR_UP: 	draw_sprite(activate_pixels, app->snake[0].x * CELL_SIZE, app->snake[0].y * CELL_SIZE, ICON_SIZE, ICON_SIZE, (const uint32_t *)SNAKE_UP); break;
                    case DIR_DOWN:	draw_sprite(activate_pixels, app->snake[0].x * CELL_SIZE, app->snake[0].y * CELL_SIZE, ICON_SIZE, ICON_SIZE, (const uint32_t *)SNAKE_DOWN); break;
                    case DIR_RIGHT: draw_sprite(activate_pixels, app->snake[0].x * CELL_SIZE, app->snake[0].y * CELL_SIZE, ICON_SIZE, ICON_SIZE, (const uint32_t *)SNAKE_RIGHT); break;
                    case DIR_LEFT: 	draw_sprite(activate_pixels, app->snake[0].x * CELL_SIZE, app->snake[0].y * CELL_SIZE, ICON_SIZE, ICON_SIZE, (const uint32_t *)SNAKE_LEFT); break;
                }
        // Draw Body
        for (unsigned int i = 1; i < app->snake_len; i++) {
                int center_x = app->snake[i].x * CELL_SIZE + (CELL_SIZE / 2);
                int center_y = app->snake[i].y * CELL_SIZE + (CELL_SIZE / 2);
                int radius = (CELL_SIZE / 2) - 2;
                draw_circle(activate_pixels, center_x, center_y, radius, G);
        }
    }
    else if (app->snake_shape == SQUARE_SHAPE) {
        // Draw food
        draw_rect(activate_pixels, app->food.x * CELL_SIZE, app->food.y * CELL_SIZE, CELL_SIZE, CELL_SIZE, R);
        // Draw Head
        draw_rect(activate_pixels, app->snake[0].x * CELL_SIZE, app->snake[0].y * CELL_SIZE, CELL_SIZE, CELL_SIZE, CR_SNAKE_HEAD);
        // Draw Body
        for (unsigned int i = 1; i < app->snake_len; i++) {
            draw_rect(activate_pixels, app->snake[i].x * CELL_SIZE, app->snake[i].y * CELL_SIZE, CELL_SIZE, CELL_SIZE, CR_SNAKE_BODY);
        }
    }
    // Draw Score
    draw_score(app->session_score, activate_pixels, SCORE_INT_X, MIDDLE_SCR_ARE, TEXT_SCALE); // Draw session score
    draw_score(app->high_score, activate_pixels, HIG_SCR_INT_X, MIDDLE_SCR_ARE, TEXT_SCALE); // Draw High score
    overlay_message_ultimate(activate_pixels, app->score_msg.msg_buf, app->score_msg.pix_wid, app->score_msg.pix_hig, SCORE_MSG_X, MIDDLE_SCR_ARE);
    overlay_message_ultimate(activate_pixels, app->high_score_msg.msg_buf, app->high_score_msg.pix_wid, app->high_score_msg.pix_hig, HIG_SCR_MSG_X, MIDDLE_SCR_ARE);
}

static const uint32_t WORM_UP[ICON_SIZE][ICON_SIZE] = {
    {_, _, _, _, _, _, _, _, _, R, _, _, _, _, _, R, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, R, R, _, R, R, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, R, R, R, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, _, R, _, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, _, R, _, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _, _, _, _},
    {_, _, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _, _},
    {_, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _},
    {_, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _},
    {_, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _},
    {_, G, G, G, B, B, B, B, G, G, G, G, G, G, G, G, G, B, B, B, B, G, G, G, _},
    {_, G, G, G, B, BL, BL, B, G, G, G, G, G, G, G, G, G, B, BL, BL, B,G,G,G,_},
    {_, G, G, G, B, BL, BL, B, G, G, G, G, G, G, G, G, G, B, BL, BL, B,G,G,G,_},
    {_, G, G, G, B, B, B, B, G, G, G, G, G, G, G, G, G, B, B, B, B, G, G, G, _},
    {_, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _},
    {_, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _},
    {_, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _},
    {_, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _},
    {_, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _},
    {_, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _},
    {_, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _},
    {_, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _},
    {_, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _},
    {_, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _},
    {_, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _}
};

static const uint32_t WORM_DOWN[ICON_SIZE][ICON_SIZE] = {
    {_, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _},
    {_, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _},
    {_, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _},
    {_, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _},
    {_, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _},
    {_, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _},
    {_, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _},
    {_, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _},
    {_, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _},
    {_, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _},
    {_, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _},
    {_, G, G, G, B, B, B, B, G, G, G, G, G, G, G, G, G, B, B, B, B, G, G, G, _},
    {_, G, G, G, B, BL, BL, B, G, G, G, G, G, G, G, G, G, B, BL, BL, B, G, G, G, _},
    {_, G, G, G, B, BL, BL, B, G, G, G, G, G, G, G, G, G, B, BL, BL, B, G, G, G, _},
    {_, G, G, G, B, B, B, B, G, G, G, G, G, G, G, G, G, B, B, B, B, G, G, G, _},
    {_, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _},
    {_, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _},
    {_, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _},
    {_, _, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _, _},
    {_, _, _, _, _, _, _, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, _, R, _, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, _, R, _, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, R, R, R, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, R, R, _, R, R, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, R, _, _, _, _, _, R, _, _, _, _, _, _, _, _, _}
};

static const uint32_t WORM_RIGHT[ICON_SIZE][ICON_SIZE] = {
    {_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _, _, _, _},
    {_, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _, _, _},
    {_, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _, _},
    {G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _},
    {G, G, G, G, G, G, G, G, G, G, G, G, G, B, B, B, B, G, G, G, G, _, _, _, _},
    {G, G, G, G, G, G, G, G, G, G, G, G, G, B, BL, BL, B, G, G, G, G, _, _, _, _},
    {G, G, G, G, G, G, G, G, G, G, G, G, G, B, BL, BL, B, G, G, G, G, _, _, _, _},
    {G, G, G, G, G, G, G, G, G, G, G, G, G, B, B, B, B, G, G, G, G, _, _, _, _},
    {G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, R},
    {G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, R, R},
    {G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, R, R, R, _},
    {G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, R, R},
    {G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, R},
    {G, G, G, G, G, G, G, G, G, G, G, G, G, B, B, B, B, G, G, G, G, _, _, _, _},
    {G, G, G, G, G, G, G, G, G, G, G, G, G, B, BL, BL, B, G, G, G, G, _, _, _, _},
    {G, G, G, G, G, G, G, G, G, G, G, G, G, B, BL, BL, B, G, G, G, G, _, _, _, _},
    {G, G, G, G, G, G, G, G, G, G, G, G, G, B, B, B, B, G, G, G, G, _, _, _, _},
    {G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _},
    {_, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _, _},
    {_, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _, _, _},
    {_, _, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _}
};

static const uint32_t WORM_LEFT[ICON_SIZE][ICON_SIZE] = {
    {_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _, _},
    {_, _, _, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _},
    {_, _, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _},
    {_, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G},
    {_, _, _, _, G, G, G, G, B, B, B, B, G, G, G, G, G, G, G, G, G, G, G, G, G},
    {_, _, _, _, G, G, G, G, B, BL, BL, B, G, G, G, G, G, G, G, G, G, G, G, G, G},
    {_, _, _, _, G, G, G, G, B, BL, BL, B, G, G, G, G, G, G, G, G, G, G, G, G, G},
    {_, _, _, _, G, G, G, G, B, B, B, B, G, G, G, G, G, G, G, G, G, G, G, G, G},
    {R, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G},
    {R, R, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G},
    {_, R, R, R, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G},
    {R, R, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G},
    {R, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G},
    {_, _, _, _, G, G, G, G, B, B, B, B, G, G, G, G, G, G, G, G, G, G, G, G, G},
    {_, _, _, _, G, G, G, G, B, BL, BL, B, G, G, G, G, G, G, G, G, G, G, G, G, G},
    {_, _, _, _, G, G, G, G, B, BL, BL, B, G, G, G, G, G, G, G, G, G, G, G, G, G},
    {_, _, _, _, G, G, G, G, B, B, B, B, G, G, G, G, G, G, G, G, G, G, G, G, G},
    {_, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G},
    {_, _, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _},
    {_, _, _, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _},
    {_, _, _, _, _, _, _, G, G, G, G, G, G, G, G, G, G, G, G, G, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _}
};
static const uint32_t APPLE_SPRITE[ICON_SIZE][ICON_SIZE] = {
    {_, _, _, _, _, _, _, _, G, G, G, G, B, B, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, G, G, G, G, G, G, B, B, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, G, G, G, G, G, G, G, _, _, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, G, G, G, G, G, G, G, _, _, _, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, G, G, G, G, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, R, R, R, R, R, _, _, _, _, R, R, R, R, R, _, _, _, _, _, _, _, _},
    {_, _, R, R, R, R, R, R, R, _, _, R, R, R, R, R, R, R, _, _, _, _, _, _, _},
    {_, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, _, _, _, _, _, _},
    {_, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, _, _, _, _, _},
    {R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, _, _, _, _},
    {R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, _, _, _, _},
    {R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, _, _, _, _},
    {R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, _, _, _, _},
    {R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, _, _, _, _},
    {R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, _, _, _, _},
    {_, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, _, _, _, _, _},
    {_, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, _, _, _, _, _, _},
    {_, _, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, R, _, _, _, _, _, _, _},
    {_, _, _, R, R, R, R, R, R, R, R, R, R, R, R, R, R, _, _, _, _, _, _, _, _},
    {_, _, _, _, R, R, R, R, R, _, _, R, R, R, R, R, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, R, R, R, _, _, _, _, R, R, R, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, R, _, _, _, _, _, _, R, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _}
};

// -------------------------------------------------------------
static const uint32_t SNAKE_UP[ICON_SIZE][ICON_SIZE] = {
    { _, _, _, _, _, _, _, _, _, _, T, _, _, _, T, _, _, _, _, _, _, _, _, _, _ },
    { _, _, _, _, _, _, _, _, _, _, T, _, _, _, T, _, _, _, _, _, _, _, _, _, _ },
    { _, _, _, _, _, _, _, _, _, _, _, T, T, T, _, _, _, _, _, _, _, _, _, _, _ },
    { _, _, _, _, _, _, _, _, _, _, _, _, T, _, _, _, _, _, _, _, _, _, _, _, _ },
    { _, _, _, _, _, _, _, _, _, D, D, D, D, D, D, D, _, _, _, _, _, _, _, _, _ },
    { _, _, _, _, _, _, _, D, H, H, H, H, H, H, H, H, H, D, _, _, _, _, _, _, _ },
    { _, _, _, _, _, D, H, H, H, H, M, M, K, M, M, H, H, H, H, D, _, _, _, _, _ },
    { _, _, _, _, D, H, H, H, M, M, M, K, K, K, M, M, M, H, H, H, D, _, _, _, _ },
    { _, _, _, D, H, H, H, M, M, K, M, M, K, M, M, K, M, M, H, H, H, D, _, _, _ },
    { _, _, D, H, H, H, M, M, K, M, M, K, K, K, M, M, K, M, M, H, H, H, D, _, _ },
    { _, D, H, H, M, E, E, E, B, M, M, M, K, M, M, M, B, E, E, E, M, H, H, D, _ },
    { D, H, M, M, E, B, B, B, M, M, K, K, K, K, K, M, M, B, B, B, E, M, M, H, D },
    { D, H, H, M, M, M, B, B, M, M, M, M, K, M, M, M, M, B, B, M, M, M, H, H, D },
    { D, H, H, M, M, M, M, K, K, M, M, K, K, K, M, M, K, K, M, M, M, M, H, H, D },
    { _, D, H, M, M, M, K, K, M, M, K, K, M, K, K, M, M, K, K, M, M, M, H, D, _ },
    { _, _, D, S, M, M, K, M, M, M, K, K, K, K, K, M, M, M, K, M, M, S, D, _, _ },
    { _, _, _, D, S, S, M, M, K, K, M, M, K, M, M, K, K, M, M, S, S, D, _, _, _ },
    { _, _, _, D, D, S, M, M, M, M, K, K, K, K, K, M, M, M, M, S, D, _, _, _, _ },
    { _, _, _, D, D, S, M, M, M, M, K, M, M, M, K, M, M, M, M, S, D, _, _, _, _ },
    { _, _, _, D, D, S, M, M, M, M, M, M, M, M, M, M, M, M, M, S, D, _, _, _, _ },
    { _, _, _, D, D, S, M, M, M, M, M, M, M, M, M, M, M, M, M, S, D, _, _, _, _ },
    { _, _, _, D, D, S, S, M, M, M, M, M, M, M, M, M, M, M, S, S, D, _, _, _, _ },
    { _, _, _, D, D, S, S, M, M, M, M, M, M, M, M, M, M, M, S, S, D, _, _, _, _ },
    { _, _, _, D, D, S, S, M, M, M, M, M, M, M, M, M, M, M, S, S, D, _, _, _, _ },
    { _, _, _, D, D, S, S, M, M, M, M, M, M, M, M, M, M, M, S, S, D, _, _, _, _ }
};

static const uint32_t SNAKE_DOWN[ICON_SIZE][ICON_SIZE] = {
    { _, _, _, D, D, S, S, M, M, M, M, M, M, M, M, M, M, M, S, S, D, D, _, _, _ },
    { _, _, _, D, D, S, S, M, M, M, M, M, M, M, M, M, M, M, S, S, D, D, _, _, _ },
    { _, _, _, D, D, S, S, M, M, M, M, M, M, M, M, M, M, M, S, S, D, D, _, _, _ },
    { _, _, _, D, D, S, S, M, M, M, M, M, M, M, M, M, M, M, S, S, D, D, _, _, _ },
    { _, _, _, D, D, S, M, M, M, M, M, M, M, M, M, M, M, M, M, S, D, D, _, _, _ },
    { _, _, _, D, D, S, M, M, M, M, M, M, M, M, M, M, M, M, M, S, D, D, _, _, _ },
    { _, _, _, D, D, S, M, M, M, M, K, M, M, M, K, M, M, M, M, S, D, D, _, _, _ },
    { _, _, _, D, D, S, M, M, M, M, K, K, K, K, K, M, M, M, M, S, D, D, _, _, _ },
    { _, _, _, D, S, S, M, M, K, K, M, M, K, M, M, K, K, M, M, S, S, D, _, _, _ },
    { _, _, D, S, M, M, K, M, M, M, K, K, K, K, K, M, M, M, K, M, M, S, D, _, _ },
    { _, D, H, M, M, M, K, K, M, M, K, K, M, K, K, M, M, K, K, M, M, M, H, D, _ },
    { D, H, H, M, M, M, M, K, K, M, M, K, K, K, M, M, K, K, M, M, M, M, H, H, D },
    { D, H, H, M, M, M, B, B, M, M, M, M, K, M, M, M, M, B, B, M, M, M, H, H, D },
    { D, H, M, M, E, B, B, B, M, M, K, K, K, K, K, M, M, B, B, B, E, M, M, H, D },
    { _, D, H, H, M, E, E, E, B, M, M, M, K, M, M, M, B, E, E, E, M, H, H, D, _ },
    { _, _, D, H, H, H, M, M, K, M, M, K, K, K, M, M, K, M, M, H, H, H, D, _, _ },
    { _, _, _, D, H, H, H, M, M, K, M, M, K, M, M, K, M, M, H, H, H, D, _, _, _ },
    { _, _, _, _, D, H, H, H, M, M, M, K, K, K, M, M, M, H, H, H, D, _, _, _, _ },
    { _, _, _, _, _, D, H, H, H, H, M, M, K, M, M, H, H, H, H, D, _, _, _, _, _ },
    { _, _, _, _, _, _, _, D, H, H, H, H, H, H, H, H, H, D, _, _, _, _, _, _, _ },
    { _, _, _, _, _, _, _, _, _, D, D, D, D, D, D, D, _, _, _, _, _, _, _, _, _ },
    { _, _, _, _, _, _, _, _, _, _, _, _, T, _, _, _, _, _, _, _, _, _, _, _, _ },
    { _, _, _, _, _, _, _, _, _, _, _, T, T, T, _, _, _, _, _, _, _, _, _, _, _ },
    { _, _, _, _, _, _, _, _, _, _, T, _, _, _, T, _, _, _, _, _, _, _, _, _, _ },
    { _, _, _, _, _, _, _, _, _, _, T, _, _, _, T, _, _, _, _, _, _, _, _, _, _ }
};

static const uint32_t SNAKE_LEFT[ICON_SIZE][ICON_SIZE] = {
  { _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _ },
  { _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, D, D, D, D, D, D, D, D, D, D },
  { _, _, _, _, _, _, _, _, _, _, _, _, _, D, D, H, H, S, S, S, S, S, S, S, S },
  { _, _, _, _, _, _, _, _, _, _, _, D, D, H, H, H, H, S, S, S, S, S, S, S, S },
  { _, _, _, _, _, _, _, _, _, D, D, H, H, M, M, M, M, S, S, S, S, S, S, S, S },
  { _, _, _, _, _, _, _, D, D, H, H, M, M, M, K, M, M, M, M, M, M, M, M, M, M },
  { _, _, _, _, _, D, D, H, H, M, M, E, E, M, M, K, K, M, M, M, M, M, M, M, M },
  { _, _, _, _, D, H, H, M, M, K, E, E, B, B, M, M, M, K, M, M, M, M, M, M, M },
  { _, _, _, D, H, H, M, M, D, M, E, B, B, M, M, K, K, K, K, K, K, K, K, K, K },
  { _, _, D, H, H, H, M, M, M, K, B, B, M, K, K, M, M, M, M, K, M, M, M, M, M },
  { T, T, _, D, H, M, M, M, M, M, M, K, M, K, K, K, K, K, K, K, M, M, M, M, M },
  { _, T, T, D, H, K, M, M, M, K, K, K, K, K, K, M, M, K, K, M, M, M, M, M, M },
  { T, _, T, D, D, H, H, M, M, M, M, K, M, K, K, K, K, K, K, K, M, M, M, M, M },
  { _, T, T, D, H, K, M, M, M, K, K, K, K, K, K, M, M, K, K, M, M, M, M, M, M },
  { T, T, _, D, H, M, M, M, M, M, M, K, M, K, K, K, K, K, K, K, M, M, M, M, M },
  { _, _, D, H, H, H, M, M, M, K, B, B, M, K, K, M, M, M, M, K, M, M, M, M, M },
  { _, _, _, D, H, H, M, M, D, M, E, B, B, M, M, K, K, K, K, K, K, K, K, K, K },
  { _, _, _, _, D, H, H, M, M, K, E, E, B, B, M, M, M, K, M, M, M, M, M, M, M },
  { _, _, _, _, _, D, D, H, H, M, M, E, E, M, M, K, K, M, M, M, M, M, M, M, M },
  { _, _, _, _, _, _, _, D, D, H, H, M, M, M, K, M, M, M, M, M, M, M, M, M, M },
  { _, _, _, _, _, _, _, _, _, D, D, H, H, M, M, M, M, S, S, S, S, S, S, S, S },
  { _, _, _, _, _, _, _, _, _, _, _, D, D, H, H, H, H, S, S, S, S, S, S, S, S },
  { _, _, _, _, _, _, _, _, _, _, _, _, _, D, D, H, H, S, S, S, S, S, S, S, S },
  { _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, D, D, D, D, D, D, D, D, D, D },
  { _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _ }
};

static const uint32_t SNAKE_RIGHT[ICON_SIZE][ICON_SIZE] = {
  { _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _ },
  { D, D, D, D, D, D, D, D, D, D, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _ },
  { S, S, S, S, S, S, S, S, H, H, D, D, _, _, _, _, _, _, _, _, _, _, _, _, _ },
  { S, S, S, S, S, S, S, S, H, H, H, H, D, D, _, _, _, _, _, _, _, _, _, _, _ },
  { S, S, S, S, S, S, S, S, M, M, M, M, H, H, D, D, _, _, _, _, _, _, _, _, _ },
  { M, M, M, M, M, M, M, M, M, K, M, M, M, H, H, D, D, _, _, _, _, _, _, _, _ },
  { M, M, M, M, M, M, M, M, K, K, M, M, E, E, M, M, H, H, D, D, _, _, _, _, _ },
  { M, M, M, M, M, M, M, K, M, M, M, B, B, E, E, K, M, M, H, H, D, _, _, _, _ },
  { K, K, K, K, K, K, K, K, K, K, M, M, B, B, E, M, D, M, M, H, H, D, _, _, _ },
  { M, M, M, M, M, K, M, M, M, M, K, K, M, B, B, K, M, M, M, H, H, H, D, _, _ },
  { M, M, M, M, M, K, K, K, K, K, K, K, M, K, M, M, M, M, M, M, H, D, _, T, T },
  { M, M, M, M, M, M, M, K, K, M, M, K, K, K, K, K, K, M, M, M, K, H, D, T, T },
  { M, M, M, M, M, K, K, K, K, K, K, K, M, K, M, M, M, M, H, H, D, D, T, _, T },
  { M, M, M, M, M, M, M, K, K, M, M, K, K, K, K, K, K, M, M, M, K, H, D, T, T },
  { M, M, M, M, M, K, K, K, K, K, K, K, M, K, M, M, M, M, M, M, H, D, _, T, T },
  { M, M, M, M, M, K, M, M, M, M, K, K, M, B, B, K, M, M, M, H, H, H, D, _, _ },
  { K, K, K, K, K, K, K, K, K, K, M, M, B, B, E, M, D, M, M, H, H, D, _, _, _ },
  { M, M, M, M, M, M, M, K, M, M, M, B, B, E, E, K, M, M, H, H, D, _, _, _, _ },
  { M, M, M, M, M, M, M, M, K, K, M, M, E, E, M, M, H, H, D, D, _, _, _, _, _ },
  { M, M, M, M, M, M, M, M, M, K, M, M, M, H, H, D, D, _, _, _, _, _, _, _, _ },
  { S, S, S, S, S, S, S, S, M, M, M, M, H, H, D, D, _, _, _, _, _, _, _, _, _ },
  { S, S, S, S, S, S, S, S, H, H, H, H, D, D, _, _, _, _, _, _, _, _, _, _, _ },
  { S, S, S, S, S, S, S, S, H, H, D, D, _, _, _, _, _, _, _, _, _, _, _, _, _ },
  { D, D, D, D, D, D, D, D, D, D, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _ },
  { _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _ }
};

static const uint32_t MOUSE_SPRITE[ICON_SIZE][ICON_SIZE] = {
    {_, _, _, _, _, DG,  DG,  DG,  DG,  DG,  _, _, _, _, _, DG,  DG,  DG,  DG,  DG,  _, _, _, _, _},
    {_, _, _, _, DG,DG,DG,DG,DG,DG,DG,_, _, _, DG,DG,DG,DG,DG,DG,DG,_, _, _, _},
    {_, _, _, _, DG,DG,DG,PI,DG,DG,DG,_, _, _, DG,DG,DG,PI,DG,DG,DG,_, _, _, _},
    {_, _, _, _, DG,DG,DG,DG,DG,DG,DG,_, _, _, DG,DG,DG,DG,DG,DG,DG,_, _, _, _},
    {_, _, _, _, _, DG,DG,DG,DG,DG,_, _, _, _, _, DG,DG,DG,DG,DG,_, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, DG,DG,DG,DG,DG,DG,DG,DG,DG,_, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,_, _, _, _, _, _},
    {_, _, _, _, _, DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,_, _, _, _, _},
    {_, _, _, _, _, DG,DG,DG,DG,B, DG,DG,DG,DG,DG,B, DG,DG,DG,DG,_, _, _, _, _},
    {_, _, _, _, DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,_, _, _, _},
    {_, _, _, _, DG,DG,DG,DG,DG,DG,DG,PI,PI,PI,DG,DG,DG,DG,DG,DG,DG,_, _, _, _},
    {_, _, _, DG,DG,DG,DG,DG,DG,DG,DG,PI,PI,PI,DG,DG,DG,DG,DG,DG,DG,DG,_, _, _},
    {_, _, _, DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,_, _, _},
    {_, _, _, DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,_, _, _},
    {_, _, _, DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG, _, _,_},
    {_, _, _, DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG, DG,_,_},
    {_, _, _, _, DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,_, _,DG,DG},
    {_, _, _, _, DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,_, _,_, DG},
    {_, _, _, _, _, DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,_, _, _,DG,DG},
    {_, _, _, _, _, DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,_, _, DG,DG,_},
    {_, _, _, _, _, _, DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,_, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, DG,PI,PI,DG,DG,DG,PI,PI,DG,_, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _},
    {_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _}
};

#undef COLOR_BG
#undef CR_SNAKE_HEAD
#undef CR_SNAKE_BODY
#undef _
#undef R
#undef G
#undef B
#undef BL
#undef GR
#undef DG
#undef PI  
#undef D
#undef S
#undef M
#undef H
#undef K
#undef E
#undef T
#undef ICON_SIZE


static const uint8_t FONT[128][FONT_BASE_SIZE] = {
// --- (Uppercase A-Z) ---
    ['A'] = {0x18, 0x3c, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x00},
    ['B'] = {0x7c, 0x66, 0x66, 0x7c, 0x66, 0x66, 0x7c, 0x00},
    ['C'] = {0x3c, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3c, 0x00},
    ['D'] = {0x78, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0x78, 0x00},
    ['E'] = {0x7e, 0x60, 0x60, 0x78, 0x60, 0x60, 0x7e, 0x00},
    ['F'] = {0x7e, 0x60, 0x60, 0x78, 0x60, 0x60, 0x60, 0x00},
    ['G'] = {0x3c, 0x66, 0x60, 0x6e, 0x66, 0x66, 0x3e, 0x00},
    ['H'] = {0x66, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00},
    ['I'] = {0x3c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00},
    ['J'] = {0x1e, 0x0c, 0x0c, 0x0c, 0x0c, 0x6c, 0x38, 0x00},
    ['K'] = {0x66, 0x6c, 0x78, 0x70, 0x78, 0x6c, 0x66, 0x00},
    ['L'] = {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7e, 0x00},
    ['M'] = {0xc6, 0xee, 0xfe, 0xd6, 0xc6, 0xc6, 0xc6, 0x00},
    ['N'] = {0x66, 0x76, 0x7e, 0x7e, 0x6e, 0x66, 0x66, 0x00},
    ['O'] = {0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00},
    ['P'] = {0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 0x60, 0x00},
    ['Q'] = {0x3c, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x0f, 0x00},
    ['R'] = {0x7c, 0x66, 0x66, 0x7c, 0x6c, 0x66, 0x66, 0x00},
    ['S'] = {0x3c, 0x66, 0x60, 0x3c, 0x06, 0x66, 0x3c, 0x00},
    ['T'] = {0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
    ['U'] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00},
    ['V'] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00},
    ['W'] = {0x81, 0x81, 0x81, 0x81, 0x99, 0x99, 0x66, 0x00},
    ['X'] = {0x66, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x66, 0x00},
    ['Y'] = {0x66, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x00},
    ['Z'] = {0x7e, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x7e, 0x00},

    // --- (Lowercase a-z) ---
    ['a'] = {0x00, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x76, 0x00},
    ['b'] = {0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x7c, 0x00},
    ['c'] = {0x00, 0x00, 0x3c, 0x66, 0x60, 0x66, 0x3c, 0x00},
    ['d'] = {0x0c, 0x0c, 0x0c, 0x3c, 0x6c, 0x6c, 0x3e, 0x00},
    ['e'] = {0x00, 0x00, 0x3c, 0x66, 0x7e, 0x60, 0x3c, 0x00},
    ['f'] = {0x1c, 0x30, 0x30, 0x7c, 0x30, 0x30, 0x30, 0x00},
    ['g'] = {0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x3c},
    ['h'] = {0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00},
    ['i'] = {0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3c, 0x00},
    ['j'] = {0x0c, 0x00, 0x1c, 0x0c, 0x0c, 0x0c, 0x0c, 0x38},
    ['k'] = {0x60, 0x60, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0x00},
    ['l'] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x1c, 0x00},
    ['m'] = {0x00, 0x00, 0xfe, 0x92, 0x92, 0x92, 0x92, 0x00},
    ['n'] = {0x00, 0x00, 0xdc, 0x66, 0x66, 0x66, 0x66, 0x00},
    ['o'] = {0x00, 0x00, 0x3c, 0x66, 0x66, 0x66, 0x3c, 0x00},
    ['p'] = {0x00, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60},
    ['q'] = {0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x06},
    ['r'] = {0x00, 0x00, 0x5c, 0x66, 0x60, 0x60, 0x60, 0x00},
    ['s'] = {0x00, 0x00, 0x3e, 0x60, 0x3c, 0x06, 0x7c, 0x00},
    ['t'] = {0x30, 0x30, 0x7c, 0x30, 0x30, 0x30, 0x1c, 0x00},
    ['u'] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3e, 0x00},
    ['v'] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00},
    ['w'] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x7e, 0x3c, 0x00},
    ['x'] = {0x00, 0x00, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x00},
    ['y'] = {0x00, 0x00, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8, 0x00},
    ['z'] = {0x00, 0x00, 0x7e, 0x0c, 0x18, 0x30, 0x7e, 0x00},

    // --- Numbers ---
    ['0'] = {0x3c, 0x66, 0x6e, 0x76, 0x66, 0x66, 0x3c, 0x00},
    ['1'] = {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00},
    ['2'] = {0x3c, 0x66, 0x06, 0x0c, 0x30, 0x60, 0x7e, 0x00},
    ['3'] = {0x3c, 0x66, 0x06, 0x1c, 0x06, 0x66, 0x3c, 0x00},
    ['4'] = {0x0c, 0x1c, 0x3c, 0x6c, 0x7e, 0x0c, 0x0c, 0x00},
    ['5'] = {0x7e, 0x60, 0x7c, 0x06, 0x06, 0x66, 0x3c, 0x00},
    ['6'] = {0x3c, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x3c, 0x00},
    ['7'] = {0x7e, 0x06, 0x0c, 0x18, 0x18, 0x18, 0x18, 0x00},
    ['8'] = {0x3c, 0x66, 0x66, 0x3c, 0x66, 0x66, 0x3c, 0x00},
    ['9'] = {0x3c, 0x66, 0x66, 0x66, 0x3e, 0x06, 0x3c, 0x00},

    // --- (Punctuation) ---
    ['.'] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00},
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    [':'] = {0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00}
};