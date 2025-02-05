#include "renderer/sdl2_renderer.h"
#include "renderer/sdl2_renderer_utils.h"
#include "clay.h"
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL2_gfxPrimitives.h>
#include <math.h>
#include <string.h>

// Constants
static const float SCROLLBAR_FADE_DURATION = 0.6f;
static const float SCROLLBAR_HIDE_DELAY = 0.6f;

// Touch state tracking
typedef enum {
    TOUCH_STATE_NONE,
    TOUCH_STATE_DOWN,
    TOUCH_STATE_DRAGGING,
} TouchState;


struct RocksScrollContainer {
    uint32_t elementId;
    bool openThisFrame;
};

typedef struct {
    TTF_Font* font;
} RockSDL2Font;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    float scale_factor;
    RockSDL2Font fonts[32];
    SDL_Cursor* default_cursor;
    SDL_Cursor* pointer_cursor;
    SDL_Cursor* current_cursor;
    float scrollbar_opacity;
    float last_mouse_move_time;
    SDL_Rect current_clip_rect;
    Rocks* rocks;

    // Scroll container tracking
    RocksScrollContainer scroll_containers[32];
    int scroll_container_count;
    
    // Scrolling state
    TouchState current_touch_state;
    SDL_FingerID active_touch_id;
    Uint32 last_touch_time;
    Uint32 last_successful_click_time;
    bool had_motion_between_down_and_up;
    Clay_ScrollContainerData* active_scroll_container;
    uint32_t active_scroll_container_id;
    bool is_scroll_thumb_dragging;
    bool is_horizontal_scroll_thumb_dragging;
    bool is_scroll_dragging;
    float scroll_drag_start_x;
    float scroll_drag_start_y;
    Clay_Vector2 initial_scroll_position;
    Clay_Vector2 initial_pointer_position;
} RocksSDL2Renderer;

// Function implementations...

void* rocks_sdl2_load_image(Rocks* rocks, const char* path) {
    RocksSDL2Renderer* r = rocks->renderer_data;
    if (!r || !r->renderer) return NULL;

    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("Failed to load image: %s\n", IMG_GetError());
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(r->renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        return NULL;
    }

    return texture;
}

void rocks_sdl2_unload_image(Rocks* rocks, void* image_data) {
    if (!image_data) return;
    SDL_DestroyTexture((SDL_Texture*)image_data);
}

Clay_Dimensions rocks_sdl2_get_image_dimensions(Rocks* rocks, void* image_data) {
    if (!image_data) return (Clay_Dimensions){0, 0};
    
    SDL_Texture* texture = (SDL_Texture*)image_data;
    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    
    return (Clay_Dimensions){
        .width = (float)w,
        .height = (float)h
    };
}
static Clay_ScrollContainerData* find_active_scroll_container(RocksSDL2Renderer* r, Clay_Vector2 pointerPosition) {
    Clay_SetPointerState(pointerPosition, false);

    Clay_ElementId scrollId = {0};
    bool found = false;
    
    for (int i = 0; i < r->scroll_container_count; i++) {
        if (Clay_PointerOver((Clay_ElementId){.id = r->scroll_containers[i].elementId})) {
            scrollId.id = r->scroll_containers[i].elementId;
            found = true;

            // Debug log: Print the scroll container ID under the pointer
            printf("Scroll container under pointer: ID=%u\n", scrollId.id);
            break;
        }
    }

    if (found) {
        Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(scrollId);
        if (scrollData.found) {
            Clay_ScrollContainerData* activeScroll = malloc(sizeof(Clay_ScrollContainerData));
            if (activeScroll) {
                *activeScroll = scrollData;
                r->active_scroll_container_id = scrollId.id;
                return activeScroll;
            }
        }
    }
    
    return NULL;
}

