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
CONTENT_DIR = $(EXAMPLES_DIR)/content
COMPONENTS_DIR = $(SRC_DIR)/components
RENDERER_DIR = $(SRC_DIR)/renderer
VENDOR_DIR = vendor
NANOSVG_DIR = $(VENDOR_DIR)/nanosvg/src
CMARK_DIR = $(VENDOR_DIR)/cmark
CMARK_SRC_DIR = $(CMARK_DIR)/src
CMARK_BUILD_DIR = $(CMARK_DIR)/build
CMARK_INCLUDE_DIR = $(CMARK_BUILD_DIR)/src  # Add this for generated headers

# SDL2 specific
SDL_BUILD_DIR = $(BUILD_DIR)/sdl
SDL_FLAGS = $(shell pkg-config --cflags sdl2 SDL2_ttf SDL2_image SDL2_gfx) -I$(CMARK_SRC_DIR) -I$(CMARK_INCLUDE_DIR)
SDL_LIBS = $(shell pkg-config --libs sdl2 SDL2_ttf SDL2_image SDL2_gfx) -L$(CMARK_BUILD_DIR)/src -lcmark
SDL_DEFINES = -DROCKS_USE_SDL2

# Raylib specific
RAYLIB_BUILD_DIR = $(BUILD_DIR)/raylib
RAYLIB_FLAGS = $(shell pkg-config --cflags raylib) -I$(CMARK_SRC_DIR) -I$(CMARK_INCLUDE_DIR)
RAYLIB_LIBS = $(shell pkg-config --libs raylib) -L$(CMARK_BUILD_DIR)/src -lcmark
RAYLIB_DEFINES = -DROCKS_USE_RAYLIB

# Common flags
COMMON_FLAGS = -I$(INCLUDE_DIR) -I$(CLAY_DIR) -I$(NANOSVG_DIR) -I$(CMARK_SRC_DIR) -D_CRT_SECURE_NO_WARNINGS
COMMON_LIBS = -lm -ldl -lpthread

# Source files
MAIN_SRCS = $(wildcard $(SRC_DIR)/*.c)
COMPONENT_SRCS = $(wildcard $(COMPONENTS_DIR)/*.c)
SDL_RENDERER_SRCS = $(wildcard $(RENDERER_DIR)/sdl2_*.c)
RAYLIB_RENDERER_SRCS = $(RENDERER_DIR)/raylib_renderer.c

# Example files
EXAMPLES = hello_world image_viewer scroll_container text_input dropdown modal grid svg_viewer markdown_viewer

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
sdl: $(CMARK_BUILD_DIR)/src/libcmark.a $(SDL_BUILD_DIR)/librocks.a examples_sdl

$(SDL_BUILD_DIR)/librocks.a: $(SDL_OBJS)
	$(MKDIR) $(SDL_BUILD_DIR)
	$(AR) rcs $@ $^

$(SDL_BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(CMARK_BUILD_DIR)/src/libcmark.a
	$(MKDIR) $(SDL_BUILD_DIR)
	$(CC) $(SDL_FLAGS) $(COMMON_FLAGS) $(SDL_DEFINES) -c $< -o $@

$(SDL_BUILD_DIR)/%.o: $(COMPONENTS_DIR)/%.c | $(CMARK_BUILD_DIR)/src/libcmark.a
	$(MKDIR) $(SDL_BUILD_DIR)
	$(CC) $(SDL_FLAGS) $(COMMON_FLAGS) $(SDL_DEFINES) -c $< -o $@

$(SDL_BUILD_DIR)/%.o: $(RENDERER_DIR)/%.c | $(CMARK_BUILD_DIR)/src/libcmark.a
	$(MKDIR) $(SDL_BUILD_DIR)
	$(CC) $(SDL_FLAGS) $(COMMON_FLAGS) $(SDL_DEFINES) -c $< -o $@

# Raylib build
raylib: $(CMARK_BUILD_DIR)/src/libcmark.a $(RAYLIB_BUILD_DIR)/librocks.a examples_raylib

$(RAYLIB_BUILD_DIR)/librocks.a: $(RAYLIB_OBJS)
	$(MKDIR) $(RAYLIB_BUILD_DIR)
	$(AR) rcs $@ $^

$(RAYLIB_BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(CMARK_BUILD_DIR)/src/libcmark.a
	$(MKDIR) $(RAYLIB_BUILD_DIR)
	$(CC) $(RAYLIB_FLAGS) $(COMMON_FLAGS) $(RAYLIB_DEFINES) -c $< -o $@

$(RAYLIB_BUILD_DIR)/%.o: $(COMPONENTS_DIR)/%.c | $(CMARK_BUILD_DIR)/src/libcmark.a
	$(MKDIR) $(RAYLIB_BUILD_DIR)
	$(CC) $(RAYLIB_FLAGS) $(COMMON_FLAGS) $(RAYLIB_DEFINES) -c $< -o $@

$(RAYLIB_BUILD_DIR)/%.o: $(RENDERER_DIR)/%.c | $(CMARK_BUILD_DIR)/src/libcmark.a
	$(MKDIR) $(RAYLIB_BUILD_DIR)
	$(CC) $(RAYLIB_FLAGS) $(COMMON_FLAGS) $(RAYLIB_DEFINES) -c $< -o $@

# Build cmark static library
$(CMARK_BUILD_DIR)/src/libcmark.a:
	$(MKDIR) $(CMARK_BUILD_DIR)
	cd $(CMARK_DIR) && cmake -S . -B build -DCMARK_TESTS=OFF -DCMARK_SHARED=OFF
	cd $(CMARK_BUILD_DIR) && make

# Examples
examples_sdl: $(SDL_BUILD_DIR)/librocks.a
	$(MKDIR) $(SDL_BUILD_DIR)
	for example in $(EXAMPLES); do \
		$(CC) $(EXAMPLES_DIR)/$$example.c -o $(SDL_BUILD_DIR)/$$example \
		$(SDL_BUILD_DIR)/librocks.a $(SDL_FLAGS) $(SDL_LIBS) \
		$(COMMON_FLAGS) $(SDL_DEFINES) $(COMMON_LIBS); \
	done
	if [ -d "$(ASSETS_DIR)" ]; then $(CP) $(ASSETS_DIR) $(SDL_BUILD_DIR)/; fi
	if [ -d "$(CONTENT_DIR)" ]; then $(CP) $(CONTENT_DIR) $(SDL_BUILD_DIR)/; fi

examples_raylib: $(RAYLIB_BUILD_DIR)/librocks.a $(CMARK_BUILD_DIR)/src/libcmark.a
	$(MKDIR) $(RAYLIB_BUILD_DIR)
	for example in $(EXAMPLES); do \
		$(CC) $(EXAMPLES_DIR)/$$example.c -o $(RAYLIB_BUILD_DIR)/$$example \
		$(RAYLIB_BUILD_DIR)/librocks.a $(RAYLIB_FLAGS) $(RAYLIB_LIBS) \
		$(COMMON_FLAGS) $(RAYLIB_DEFINES) $(COMMON_LIBS); \
	done
	if [ -d "$(ASSETS_DIR)" ]; then $(CP) $(ASSETS_DIR) $(RAYLIB_BUILD_DIR)/; fi
	if [ -d "$(CONTENT_DIR)" ]; then $(CP) $(CONTENT_DIR) $(RAYLIB_BUILD_DIR)/; fi

# Clean
clean:
	$(RM) $(BUILD_DIR)
	$(RM) $(CMARK_BUILD_DIR)
	