#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <wayland-client-protocol.h>

#include "game_types.h"
#include "wayland_core.h"
#include "game.h"
#include "IO.h"
#include "render.h"

int main(void)
{
    struct app_state app = {0};

    uint32_t *msgs_buf = (uint32_t *) malloc(BUFFER_SIZE * 3);
    if (!init_wayland(&app) || !msgs_buf) {
        fprintf(stderr, "Failed to allocate a Buffer for messages. Or connect with Wayland display\n");
        cleanup_wayland(&app);
        return -1;
    }

    // Init messages
    uint32_t buf[BUFFER_SIZE];
    app.start_msg = create_message(msgs_buf, START_MESSAGE);
    app.end_msg = create_message(msgs_buf + (BUFFER_SIZE / sizeof(uint32_t)), END_MESSAGE);
    app.score_msg = create_message(msgs_buf + ((BUFFER_SIZE * 2) / sizeof(uint32_t)), SCORE_TEXT);//-----------|
    app.high_score_msg = create_message(msgs_buf + ((BUFFER_SIZE * 2) / sizeof(uint32_t)), HTH_SCORE_TEXT);//--|--> Use the same buffer
    app.pause_msg = create_message(msgs_buf + ((BUFFER_SIZE * 2) / sizeof(uint32_t)), PAUSE_TEXT);//-----------|

    draw_message(&app.start_msg, 0, 0, TEXT_SCALE);
    draw_message(&app.end_msg, 0, 0, TEXT_SCALE);
    draw_message(&app.score_msg, SCORE_MSG_X, MIDDLE_SCR_ARE, TEXT_SCALE);
    draw_message(&app.high_score_msg, HIG_SCR_MSG_X, MIDDLE_SCR_ARE, TEXT_SCALE);
    draw_message(&app.pause_msg, 0, 0, TEXT_SCALE);

    // Init game session
    app.running = true;
    app.refresh = true;
    app.snake_shape = WORM_SHAPE;
    app.high_score = load_high_score(RELATIVE_PATH);

    // Poll
    int wayland_fd = wl_display_get_fd(app.wl_display);
    app.timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if(app.timer_fd < 0) {
        fprintf(stderr, "Failed to create timerfd\n");
        cleanup_wayland(&app); free(msgs_buf);
        return -1;
    }

    reset_snake(&app);
    struct pollfd fds[2] = {
        {.fd = wayland_fd, .events = POLLIN},
        {.fd = app.timer_fd, .events = POLLIN}
    };

    while (app.running) {

        wl_display_flush(app.wl_display);

        int ret = poll(fds, 2, -1);
        if (ret > 0) {

            if (fds[0].revents & POLLIN) {
                wl_display_dispatch(app.wl_display);
            }
            if (fds[1].revents & POLLIN) {
                uint64_t expirations;
                read(app.timer_fd, &expirations, sizeof(expirations));

                if (app.game_started && !app.game_over) {
                    update_game(&app);
                    app.refresh = true;
                }
            }
        }

        if (app.game_over) {
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);

            if (now.tv_sec > app.game_over_until.tv_sec || (now.tv_sec == app.game_over_until.tv_sec && now.tv_nsec >= app.game_over_until.tv_nsec)) {
                app.game_over = false;
                app.game_started = false;
                app.refresh = true;
                reset_snake(&app);
                frame_handler(&app, NULL, 0);
            }
        }

        if (app.refresh && !app.frame_callback) {
            frame_handler(&app, NULL, 0);
        }
    }
    save_high_score(RELATIVE_PATH, app.high_score);
    // Clean Up
    cleanup_wayland(&app);
    free(msgs_buf);
    if (close(app.timer_fd) != 0) {fprintf(stderr, "Failed to close timerfd file\n"); return -1;}
    return 0;
}