static void handle_pointer_dragging(RocksSDL2Renderer* r, float x, float y) {
    Clay_ScrollContainerData* scrollData = r->active_scroll_container;
    if (!scrollData) return;

    if (r->is_scroll_thumb_dragging) {

        float viewportHeight = scrollData->scrollContainerDimensions.height;
        float contentHeight = scrollData->contentDimensions.height;
        float mouseDelta = (y / r->scale_factor) - (r->scroll_drag_start_y / r->scale_factor);
        float scrollDelta = -(mouseDelta * (contentHeight / viewportHeight));
        float newScrollY = r->initial_scroll_position.y + scrollDelta;
        
        float scrollableHeight = contentHeight - viewportHeight;
        newScrollY = CLAY__MIN(0, CLAY__MAX(newScrollY, -scrollableHeight));
        
        scrollData->scrollPosition->y = newScrollY;
    } 
    else if (r->is_horizontal_scroll_thumb_dragging) {
        float viewportWidth = scrollData->scrollContainerDimensions.width;
        float contentWidth = scrollData->contentDimensions.width;
        float mouseDelta = (x / r->scale_factor) - (r->scroll_drag_start_x / r->scale_factor);
        float scrollDelta = -(mouseDelta * (contentWidth / viewportWidth));
        float newScrollX = r->initial_scroll_position.x + scrollDelta;
        
        float scrollableWidth = contentWidth - viewportWidth;
        newScrollX = CLAY__MIN(0, CLAY__MAX(newScrollX, -scrollableWidth));
        
        scrollData->scrollPosition->x = newScrollX;
    }
    else if (r->is_scroll_dragging) {

        float deltaY = (r->scroll_drag_start_y - y) / r->scale_factor;
        float deltaX = (r->scroll_drag_start_x - x) / r->scale_factor;
        
        float newScrollY = r->initial_scroll_position.y;
        float newScrollX = r->initial_scroll_position.x;
        
        if (scrollData->config.vertical) {
            float scrollableHeight = scrollData->contentDimensions.height - scrollData->scrollContainerDimensions.height;
            newScrollY = r->initial_scroll_position.y - deltaY;
            newScrollY = CLAY__MIN(0, CLAY__MAX(newScrollY, -scrollableHeight));
        }
        
        if (scrollData->config.horizontal) {
            float scrollableWidth = scrollData->contentDimensions.width - scrollData->scrollContainerDimensions.width;
            newScrollX = r->initial_scroll_position.x - deltaX;
            newScrollX = CLAY__MIN(0, CLAY__MAX(newScrollX, -scrollableWidth));
        }
        
        *scrollData->scrollPosition = (Clay_Vector2){newScrollX, newScrollY};
        
        r->scroll_drag_start_y = y;
        r->scroll_drag_start_x = x;
        r->initial_scroll_position = *scrollData->scrollPosition;
    }
}


static void cleanup_active_scroll_container(RocksSDL2Renderer* r) {
    if (r->active_scroll_container) {
        free(r->active_scroll_container);
        r->active_scroll_container = NULL;
    }
    r->active_scroll_container_id = 0;
    r->is_scroll_dragging = false;
    r->is_scroll_thumb_dragging = false;
    r->is_horizontal_scroll_thumb_dragging = false;
}

static void reset_scroll_container(RocksSDL2Renderer* r) {
    r->is_scroll_thumb_dragging = false;
    r->is_horizontal_scroll_thumb_dragging = false;
    r->is_scroll_dragging = false;
    r->active_scroll_container_id = 0;
}

typedef struct {
    float velocity_x;
    float velocity_y;
    float last_scroll_time;
} InertialScrollState;

static InertialScrollState inertial_scroll_state = {0};

static float get_scroll_sensitivity(Clay_ScrollContainerData* scrollData) {
    float base_sensitivity = 5.0f;
    float content_size_factor = scrollData->contentDimensions.height / scrollData->scrollContainerDimensions.height;
    return base_sensitivity * content_size_factor;
}

static void clamp_scroll_position(Clay_ScrollContainerData* scrollData) {
    float scrollableWidth = scrollData->contentDimensions.width - scrollData->scrollContainerDimensions.width;
    float scrollableHeight = scrollData->contentDimensions.height - scrollData->scrollContainerDimensions.height;

    scrollData->scrollPosition->x = CLAY__MIN(0, CLAY__MAX(scrollData->scrollPosition->x, -scrollableWidth));
    scrollData->scrollPosition->y = CLAY__MIN(0, CLAY__MAX(scrollData->scrollPosition->y, -scrollableHeight));
}

static Uint32 last_scroll_time = 0;
static const Uint32 SCROLL_DEBOUNCE_TIME = 50; // 50ms debounce time

static bool should_process_scroll_event() {
    Uint32 current_time = SDL_GetTicks();
    if (current_time - last_scroll_time < SCROLL_DEBOUNCE_TIME) {
        return false;
    }
    last_scroll_time = current_time;
    return true;
}

