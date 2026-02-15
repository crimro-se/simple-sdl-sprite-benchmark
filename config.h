#ifndef CONFIG_H
#define CONFIG_H

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

#define SPRITE_WIDTH 48
#define SPRITE_HEIGHT 48

// note: 24.8 fixed point
#define SPRITE_MAX_SPEED (3 << 8)

#define NUM_FRAMES 2
#define FRAME_DURATION_MS 70

#define MAX_SPRITES 10000
#define INITIAL_SPRITES 100
#define SPRITE_INCREMENT 100

#define MOVEMENT_ENABLED_DEFAULT true

// minimum interval for updating sprite positions and frames. time deltas scale on this, too.
#define UPDATE_INTERVAL_MS 10

#endif
