#ifdef SDL3
    #include <SDL3/SDL.h>
    #include <SDL3/SDL_main.h>
    #include <SDL3/SDL_render.h>
#else
    #include <SDL.h>
    #include <SDL_image.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "sprite_data.h"
#include <stdbool.h>

#define DEBUG_FONT_IMPLEMENTATION
#include "debug_font.h"

typedef struct {
    // positions and velocities are 24.8 fixed point
    Sint32 x, y, dx, dy;
    int frame;
    Uint32 frame_timer;
    Uint32 frame_duration;
} Sprite;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Texture *ui_texture;
#ifdef SDL3
    SDL_Gamepad *gamepad;
    SDL_JoystickID gamepad_id;
#else
    SDL_GameController *gamepad;
    Sint32 gamepad_id;
#endif
    int texture_width;
    int texture_height;
    Sprite *sprites;
    int num_sprites;
    int active_sprites;
    bool movement_enabled;
    bool rotation_enabled;
    bool dirty_ui;
    Uint32 last_frame_time;
    Uint32 fps_update_time;
    Uint32 animate_update_time;
    int frame_count;
    int current_fps;
    bool running;
} AppState;


static int rand_range(int min, int max) {
    return min + (rand() % (1 + (max - min)));
}


static void init_sprite(Sprite *s) {
    s->x = rand_range(0, (SCREEN_WIDTH - SPRITE_WIDTH) << 8);
    s->y = rand_range(0, (SCREEN_HEIGHT - SPRITE_HEIGHT) << 8);
    s->dx = rand_range(1, SPRITE_MAX_SPEED);
    s->dy = rand_range(1, SPRITE_MAX_SPEED);
    if (rand_range(1, 2) == 2) s->dx = -1 * s->dx;
    if (rand_range(1, 2) == 2) s->dy = -1 * s->dy;
    s->frame = rand_range(0, NUM_FRAMES - 1);
    s->frame_timer = 0;
    s->frame_duration = FRAME_DURATION_MS + rand_range(0, FRAME_DURATION_MS / 2);
}