static void handle_mouse_scrollbar_interaction(
    RocksSDL2Renderer* r,
    SDL_MouseButtonEvent* event,
    Clay_ScrollContainerData* scrollData,
    Clay_ElementId elementId
) {
    Clay_ElementData elementData = Clay_GetElementData(elementId);
    if (!elementData.found) return;

    Clay_BoundingBox containerBox = elementData.boundingBox;
    float scaledMouseX = event->x / r->scale_factor;
    float scaledMouseY = event->y / r->scale_factor;
    
    if (scrollData->config.vertical) {
        bool inScrollbarArea = (scaledMouseX >= containerBox.x + containerBox.width - 15 && 
                              scaledMouseX <= containerBox.x + containerBox.width);
        
        if (inScrollbarArea) {
            float viewportHeight = containerBox.height;
            float contentHeight = scrollData->contentDimensions.height;
            
            float thumbHeight = CLAY__MAX(20.0f, (viewportHeight / contentHeight) * viewportHeight);
            float scrollProgress = -scrollData->scrollPosition->y / (contentHeight - viewportHeight);
            float thumbPosY = containerBox.y + (scrollProgress * (viewportHeight - thumbHeight));
            
            if (scaledMouseY >= thumbPosY && scaledMouseY <= thumbPosY + thumbHeight) {
                r->is_scroll_thumb_dragging = true;
                r->scroll_drag_start_y = event->y;
                r->initial_scroll_position = *scrollData->scrollPosition;
                return;
            }
            
            float clickPositionInScrollbar = scaledMouseY - containerBox.y;
            float relativeClickPosition = clickPositionInScrollbar / viewportHeight;
            
            if (relativeClickPosition < scrollProgress) {
                scrollData->scrollPosition->y = -(contentHeight - viewportHeight) * relativeClickPosition;
            } else {
                scrollData->scrollPosition->y = -(contentHeight - viewportHeight) * relativeClickPosition - thumbHeight;
            }
            
            scrollData->scrollPosition->y = CLAY__MIN(0, CLAY__MAX(scrollData->scrollPosition->y, -(contentHeight - viewportHeight)));
            return;
        }
    }

    if (scrollData->config.horizontal) {
        bool inScrollbarArea = (scaledMouseY >= containerBox.y + containerBox.height - 15 && 
                              scaledMouseY <= containerBox.y + containerBox.height);
        
        if (inScrollbarArea) {
            float viewportWidth = containerBox.width;
            float contentWidth = scrollData->contentDimensions.width;
            
            float thumbWidth = CLAY__MAX(20.0f, (viewportWidth / contentWidth) * viewportWidth);
            float scrollProgress = -scrollData->scrollPosition->x / (contentWidth - viewportWidth);
            float thumbPosX = containerBox.x + (scrollProgress * (viewportWidth - thumbWidth));
            
            if (scaledMouseX >= thumbPosX && scaledMouseX <= thumbPosX + thumbWidth) {
                r->is_horizontal_scroll_thumb_dragging = true;
                r->scroll_drag_start_x = event->x;
                r->initial_scroll_position = *scrollData->scrollPosition;
                return;
            }
            
            float clickPositionInScrollbar = scaledMouseX - containerBox.x;
            float relativeClickPosition = clickPositionInScrollbar / viewportWidth;
            
            if (relativeClickPosition < scrollProgress) {
                scrollData->scrollPosition->x = -(contentWidth - viewportWidth) * relativeClickPosition;
            } else {
                scrollData->scrollPosition->x = -(contentWidth - viewportWidth) * relativeClickPosition - thumbWidth;
            }
            
            scrollData->scrollPosition->x = CLAY__MIN(0, CLAY__MAX(scrollData->scrollPosition->x, -(contentWidth - viewportWidth)));
            return;
        }
    }

    r->is_scroll_dragging = true;
    r->scroll_drag_start_y = event->y;
    r->scroll_drag_start_x = event->x;
    r->initial_scroll_position = *scrollData->scrollPosition;
}


static Clay_Dimensions rocks_sdl2_measure_text(Clay_StringSlice text, Clay_TextElementConfig* config, uintptr_t userData) {
    RocksSDL2Renderer* r = (RocksSDL2Renderer*)userData;
    
    if (config->fontId >= 32 || !r->fonts[config->fontId].font) {
        return (Clay_Dimensions){0, 0};
    }

    TTF_Font* font = r->fonts[config->fontId].font;
    
    if (!text.chars || text.length == 0) {
        return (Clay_Dimensions){0, (float)TTF_FontHeight(font)};
    }

    char* buffer = (char*)SDL_AllocateAligned(8, text.length + 1);
    if (!buffer) {
        return (Clay_Dimensions){0, 0};
    }

    memcpy(buffer, text.chars, text.length);
    buffer[text.length] = '\0';

    int width = 0, height = 0;
    if (TTF_SizeUTF8(font, buffer, &width, &height) != 0) {
        SDL_FreeAligned(buffer);
        return (Clay_Dimensions){0, 0};
    }

    SDL_FreeAligned(buffer);
    return (Clay_Dimensions){
        .width = (float)width,
        .height = (float)height
    };
}

