# Compiler and flags
CC = gcc
AR = ar
MKDIR = mkdir -p
CP = cp -r
RM = rm -rf

# Directories
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include
CLAY_DIR = clay
EXAMPLES_DIR = examples
ASSETS_DIR = $(EXAMPLES_DIR)/assets
COMPONENTS_DIR = $(SRC_DIR)/components
RENDERER_DIR = $(SRC_DIR)/renderer
VENDOR_DIR = vendor
NANOSVG_DIR = $(VENDOR_DIR)/nanosvg/src

# SDL2 specific
SDL_BUILD_DIR = $(BUILD_DIR)/sdl
SDL_FLAGS = $(shell pkg-config --cflags sdl2 SDL2_ttf SDL2_image SDL2_gfx)
SDL_LIBS = $(shell pkg-config --libs sdl2 SDL2_ttf SDL2_image SDL2_gfx)
SDL_DEFINES = -DROCKS_USE_SDL2

# Raylib specific
RAYLIB_BUILD_DIR = $(BUILD_DIR)/raylib
RAYLIB_FLAGS = $(shell pkg-config --cflags raylib)
RAYLIB_LIBS = $(shell pkg-config --libs raylib)
RAYLIB_DEFINES = -DROCKS_USE_RAYLIB

# Common flags
COMMON_FLAGS = -I$(INCLUDE_DIR) -I$(CLAY_DIR) -I$(NANOSVG_DIR) -D_CRT_SECURE_NO_WARNINGS
COMMON_LIBS = -lm -ldl -lpthread

# Source files
MAIN_SRCS = $(wildcard $(SRC_DIR)/*.c)
COMPONENT_SRCS = $(wildcard $(COMPONENTS_DIR)/*.c)
SDL_RENDERER_SRCS = $(wildcard $(RENDERER_DIR)/sdl2_*.c)
RAYLIB_RENDERER_SRCS = $(RENDERER_DIR)/raylib_renderer.c

# Example files
EXAMPLES = hello_world image_viewer scroll_container text_input dropdown modal grid svg_viewer

# Object files
SDL_OBJS = $(MAIN_SRCS:$(SRC_DIR)/%.c=$(SDL_BUILD_DIR)/%.o) \
           $(COMPONENT_SRCS:$(COMPONENTS_DIR)/%.c=$(SDL_BUILD_DIR)/%.o) \
           $(SDL_RENDERER_SRCS:$(RENDERER_DIR)/%.c=$(SDL_BUILD_DIR)/%.o)

RAYLIB_OBJS = $(MAIN_SRCS:$(SRC_DIR)/%.c=$(RAYLIB_BUILD_DIR)/%.o) \
              $(COMPONENT_SRCS:$(COMPONENTS_DIR)/%.c=$(RAYLIB_BUILD_DIR)/%.o) \
              $(RAYLIB_RENDERER_SRCS:$(RENDERER_DIR)/%.c=$(RAYLIB_BUILD_DIR)/%.o)

# Targets
.PHONY: all clean sdl raylib examples_sdl examples_raylib

all: sdl raylib

# SDL2 build
sdl: $(SDL_BUILD_DIR)/librocks.a examples_sdl

$(SDL_BUILD_DIR)/librocks.a: $(SDL_OBJS)
	$(MKDIR) $(SDL_BUILD_DIR)
	$(AR) rcs $@ $^

$(SDL_BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(MKDIR) $(SDL_BUILD_DIR)
	$(CC) $(SDL_FLAGS) $(COMMON_FLAGS) $(SDL_DEFINES) -c $< -o $@

$(SDL_BUILD_DIR)/%.o: $(COMPONENTS_DIR)/%.c
	$(MKDIR) $(SDL_BUILD_DIR)
	$(CC) $(SDL_FLAGS) $(COMMON_FLAGS) $(SDL_DEFINES) -c $< -o $@

$(SDL_BUILD_DIR)/%.o: $(RENDERER_DIR)/%.c
	$(MKDIR) $(SDL_BUILD_DIR)
	$(CC) $(SDL_FLAGS) $(COMMON_FLAGS) $(SDL_DEFINES) -c $< -o $@

# Raylib build
raylib: $(RAYLIB_BUILD_DIR)/librocks.a examples_raylib

$(RAYLIB_BUILD_DIR)/librocks.a: $(RAYLIB_OBJS)
	$(MKDIR) $(RAYLIB_BUILD_DIR)
	$(AR) rcs $@ $^

$(RAYLIB_BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(MKDIR) $(RAYLIB_BUILD_DIR)
	$(CC) $(RAYLIB_FLAGS) $(COMMON_FLAGS) $(RAYLIB_DEFINES) -c $< -o $@

$(RAYLIB_BUILD_DIR)/%.o: $(COMPONENTS_DIR)/%.c
	$(MKDIR) $(RAYLIB_BUILD_DIR)
	$(CC) $(RAYLIB_FLAGS) $(COMMON_FLAGS) $(RAYLIB_DEFINES) -c $< -o $@

$(RAYLIB_BUILD_DIR)/%.o: $(RENDERER_DIR)/%.c
	$(MKDIR) $(RAYLIB_BUILD_DIR)
	$(CC) $(RAYLIB_FLAGS) $(COMMON_FLAGS) $(RAYLIB_DEFINES) -c $< -o $@

# Examples
examples_sdl: $(SDL_BUILD_DIR)/librocks.a
	$(MKDIR) $(SDL_BUILD_DIR)
	for example in $(EXAMPLES); do \
		$(CC) $(EXAMPLES_DIR)/$$example.c -o $(SDL_BUILD_DIR)/$$example \
		$(SDL_BUILD_DIR)/librocks.a $(SDL_FLAGS) $(SDL_LIBS) \
		$(COMMON_FLAGS) $(SDL_DEFINES) $(COMMON_LIBS); \
	done
	if [ -d "$(ASSETS_DIR)" ]; then $(CP) $(ASSETS_DIR) $(SDL_BUILD_DIR)/; fi

examples_raylib: $(RAYLIB_BUILD_DIR)/librocks.a
	$(MKDIR) $(RAYLIB_BUILD_DIR)
	for example in $(EXAMPLES); do \
		$(CC) $(EXAMPLES_DIR)/$$example.c -o $(RAYLIB_BUILD_DIR)/$$example \
		$(RAYLIB_BUILD_DIR)/librocks.a $(RAYLIB_FLAGS) $(RAYLIB_LIBS) \
		$(COMMON_FLAGS) $(RAYLIB_DEFINES) $(COMMON_LIBS); \
	done
	if [ -d "$(ASSETS_DIR)" ]; then $(CP) $(ASSETS_DIR) $(RAYLIB_BUILD_DIR)/; fi

# Clean
clean:
	$(RM) $(BUILD_DIR)
