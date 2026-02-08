#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>

#include <stdlib.h>
#include "config.h"
#include "sprite_data.h"

#define DEBUG_FONT_IMPLEMENTATION
#include "debug_font.h"

typedef struct {
    int32_t x, y;
    int32_t dx, dy; // velocity
    int frame;
    Uint64 frame_timer;
    Uint64 frame_duration;
} Sprite;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Texture *ui_texture;
    SDL_Gamepad *gamepad;
    SDL_JoystickID gamepad_id;
    int texture_width;
    int texture_height;
    Sprite *sprites;
    int num_sprites;
    int active_sprites;
    bool movement_enabled;
    bool rotation_enabled;
    bool dirty_ui;
    Uint64 last_frame_time;
    Uint64 fps_update_time;
    Uint64 animate_update_time;
    int frame_count;
    int current_fps;
} AppState;

static int rand_range(int min, int max) {
    return min + (rand() % (1+(max - min)));
}

static void init_sprite(Sprite *s) {
    s->x = rand_range(0, SCREEN_WIDTH - SPRITE_WIDTH);
    s->y = rand_range(0, SCREEN_HEIGHT - SPRITE_HEIGHT);
    // randomizes movement speed whilst avoiding 0.
    s->dx = rand_range(1, 4);
    s->dy = rand_range(1, 4);
    if (rand_range(1, 2) == 2){ s->dx = -1 * s->dx;}
    if (rand_range(1, 2) == 2){ s->dy = -1 * s->dy;}
    s->frame = rand_range(0, NUM_FRAMES - 1);
    s->frame_timer = 0;
    s->frame_duration = FRAME_DURATION_MS + rand_range(0, FRAME_DURATION_MS/2);
}

static void update_sprite_position(Sprite *s, bool movement_enabled) {
    static int bound_left = 0 - SPRITE_WIDTH/2;
    static int bound_right = SCREEN_WIDTH - SPRITE_WIDTH/2;
    static int bound_top = 0 - SPRITE_HEIGHT/2;
    static int bound_bottom = SCREEN_HEIGHT - SPRITE_HEIGHT/2;
    if (!movement_enabled) {
        return;
    }
    
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
    int src_x = (s->frame) * SPRITE_WIDTH;
    int src_y = 0;
    
    SDL_FRect src_rect = {
        src_x,
        src_y,
        SPRITE_WIDTH,
        SPRITE_HEIGHT
    };
    SDL_FRect dst_rect = {
        s->x,
        s->y,
        SPRITE_WIDTH,
        SPRITE_HEIGHT
    };
    if (state->rotation_enabled){
        SDL_RenderTextureRotated(state->renderer, state->texture, &src_rect, &dst_rect, 22, NULL, SDL_FLIP_NONE);
    }else{
        SDL_RenderTexture(state->renderer, state->texture, &src_rect, &dst_rect);
    }
}

static void adjust_sprite_count(AppState *state, int delta) {
    state->active_sprites += delta;
    if (state->active_sprites > state->num_sprites) {
        state->active_sprites = state->num_sprites;
    }
    if (state->active_sprites < 0) {
        state->active_sprites = 0;
    }
    state->dirty_ui = true;
}

static inline void render_ui(AppState *state) {
   if (state->dirty_ui) {
       char fps_text[128];
       SDL_snprintf(fps_text, sizeof(fps_text), "fps %d   sprites %d", state->current_fps, state->active_sprites);
       SDL_Color black = {0,0,0,255};
       
       SDL_SetRenderTarget(state->renderer, state->ui_texture);
       SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
       SDL_RenderClear(state->renderer);
       debug_font_draw_string(state->renderer, fps_text, 10, 10, black);
       SDL_SetRenderTarget(state->renderer, NULL);
       
       state->dirty_ui = false;
   }
   
   SDL_RenderTexture(state->renderer, state->ui_texture, NULL, &(SDL_FRect){10, 10, 200, 30});
}