bool rocks_sdl2_init(Rocks* rocks, void* config) {
    printf("Initializing SDL2 renderer...\n");

    if (!rocks || !config) {
        printf("Error: Invalid arguments to rocks_sdl2_init\n");
        return false;
    }

    RocksSDL2Config* sdl_config = (RocksSDL2Config*)config;

    printf("Initializing <SDL> and TTF...\n");
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() < 0) {
        printf("Error: Failed to initialize SDL or TTF: %s\n", SDL_GetError());
        return false;
    }

    RocksSDL2Renderer* r = calloc(1, sizeof(RocksSDL2Renderer));
    if (!r) {
        printf("Error: Failed to allocate memory for SDL2 renderer\n");
        return false;
    }

    r->rocks = rocks;
    r->scale_factor = sdl_config->scale_factor;

#ifdef CLAY_MOBILE
    // Mobile-specific initialization
    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);

    float ddpi, hdpi, vdpi;
    if (SDL_GetDisplayDPI(0, &ddpi, &hdpi, &vdpi) != 0) {
        ddpi = hdpi = vdpi = 160.0f;
    }

    r->scale_factor = ddpi / 160.0f;
    rocks->config.window_width = displayMode.w / r->scale_factor;
    rocks->config.window_height = displayMode.h / r->scale_factor;

    r->window = SDL_CreateWindow(
        rocks->config.window_title ? rocks->config.window_title : "Untitled",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        displayMode.w,
        displayMode.h,
        SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN
    );
#else
    // Desktop initialization
    r->window = SDL_CreateWindow(
        rocks->config.window_title ? rocks->config.window_title : "Untitled",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        (int)(rocks->config.window_width * r->scale_factor),
        (int)(rocks->config.window_height * r->scale_factor),
        sdl_config->window_flags
    );
