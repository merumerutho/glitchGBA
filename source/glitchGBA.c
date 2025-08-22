/*
	glitchGBA
	by meru
	
	github: merumerutho
	twitter: merumerutho
	website: meru.cloud
	
	merutochan [[at]] gmail [[dot]] com
	
	credits to: 
	- libgba devs, 
	- coranac for its helpful manual, 
	- infu_av for his precious advices and inspiration, 
	- NO_CARRIER for inpsiration
*/

#include <gba.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#include "graphics.h"
#include "gamelink.h"
#include "display.h"
#include "input.h"
#include "animation.h"
#include "drawing.h"

// ===== GLOBAL STATE VARIABLES =====
// Display and timing control
bool vsync = TRUE;  // Enable/disable vertical sync wait

// Layer movement state
int x_pos[NUM_LAYERS] = {0, 1, 2, 3};    // Current X positions
int y_pos[NUM_LAYERS] = {0, 1, 2, 3};    // Current Y positions
int x_speed[NUM_LAYERS] = {0, 1, 2, 3};  // X movement speeds
int y_speed[NUM_LAYERS] = {0, 1, 2, 3};  // Y movement speeds
int moveRateX[NUM_LAYERS] = {0,0,0,0};   // Platform-adjusted X rates
int moveRateY[NUM_LAYERS] = {0,0,0,0};   // Platform-adjusted Y rates

// Tileset and layer selection
int currentTiles = 0;  // Currently active tileset index
int bglevel = 0;       // Currently selected background layer

// Application modes
typedef enum {
    MODE_NORMAL,    // Standard glitch effects mode
    MODE_ROTATION,  // Rotation/scaling mode
    MODE_DRAWING    // Drawing mode
} AppMode;

AppMode currentMode = MODE_NORMAL;  // Current application mode

// Application state flags
bool paused = FALSE;            // Pause/resume animation
bool paletteChange = FALSE;     // Dynamic palette modification
bool parallax = FALSE;          // Independent layer movement
bool remap = FALSE;             // Dynamic tileset remapping (CPU intensive)
bool bg_remap = FALSE;          // Background layer remapping (CPU intensive)

bool chaos = FALSE;             // Chaos mode - extreme randomization
bool frame_scroll_mode = FALSE; // Automatic tileset cycling

// Legacy mode flags (derived from currentMode)
bool rotationMode = FALSE;      // Enable rotation/scaling mode
bool drawingMode = FALSE;       // Drawing mode toggle

// Frame counter
int frameCounter=0;

// Rotation mode scaling and speed
int scale2 = 1<<8;              // BG2 zoom scale (fixed point)
int scale3 = 1<<8;              // BG3 zoom scale (fixed point)
int BG2_angular_speed = 0;      // BG2 rotation speed
int BG3_angular_speed = 0;      // BG3 rotation speed


// Background affine transformation structures (for rotation/scaling)
BGAffineSource BG_AFF_S2 = {0, 0, 0, 0, 1<<8, 1<<8, 0x0000};  // BG2 source params
BGAffineDest BG_AFF_D2;                                          // BG2 dest params
BGAffineSource BG_AFF_S3 = {0, 0, 0, 0, 1<<8, 1<<8, 0x0000};  // BG3 source params
BGAffineDest BG_AFF_D3;                                          // BG3 dest params

// ===== DRAWING MODE STATE =====
// Drawing cursor position (in tile coordinates)
int draw_cursor_x = 0;          // Cursor X position (0-29 for 240/8)
int draw_cursor_y = 0;          // Cursor Y position (0-19 for 160/8)
int draw_selected_tile = 0;     // Currently selected tile from tileset
int draw_active_layer = 0;      // Currently active drawing layer (0-3)
int draw_cursor_blink = 0;      // Cursor blink animation counter
int draw_prev_cursor_x = -1;    // Previous cursor X position
int draw_prev_cursor_y = -1;    // Previous cursor Y position
u16 draw_original_tile = 0;     // Original tile at cursor position
u16 draw_mirror_original_tiles[3] = {0, 0, 0}; // Original tiles at mirror positions [horizontal, vertical, diagonal]
TileRotation draw_tile_rotation = TILE_ROT_0; // Current tile rotation
MirrorMode draw_mirror_mode = MIRROR_NONE;    // Current mirror mode

// Key repeat timing state
int draw_key_repeat_timer = 0;  // Timer for key repeat functionality
int draw_last_held_keys = 0;    // Previously held keys for repeat detection
	

//

void init();
void cycle_app_mode();
void update_mode_flags();
int main();