static void update_sprite_position(Sprite *s, Sint32 delta) {
    static const int bound_left = (0 - SPRITE_WIDTH / 2) << 8;
    static const int bound_right = (SCREEN_WIDTH - SPRITE_WIDTH / 2) << 8;
    static const int bound_top = (0 - SPRITE_HEIGHT / 2) << 8;
    static const int bound_bottom = (SCREEN_HEIGHT - SPRITE_HEIGHT / 2) << 8;

    const Sint32 delta_fp = delta << 8;

    s->x += (s->dx * delta_fp/UPDATE_INTERVAL_MS) >> 8;
    s->y += (s->dy * delta_fp/UPDATE_INTERVAL_MS) >> 8;

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


static inline void update_sprite_animation(Sprite *s, Uint32 delta_ms) {
    s->frame_timer += delta_ms;
    if (s->frame_timer >= s->frame_duration) {
        s->frame = (s->frame + 1) % NUM_FRAMES;
        s->frame_timer -= s->frame_duration;
    }
}


static inline void render_sprite(AppState *state, Sprite *s) {
    SDL_FRect dst_rect = {s->x >> 8, s->y >> 8, SPRITE_WIDTH, SPRITE_HEIGHT};

#ifdef SDL3
    SDL_FRect src_rect = {(s->frame) * SPRITE_WIDTH, 0, SPRITE_WIDTH, SPRITE_HEIGHT};
    if (state->rotation_enabled) {
        SDL_RenderTextureRotated(state->renderer, state->texture, &src_rect, &dst_rect, 22, NULL, SDL_FLIP_NONE);
    } else {
        SDL_RenderTexture(state->renderer, state->texture, &src_rect, &dst_rect);
    }
#else
    SDL_Rect src_rect = {(s->frame) * SPRITE_WIDTH, 0, SPRITE_WIDTH, SPRITE_HEIGHT};
    if (state->rotation_enabled) {
        SDL_RenderCopyExF(state->renderer, state->texture, &src_rect, &dst_rect, 22.0, NULL, SDL_FLIP_NONE);
    } else {
        SDL_RenderCopyF(state->renderer, state->texture, &src_rect, &dst_rect);
    }
#endif
}


static void adjust_sprite_count(AppState *state, int delta) {
    state->active_sprites += delta;
    if (state->active_sprites > state->num_sprites)
        state->active_sprites = state->num_sprites;
    if (state->active_sprites < 0)
        state->active_sprites = 0;
    state->dirty_ui = true;
}


static void handle_gamepad_button(AppState *state, int button) {
#ifdef SDL3
    switch (button) {
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
            adjust_sprite_count(state, -SPRITE_INCREMENT);
            break;
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
            adjust_sprite_count(state, SPRITE_INCREMENT);
            break;
        case SDL_GAMEPAD_BUTTON_DPAD_UP:
            state->movement_enabled = !state->movement_enabled;
            break;
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
            state->rotation_enabled = !state->rotation_enabled;
            break;
        case SDL_GAMEPAD_BUTTON_EAST:
            state->running = false;
            break;
    }
#else
    switch (button) {
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            adjust_sprite_count(state, -SPRITE_INCREMENT);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            adjust_sprite_count(state, SPRITE_INCREMENT);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            state->movement_enabled = !state->movement_enabled;
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            state->rotation_enabled = !state->rotation_enabled;
            break;
        case SDL_CONTROLLER_BUTTON_B:
            state->running = false;
            break;
    }
#endif
}


static void handle_key_event(AppState *state, int key) {
    switch (key) {
        case SDLK_ESCAPE:
            state->running = false;
            break;
        case SDLK_RIGHT:
        case SDLK_EQUALS:
            adjust_sprite_count(state, SPRITE_INCREMENT);
            break;
        case SDLK_LEFT:
        case SDLK_MINUS:
            adjust_sprite_count(state, -SPRITE_INCREMENT);
            break;
        case SDLK_UP:
            state->movement_enabled = !state->movement_enabled;
            break;
        case SDLK_DOWN:
            state->rotation_enabled = !state->rotation_enabled;
            break;
    }
}


static void process_event(AppState *state, SDL_Event *event) {
#ifdef SDL3
    switch (event->type) {
        case SDL_EVENT_QUIT:
            state->running = false;
            break;
        case SDL_EVENT_GAMEPAD_ADDED:
            state->gamepad = SDL_OpenGamepad(event->gdevice.which);
            state->gamepad_id = event->gdevice.which;
            break;
        case SDL_EVENT_GAMEPAD_REMOVED:
            if (state->gamepad && state->gamepad_id == event->gdevice.which) {
                SDL_CloseGamepad(state->gamepad);
                state->gamepad = NULL;
                state->gamepad_id = 0;
            }
            break;
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            handle_gamepad_button(state, event->gbutton.button);
            break;
        case SDL_EVENT_KEY_DOWN:
            handle_key_event(state, event->key.key);
            break;
    }
#else
    switch (event->type) {
        case SDL_QUIT:
            state->running = false;
            break;
        case SDL_CONTROLLERDEVICEADDED:
            if (SDL_IsGameController(event->cdevice.which)) {
                state->gamepad = SDL_GameControllerOpen(event->cdevice.which);
                state->gamepad_id = event->cdevice.which;
            }
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            if (state->gamepad && state->gamepad_id == event->cdevice.which) {
                SDL_GameControllerClose(state->gamepad);
                state->gamepad = NULL;
                state->gamepad_id = 0;
            }
            break;
        case SDL_CONTROLLERBUTTONDOWN:
            handle_gamepad_button(state, event->cbutton.button);
            break;
        case SDL_KEYDOWN:
            handle_key_event(state, event->key.keysym.sym);
            break;
    }
#endif
}


static inline void render_ui(AppState *state) {
    if (state->dirty_ui) {
        char fps_text[128];
        SDL_snprintf(fps_text, sizeof(fps_text), "fps %d   sprites %d", state->current_fps, state->active_sprites);
        SDL_Color black = {0, 0, 0, 255};

        SDL_SetRenderTarget(state->renderer, state->ui_texture);
        SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);
        SDL_RenderClear(state->renderer);
        debug_font_draw_string(state->renderer, fps_text, 10, 10, black);
        SDL_SetRenderTarget(state->renderer, NULL);

        state->dirty_ui = false;
    }

#ifdef SDL3
    SDL_RenderTexture(state->renderer, state->ui_texture, NULL, &(SDL_FRect){10, 10, 200, 30});
#else
    SDL_Rect ui_rect = {10, 10, 200, 30};
    SDL_RenderCopy(state->renderer, state->ui_texture, NULL, &ui_rect);
#endif
}