#endif

    if (!r->window) {
        printf("Error: Failed to create window: %s\n", SDL_GetError());
        free(r);
        return false;
    }

    printf("Creating SDL renderer...\n");
    r->renderer = SDL_CreateRenderer(
        r->window, -1, 
        sdl_config->renderer_flags | SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE
    );

    if (!r->renderer) {
        printf("Error: Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(r->window);
        free(r);
        return false;
    }

    SDL_SetRenderDrawBlendMode(r->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    printf("Setting up cursors...\n");
    r->default_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    r->pointer_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    r->current_cursor = r->default_cursor;
    SDL_SetCursor(r->current_cursor);

    printf("Setting up text measurement function...\n");
    Clay_SetMeasureTextFunction(rocks_sdl2_measure_text, (uintptr_t)r);
    rocks->renderer_data = r;

    printf("SDL2 renderer initialized successfully\n");
    return true;
}

void rocks_sdl2_cleanup(Rocks* rocks) {
    RocksSDL2Renderer* r = rocks->renderer_data;
    if (!r) return;
    
    for (int i = 0; i < 32; i++) {
        if (r->fonts[i].font) {
            TTF_CloseFont(r->fonts[i].font);
        }
    }

    SDL_FreeCursor(r->default_cursor);
    SDL_FreeCursor(r->pointer_cursor);
    SDL_DestroyRenderer(r->renderer);
    SDL_DestroyWindow(r->window);
    free(r);
    
    TTF_Quit();
    SDL_Quit();
}

uint16_t rocks_sdl2_load_font(Rocks* rocks, const char* path, int size, uint16_t expected_id) {
    RocksSDL2Renderer* r = rocks->renderer_data;
    if (!r) return UINT16_MAX;

    // Validate expected_id
    if (expected_id >= 32) {
        printf("ERROR: Invalid font ID %u (max 31)\n", expected_id);
        return UINT16_MAX;
    }

    // Check if this slot is already taken
    if (r->fonts[expected_id].font) {
        printf("ERROR: Font ID %u is already in use\n", expected_id);
        return UINT16_MAX;
    }

    FILE* f = fopen(path, "rb");
    if (!f) {
        printf("ERROR: Could not open font file: %s\n", path);
        return UINT16_MAX;
    }
    fclose(f);
    
    TTF_Font* font = TTF_OpenFont(path, size * r->scale_factor);

    if (!font) {
        printf("ERROR: TTF_OpenFont failed for %s: %s\n", path, TTF_GetError());
        return UINT16_MAX;
    }

    r->fonts[expected_id].font = font;
    return expected_id;
}

void rocks_sdl2_unload_font(Rocks* rocks, uint16_t font_id) {
    RocksSDL2Renderer* r = rocks->renderer_data;
    if (!r || font_id >= 32) return;

    if (r->fonts[font_id].font) {
        TTF_CloseFont(r->fonts[font_id].font);
        r->fonts[font_id].font = NULL;
    }
}

float rocks_sdl2_get_time(void) {
    return SDL_GetTicks() / 1000.0f;
}

void rocks_sdl2_set_window_size(Rocks* rocks, int width, int height) {
    RocksSDL2Renderer* r = rocks->renderer_data;
    if (!r || !r->window) return;

    SDL_SetWindowSize(r->window, width, height);
    rocks->config.window_width = width / r->scale_factor;
    rocks->config.window_height = height / r->scale_factor;
    
    Clay_SetLayoutDimensions((Clay_Dimensions){
        rocks->config.window_width,
        rocks->config.window_height
    });
}

void rocks_sdl2_toggle_fullscreen(Rocks* rocks) {
    RocksSDL2Renderer* r = rocks->renderer_data;
    if (!r || !r->window) return;

    bool isFullscreen = SDL_GetWindowFlags(r->window) & SDL_WINDOW_FULLSCREEN;
    SDL_SetWindowFullscreen(r->window, isFullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
    
    if (!isFullscreen) {
        // Store window dimensions for restoring later
        int w, h;
        SDL_GetWindowSize(r->window, &w, &h);
        rocks_sdl2_set_window_size(rocks, w, h);
    }
}

void rocks_sdl2_handle_event(Rocks* rocks, void* event) {
    RocksSDL2Renderer* r = rocks->renderer_data;
    SDL_Event* sdl_event = (SDL_Event*)event;
    static float last_time = 0;
    float current_time = SDL_GetTicks() / 1000.0f;
    float delta_time = current_time - last_time;
    last_time = current_time;

    switch (sdl_event->type) {
        case SDL_QUIT:
            rocks->is_running = false;
            break;

        case SDL_WINDOWEVENT:
            if (sdl_event->window.event == SDL_WINDOWEVENT_RESIZED) {
                rocks->config.window_width = sdl_event->window.data1 / r->scale_factor;
                rocks->config.window_height = sdl_event->window.data2 / r->scale_factor;
                Clay_SetLayoutDimensions((Clay_Dimensions){
                    rocks->config.window_width,
                    rocks->config.window_height
                });
            }
            break;
        case SDL_MOUSEWHEEL: {
            if (!should_process_scroll_event()) {
                break;
            }

            Clay_Vector2 currentPos = {
                (float)sdl_event->wheel.mouseX / r->scale_factor,
                (float)sdl_event->wheel.mouseY / r->scale_factor
            };

            r->active_scroll_container = find_active_scroll_container(r, currentPos);

            // Check if active_scroll_container is valid
            if (!r->active_scroll_container) {
                printf("No active scroll container found under pointer.\n");
                break;
            }

            float scrollMultiplier = get_scroll_sensitivity(r->active_scroll_container);
            Clay_Vector2 scrollDelta = {0, 0};

            if (sdl_event->wheel.x != 0 && r->active_scroll_container->config.horizontal) {
                scrollDelta.x = -sdl_event->wheel.x * scrollMultiplier;
            }
            else if (sdl_event->wheel.y != 0) {
                bool preferHorizontal = (r->active_scroll_container->contentDimensions.width > r->active_scroll_container->contentDimensions.height) ||
                                    (r->active_scroll_container->config.horizontal && !r->active_scroll_container->config.vertical);

                if (preferHorizontal && r->active_scroll_container->config.horizontal) {
                    scrollDelta.x = sdl_event->wheel.y * scrollMultiplier;
                } else if (r->active_scroll_container->config.vertical) {
                    scrollDelta.y = sdl_event->wheel.y * scrollMultiplier;
                }
            }

            Clay_UpdateScrollContainers(true, scrollDelta, delta_time);
            clamp_scroll_position(r->active_scroll_container);

            // Update inertial scroll velocity
            inertial_scroll_state.velocity_x = scrollDelta.x / delta_time;
            inertial_scroll_state.velocity_y = scrollDelta.y / delta_time;
            inertial_scroll_state.last_scroll_time = SDL_GetTicks();
            break;
        }

        case SDL_MOUSEBUTTONDOWN:
            r->initial_pointer_position = (Clay_Vector2){
                (float)sdl_event->button.x / r->scale_factor,
                (float)sdl_event->button.y / r->scale_factor
            };
            r->had_motion_between_down_and_up = false;
            
            if (sdl_event->button.button == SDL_BUTTON_LEFT) {
                cleanup_active_scroll_container(r);
                
                r->active_scroll_container = find_active_scroll_container(r, r->initial_pointer_position);
                
                if (r->active_scroll_container) {
                    if (r->active_scroll_container_id == 0) {
                        cleanup_active_scroll_container(r);
                        break;
                    }
                    
                    handle_mouse_scrollbar_interaction(
                        r,
                        &sdl_event->button,
                        r->active_scroll_container,
                        (Clay_ElementId){.id = r->active_scroll_container_id}
                    );
                }
            }
            break;

        case SDL_MOUSEMOTION: {
            Clay_Vector2 currentPos = {
                (float)sdl_event->motion.x / r->scale_factor,
                (float)sdl_event->motion.y / r->scale_factor
            };
            
            r->last_mouse_move_time = current_time;
            Clay_SetPointerState(currentPos, false);
            
            if (r->active_scroll_container && 
                (r->is_scroll_thumb_dragging || r->is_horizontal_scroll_thumb_dragging || r->is_scroll_dragging)) {
                handle_pointer_dragging(r, sdl_event->motion.x, sdl_event->motion.y);
            }
            break;
        }

        case SDL_MOUSEBUTTONUP: {
            Clay_Vector2 upPosition = {
                (float)sdl_event->button.x / r->scale_factor,
                (float)sdl_event->button.y / r->scale_factor
            };
            
            float dx = upPosition.x - r->initial_pointer_position.x;
            float dy = upPosition.y - r->initial_pointer_position.y;
            float distanceSquared = dx*dx + dy*dy;
            
            if (distanceSquared < 25.0f) {
                Clay_SetPointerState(upPosition, true);
            }
            
            Clay_SetPointerState(upPosition, false);
            
            reset_scroll_container(r);
            cleanup_active_scroll_container(r);
            break;
        }

        case SDL_FINGERDOWN: {
            Uint32 currentTime = SDL_GetTicks();
            
            const Uint32 TOUCH_DEBOUNCE_MS = 200;
            const Uint32 POST_CLICK_QUIET_PERIOD_MS = 250;
            
            if (currentTime - r->last_touch_time < TOUCH_DEBOUNCE_MS ||
                currentTime - r->last_successful_click_time < POST_CLICK_QUIET_PERIOD_MS) {
                break;
            }
            
            if (r->current_touch_state != TOUCH_STATE_NONE) {
                break;
            }

            r->active_touch_id = sdl_event->tfinger.fingerId;
            r->current_touch_state = TOUCH_STATE_DOWN;
            
            int actualWidth, actualHeight;
            SDL_GetWindowSize(SDL_GetWindowFromID(sdl_event->tfinger.windowID), 
                            &actualWidth, &actualHeight);
            
            float screenX = sdl_event->tfinger.x * actualWidth;
            float screenY = sdl_event->tfinger.y * actualHeight;
            
            r->initial_pointer_position = (Clay_Vector2){
                screenX / r->scale_factor,
                screenY / r->scale_factor
            };
            
            Clay_SetPointerState(r->initial_pointer_position, false);
            r->had_motion_between_down_and_up = false;

            r->active_scroll_container = find_active_scroll_container(r, r->initial_pointer_position);

            if (r->active_scroll_container) {
                r->scroll_drag_start_x = screenX;
                r->scroll_drag_start_y = screenY;
                r->initial_scroll_position = *r->active_scroll_container->scrollPosition;
            }
            break;
        }

        case SDL_FINGERMOTION: {
            if (sdl_event->tfinger.fingerId != r->active_touch_id || 
                r->current_touch_state == TOUCH_STATE_NONE) {
                break;
            }

            int actualWidth, actualHeight;
            SDL_GetWindowSize(SDL_GetWindowFromID(sdl_event->tfinger.windowID), 
                            &actualWidth, &actualHeight);
            
            float screenX = sdl_event->tfinger.x * actualWidth;
            float screenY = sdl_event->tfinger.y * actualHeight;
            
            Clay_Vector2 currentPos = {
                screenX / r->scale_factor,
                screenY / r->scale_factor
            };
            
            float dx = screenX - r->scroll_drag_start_x;
            float dy = screenY - r->scroll_drag_start_y;
            float moveDistance = sqrtf(dx*dx + dy*dy);
            
            if (r->current_touch_state == TOUCH_STATE_DOWN && moveDistance > 40.0f) {
                r->current_touch_state = TOUCH_STATE_DRAGGING;
                r->is_scroll_dragging = true;
            }
            
            Clay_SetPointerState(currentPos, false);
            
            if (r->current_touch_state == TOUCH_STATE_DRAGGING) {
                if (!r->active_scroll_container) {
                    r->active_scroll_container = find_active_scroll_container(r, currentPos);
                }
                
                if (r->active_scroll_container) {
                    handle_pointer_dragging(r, screenX, screenY);
                }
            }
            
            if (moveDistance > 15.0f) {
                r->had_motion_between_down_and_up = true;
            }
            break;
        }

        case SDL_KEYDOWN:
            if (sdl_event->key.keysym.sym == SDLK_F11 ||
                (sdl_event->key.keysym.sym == SDLK_RETURN && 
                 sdl_event->key.keysym.mod & KMOD_ALT)) {
                rocks_sdl2_toggle_fullscreen(rocks);
            }
            break;

        case SDL_FINGERUP: {
            if (sdl_event->tfinger.fingerId != r->active_touch_id || 
                r->current_touch_state == TOUCH_STATE_NONE) {
                break;
            }
            
            Uint32 currentTime = SDL_GetTicks();
            r->last_touch_time = currentTime;
            
            int actualWidth, actualHeight;
            SDL_GetWindowSize(SDL_GetWindowFromID(sdl_event->tfinger.windowID), 
                            &actualWidth, &actualHeight);
            
            float screenX = sdl_event->tfinger.x * actualWidth;
            float screenY = sdl_event->tfinger.y * actualHeight;
            
            Clay_Vector2 upPosition = {
                screenX / r->scale_factor,
                screenY / r->scale_factor
            };
            
            float dx = (screenX - (r->initial_pointer_position.x * r->scale_factor));
            float dy = (screenY - (r->initial_pointer_position.y * r->scale_factor));
            float distanceSquared = dx*dx + dy*dy;
            
            if (distanceSquared < 100.0f && r->current_touch_state == TOUCH_STATE_DOWN) {
                Clay_SetPointerState(upPosition, true);
                SDL_Delay(25);
                Clay_SetPointerState(upPosition, false);
                r->last_successful_click_time = SDL_GetTicks();
            } else {
                Clay_SetPointerState(upPosition, false);
            }
            
            r->current_touch_state = TOUCH_STATE_NONE;
            r->active_touch_id = 0;
            r->is_scroll_thumb_dragging = false;
            r->is_horizontal_scroll_thumb_dragging = false;
            r->is_scroll_dragging = false;
            r->had_motion_between_down_and_up = false;
            break;
        }
    }
}

void rocks_sdl2_process_events(Rocks* rocks) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        rocks_sdl2_handle_event(rocks, &event);
    }
}

