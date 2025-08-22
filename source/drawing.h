#ifndef DRAWING_H
#define DRAWING_H

#include <gba.h>

// Drawing mode constants
#define DRAW_CURSOR_MAX_X 30  // 240/8 - 1
#define DRAW_CURSOR_MAX_Y 19  // 160/8 - 1
#define DRAW_CURSOR_BLINK_SPEED 15  // Frames between blink changes

// Key repeat timing constants
#define KEY_REPEAT_DELAY 15     // Initial delay before repeat starts (frames)
#define KEY_REPEAT_RATE 4       // Repeat rate once started (frames between repeats)

// Tile rotation modes
typedef enum {
    TILE_ROT_0,     // Normal (0°)
    TILE_ROT_180    // 180° rotation using hardware flips
} TileRotation;

// Mirror modes for cursor
typedef enum {
    MIRROR_NONE,           // Single cursor only
    MIRROR_HORIZONTAL,     // Cursor mirrored horizontally
    MIRROR_VERTICAL,       // Cursor mirrored vertically
    MIRROR_BOTH            // Cursor mirrored both horizontally and vertically
} MirrorMode;

// Drawing mode state
extern int draw_cursor_x;
extern int draw_cursor_y;
extern int draw_selected_tile;
extern int draw_active_layer;
extern int draw_cursor_blink;
extern int draw_prev_cursor_x;
extern int draw_prev_cursor_y;
extern u16 draw_original_tile;
extern u16 draw_mirror_original_tiles[3]; // Original tiles at mirror positions [horizontal, vertical, diagonal]
extern TileRotation draw_tile_rotation;
extern MirrorMode draw_mirror_mode;
extern int draw_key_repeat_timer;
extern int draw_last_held_keys;
extern bool drawingMode;

// Function declarations
void drawing_init(void);
void drawing_update(void);
void drawing_move_cursor(int dx, int dy);
void drawing_place_tile(void);
void drawing_erase_tile(void);
void drawing_clear_layer(void);
void drawing_next_tile(void);
void drawing_prev_tile(void);
void drawing_next_layer(void);
void drawing_toggle_rotation(void);
void drawing_cycle_mirror_mode(void);
void drawing_handle_continuous_input(int keys_held);
void drawing_render_cursor(void);

// Mirror coordinate calculation helpers
void drawing_get_mirror_positions(int x, int y, int *mirror_x, int *mirror_y, MirrorMode mode);

#endif // DRAWING_H