// Unified mode switching system
void cycle_app_mode() {
    // Cycle through: Normal -> Rotation -> Drawing -> Normal
    switch (currentMode) {
        case MODE_NORMAL:
            currentMode = MODE_ROTATION;
            break;
        case MODE_ROTATION:
            currentMode = MODE_DRAWING;
            break;
        case MODE_DRAWING:
            currentMode = MODE_NORMAL;
            break;
    }
    
    // Update legacy mode flags and display settings
    update_mode_flags();
}

void update_mode_flags() {
    // Update legacy boolean flags based on current mode
    rotationMode = (currentMode == MODE_ROTATION);
    drawingMode = (currentMode == MODE_DRAWING);
    
    // Update display control register based on mode
    switch (currentMode) {
        case MODE_NORMAL:
            // Standard tiled mode
            REG_DISPCNT = MODE_0 | BG0_ON | BG1_ON | BG2_ON | BG3_ON;
            break;
        case MODE_ROTATION:
            // Rotation/scaling mode - preserve existing tileset mapping
            REG_DISPCNT = MODE_2 | BG0_ON | BG1_ON | BG2_ON | BG3_ON;
            break;
        case MODE_DRAWING:
            // Drawing mode uses standard tiled mode
            REG_DISPCNT = MODE_0 | BG0_ON | BG1_ON | BG2_ON | BG3_ON;
            // Initialize drawing state when entering drawing mode
            if (drawingMode) {
                drawing_init();
            }
            break;
    }
}

//


// Initialization/Reset function
void init() 
{	
	currentTiles = 0;
	dmaCopy(palsPointer[currentTiles], BG_PALETTE, PalsLen[currentTiles]);
	dmaCopy(tilesPointer[currentTiles], &MAP[0][0], TilesLen[currentTiles]);
	
	// Reset map
	init_map();
	bglevel = 0;
	
	for (int i = 0; i < NUM_LAYERS; i++)
	{
		x_speed[i] = 0;
		y_speed[i] = 0;
	}
	
	// eugh brother...
	chaos = FALSE;
	bg_remap = FALSE;
	remap = FALSE;
	parallax = FALSE;
	frame_scroll_mode = FALSE;
	drawingMode = FALSE;
	frameCounter=0;
	paused = FALSE;
	vsync = TRUE;
	scale2 = 1<<8;
	scale3 = 1<<8;
	BG_AFF_S2.sX = BG_AFF_S2.sY = (1<<16)/scale2;
	BG_AFF_S3.sX = BG_AFF_S3.sY = (1<<16)/scale3;
	BG_AFF_S2.theta = BG_AFF_S3.theta = 0;
	BG2_angular_speed = BG3_angular_speed = 0;
	BG_AFF_S2.x=0;
	BG_AFF_S2.y=0;
	BG_AFF_S3.x=0;
	BG_AFF_S3.y=0;
	
	// Reset Game Link Cable state
	gamelink_enabled = TRUE;
	gamelink_cmd_head = 0;
	gamelink_cmd_tail = 0;
	
	return;
}

int main() 
{
	// Input state
	InputState input = {0, 0, FALSE};
	
	// Enable interrupt VBLANK
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	// Initialize Game Link Cable for command reception
	gamelink_init();
	
	// Randomize
	scanKeys();
	srand(keysHeld()+1);
	
	// Initialize display system
	display_init();
	
	// For RotoScale mode as well
	BG_AFF_S2.x = rand() % SCREEN_WIDTH;
	BG_AFF_S2.y = rand() % SCREEN_HEIGHT;
	BG_AFF_S3.x = rand() % SCREEN_WIDTH;
	BG_AFF_S3.y = rand() % SCREEN_HEIGHT;
	
	// Set up display registers
	display_setup_registers();
	
	do {
		// Wait for vsync interrupt (60fps lock)
		if(vsync)
		{
			VBlankIntrWait();
		}
		
		frameCounter++;
		
		/* 
			UPDATE DRAW SECTION
		*/
		if (!drawingMode) {
			// Normal animation updates only when not in drawing mode
			animation_update_screen_movement();
			animation_update_remap();
			animation_update_frame_scroll();
			animation_update_palette();
		} else {
			// Drawing mode updates
			drawing_update();
		}
		
		// Game Link Cable command processing
		gamelink_poll();
		while (gamelink_has_command())
		{
			u8 command = gamelink_get_command();
			gamelink_process_command(command);
		}
		
		// ---
		
		// Handle input
		input_update(&input);
		input_handle_single_keys(&input);
		input_handle_multi_keys(&input);
		
	} while( 1 );
}