void rocks_sdl2_render(Rocks* rocks, Clay_RenderCommandArray commands) {
    RocksSDL2Renderer* r = rocks->renderer_data;
    if (!r || !r->renderer) {
        printf("Error: Renderer or renderer data is NULL\n");
        return;
    }

    static float currentTime = 0;
    currentTime = SDL_GetTicks() / 1000.0f;
    
    // Update scrollbar opacity
    float timeSinceLastMove = currentTime - r->last_mouse_move_time;
    if (timeSinceLastMove < SCROLLBAR_HIDE_DELAY) {
        r->scrollbar_opacity = SDL_min(r->scrollbar_opacity + 
            (1.0f / SCROLLBAR_FADE_DURATION) * (1.0f/60.0f), 1.0f);
    } else {
        r->scrollbar_opacity = SDL_max(r->scrollbar_opacity - 
            (1.0f / SCROLLBAR_FADE_DURATION) * (1.0f/60.0f), 0.0f);
    }

    // Clear screen
    SDL_SetRenderDrawColor(r->renderer, 0, 0, 0, 255);
    SDL_RenderClear(r->renderer);

    bool hasPointerElement = false;

    // Get scaled mouse coordinates for hit testing
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    mouseX /= r->scale_factor;
    mouseY /= r->scale_factor;

    // Process render commands

    for (uint32_t i = 0; i < commands.length; i++) {
        Clay_RenderCommand* cmd = Clay_RenderCommandArray_Get(&commands, i);
        if (!cmd) {
            printf("Command %d: NULL command\n", i);
            continue;
        }
        Clay_BoundingBox boundingBox = cmd->boundingBox;
        SDL_FRect scaledBox = ScaleBoundingBox(r->renderer, r->scale_factor, cmd->boundingBox);

        switch (cmd->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                Clay_RectangleElementConfig* config = cmd->config.rectangleElementConfig;
                if (!config) continue;

                // Check for pointer hover
                if (config->cursorPointer && 
                    mouseX >= cmd->boundingBox.x && 
                    mouseX <= cmd->boundingBox.x + cmd->boundingBox.width &&
                    mouseY >= cmd->boundingBox.y && 
                    mouseY <= cmd->boundingBox.y + cmd->boundingBox.height) {
                    hasPointerElement = true;
                    }

                // Render rounded rectangle with shadow if enabled
                RenderRoundedRectangle(
                    r->renderer, 
                    scaledBox,
                    config->cornerRadius,
                    config->color,
                    config->shadowEnabled,
                    config->shadowColor,
                    config->shadowOffset,
                    config->shadowBlurRadius,
                    config->shadowSpread,
                    r->scale_factor
                );
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                Clay_TextElementConfig* config = cmd->config.textElementConfig;
                if (!config || config->fontId >= 32 || !r->fonts[config->fontId].font) {
                    printf("Error: Invalid text config or font (Font ID: %d)\n", 
                        config ? config->fontId : -1);
                    continue;
                }

                if (!cmd->text.chars || cmd->text.length == 0) {
                    printf("Error: No text to render\n");
                    continue;
                }


                // Get current clip rect
                SDL_Rect clip;
                SDL_RenderGetClipRect(r->renderer, &clip);

                TTF_Font* font = r->fonts[config->fontId].font;

                // Aligned memory allocation for text
                size_t bufferSize = cmd->text.length + 1;
                char* text = (char*)SDL_AllocateAligned(8, bufferSize);
                if (!text) {
                    printf("Failed to allocate text buffer\n");
                    continue;
                }
                
                memset(text, 0, bufferSize);
                memcpy(text, cmd->text.chars, cmd->text.length);

                SDL_Color color = {
                    config->textColor.r,
                    config->textColor.g,
                    config->textColor.b,
                    config->textColor.a
                };
                SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, color);
                if (!surface) {
                    printf("Failed to create text surface: %s\n", TTF_GetError());
                    SDL_FreeAligned(text);
                    continue;
                }

                SDL_Texture* texture = SDL_CreateTextureFromSurface(r->renderer, surface);
                if (!texture) {
                    printf("Failed to create texture from surface: %s\n", SDL_GetError());
                    SDL_FreeSurface(surface);
                    SDL_FreeAligned(text);
                    continue;
                }
                            
                SDL_RenderCopyF(r->renderer, texture, NULL, &scaledBox);

                SDL_DestroyTexture(texture);
                SDL_FreeSurface(surface);
                SDL_FreeAligned(text);
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_BORDER: {
                Clay_BorderElementConfig* config = cmd->config.borderElementConfig;
                if (!config) continue;

                RenderBorder(r->renderer, scaledBox, config->top, config->cornerRadius, 
                            true, false, false, false, r->scale_factor);
                RenderBorder(r->renderer, scaledBox, config->bottom, config->cornerRadius, 
                            false, true, false, false, r->scale_factor);
                RenderBorder(r->renderer, scaledBox, config->left, config->cornerRadius, 
                            false, false, true, false, r->scale_factor);
                RenderBorder(r->renderer, scaledBox, config->right, config->cornerRadius, 
                            false, false, false, true, r->scale_factor);
                break;
            }
            
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                if (cmd->config.scrollElementConfig) {
                    // Track scroll container
                    if (r->scroll_container_count < 32) {
                        r->scroll_containers[r->scroll_container_count].elementId = cmd->id;
                        r->scroll_containers[r->scroll_container_count].openThisFrame = true;
                        r->scroll_container_count++;

                        // Debug log: Print scroll container ID and dimensions
                        printf("Scroll container detected: ID=%u, Dimensions=(%.2f, %.2f)\n",
                            cmd->id, cmd->boundingBox.width, cmd->boundingBox.height);
                    }

                    Clay_ScrollElementConfig* config = cmd->config.scrollElementConfig;
                    Clay_ElementId elementId = { .id = cmd->id };
                    
                    if (config->vertical) {
                        RenderScrollbar(r->renderer, r->rocks, boundingBox, true, mouseX, mouseY, config, elementId, r->scale_factor);
                    }
                    if (config->horizontal) {
                        RenderScrollbar(r->renderer, r->rocks, boundingBox, false, mouseX, mouseY, config, elementId, r->scale_factor);
                    }
                }

                SDL_Rect clip = {
                    .x = (int)(scaledBox.x),
                    .y = (int)(scaledBox.y),
                    .w = (int)(scaledBox.w),
                    .h = (int)(scaledBox.h)
                };
                SDL_RenderSetClipRect(r->renderer, &clip);
                break;
            }

            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
                SDL_RenderSetClipRect(r->renderer, NULL);
                break;
            case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
                Clay_ImageElementConfig* config = cmd->config.imageElementConfig;
                if (!config || !config->imageData) {
                    printf("Error: Invalid image config or data\n");
                    continue;
                }

                SDL_Texture* texture = (SDL_Texture*)config->imageData;
                SDL_RenderCopyF(r->renderer, texture, NULL, &scaledBox);
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_CUSTOM: {
                Clay_CustomElementConfig* config = cmd->config.customElementConfig;
                if (config && config->drawCallback) {
                    config->drawCallback(cmd, config->userData);
                }
                break;
            }

            default: {
                fprintf(stderr, "Error: unhandled render command: %d\n", cmd->commandType);
                exit(1);
            }
        }
    }

    // Update cursor based on hover state
    SDL_Cursor* targetCursor = hasPointerElement ? r->pointer_cursor : r->default_cursor;
    if (targetCursor != r->current_cursor) {
        r->current_cursor = targetCursor;
        SDL_SetCursor(r->current_cursor);
    }

    SDL_RenderPresent(r->renderer);
}