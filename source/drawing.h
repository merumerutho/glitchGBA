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

// Cursor position and visual state
typedef struct {
    int x;                 // Current cursor X position (0-30)
    int y;                 // Current cursor Y position (0-19)
    int prev_x;            // Previous cursor X position
    int prev_y;            // Previous cursor Y position
    int blink_counter;     // Blink animation counter
} CursorState;

// Tile and layer selection state
typedef struct {
    int selected_tile;     // Currently selected tile from tileset
    int active_layer;      // Currently active drawing layer (0-3)
    TileRotation rotation; // Current tile rotation mode
    MirrorMode mirror_mode; // Current mirror mode
} TileState;

// Input handling state for continuous input
typedef struct {
    int repeat_timer;      // Timer for key repeat functionality
    int last_held_keys;    // Previously held keys for repeat detection
} DrawingInputState;

// Original tiles backup for cursor rendering
typedef struct {
    u16 primary;           // Original tile at primary cursor position
    u16 mirrors[3];        // Original tiles at mirror positions [horizontal, vertical, diagonal]
} OriginalTiles;

// Main drawing system state
typedef struct {
    CursorState cursor;
    TileState tile;
    DrawingInputState input;
    OriginalTiles backup;
    bool enabled;          // Drawing mode toggle
} DrawingSystem;

// Global drawing system instance
extern DrawingSystem drawing;

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