SDL_AppResult init_window(AppState *state){
    state->window = SDL_CreateWindow("SDL3 Sprite Benchmark", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if(state->window == NULL){
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        SDL_free(state);
        return SDL_APP_FAILURE;
    }
    state->renderer = SDL_CreateGPURenderer(NULL, state->window);
    if(state->renderer == NULL){
        SDL_Log("Couldn't create GPU renderer: %s", SDL_GetError());
        state->renderer = SDL_CreateRenderer(state->window, 
        "vulkan,opengl,PSP,psp,gpu,opengles2,software");
    }

    if(state->renderer == NULL){
        SDL_Log("Couldn't create renderer: %s", SDL_GetError());
        SDL_free(state);
        return SDL_APP_FAILURE;
    }

    // for debugging / optimizing.
    for(int i=0; i < SDL_GetNumRenderDrivers(); i++){
        SDL_Log(SDL_GetRenderDriver(i));
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    SDL_Surface *surface = NULL;
    AppState *state = NULL;

    SDL_SetAppMetadata("SDL3 Sprite Benchmark", "1.0", "se.crimro.sprite-benchmark");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    state = (AppState *)SDL_calloc(1, sizeof(AppState));
    if (!state) {
        SDL_Log("Couldn't allocate app state");
        return SDL_APP_FAILURE;
    }
    state->movement_enabled = MOVEMENT_ENABLED_DEFAULT;

    if(init_window(state)!= SDL_APP_CONTINUE){
        return SDL_APP_FAILURE;
    }

    //SDL_SetRenderLogicalPresentation(state->renderer, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_LOGICAL_PRESENTATION_DISABLED);

    SDL_IOStream *io = SDL_IOFromMem(sprite_png, sprite_png_len);
    if (!io) {
        SDL_Log("Couldn't create IO: %s", SDL_GetError());
        SDL_free(state);
        return SDL_APP_FAILURE;
    }
    surface = SDL_LoadPNG_IO(io, true);
    if (!surface) {
        SDL_Log("Couldn't load png: %s", SDL_GetError());
        SDL_free(state);
        return SDL_APP_FAILURE;
    }

    state->texture_width = surface->w;
    state->texture_height = surface->h;

    state->texture = SDL_CreateTextureFromSurface(state->renderer, surface);
    if (!state->texture) {
        SDL_Log("Couldn't create static texture: %s", SDL_GetError());
        SDL_DestroySurface(surface);
        SDL_free(state);
        return SDL_APP_FAILURE;
    }

    SDL_DestroySurface(surface);

    state->sprites = (Sprite *)SDL_calloc(MAX_SPRITES, sizeof(Sprite));
    if (!state->sprites) {
        SDL_Log("Couldn't allocate sprite array");
        SDL_free(state);
        return SDL_APP_FAILURE;
    }

    srand(2026); // deliberate
    state->num_sprites = MAX_SPRITES;
    state->active_sprites = INITIAL_SPRITES;
    state->dirty_ui = true;
    
    state->ui_texture = SDL_CreateTexture(state->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 230, 30);
    
    for (int i = 0; i < state->num_sprites; i++) {
        init_sprite(&state->sprites[i]);
    }

    state->last_frame_time = SDL_GetTicks();
    state->fps_update_time = state->last_frame_time;
    state->animate_update_time = state->last_frame_time;
    state->rotation_enabled = false;

    *appstate = state;
    SDL_Log("Init complete!");
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    AppState *state = (AppState *)appstate;
    
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    
    if (event->type == SDL_EVENT_GAMEPAD_ADDED) {
        state->gamepad = SDL_OpenGamepad(event->gdevice.which);
        state->gamepad_id = event->gdevice.which;
    }
    
    if (event->type == SDL_EVENT_GAMEPAD_REMOVED && state->gamepad && state->gamepad_id == event->gdevice.which) {
        SDL_CloseGamepad(state->gamepad);
        state->gamepad = NULL;
        state->gamepad_id = 0;
    }
    
    if (event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
        if (event->gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_LEFT) {
            adjust_sprite_count(state, -SPRITE_INCREMENT);
        }
        if (event->gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_RIGHT) {
            adjust_sprite_count(state, SPRITE_INCREMENT);
        }
        if (event->gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_UP) {
            state->movement_enabled = !state->movement_enabled;
        }
        if (event->gbutton.button == SDL_GAMEPAD_BUTTON_EAST){
            return SDL_APP_SUCCESS;
        }
        if (event->gbutton.button == SDL_GAMEPAD_BUTTON_DPAD_DOWN) {
            state->rotation_enabled = !state->rotation_enabled;
        }
    }
    
    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_ESCAPE) {
            return SDL_APP_SUCCESS;
        }
        if (event->key.key == SDLK_RIGHT || event->key.key == SDLK_EQUALS) {
            adjust_sprite_count(state, SPRITE_INCREMENT);
        }
        if (event->key.key == SDLK_LEFT || event->key.key == SDLK_MINUS) {
            adjust_sprite_count(state, -SPRITE_INCREMENT);
        }
        if (event->key.key == SDLK_UP) {
            state->movement_enabled = !state->movement_enabled;
        }
        if (event->key.key == SDLK_DOWN) {
            state->rotation_enabled = !state->rotation_enabled;
        }
    }
    
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState *state = (AppState *)appstate;
    Uint64 now = SDL_GetTicks();
    Uint64 delta = now - state->last_frame_time;
    bool animate = false;
    
    state->last_frame_time = now;
    state->frame_count++;
    
    if (now - state->fps_update_time >= 3000) {
        state->current_fps = state->frame_count / 3;
        state->frame_count = 0;
        state->fps_update_time = now;
        state->dirty_ui = true;
    }

    SDL_SetRenderDrawColor(state->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(state->renderer);

    // do we update sprites this frame?
    if (now - state->animate_update_time >= 15){
        animate = true;
        state->animate_update_time = now;
    }


    for (int i = 0; i < state->active_sprites; i++) {
        Sprite *s = &state->sprites[i];
        if (animate){
            update_sprite_position(s, state->movement_enabled);
            update_sprite_animation(s, 15);
        }
        render_sprite(state, s);
    }

    SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    render_ui(state);

    SDL_RenderPresent(state->renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    AppState *state = (AppState *)appstate;
    if (!state) {
        return;
    }
    
    if (state->sprites) {
        SDL_free(state->sprites);
    }
    if (state->texture) {
        SDL_DestroyTexture(state->texture);
    }
    if (state->ui_texture) {
        SDL_DestroyTexture(state->ui_texture);
    }
    if (state->gamepad) {
        SDL_CloseGamepad(state->gamepad);
    }
    if (state->renderer) {
        SDL_DestroyRenderer(state->renderer);
    }
    if (state->window) {
        SDL_DestroyWindow(state->window);
    }
    SDL_free(state);
}
