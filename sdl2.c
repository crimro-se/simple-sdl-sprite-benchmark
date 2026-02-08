#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "sprite_data.h"
#include <stdbool.h>

#define DEBUG_FONT_IMPLEMENTATION
#include "debug_font.h"

typedef struct {
    int32_t x, y;
    int32_t dx, dy;
    int frame;
    Uint64 frame_timer;
    Uint64 frame_duration;
} Sprite;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_GameController *gamepad;
    Sint32 gamepad_id;  // SDL2 uses Sint32 for joystick IDs
    int texture_width;
    int texture_height;
    Sprite *sprites;
    int num_sprites;
    int active_sprites;
    bool movement_enabled;
    bool rotation_enabled;
    Uint64 last_frame_time;
    Uint64 fps_update_time;
    Uint64 animate_update_time;
    int frame_count;
    int current_fps;
    bool running;
} AppState;

static int rand_range(int min, int max) {
    return min + (rand() % (1 + (max - min)));
}

static void init_sprite(Sprite *s) {
    s->x = rand_range(0, SCREEN_WIDTH - SPRITE_WIDTH);
    s->y = rand_range(0, SCREEN_HEIGHT - SPRITE_HEIGHT);
    s->dx = rand_range(1, 4);
    s->dy = rand_range(1, 4);
    if (rand_range(1, 2) == 2) s->dx = -1 * s->dx;
    if (rand_range(1, 2) == 2) s->dy = -1 * s->dy;
    s->frame = rand_range(0, NUM_FRAMES - 1);
    s->frame_timer = 0;
    s->frame_duration = FRAME_DURATION_MS + rand_range(0, FRAME_DURATION_MS / 2);
}

static void update_sprite_position(Sprite *s, bool movement_enabled) {
    static int bound_left = 0 - SPRITE_WIDTH / 2;
    static int bound_right = SCREEN_WIDTH - SPRITE_WIDTH / 2;
    static int bound_top = 0 - SPRITE_HEIGHT / 2;
    static int bound_bottom = SCREEN_HEIGHT - SPRITE_HEIGHT / 2;

    if (!movement_enabled) return;

    s->x += s->dx;
    s->y += s->dy;

    if (s->x < bound_left) {
        s->x = bound_left;
        if (s->dx < 0) s->dx = -s->dx;
    }
    if (s->x > bound_right) {
        s->x = bound_right;
        if (s->dx > 0) s->dx = -s->dx;
    }
    if (s->y < bound_top) {
        s->y = bound_top;
        if (s->dy < 0) s->dy = -s->dy;
    }
    if (s->y > bound_bottom) {
        s->y = bound_bottom;
        if (s->dy > 0) s->dy = -s->dy;
    }
}

static inline void update_sprite_animation(Sprite *s, Uint64 delta_ms) {
    s->frame_timer += delta_ms;
    if (s->frame_timer >= s->frame_duration) {
        s->frame = (s->frame + 1) % NUM_FRAMES;
        s->frame_timer -= s->frame_duration;
    }
}

static inline void render_sprite(AppState *state, Sprite *s) {
    // SDL2 uses integer source rectangles
    SDL_Rect src_rect = { 
        (s->frame) * SPRITE_WIDTH, 
        0, 
        SPRITE_WIDTH, 
        SPRITE_HEIGHT 
    };
    
    SDL_Rect dst_rect = { 
        s->x, 
        s->y, 
        SPRITE_WIDTH, 
        SPRITE_HEIGHT 
    };

    if (state->rotation_enabled) {
        SDL_RenderCopyEx(state->renderer, state->texture, &src_rect, &dst_rect, 
                         22.0, NULL, SDL_FLIP_NONE);
    } else {
        SDL_RenderCopy(state->renderer, state->texture, &src_rect, &dst_rect);
    }
}

static void adjust_sprite_count(AppState *state, int delta) {
    state->active_sprites += delta;
    if (state->active_sprites > state->num_sprites)
        state->active_sprites = state->num_sprites;
    if (state->active_sprites < 0)
        state->active_sprites = 0;
}

