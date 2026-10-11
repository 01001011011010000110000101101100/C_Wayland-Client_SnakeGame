# Snake Game

## Overview
Snake Game is a native desktop implementation of the classic arcade game written in C. Designed specifically for Wayland display servers, the project emphasizes low-level systems programming, clean architectural separation, and efficient custom software rendering without relying on heavy external graphical toolkits.   

## Architecture and Core Design
The application is structured around a centralized state container (app_state) that manages the entire lifecycle of the application. This includes window management, rendering buffers, game logic, input states, and scoring parameters. The codebase is modularly separated into distinct components handling core game logic, input events, rendering routines, file I/O operations, and Wayland protocol interactions.   

## Wayland and Window Management
The application interfaces directly with the Wayland display server protocol. It registers as an XDG-shell client, utilizing xdg_wm_base, xdg_surface, and xdg_toplevel interfaces to create and manage the application window. Furthermore, it handles window server interactions such as configure events and graceful closure requests. Keyboard inputs are captured through the Wayland seat interface (wl_seat and wl_keyboard).   

## Rendering Pipeline and Memory Management
Rather than utilizing hardware-accelerated graphics APIs, the project implements a custom software rendering pipeline:
- The application allocates a shared memory pool using POSIX shared memory (shm_open and mmap) to facilitate efficient communication with the Wayland compositor.   
- It utilizes a dual-buffer system (NUM_OF_BUFFERS = 2) to prevent visual tearing and ensure smooth updates synchronized via Wayland frame callbacks (wl_surface_frame).   
- The rendering engine features predefined bitmap sprite arrays for different snake shapes and food items alongside a custom built-in font renderer for on-screen text and score displays.   

## Game State and Logic Design
The game mechanics are governed by discrete time intervals and state transitions:
- The playfield is modeled on a discrete grid where the snake's body segments are tracked as coordinate points, bounded by map limits and self-collision detection rules.   
- As the snake consumes food items, the session score increases, and the tick interval dynamically scales to increase game speed.   
- Players can choose between multiple visual shapes (WORM_SHAPE, SNAKE_SHAPE, and SQUARE_SHAPE) which alter the rendered sprites and drawing primitives.   

## Event Loop and Timing
The main execution loop relies on POSIX poll() to multiplex events efficiently between the Wayland display file descriptor and a monotonic timer file descriptor (timerfd). This architecture ensures precise game tick timing (update_game) and responsive event dispatching without busy-waiting.   

## Persistence Layer
High scores are managed through a dedicated I/O module that interacts with the local file system. Scores are read from and written to a designated local data path relative to the user home directory, ensuring that high scores persist across gaming sessions.
