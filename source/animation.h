#ifndef ANIMATION_H
#define ANIMATION_H

#include <gba.h>

// Frame timing constants
#define FRAME_COUNT_SCROLLING_MODE 30

// Function declarations
void animation_update_screen_movement(void);
void animation_update_remap(void);
void animation_update_frame_scroll(void);
void animation_update_palette(void);

#endif // ANIMATION_H