static void process_event(AppState *state, SDL_Event *event) {
    if (event->type == SDL_QUIT) {
        state->running = false;
    }
    
    // Gamepad handling (SDL_CONTROLLER vs SDL_GAMEPAD)
    if (event->type == SDL_CONTROLLERDEVICEADDED) {
        if (SDL_IsGameController(event->cdevice.which)) {
            state->gamepad = SDL_GameControllerOpen(event->cdevice.which);
            state->gamepad_id = event->cdevice.which;
        }
    }
    
    if (event->type == SDL_CONTROLLERDEVICEREMOVED) {
        if (state->gamepad && state->gamepad_id == event->cdevice.which) {
            SDL_GameControllerClose(state->gamepad);
            state->gamepad = NULL;
            state->gamepad_id = 0;
        }
    }
    
    if (event->type == SDL_CONTROLLERBUTTONDOWN) {
        if (event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) {
            adjust_sprite_count(state, -SPRITE_INCREMENT);
        }
        if (event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
            adjust_sprite_count(state, SPRITE_INCREMENT);
        }
        if (event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) {
            state->movement_enabled = !state->movement_enabled;
        }
        if (event->cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
            state->rotation_enabled = !state->rotation_enabled;
        }
        if (event->cbutton.button == SDL_CONTROLLER_BUTTON_A) { // Mapped to EAST on many layouts
            state->running = false;
        }
    }
    
    // Keyboard handling
    if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_ESCAPE) {
            state->running = false;
        }
        if (event->key.keysym.sym == SDLK_RIGHT || event->key.keysym.sym == SDLK_EQUALS) {
            adjust_sprite_count(state, SPRITE_INCREMENT);
        }
        if (event->key.keysym.sym == SDLK_LEFT || event->key.keysym.sym == SDLK_MINUS) {
            adjust_sprite_count(state, -SPRITE_INCREMENT);
        }
        if (event->key.keysym.sym == SDLK_UP) {
            state->movement_enabled = !state->movement_enabled;
        }
        if (event->key.keysym.sym == SDLK_DOWN) {
            state->rotation_enabled = !state->rotation_enabled;
        }
    }
}

int main(int argc, char *argv[]) {
    AppState *state = (AppState *)calloc(1, sizeof(AppState));
    if (!state) {
        SDL_Log("Couldn't allocate app state");
        return 1;
    }
    
    state->running = true;
    state->movement_enabled = MOVEMENT_ENABLED_DEFAULT;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) < 0) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return 1;
    }

    state->window = SDL_CreateWindow("SDL2 Sprite Benchmark",
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     SCREEN_WIDTH, SCREEN_HEIGHT,
                                     SDL_WINDOW_RESIZABLE);
    if (!state->window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return 1;
    }

    state->renderer = SDL_CreateRenderer(state->window, -1, 
                                        SDL_RENDERER_ACCELERATED);
    if (!state->renderer) {
        SDL_Log("Couldn't create renderer: %s", SDL_GetError());
        return 1;
    }

    // Load PNG from memory using SDL_image
    SDL_RWops *rw = SDL_RWFromMem(sprite_png, sprite_png_len);
    if (!rw) {
        SDL_Log("Couldn't create RW: %s", SDL_GetError());
        return 1;
    }
    
    SDL_Surface *surface = IMG_Load_RW(rw, 1); // 1 = auto-close RW
    if (!surface) {
        SDL_Log("Couldn't load png: %s", IMG_GetError());
        return 1;
    }

    state->texture_width = surface->w;
    state->texture_height = surface->h;
    state->texture = SDL_CreateTextureFromSurface(state->renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!state->texture) {
        SDL_Log("Couldn't create texture: %s", SDL_GetError());
        return 1;
    }

    state->sprites = (Sprite *)calloc(MAX_SPRITES, sizeof(Sprite));
    if (!state->sprites) {
        SDL_Log("Couldn't allocate sprite array");
        return 1;
    }

    srand(2026);
    state->num_sprites = MAX_SPRITES;
    state->active_sprites = INITIAL_SPRITES;
    
    for (int i = 0; i < state->num_sprites; i++) {
        init_sprite(&state->sprites[i]);
    }

    state->last_frame_time = SDL_GetTicks();
    state->fps_update_time = state->last_frame_time;
    state->animate_update_time = state->last_frame_time;

    // Main loop
    SDL_Event event;
    while (state->running) {
        // Event polling
        while (SDL_PollEvent(&event)) {
            process_event(state, &event);
        }

        // Update timing
        Uint64 now = SDL_GetTicks();
        Uint64 delta = now - state->last_frame_time;
        state->last_frame_time = now;
        state->frame_count++;

        if (now - state->fps_update_time >= 3000) {
            state->current_fps = state->frame_count / 3;
            state->frame_count = 0;
            state->fps_update_time = now;
        }

        // Clear
        SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);
        SDL_RenderClear(state->renderer);

        // Update and render sprites
        bool animate = false;
        if (now - state->animate_update_time >= 15) {
            animate = true;
            state->animate_update_time = now;
        }

        for (int i = 0; i < state->active_sprites; i++) {
            Sprite *s = &state->sprites[i];
            if (animate) {
                update_sprite_position(s, state->movement_enabled);
                update_sprite_animation(s, 15);
            }
            render_sprite(state, s);
        }

        char title[128];
        SDL_Color black = {0, 0, 0, 255};
        snprintf(title, sizeof(title), "fps: %d  sprites: %d", 
                 state->current_fps, state->active_sprites);
        debug_font_draw_string(state->renderer, title, 10, 10, black);

        SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
        SDL_RenderPresent(state->renderer);
    }

    // Cleanup
    if (state->sprites) free(state->sprites);
    if (state->texture) SDL_DestroyTexture(state->texture);
    if (state->gamepad) SDL_GameControllerClose(state->gamepad);
    if (state->renderer) SDL_DestroyRenderer(state->renderer);
    if (state->window) SDL_DestroyWindow(state->window);
    SDL_Quit();
    free(state);

    return 0;
}
