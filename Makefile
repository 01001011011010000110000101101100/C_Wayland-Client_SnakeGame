CC = gcc
CFLAGS = $(shell pkg-config --cflags wayland-client) -D_POSIX_C_SOURCE=200112L
LIBS = $(shell pkg-config --libs wayland-client) -lrt -lm

TARGET = snake_game
SRCS = main.c game.c input.c render.c wayland_core.c IO.c xdg-shell-protocol.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) -O3 $(SRCS) -o $(TARGET) $(CFLAGS) $(LIBS)


clean:
	rm -f $(TARGET)