static int init_sdl(void) {
#ifdef SDL3
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
#else
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) < 0) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
#endif
    return EXIT_SUCCESS;
}


static void init_window(AppState *state) {
#ifdef SDL3
    SDL_Log("SDL3");
    state->window = SDL_CreateWindow("SDL3 Sprite Benchmark", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
#else
    SDL_Log("SDL2");
    state->window = SDL_CreateWindow("SDL2 Sprite Benchmark", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
#endif
}


static void init_renderer(AppState *state) {
#ifdef SDL3
    state->renderer = SDL_CreateGPURenderer(NULL, state->window);
    if (state->renderer == NULL) {
        state->renderer = SDL_CreateRenderer(state->window, "vulkan,opengl,PSP,psp,VITA gxm,opengles2,gpu,software");
    }
#else
    state->renderer = SDL_CreateRenderer(state->window, -1, SDL_RENDERER_ACCELERATED);
#endif
}


static void print_renderers(AppState *state) {
#ifdef SDL3
    SDL_Log("GPU Drivers:");
    for (int i = 0; i < SDL_GetNumGPUDrivers(); i++) {
        SDL_Log(SDL_GetGPUDriver(i));
    }
#endif

    SDL_Log("Render Drivers:");
    for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
#ifdef SDL3
        SDL_Log(SDL_GetRenderDriver(i));
#else
        SDL_RendererInfo ri;
        SDL_GetRenderDriverInfo(i, &ri);
        SDL_Log(ri.name);
#endif
    }

#ifdef SDL3
    SDL_Log("renderer chosen: ");
    SDL_Log(SDL_GetRendererName(state->renderer));
#else
    SDL_RendererInfo info;
    SDL_GetRendererInfo(state->renderer, &info);
    SDL_Log("renderer chosen: ");
    SDL_Log(info.name);
#endif
}


static int load_sprite_texture(AppState *state) {
#ifdef SDL3
    SDL_IOStream *io = SDL_IOFromMem(sprite_png, sprite_png_len);
    if (!io) {
        SDL_Log("Couldn't create IO: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_Surface *surface = SDL_LoadPNG_IO(io, true);
#else
    SDL_RWops *rw = SDL_RWFromMem(sprite_png, sprite_png_len);
    if (!rw) {
        SDL_Log("Couldn't create RW: %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_Surface *surface = IMG_Load_RW(rw, 1);
#endif
    if (!surface) {
        SDL_Log("Couldn't load png: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    state->texture_width = surface->w;
    state->texture_height = surface->h;
    state->texture = SDL_CreateTextureFromSurface(state->renderer, surface);
#ifdef SDL3
    SDL_DestroySurface(surface);
#else
    SDL_FreeSurface(surface);
#endif

    if (!state->texture) {
        SDL_Log("Couldn't create texture: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

#ifdef SDL3
    SDL_Log("Sprite format: ");
    SDL_Log(SDL_GetPixelFormatName(state->texture->format));
#else
    Uint32 format;
    SDL_QueryTexture(state->texture, &format, NULL, NULL, NULL);
    SDL_Log("Sprite format:");
    SDL_Log(SDL_GetPixelFormatName(format));
#endif

    return EXIT_SUCCESS;
}


static int init_sprites(AppState *state) {
#ifdef SDL3
    state->sprites = (Sprite *)SDL_calloc(MAX_SPRITES, sizeof(Sprite));
#else
    state->sprites = (Sprite *)calloc(MAX_SPRITES, sizeof(Sprite));
#endif
    if (!state->sprites) {
        SDL_Log("Couldn't allocate sprite array");
        return EXIT_FAILURE;
    }

    srand(2026);
    state->num_sprites = MAX_SPRITES;
    state->active_sprites = INITIAL_SPRITES;
    state->dirty_ui = true;

    for (int i = 0; i < state->num_sprites; i++) {
        init_sprite(&state->sprites[i]);
    }

    return EXIT_SUCCESS;
}


int init_app(AppState **appstate) {
    AppState *state = (AppState *)calloc(1, sizeof(AppState));
    if (!state) {
        SDL_Log("Couldn't allocate app state");
        return EXIT_FAILURE;
    }

    state->running = true;
    state->movement_enabled = MOVEMENT_ENABLED_DEFAULT;

    if (init_sdl() != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    init_window(state);
    if (!state->window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    init_renderer(state);
    if (!state->renderer) {
        SDL_Log("Couldn't create renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_ShowWindow(state->window);
    print_renderers(state);

    if (load_sprite_texture(state) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    if (init_sprites(state) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    state->ui_texture = SDL_CreateTexture(state->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 230, 30);
    if (!state->ui_texture) {
        SDL_Log("Couldn't create texture: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    state->last_frame_time = SDL_GetTicks();
    state->fps_update_time = state->last_frame_time;
    state->animate_update_time = state->last_frame_time;

    *appstate = state;
    return EXIT_SUCCESS;
}


void cleanup_app(AppState *state) {
    if (state == NULL) {
        return;
    }

#ifdef SDL3
    if (state->sprites) SDL_free(state->sprites);
#else
    if (state->sprites) free(state->sprites);
#endif

    if (state->texture) SDL_DestroyTexture(state->texture);
    if (state->ui_texture) SDL_DestroyTexture(state->ui_texture);

#ifdef SDL3
    if (state->gamepad) SDL_CloseGamepad(state->gamepad);
#else
    if (state->gamepad) SDL_GameControllerClose(state->gamepad);
#endif

    if (state->renderer) SDL_DestroyRenderer(state->renderer);
    if (state->window) SDL_DestroyWindow(state->window);
    SDL_Quit();

#ifdef SDL3
    SDL_free(state);
#else
    free(state);
#endif
}


static void update_fps(AppState *state, Uint32 now) {
    if (now - state->fps_update_time >= 3000) {
        state->current_fps = state->frame_count / 3;
        state->frame_count = 0;
        state->fps_update_time = now;
        state->dirty_ui = true;
    }
}


static void update_and_render_sprites(AppState *state, Uint32 now) {
    bool animate = false;
    Uint32  delta = now - state->animate_update_time;
    if (delta >= UPDATE_INTERVAL_MS) {
        animate = true;
        state->animate_update_time = now;
    }

    for (int i = 0; i < state->active_sprites; i++) {
        Sprite *s = &state->sprites[i];
        if (animate) {
            if(state->movement_enabled) update_sprite_position(s, delta);
            update_sprite_animation(s, delta);
        }
        render_sprite(state, s);
    }
}


int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    AppState *state = NULL;
    if (init_app(&state) != EXIT_SUCCESS) {
        cleanup_app(state);
        return EXIT_FAILURE;
    }

    SDL_Event event;
    while (state->running) {
        while (SDL_PollEvent(&event)) {
            process_event(state, &event);
        }

        Uint32 now = SDL_GetTicks();
        state->last_frame_time = now;
        state->frame_count++;

        update_fps(state, now);

        SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, 255);
        SDL_RenderClear(state->renderer);

        update_and_render_sprites(state, now);
        render_ui(state);

        SDL_RenderPresent(state->renderer);
    }

    cleanup_app(state);
    return EXIT_SUCCESS;
}
