/* 
 * debug_font.h - Simple embedded bitmap font renderer for SDL2/SDL3
 *                Lowercase and 0-9 text only. Not efficient!
 * 
 *  This file is 99.9% AI Generated. No rights reserved.
 * 
 * Usage:
 *   // AFTER including SDL2 or SDL3 !
 *   #include "debug_font.h"
 *   
 *   // In your render loop:
 *   SDL_Color white = {255, 255, 255, 255};
 *   debug_font_draw_string(renderer, "hello world 123", 10, 10, white);
 *   debug_font_draw_string_scale(renderer, "scaled 2x", 10, 30, 2, white);
 *
 * Implementation:
 *   #define DEBUG_FONT_IMPLEMENTATION
 *   #include "debug_font.h"
 */

#ifndef DEBUG_FONT_H
#define DEBUG_FONT_H

#if SDL_VERSION_ATLEAST(3, 0, 0)
#include <SDL3/SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Draw a single character at (x, y) */
void debug_font_draw_char(SDL_Renderer *renderer, char c, int x, int y, SDL_Color color);

/* Draw a null-terminated string at (x, y). Supports '\n' for newlines. */
void debug_font_draw_string(SDL_Renderer *renderer, const char *str, int x, int y, SDL_Color color);

/* Draw with integer scaling (1 = 8x8 pixels, 2 = 16x16, etc.) */
void debug_font_draw_string_scale(SDL_Renderer *renderer, const char *str, int x, int y, int scale, SDL_Color color);

/* Returns 8 (the native character width/height) */
int debug_font_char_size(void);

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_FONT_H */

/* --------------------------------------------------------------------------- */
/* Implementation                                                              */
/* --------------------------------------------------------------------------- */

#ifdef DEBUG_FONT_IMPLEMENTATION

/* Font data: 8x8 pixels per character, MSB is leftmost pixel.
 * Index 0-9:    '0'-'9'
 * Index 10-35:  'a'-'z'
 * Index 36:     ' ' (space)
 */
static const unsigned char DEBUG_FONT[37][8] = {
    /* 0 */ {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00},
    /* 1 */ {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},
    /* 2 */ {0x3C, 0x66, 0x06, 0x0C, 0x30, 0x60, 0x7E, 0x00},
    /* 3 */ {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00},
    /* 4 */ {0x06, 0x0E, 0x1E, 0x66, 0x7F, 0x06, 0x06, 0x00},
    /* 5 */ {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00},
    /* 6 */ {0x3C, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x3C, 0x00},
    /* 7 */ {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00},
    /* 8 */ {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00},
    /* 9 */ {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x66, 0x3C, 0x00},
    
    /* a */ {0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00},
    /* b */ {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x00},
    /* c */ {0x00, 0x00, 0x3C, 0x60, 0x60, 0x60, 0x3C, 0x00},
    /* d */ {0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x00},
    /* e */ {0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00},
    /* f */ {0x0C, 0x18, 0x18, 0x7C, 0x18, 0x18, 0x18, 0x00},
    /* g */ {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x3C},
    /* h */ {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00},
    /* i */ {0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00},
    /* j */ {0x0C, 0x00, 0x0C, 0x0C, 0x0C, 0x6C, 0x38, 0x00},
    /* k */ {0x60, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0x00},
/* l */ {0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00},
/* m */ {0x00, 0x00, 0xEC, 0x76, 0x66, 0x66, 0x66, 0x00},
/* n */ {0x00, 0x00, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00},
/* o */ {0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00},
/* p */ {0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60},
/* q */ {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x06},
/* r */ {0x00, 0x00, 0x6C, 0x38, 0x30, 0x30, 0x30, 0x00},
/* s */ {0x00, 0x00, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x00},
/* t */ {0x00, 0x18, 0x7E, 0x18, 0x18, 0x18, 0x0E, 0x00},
/* u */ {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00},
/* v */ {0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00},
/* w */ {0x00, 0x00, 0x66, 0x6E, 0x7E, 0x76, 0x62, 0x00},
/* x */ {0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x00},
/* y */ {0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x3C},
/* z */ {0x00, 0x00, 0x7E, 0x0C, 0x18, 0x30, 0x7E, 0x00},
/* space */ {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

static inline int debug_font_get_index(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'z') return 10 + (c - 'a');
    if (c == ' ') return 36;
    return -1;
}

int debug_font_char_size(void) {
    return 8;
}

void debug_font_draw_char(SDL_Renderer *renderer, char c, int x, int y, SDL_Color color) {
    int idx = debug_font_get_index(c);
    if (idx < 0) return;
    
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    const unsigned char *data = DEBUG_FONT[idx];
    
#if SDL_VERSION_ATLEAST(3, 0, 0)
    for (int row = 0; row < 8; row++) {
        unsigned char row_data = data[row];
        for (int col = 0; col < 8; col++) {
            if (row_data & (0x80 >> col)) {
                SDL_RenderPoint(renderer, (float)(x + col), (float)(y + row));
            }
        }
    }
#else
    for (int row = 0; row < 8; row++) {
        unsigned char row_data = data[row];
        for (int col = 0; col < 8; col++) {
            if (row_data & (0x80 >> col)) {
                SDL_RenderDrawPoint(renderer, x + col, y + row);
            }
        }
    }
#endif
}

static void debug_font_draw_char_scale(SDL_Renderer *renderer, char c, int x, int y, int scale, SDL_Color color) {
    int idx = debug_font_get_index(c);
    if (idx < 0) return;
    
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    const unsigned char *data = DEBUG_FONT[idx];
    
#if SDL_VERSION_ATLEAST(3, 0, 0)
    SDL_FRect rect = {0, 0, 0, 0};
    for (int row = 0; row < 8; row++) {
        unsigned char row_data = data[row];
        for (int col = 0; col < 8; col++) {
            if (row_data & (0x80 >> col)) {
                rect.x = (float)(x + col * scale);
                rect.y = (float)(y + row * scale);
                rect.w = (float)scale;
                rect.h = (float)scale;
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
#else
    SDL_Rect rect = {0, 0, 0, 0};
    for (int row = 0; row < 8; row++) {
        unsigned char row_data = data[row];
        for (int col = 0; col < 8; col++) {
            if (row_data & (0x80 >> col)) {
                rect.x = x + col * scale;
                rect.y = y + row * scale;
                rect.w = scale;
                rect.h = scale;
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
#endif
}

void debug_font_draw_string(SDL_Renderer *renderer, const char *str, int x, int y, SDL_Color color) {
    if (!str) return;
    int start_x = x;
    const char *p = str;
    
    while (*p) {
        if (*p == '\n') {
            y += 8;
            x = start_x;
        } else {
            debug_font_draw_char(renderer, *p, x, y, color);
            x += 8;
        }
        p++;
    }
}

void debug_font_draw_string_scale(SDL_Renderer *renderer, const char *str, int x, int y, int scale, SDL_Color color) {
    if (!str || scale <= 0) return;
    if (scale == 1) {
        debug_font_draw_string(renderer, str, x, y, color);
        return;
    }
    
    int start_x = x;
    int advance = 8 * scale;
    const char *p = str;
    
    while (*p) {
        if (*p == '\n') {
            y += advance;
            x = start_x;
        } else {
            debug_font_draw_char_scale(renderer, *p, x, y, scale, color);
            x += advance;
        }
        p++;
    }
}

#endif /* DEBUG_FONT_IMPLEMENTATION */
